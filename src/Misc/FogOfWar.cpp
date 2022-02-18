#include <Phobos.h>

#include <Helpers/Macro.h>

#include <Ext/Cell/Body.h>
#include <New/Entity/FoggedObject.h>

#include <Misc/MapRevealer.h>

#include <AnimClass.h>
#include <TerrainClass.h>
#include <ScenarioClass.h>
#include <Drawing.h>
#include <OverlayTypeClass.h>
#include <InfantryClass.h>
#include <InfantryTypeClass.h>

DEFINE_HOOK(0x6B8E7A, ScenarioClass_LoadSpecialFlags, 0x5)
{
	ScenarioClass::Instance->SpecialFlags.FogOfWar =
		RulesClass::Instance->FogOfWar || R->EAX() || GameModeOptionsClass::Instance->FogOfWar;

	R->ECX(R->EDI());
	return 0x6B8E8B;
}

DEFINE_HOOK(0x686C03, SetScenarioFlags_FogOfWar, 0x5)
{
	GET(ScenarioFlags, SFlags, EAX);

	SFlags.FogOfWar = RulesClass::Instance->FogOfWar || GameModeOptionsClass::Instance->FogOfWar;

	R->EDX<int>(*reinterpret_cast<int*>(&SFlags));
	return 0x686C0E;
}

DEFINE_HOOK(0x5F4B3E, ObjectClass_DrawIfVisible, 0x6)
{
	GET(ObjectClass*, pThis, ESI);

	if (pThis->InLimbo)
		return 0x5F4B7F;

	if (!ScenarioClass::Instance->SpecialFlags.FogOfWar)
		return 0x5F4B48;

	switch (pThis->WhatAmI())
	{
	case AbstractType::Anim:
		if (!static_cast<AnimClass*>(pThis)->Type->ShouldFogRemove)
			return 0x5F4B48;
		break;

	case AbstractType::Unit:
	case AbstractType::Cell:
		break;

	default:
		return 0x5F4B48;
	}

	if (!MapClass::Instance->IsLocationFogged(pThis->GetCoords()))
		return 0x5F4B48;

	pThis->NeedsRedraw = false;
	return 0x5F4D06;
}

DEFINE_HOOK(0x6F5190, TechnoClass_DrawExtras_CheckFog, 0x6)
{
	GET(TechnoClass*, pThis, ECX);

	return MapClass::Instance->IsLocationFogged(pThis->GetCoords()) ? 0x6F5EEC : 0;
}

DEFINE_HOOK(0x6D6EDA, TacticalClass_Overlay_CheckFog1, 0xA)
{
	GET(CellClass*, pCell, EAX);

	return pCell->OverlayTypeIndex == -1 || pCell->IsFogged() ? 0x6D7006 : 0x6D6EE4;
}

DEFINE_HOOK(0x6D70BC, TacticalClass_Overlay_CheckFog2, 0xA)
{
	GET(CellClass*, pCell, EAX);

	return pCell->OverlayTypeIndex == -1 || pCell->IsFogged() ? 0x6D71A4 : 0x6D70C6;
}

DEFINE_HOOK(0x71CC8C, TerrainClass_DrawIfVisible, 0x6)
{
	GET(TerrainClass*, pThis, EDI);
	pThis->NeedsRedraw = false;

	return pThis->InLimbo || MapClass::Instance->IsLocationFogged(pThis->GetCoords()) ? 0x71CD8D : 0x71CC9A;
}

DEFINE_HOOK(0x4804A4, CellClass_DrawTile_DrawSmudgeIfVisible, 0x6)
{
	GET(CellClass*, pThis, ESI);

	return pThis->IsFogged() ? 0x4804FB : 0;
}

DEFINE_HOOK(0x5865E2, MapClass_IsLocationFogged, 0x3)
{
	GET_STACK(CoordStruct*, pCoord, 0x4);

	MapRevealer revealer(*pCoord);
	auto pCell = MapClass::Instance->GetCellAt(revealer.Base());

	R->EAX(pCell->Flags & CellFlags::EdgeRevealed ?
		false :
		!(pCell->GetNeighbourCell(FACING_SE)->Flags & CellFlags::EdgeRevealed));

	return 0;
}

DEFINE_HOOK(0x6D7A4F, TacticalClass_DrawPixelEffects_FullFogged, 0x6)
{
	GET(CellClass*, pCell, ESI);

	return pCell->IsFogged() ? 0x6D7BB8 : 0;
}

DEFINE_HOOK(0x4ACE3C, MapClass_TryReshroudCell_SetCopyFlag, 0x6)
{
	GET(CellClass*, pCell, EAX);

	bool bNoFog = static_cast<bool>(pCell->AltFlags & AltCellFlags::NoFog);
	pCell->AltFlags &= ~AltCellFlags::NoFog;
	auto Index = TacticalClass::Instance->GetOcclusion(pCell->MapCoords, false);

	if ((bNoFog || pCell->Visibility != Index) && Index >= 0 && pCell->Visibility >= -1)
	{
		pCell->AltFlags |= AltCellFlags::Mapped;
		pCell->Visibility = Index;
	}

	TacticalClass::Instance->RegisterCellAsVisible(pCell);

	return 0x4ACE57;
}

DEFINE_HOOK(0x4A9CA0, MapClass_RevealFogShroud, 0x8)
{
	GET(MapClass*, pThis, ECX);
	GET_STACK(CellStruct*, pMapCoords, 0x4);
	GET_STACK(HouseClass*, pHouse, 0x8);
	GET_STACK(bool, bIncreaseShroudCounter, 0xC);

	auto const pCell = pThis->GetCellAt(*pMapCoords);
	bool bRevealed = false;
	bool const bShouldCleanFog = !(pCell->Flags & CellFlags::EdgeRevealed);

	if (bShouldCleanFog || !(pCell->AltFlags & AltCellFlags::Mapped))
		bRevealed = true;

	bool const bWasRevealed = bRevealed;

	pCell->Flags = pCell->Flags & ~CellFlags::IsPlot | CellFlags::EdgeRevealed;
	pCell->AltFlags = pCell->AltFlags & ~AltCellFlags::NoFog | AltCellFlags::Mapped;

	if (bIncreaseShroudCounter)
		pCell->IncreaseShroudCounter();
	else
		pCell->ReduceShroudCounter();

	char Visibility = TacticalClass::Instance->GetOcclusion(*pMapCoords, false);
	if (pCell->Visibility != Visibility)
	{
		bRevealed = true;
		pCell->Visibility = Visibility;
	}
	if (pCell->Visibility == -1)
		pCell->AltFlags |= AltCellFlags::NoFog;

	char Foggness = TacticalClass::Instance->GetOcclusion(*pMapCoords, true);
	if (pCell->Foggedness != Foggness)
	{
		bRevealed = true;
		pCell->Foggedness = Foggness;
	}
	if (pCell->Foggedness == -1)
		pCell->Flags |= CellFlags::CenterRevealed;

	if (bRevealed)
	{
		TacticalClass::Instance->RegisterCellAsVisible(pCell);
		pThis->RevealCheck(pCell, pHouse, bWasRevealed);
	}

	if (bShouldCleanFog)
		pCell->CleanFog();

	R->EAX(bRevealed);

	return 0x4A9DC6;
}

// CellClass_CleanFog
DEFINE_HOOK(0x486C50, CellClass_ClearFoggedObjects, 0x6)
{
	GET(CellClass*, pThis, ECX);

	auto pExt = CellExt::ExtMap.Find(pThis);
	for (auto const pObject : pExt->FoggedObjects)
	{
		if (pObject->CoveredType == FoggedObject::CoveredType::Building)
		{
			auto pRealCell = MapClass::Instance->GetCellAt(pObject->Location);

			for (auto pFoundation = pObject->BuildingData.Type->GetFoundationData(false);
				pFoundation->X != 0x7FFF || pFoundation->Y != 0x7FFF;
				++pFoundation)
			{
				CellStruct mapCoord =
				{
					pRealCell->MapCoords.X + pFoundation->X,
					pRealCell->MapCoords.Y + pFoundation->Y
				};

				auto pCell = MapClass::Instance->GetCellAt(mapCoord);
				if (pCell != pThis)
					CellExt::ExtMap.Find(pCell)->FoggedObjects.Remove(pObject);
			}

		}
		GameDelete(pObject);
	}
	pExt->FoggedObjects.Clear();

	return 0x486D8A;
}

DEFINE_HOOK(0x486A70, CellClass_FogCell, 0x5)
{
	GET(CellClass*, pThis, ECX);

	if (ScenarioClass::Instance->SpecialFlags.FogOfWar)
	{
		auto location = pThis->MapCoords;
		for (int i = 1; i < 15; i += 2)
		{
			auto pCell = MapClass::Instance->GetCellAt(location);
			auto nLevel = pCell->Level;

			if (nLevel >= i - 2 && nLevel <= i)
			{
				if (!(pCell->Flags & CellFlags::Fogged))
				{
					pCell->Flags |= CellFlags::Fogged;

					for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
					{
						switch (pObject->WhatAmI())
						{
						case AbstractType::Unit:
						case AbstractType::Infantry:
						case AbstractType::Aircraft:
							pObject->Deselect();
							break;

						case AbstractType::Building:
							if (auto pBld = abstract_cast<BuildingClass*>(pObject))
							{
								if (pBld->IsAllFogged())
									pBld->FreezeInFog(nullptr, pCell, !pBld->IsStrange() && pBld->Translucency != 15);
							}
							break;

						case AbstractType::Terrain:
							if (auto pTerrain = abstract_cast<TerrainClass*>(pObject))
							{
								// pTerrain
								auto pFoggedTer = GameCreate<FoggedObject>(pTerrain);
								CellExt::ExtMap.Find(pCell)->FoggedObjects.AddItem(pFoggedTer);
							}
							break;

						default:
							continue;
						}
					}
					if (pCell->OverlayTypeIndex != -1)
					{
						auto pFoggedOvl = GameCreate<FoggedObject>(pCell, true);
						CellExt::ExtMap.Find(pCell)->FoggedObjects.AddItem(pFoggedOvl);
					}
					if (pCell->SmudgeTypeIndex != -1)
					{
						auto pFoggedSmu = GameCreate<FoggedObject>(pCell, false);
						CellExt::ExtMap.Find(pCell)->FoggedObjects.AddItem(pFoggedSmu);
					}
				}
			}

			++location.X;
			++location.Y;
		}
	}

	return 0x486BE6;
}

DEFINE_HOOK(0x457AA0, BuildingClass_FreezeInFog, 0x5)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(CellClass*, pCell, 0x8);
	GET_STACK(bool, IsVisible, 0xC);

	if (!pCell)
		pCell = pThis->GetCell();

	pThis->Deselect();
	auto pFoggedBld = GameCreate<FoggedObject>(pThis, IsVisible);
	CellExt::ExtMap.Find(pCell)->FoggedObjects.AddItem(pFoggedBld);

	auto MapCoords = pThis->GetMapCoords();

	for (auto pFoundation = pThis->Type->GetFoundationData(false);
		pFoundation->X != 0x7FFF || pFoundation->Y != 0x7FFF;
		++pFoundation)
	{
		CellStruct currentMapCoord { MapCoords.X + pFoundation->X,MapCoords.Y + pFoundation->Y };
		auto pCurrentCell = MapClass::Instance->GetCellAt(currentMapCoord);
		if (pCurrentCell != pCell)
			CellExt::ExtMap.Find(pCurrentCell)->FoggedObjects.AddItem(pFoggedBld);
	}

	return 0x457C80;
}

// 486C50 = CellClass_ClearFoggedObjects, 6 Dumplcate

DEFINE_HOOK(0x70076E, TechnoClass_GetCursorOverCell_OverFog, 0x5)
{
	GET(CellClass*, pCell, EBP);
	auto const pExt = CellExt::ExtMap.Find(pCell);

	int nOvlIdx = -1;
	for (auto const pObject : pExt->FoggedObjects)
	{
		if (pObject->Visible)
		{
			if (pObject->CoveredType == FoggedObject::CoveredType::Overlay)
				nOvlIdx = pObject->OverlayData.Overlay;
			else if (pObject->CoveredType == FoggedObject::CoveredType::Building)
			{
				if (HouseClass::Player->IsAlliedWith(pObject->BuildingData.Owner) && pObject->BuildingData.Type->LegalTarget)
					R->Stack<bool>(STACK_OFFS(0x2C, 0x19), true);
			}
		}
	}

	if (nOvlIdx != -1)
		R->Stack<OverlayTypeClass*>(STACK_OFFS(0x2C, 0x18), OverlayTypeClass::Array->GetItem(nOvlIdx));

	return 0x700815;
}

DEFINE_HOOK(0x51F95F, InfantryClass_GetCursorOverCell_OverFog, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	GET(CellClass*, pCell, EAX);
	GET_STACK(const bool, bFog, STACK_OFFS(0x1C, -0x8));

	BuildingTypeClass* pType = nullptr;
	int ret = 0x51FA93;

	do
	{
		if (!ScenarioClass::Instance->SpecialFlags.FogOfWar || !bFog)
			break;

		for (const auto pObject : CellExt::ExtMap.Find(pCell)->FoggedObjects)
		{
			if (pObject->Visible && pObject->CoveredType == FoggedObject::CoveredType::Building)
			{
				pType = pObject->BuildingData.Type;

				if (pThis->Type->Engineer && pThis->Owner->ControlledByPlayer())
				{
					if (pType->BridgeRepairHut)
					{
						R->EAX(&pObject->Location);
						ret = 0x51FA35;
					}
					else // Ares hook 51FA82 = InfantryClass_GetActionOnCell_EngineerRepairable, 6
						ret = 0x51FA82;
				}
				break;
			}
		}
	}
	while (false);

	R->EBP(pType);
	return ret;
}

DEFINE_HOOK(0x6D3470, TacticalClass_DrawFoggedObject, 0x8)
{
	GET(TacticalClass*, pThis, ECX);
	GET_STACK(RectangleStruct*, pRect1, 0x4);
	GET_STACK(RectangleStruct*, pRect2, 0x8);
	GET_STACK(bool, bForceViewBounds, 0xC);

	RectangleStruct finalRect { 0,0,0,0 };
	if (bForceViewBounds && DSurface::ViewBounds->Width > 0 && DSurface::ViewBounds->Height > 0)
		finalRect = std::move(FoggedObject::Union(finalRect, DSurface::ViewBounds));
	else
	{
		if (pRect1->Width > 0 && pRect1->Height > 0)
			finalRect = std::move(FoggedObject::Union(finalRect, *pRect1));
		if (pRect2->Width > 0 && pRect2->Height > 0)
			finalRect = std::move(FoggedObject::Union(finalRect, *pRect2));

		if (const auto nVisibleCellCount = pThis->VisibleCellCount)
		{
			RectangleStruct buffer;
			buffer.Width = buffer.Height = 60;

			for (int i = 0; i < nVisibleCellCount; ++i)
			{
				auto const pCell = pThis->VisibleCells[i];
				auto location = pCell->GetCoords();
				Point2D point;
				TacticalClass::Instance->CoordsToClient(location, &point);
				buffer.X = DSurface::ViewBounds->X + point.X - 30;
				buffer.Y = DSurface::ViewBounds->Y + point.Y;
				finalRect = std::move(FoggedObject::Union(finalRect, buffer));
			}
		}

		for (const auto& dirty : Drawing::DirtyAreas())
		{
			RectangleStruct buffer = dirty.Rect;
			buffer.Y += DSurface::ViewBounds->Y;
			if (buffer.Width > 0 && buffer.Height > 0)
				finalRect = std::move(FoggedObject::Union(finalRect, buffer));
		}
	}

	if (finalRect.Width > 0 && finalRect.Height > 0)
	{
		finalRect.X = std::max(finalRect.X, 0);
		finalRect.Y = std::max(finalRect.Y, 0);
		finalRect.Width = std::min(finalRect.Width, DSurface::ViewBounds->Width - finalRect.X);
		finalRect.Height = std::min(finalRect.Height, DSurface::ViewBounds->Height - finalRect.Y);

		for (const auto pObject : FoggedObject::FoggedObjects)
			pObject->Render(finalRect);
	}

	return 0x6D3650;
}

DEFINE_HOOK(0x4ADFF0, MapClass_RevealMapShroud, 0x5)
{
	GET_STACK(bool, bHideBuilding, 0x4);
	GET_STACK(bool, bFog, 0x8);

	for (auto const pTechno : *TechnoClass::Array)
	{
		if (pTechno->WhatAmI() != AbstractType::Building || !bHideBuilding)
		{
			if (pTechno->GetTechnoType()->RevealToAll ||
				pTechno->DiscoveredByPlayer && pTechno->Owner->ControlledByPlayer() ||
				RulesClass::Instance->AllyReveal && pTechno->Owner->IsAlliedWith(HouseClass::Player))
			{
				pTechno->See(0, bFog);
				if (pTechno->IsInAir())
					MapClass::Instance->RevealArea3(&pTechno->Location,
						pTechno->LastSightRange - 3, pTechno->LastSightRange + 3, false);
			}
		}
	}

	return 0x4AE0A5;
}

DEFINE_HOOK(0x577EBF, MapClass_Reveal, 0x6)
{
	GET(CellClass*, pCell, EAX);

	// Vanilla YR code
	pCell->ShroudCounter = 0;
	pCell->GapsCoveringThisCell = 0;
	pCell->AltFlags |= AltCellFlags::Clear;
	pCell->Flags |= CellFlags::Revealed;

	// Extra process
	pCell->CleanFog();

	return 0x577EE9;
}

DEFINE_HOOK(0x586683, CellClass_DiscoverTechno, 0x5)
{
	GET(TechnoClass*, pTechno, EAX);
	GET(CellClass*, pThis, ESI);
	GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x18, -0x8));

	if (pTechno)
		pTechno->DiscoveredBy(pHouse);
	if (pThis->Jumpjet)
		pThis->Jumpjet->DiscoveredBy(pHouse);

	// EAX seems not to be used actually, so we needn't to set EAX here

	return 0x586696;
}

DEFINE_HOOK(0x4FC1FF, HouseClass_PlayerDefeated_MapReveal, 0x6)
{
	GET(HouseClass*, pHouse, ESI);

	*(int*)0xA8B538 = 1;
	MapClass::Instance->Reveal(pHouse);

	return 0x4FC214;
}