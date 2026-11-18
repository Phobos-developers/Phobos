#include "Body.h"

#include <Ext/Rules/Body.h>

#include <algorithm>

namespace TerrainTypeTemp
{
	TerrainTypeClass* pCurrentType = nullptr;
	TerrainTypeExt* pCurrentExt = nullptr;
	double PriorHealthRatio = 0.0;
}

DEFINE_HOOK(0x71C84D, TerrainClass_AI_Animated, 0x6)
{
	enum { SkipGameCode = 0x71C8D5 };

	GET(TerrainClass*, pThis, ESI);

	auto const pType = pThis->Type;

	if (pType->IsAnimated)
	{
		auto const pTypeExt = TerrainTypeExt::Fetch(pType);

		if (pThis->Animation.Value == (pTypeExt->AnimationLength.isset() ? pTypeExt->AnimationLength.Get() : (pType->GetImage()->Frames / (2 * (pTypeExt->HasDamagedFrames + 1)))))
		{
			pThis->Animation.Value = 0;
			pThis->Animation.Start(0);

			// Spawn tiberium if enabled.
			if (pType->SpawnsTiberium)
			{
				const auto pCell = pThis->GetCell();
				const int cellCount = pTypeExt->GetCellsPerAnim();

				// Set context for CellClass hooks.
				TerrainTypeTemp::pCurrentType = pType;
				TerrainTypeTemp::pCurrentExt = pTypeExt;

				for (int i = 0; i < cellCount; i++)
				{
					if (!pCell->SpreadTiberium(true))
						break;
				}

				const int particleIdx = pTypeExt->SpawnsTiberium_Particle;

				if (particleIdx >= 0)
				{
					const auto particleSys = Make_Global<ParticleSystemClass*>(0xA8ED78);
					particleSys->SpawnParticle(ParticleTypeClass::Array[particleIdx], pThis->Location);
				}

				// Unset context for CellClass hooks.
				TerrainTypeTemp::pCurrentType = nullptr;
				TerrainTypeTemp::pCurrentExt = nullptr;
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x71C812, TerrainClass_AI_Crumbling, 0x6)
{
	enum { ReturnFromFunction = 0x71C839, SkipCheck = 0x71C7C2 };

	GET(TerrainClass*, pThis, ESI);

	auto const pType = pThis->Type;
	auto const pTypeExt = TerrainTypeExt::Fetch(pType);

	if (pTypeExt->HasDamagedFrames && pThis->Health > 0)
	{
		if (!pType->IsAnimated && !pType->IsFlammable)
			LogicClass::Instance.Remove(pThis);

		pThis->IsCrumbling = false;

		return SkipCheck;
	}

	const int animationLength = pTypeExt->AnimationLength.isset() ? pTypeExt->AnimationLength.Get() : pType->GetImage()->Frames / (2 * (pTypeExt->HasDamagedFrames + 1));
	const int currentStage = pThis->Animation.Value + (pType->IsAnimated ? animationLength * (pTypeExt->HasDamagedFrames + 1) : 0 + pTypeExt->HasDamagedFrames);

	if (currentStage + 1 == pType->GetImage()->Frames / 2)
	{
		pTypeExt->PlayDestroyEffects(pThis->GetCoords());
		TerrainTypeExt::Remove(pThis);
	}

	return ReturnFromFunction;
}

DEFINE_HOOK(0x71C1FE, TerrainClass_Draw_PickFrame, 0x6)
{
	enum { SkipGameCode = 0x71C234 };

	GET(int, frame, EBX);

	GET(TerrainClass*, pThis, ESI);

	auto const pType = pThis->Type;
	auto const pTypeExt = TerrainTypeExt::Fetch(pType);
	const bool isDamaged = pTypeExt->HasDamagedFrames && pThis->GetHealthPercentage() <= RulesExt::Global()->ConditionYellow_Terrain.Get(RulesClass::Instance->ConditionYellow);

	if (pType->IsAnimated)
	{
		const int animLength = pTypeExt->AnimationLength.isset() ? pTypeExt->AnimationLength.Get() : ((pType->GetImage()->Frames / (2 * (pTypeExt->HasDamagedFrames + 1))));

		if (pTypeExt->HasCrumblingFrames && pThis->IsCrumbling)
			frame = (animLength * (pTypeExt->HasDamagedFrames + 1)) + 1 + pThis->Animation.Value;
		else
			frame = pThis->Animation.Value + (isDamaged * animLength);
	}
	else
	{
		if (pTypeExt->HasCrumblingFrames && pThis->IsCrumbling)
			frame = 1 + pThis->Animation.Value;
		else if (isDamaged)
			frame = 1;
	}

	R->EBX(frame);
	return SkipGameCode;
}

DEFINE_HOOK(0x71C2BC, TerrainClass_Draw_Palette, 0x6)
{
	GET(TerrainClass*, pThis, ESI);

	auto const pCell = pThis->GetCell();
	const int wallOwnerIndex = pCell->WallOwnerIndex;
	int colorSchemeIndex = HouseClass::CurrentPlayer->ColorSchemeIndex;

	if (wallOwnerIndex >= 0)
		colorSchemeIndex = HouseClass::Array[wallOwnerIndex]->ColorSchemeIndex;

	auto const pTypeExt = TerrainTypeExt::Fetch(pThis->Type);

	if (pTypeExt->Palette)
	{
		R->EDX(pTypeExt->Palette->Items[colorSchemeIndex]->LightConvert);
		R->EBP(pCell->Intensity_Normal);
	}

	return 0;
}

// Overrides Ares hook at 0x5F4FF9, required for animated terrain cause game & Ares check SpawnsTiberium instead of IsAnimated
DEFINE_HOOK(0x5F4FEF, ObjectClass_Unlimbo_UpdateTerrain, 0x6)
{
	enum { SkipUpdate = 0x5F5045, ContinueChecks = 0x5F501B };

	GET(ObjectTypeClass*, pType, EBX);

	if (!pType->IsLogic)
		return SkipUpdate;

	if (pType->WhatAmI() != AbstractType::TerrainType)
		return ContinueChecks;

	auto const pTerrainType = static_cast<TerrainTypeClass*>(pType);

	if (pTerrainType->IsFlammable || pTerrainType->IsAnimated)
		return ContinueChecks;

	return SkipUpdate;
}

DEFINE_HOOK(0x483811, CellClass_SpreadTiberium_TiberiumType, 0x8)
{
	if (TerrainTypeTemp::pCurrentExt)
	{
		LEA_STACK(int*, pTibType, STACK_OFFSET(0x1C, 0x4));

		*pTibType = TerrainTypeTemp::pCurrentExt->SpawnsTiberium_Type;

		return 0x483819;
	}

	return 0;
}

DEFINE_HOOK(0x48381D, CellClass_SpreadTiberium_CellSpread, 0x6)
{
	enum { SpreadReturn = 0x4838CA, NoSpreadReturn = 0x4838B0 };

	if (TerrainTypeTemp::pCurrentExt)
	{
		GET(CellClass*, pThis, EDI);
		GET(const int, tibIndex, EAX);

		TiberiumClass* pTib = TiberiumClass::Array.GetItem(tibIndex);

		std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(TerrainTypeTemp::pCurrentExt->SpawnsTiberium_Range);
		const size_t size = adjacentCells.size();
		const int rand = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);

		for (unsigned int i = 0; i < size; i++)
		{
			const unsigned int cellIndex = (i + rand) % size;
			CellStruct tgtPos = pThis->MapCoords + adjacentCells[cellIndex];
			CellClass* tgtCell = MapClass::Instance.TryGetCellAt(tgtPos);

			if (tgtCell && tgtCell->CanTiberiumGerminate(pTib))
			{
				const int maxStage = pTib->NumFrames - 1;
				const int rawStage = TerrainTypeTemp::pCurrentExt->GetTiberiumGrowthStage();
				const int growthStage = std::clamp(rawStage, 0, maxStage);

				const bool res = tgtCell->IncreaseTiberium(tibIndex, growthStage);

				if (res && growthStage >= maxStage)
				{
					const int surfaceIdx = PriorityQueueClassNode::ToSurfaceIndex(tgtPos);
					const int maxCount = PriorityQueueClassNode::SurfaceDataCount();
					const auto& logic = pTib->SpreadLogic;

					if (surfaceIdx >= 0 && surfaceIdx < maxCount)
					{
						if (!logic.CellIndexesWithTiberium || !logic.CellIndexesWithTiberium[surfaceIdx])
							pTib->RegisterForSpread(&tgtPos);
					}
				}

				R->EAX<bool>(res);
				return SpreadReturn;
			}
		}

		return NoSpreadReturn;
	}

	return 0;
}

DEFINE_HOOK(0x71C6EE, TerrainClass_FireOut_Crumbling, 0x6)
{
	enum { StartCrumbling = 0x71C6F8, Skip = 0x71C72B };

	GET(TerrainClass*, pThis, ESI);

	auto const pTypeExt = TerrainTypeExt::Fetch(pThis->Type);

	if (!pThis->IsCrumbling && pTypeExt->HasCrumblingFrames)
	{
		// Needs to be added to the logic layer for the anim to work.
		LogicClass::Instance.AddObject(pThis, false);
		VocClass::PlayIndexAtPos(pTypeExt->CrumblingSound, pThis->GetCoords());

		return StartCrumbling;
	}

	return Skip;
}

DEFINE_HOOK(0x71B965, TerrainClass_TakeDamage_SetContext, 0x8)
{
	GET(TerrainClass*, pThis, ESI);

	TerrainTypeTemp::PriorHealthRatio = pThis->GetHealthPercentage();

	return 0;
}

DEFINE_HOOK(0x71B98B, TerrainClass_TakeDamage_RefreshDamageFrame, 0x7)
{
	GET(TerrainClass*, pThis, ESI);

	auto const pType = pThis->Type;
	auto const pTypeExt = TerrainTypeExt::Fetch(pType);
	const double condYellow = RulesExt::Global()->ConditionYellow_Terrain.Get(RulesClass::Instance->ConditionYellow);

	if (!pType->IsAnimated && pTypeExt->HasDamagedFrames && TerrainTypeTemp::PriorHealthRatio > condYellow && pThis->GetHealthPercentage() <= condYellow)
	{
		pThis->IsCrumbling = true; // Dirty hack to get game to redraw the art reliably.
		LogicClass::Instance.AddObject(pThis, false);
	}

	return 0;
}

//This one on Very end of it , let everything play first
DEFINE_HOOK(0x71BB2C, TerrainClass_TakeDamage_NowDead_Add, 0x6)
{
	GET(TerrainClass*, pThis, ESI);
	//saved for later usage !
	//REF_STACK(args_ReceiveDamage const, ReceiveDamageArgs, STACK_OFFSET(0x3C, 0x4));

	auto const pTypeExt = TerrainTypeExt::Fetch(pThis->Type);

	// Skip over the removal of the tree as well as destroy sound/anim (for now) if the tree has crumble animation.
	if (pThis->IsCrumbling && pTypeExt->HasCrumblingFrames)
	{
		// Needs to be added to the logic layer for the anim to work.
		LogicClass::Instance.AddObject(pThis, false);
		VocClass::PlayIndexAtPos(pTypeExt->CrumblingSound, pThis->GetCoords());
		pThis->Mark(MarkType::Change);
		pThis->Disappear(true);

		return 0x71BB79;
	}

	pTypeExt->PlayDestroyEffects(pThis->GetCoords());

	return 0;
}

DEFINE_HOOK(0x47C065, CellClass_CellColor_TerrainRadarColor, 0x6)
{
	enum { SkipTerrainColor = 0x47C0AE, ReturnFromFunction = 0x47C0A3 };

	GET(CellClass*, pThis, ECX);
	GET_STACK(ColorStruct*, arg0, STACK_OFFSET(0x14, 0x4));
	GET_STACK(ColorStruct*, arg4, STACK_OFFSET(0x14, 0x8));

	auto const pTerrain = pThis->GetTerrain(false);

	if (pTerrain)
	{
		auto const pType = pTerrain->Type;

		if (pType->RadarInvisible)
		{
			R->ESI(pThis);
			return SkipTerrainColor;
		}
		else
		{
			auto const pTerrainExt = TerrainTypeExt::Fetch(pType);

			if (pTerrainExt->MinimapColor.isset())
			{
				auto& color = pTerrainExt->MinimapColor.Get();

				arg0->R = color.R;
				arg0->G = color.G;
				arg0->B = color.B;

				arg4->R = color.R;
				arg4->G = color.G;
				arg4->B = color.B;

				R->ECX(arg4);
				R->AL(color.B);

				return ReturnFromFunction;
			}
		}
	}

	return 0;
}

namespace
{
	const TerrainTypeExt* GetCustomFoundation(const TerrainClass* pTerrain)
	{
		auto const pExt = TerrainTypeExt::TryFetch(pTerrain->Type);
		return pExt && pExt->HasCustomFoundation() ? pExt : nullptr;
	}

	CellClass* GetFoundationCell(MapClass* pMap, const CellStruct& anchor, const CellStruct& offset)
	{
		const int x = anchor.X + offset.X;
		const int y = anchor.Y + offset.Y;
		if (x < 0 || x >= TerrainTypeExt::MapSide || y < 0 || y >= TerrainTypeExt::MapSide)
			return nullptr;

		return pMap->TryGetCellAt(CellStruct { static_cast<short>(x), static_cast<short>(y) });
	}

	bool HasTerrainOrBuilding(const ObjectClass* pFirst)
	{
		for (auto pObject = pFirst; pObject; pObject = pObject->NextObject)
		{
			const auto type = pObject->WhatAmI();
			if (type == AbstractType::Terrain || type == AbstractType::Building)
				return true;
		}

		return false;
	}

	void UpdateTerrainNeighbours(MapClass* pMap, const CellStruct& coords, int delta)
	{
		for (auto const& direction : Unsorted::AdjacentCell)
		{
			if (auto const pNeighbour = GetFoundationCell(pMap, coords, direction))
				pNeighbour->BlockedNeighbours += delta;
		}
	}
}

// The base Unlimbo skips its placement test during ScenarioInit. Check the chain
// invariant even then: a custom terrain cannot share a cell with another terrain
// or building, since one NextObject link is shared by all its foundation cells.
DEFINE_HOOK(0x71D00C, TerrainClass_Unlimbo_CustomFoundation, 0x6)
{
	enum { Fail = 0x71D13E };

	GET(TerrainClass*, pThis, ECX);
	GET(const CoordStruct*, pLocation, EBP);

	const CellStruct anchor = CellClass::Coord2Cell(*pLocation);
	const bool custom = GetCustomFoundation(pThis) != nullptr;
	for (auto pOffset = pThis->GetFoundationData(false); *pOffset != TerrainTypeExt::FoundationEnd; ++pOffset)
	{
		auto const pCell = GetFoundationCell(&MapClass::Instance, anchor, *pOffset);
		if (custom)
		{
			if (!pCell || HasTerrainOrBuilding(pCell->FirstObject) || HasTerrainOrBuilding(pCell->AltObject))
				return Fail;
		}
		else if (pCell && (TerrainTypeExt::FindCustomFoundationTerrain(pCell, false)
			|| TerrainTypeExt::FindCustomFoundationTerrain(pCell, true)))
		{
			return Fail;
		}
	}

	return 0;
}

// Walk the full footprint directly so custom foundations are not limited
// by the engine's fixed-size temporary buffer.
DEFINE_HOOK(0x568432, MapClass_PlaceDown_TerrainFoundation, 0x8)
{
	enum { Done = 0x5687DF };

	GET(ObjectClass*, pObject, EDI);
	if (auto const pTerrain = abstract_cast<TerrainClass*>(pObject))
	{
		if (auto const pExt = GetCustomFoundation(pTerrain))
		{
			GET(MapClass*, pMap, EBX);
			GET_STACK(const CellStruct*, pAnchor, 0x8B4);
			for (auto const& offset : pExt->CustomFoundation)
			{
				if (offset == TerrainTypeExt::FoundationEnd)
					break;

				if (auto const pCell = GetFoundationCell(pMap, *pAnchor, offset))
				{
					pCell->AddContent(pTerrain, pTerrain->OnBridge);
					pCell->RecalcAttributes(-1);
					// TerrainClass::Unlimbo counts neighbours of the anchor itself.
					if (offset != CellStruct::Empty)
						UpdateTerrainNeighbours(pMap, pCell->MapCoords, 1);
				}
			}

			return Done;
		}

		if (pTerrain->Type->Foundation == static_cast<int>(Foundation::_0x0))
			return Done;
	}

	return 0;
}

DEFINE_HOOK(0x568862, MapClass_RemoveContentAt_TerrainFoundation, 0x8)
{
	enum { ContinueAfterCells = 0x568940 };

	GET(ObjectClass*, pObject, EDI);
	if (auto const pTerrain = abstract_cast<TerrainClass*>(pObject))
	{
		if (auto const pExt = GetCustomFoundation(pTerrain))
		{
			GET(MapClass*, pMap, EBX);
			GET_STACK(const CellStruct*, pAnchor, 0x8B4);
			for (auto const& offset : pExt->CustomFoundation)
			{
				if (offset == TerrainTypeExt::FoundationEnd)
					break;

				if (auto const pCell = GetFoundationCell(pMap, *pAnchor, offset))
				{
					if (TerrainTypeExt::FindCustomFoundationTerrain(pCell, pTerrain->OnBridge) != pTerrain)
						continue;

					pCell->RemoveContent(pTerrain, pTerrain->OnBridge);
					pCell->RecalcAttributes(-1);
					if (offset != CellStruct::Empty)
						UpdateTerrainNeighbours(pMap, pCell->MapCoords, -1);
				}
			}

			if (!Unsorted::ScenarioInit)
			{
				// TerrainClass::Limbo resets the zones of the anchor. Reset the
				// other cells after the entire footprint has been cleared.
				for (auto const& offset : pExt->CustomFoundation)
				{
					if (offset == TerrainTypeExt::FoundationEnd)
						break;

					if (offset == CellStruct::Empty)
						continue;

					if (auto const pCell = GetFoundationCell(pMap, *pAnchor, offset))
					{
						pMap->ResetZones(pCell->MapCoords);
						pMap->RecalculateSubZones(pCell->MapCoords);
					}
				}
			}

			return ContinueAfterCells;
		}
	}

	return 0;
}

DEFINE_HOOK(0x47E8CA, CellClass_AddContent_CustomTerrainTail, 0x5)
{
	enum { Append = 0x47E8D7, AlreadyLinked = 0x47EA80 };

	GET(ObjectClass*, pObject, EBP);
	if (auto const pTerrain = abstract_cast<TerrainClass*>(pObject); pTerrain && GetCustomFoundation(pTerrain))
	{
		GET(ObjectClass*, pFirst, ESI);
		for (auto pItem = pFirst; pItem; pItem = pItem->NextObject)
		{
			if (pItem == pTerrain)
				return AlreadyLinked;
		}

		// Buildings already use this tail insertion path. Keeping NextObject null
		// lets this terrain belong to multiple cell chains without joining them.
		return Append;
	}

	return 0;
}

DEFINE_HOOK(0x47EA52, CellClass_AddContent_CustomTerrainOccupation, 0x6)
{
	GET(ObjectClass*, pObject, EBP);
	if (auto const pTerrain = abstract_cast<TerrainClass*>(pObject); pTerrain && GetCustomFoundation(pTerrain))
		return 0x47EA10; // Occupation belongs to this footprint cell rather than the terrain anchor.

	return 0;
}

DEFINE_HOOK(0x47EB62, CellClass_RemoveContent_CustomTerrainOccupation, 0x6)
{
	GET(ObjectClass*, pObject, ESI);
	if (auto const pTerrain = abstract_cast<TerrainClass*>(pObject))
	{
		if (auto const pExt = GetCustomFoundation(pTerrain))
		{
			const auto pRules = RulesExt::Global();
			const bool passable = pExt->IsPassable.Get(pTerrain->Type->SpawnsTiberium ? pRules->Tibtree_IsPassable : pRules->Terrain_IsPassable);
			// Passable terrain never set occupation flags, so there are none to clear.
			return passable ? 0x47EB8F : 0x47EB21;
		}
	}

	return 0;
}
