#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Ext/Cell/Body.h>

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
					pCell->SpreadTiberium(true);

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
			const CellStruct tgtPos = pThis->MapCoords + adjacentCells[cellIndex];
			CellClass* tgtCell = MapClass::Instance.TryGetCellAt(tgtPos);

			if (tgtCell && tgtCell->CanTiberiumGerminate(pTib))
			{
				R->EAX<bool>(tgtCell->IncreaseTiberium(tibIndex,
					TerrainTypeTemp::pCurrentExt->GetTiberiumGrowthStage()));

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

DEFINE_HOOK(0x568432, MapClass_PlaceDown_0x0TerrainTypes, 0x8)
{
	GET(ObjectClass*, pObject, EDI);

	if (auto const pTerrain = abstract_cast<TerrainClass*>(pObject))
	{
		if (pTerrain->Type->Foundation == 21)
			return 0x5687DF;
	}

	return 0;
}

#pragma region LightSource Dirty

static bool RectangleIntersectsDiamond(const RectangleStruct& rect, int cx, int cy, int rx, int ry)
{
	const int rl = rect.X;
	const int rt = rect.Y;
	const int rr = rect.X + rect.Width;
	const int rb = rect.Y + rect.Height;

	if (rr < cx - rx || rl > cx + rx || rb < cy - ry || rt > cy + ry)
		return false;

	auto InDiamond = [=](int x, int y)
		{
			return std::abs(static_cast<double>(x - cx) / rx) + std::abs(static_cast<double>(y - cy) / ry) <= 1.0;
		};

	auto InRect = [=](int x, int y)
		{
			return x >= rl && x <= rr && y >= rt && y <= rb;
		};

	if (InRect(cx, cy))
		return true;

	if (InDiamond(rl, rt)
		|| InDiamond(rr, rt)
		|| InDiamond(rl, rb)
		|| InDiamond(rr, rb))
		return true;

	if (InRect(cx - rx, cy)
		|| InRect(cx + rx, cy)
		|| InRect(cx, cy - ry)
		|| InRect(cx, cy + ry))
		return true;

	return false;
}

static std::vector<CellClass*> GetTerrainCoveredCells(TerrainClass* pThis)
{
	const auto baseCell = pThis->GetMapCoords();
	RectangleStruct rect;
	pThis->GetRenderDimensions(&rect);

	const auto tacticalPos = TacticalClass::Instance->TacticalPos;
	const auto anchor = TacticalClass::CoordsToScreen(pThis->GetCoords()) - tacticalPos;

	const int leftW = anchor.X - rect.X;
	const int rightW = rect.X + rect.Width - anchor.X;

	if (rightW > leftW)
		rect.Width = 2 * leftW;

	const int range = std::max(4, (rect.Height + 60) / 30);
	const int rx = Unsorted::CellWidthInPixels / 2;
	const int ry = Unsorted::CellHeightInPixels / 2;
	std::vector<CellClass*> result;

	for (int cy = baseCell.Y - range; cy <= baseCell.Y + range; ++cy)
	{
		for (int cx = baseCell.X - range; cx <= baseCell.X + range; ++cx)
		{
			if (cx + cy > baseCell.X + baseCell.Y)
				continue;

			const CellStruct cell { static_cast<short>(cx), static_cast<short>(cy) };
			const auto pCell = MapClass::Instance.TryGetCellAt(cell);

			if (!pCell)
				continue;

			const auto cellClient = TacticalClass::CoordsToScreen(pCell->GetCoords()) - tacticalPos;

			if (!RectangleIntersectsDiamond(rect, cellClient.X, cellClient.Y, rx, ry))
				continue;

			result.emplace_back(pCell);
		}
	}

	return result;
}

DEFINE_HOOK(0x71D0E7, TerrainClass_Unlimbo_Covering, 0x6)
{
	enum { ReturnTrue = 0x71D132 };

	GET_STACK(TerrainClass*, pThis, STACK_OFFSET(0x20, -0x10));
	GET(CellClass*, pCell, EAX);
	const auto covering = GetTerrainCoveredCells(pThis);

	for (const auto pCovering : covering)
		CellExt::Fetch(pCovering)->CoveringTerrains.emplace_back(pThis);

	const int overlayIdx = pCell->OverlayTypeIndex;

	if (overlayIdx != -1 && OverlayTypeClass::Array[overlayIdx]->Tiberium)
	{
		pCell->OverlayTypeIndex = -1;
		pCell->OverlayData = 0;
	}

	return ReturnTrue;
}

DEFINE_HOOK(0x71CA1C, TerrainClass_Limbo_ResetCovering, 0x5)
{
	GET(TerrainClass*, pThis, EDI);
	const auto covering = GetTerrainCoveredCells(pThis);

	for (const auto pCovering : covering)
	{
		auto& coveringTerrains = CellExt::Fetch(pCovering)->CoveringTerrains;
		const auto it = std::ranges::find(coveringTerrains, pThis);

		if (it != coveringTerrains.cend())
			coveringTerrains.erase(it);
	}

	return 0;
}


#pragma endregion
