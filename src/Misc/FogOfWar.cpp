#include <Phobos.h>

#include <Helpers/Macro.h>

#include <Ext/Cell/Body.h>
#include <New/Entity/FoggedObject.h>

#include <Misc/MapRevealer.h>
#include <Misc/FogOfWar.h>

#include <AnimClass.h>
#include <TerrainClass.h>
#include <ScenarioClass.h>
#include <Drawing.h>
#include <OverlayTypeClass.h>
#include <InfantryClass.h>
#include <InfantryTypeClass.h>

// Burst-reveal detection for viewport refresh
static int  g_RevealedCellsThisFrame = 0;
static bool g_NeedViewportRefresh    = false;

// SpySat state tracking
static bool g_SpySatWasActive = false;

static inline void ForceUnfreezeAllFogProxiesForSpySat() {
	// snapshot to avoid invalidation while deleting
	DynamicVectorClass<FoggedObject*> snap;
	snap.Reserve(FoggedObject::FoggedObjects.Count);
	for (auto* fo : FoggedObject::FoggedObjects) {
		if (fo) { snap.AddItem(fo); }
	}
	for (auto* fo : snap) {
		GameDelete(fo); // FoggedObject dtor should thaw real building & restore footprint
	}
}



// ===== FoggedObject GC =====
// Requirements:
// - NO CellExt::ExtMap usage here.
// - Two-phase: collect pointers to kill, then delete.
// - Called before the fog proxy render loop each frame (or every N frames).

static void SafeCleanupOrphanedFoggedObjects()
{
	static int frameCounter = 0;

	// Run every 15 frames to avoid performance issues during mass reveals
	if ((++frameCounter % 15) != 0) {
		return;
	}

	auto& live = FoggedObject::FoggedObjects; // global container
	if (live.Count <= 0) {
		return;
	}

	// Collect candidates first to avoid mutating 'live' while scanning it.
	DynamicVectorClass<FoggedObject*> toDelete;
	// Guard reserve to something sane to avoid wild counts if memory is corrupt.
	const int cap = (live.Count > 0 && live.Count < 1000000) ? live.Count : 0;
	if (cap > 0) { toDelete.Reserve(cap); }

	for (int i = 0; i < live.Count; ++i) {
		FoggedObject* fo = live[i];
		if (!fo) { continue; }

		// Find the cell by coordinates; DO NOT use ExtMap.
		CellStruct cellCoords = CellClass::Coord2Cell(fo->Location);
		CellClass* cell = MapClass::Instance.TryGetCellAt(cellCoords);
		if (!cell) {
			// Cell missing → definitely orphaned
			toDelete.AddItem(fo);
			continue;
		}

		// Orphan rule: if the cell is *not covered* anymore (no fog & no shroud),
		// keeping a fog proxy makes no sense; schedule for deletion.
		const bool shrouded = !(cell->Flags & CellFlags::EdgeRevealed);
		const bool fogged   = (cell->Foggedness != -1) || (cell->Flags & CellFlags::Fogged);

		if (!shrouded && !fogged) {
			toDelete.AddItem(fo);
		}
	}

	// Now actually remove and free.
	// GameDelete() auto-removes from 'live', so we can skip Remove(fo).
	for (int i = 0; i < toDelete.Count; ++i) {
		FoggedObject* fo = toDelete[i];
		// Ensure it is gone from the render list before dealloc (defensive).
		FoggedObject::FoggedObjects.Remove(fo);
		GameDelete(fo);
	}
}

DEFINE_HOOK(0x6B8E7A, ScenarioClass_LoadSpecialFlags, 0x5)
{
	ScenarioClass::Instance->SpecialFlags.FogOfWar =
		RulesClass::Instance->FogOfWar || R->EAX<bool>() || GameModeOptionsClass::Instance.FogOfWar;

	R->ECX(R->EDI());
	return 0x6B8E8B;
}

DEFINE_HOOK(0x686C03, SetScenarioFlags_FogOfWar, 0x5)
{
	GET(ScenarioFlags, SFlags, EAX);

	SFlags.FogOfWar = RulesClass::Instance->FogOfWar || GameModeOptionsClass::Instance.FogOfWar;

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

	// SpySat: bypass all fog gating (no ExtMap access, just draw)
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive) {
		return 0x5F4B48; // draw normally
	}

	switch (pThis->WhatAmI())
	{
	case AbstractType::Anim:
		if (!static_cast<AnimClass*>(pThis)->Type->ShouldFogRemove)
			return 0x5F4B48;
		break;

	case AbstractType::Unit:
	case AbstractType::Cell:
	case AbstractType::Building:  // Buildings participate in fog gating
		break;

	default:
		return 0x5F4B48;
	}

	bool fogged = false;

	if (pThis->WhatAmI() == AbstractType::Building) {
		auto* bld = static_cast<BuildingClass*>(pThis);

		// Own/aligned buildings: always draw (YR behavior)
		if (bld->Owner &&
			(bld->Owner->IsControlledByCurrentPlayer() ||
			 (RulesClass::Instance->AllyReveal && bld->Owner->IsAlliedWith(HouseClass::CurrentPlayer)))) {
			return 0x5F4B48; // draw
		}

		// Enemy/neutral buildings: hide only if entire footprint is fogged
		fogged = bld->IsAllFogged();
	} else {
		// Units, anims, terrain, cells, etc. – single-cell check is fine
		fogged = MapClass::Instance.IsLocationFogged(pThis->GetCoords());
	}

	if (!fogged) {
		return 0x5F4B48;  // normal draw
	}

	// Fogged: skip draw this frame (don't keep forcing redraw while fogged)
	pThis->NeedsRedraw = false;
	return 0x5F4D06;
}

DEFINE_HOOK(0x6F5190, TechnoClass_DrawExtras_CheckFog, 0x6)
{
	GET(TechnoClass*, pThis, ECX);

	return MapClass::Instance.IsLocationFogged(pThis->GetCoords()) ? 0x6F5EEC : 0;
}


DEFINE_HOOK(0x6D6EDA, TacticalClass_Overlay_CheckFog1, 0xA)
{
	GET(CellClass*, pCell, EAX);

	// SpySat reveals entire map, so no cell is fogged when SpySat is active
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive)
		return pCell->OverlayTypeIndex == -1 ? 0x6D7006 : 0x6D6EE4;

	return pCell->OverlayTypeIndex == -1 || pCell->IsFogged() ? 0x6D7006 : 0x6D6EE4;
}

DEFINE_HOOK(0x6D70BC, TacticalClass_Overlay_CheckFog2, 0xA)
{
	GET(CellClass*, pCell, EAX);

	// SpySat reveals entire map, so no cell is fogged when SpySat is active
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive)
		return pCell->OverlayTypeIndex == -1 ? 0x6D71A4 : 0x6D70C6;

	return pCell->OverlayTypeIndex == -1 || pCell->IsFogged() ? 0x6D71A4 : 0x6D70C6;
}

DEFINE_HOOK(0x71CC8C, TerrainClass_DrawIfVisible, 0x6)
{
	GET(TerrainClass*, pThis, EDI);
	pThis->NeedsRedraw = false;

	return pThis->InLimbo || MapClass::Instance.IsLocationFogged(pThis->GetCoords()) ? 0x71CD8D : 0x71CC9A;
}

DEFINE_HOOK(0x4804A4, CellClass_DrawTile_DrawSmudgeIfVisible, 0x6)
{
	GET(CellClass*, pThis, ESI);

	// SpySat reveals entire map, so no cell is fogged when SpySat is active
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive)
		return 0;

	return pThis->IsFogged() ? 0x4804FB : 0;
}

DEFINE_HOOK(0x5865E2, MapClass_IsLocationFogged, 0x3)
{
	GET_STACK(CoordStruct*, pCoord, 0x4);

	// SpySat reveals entire map (both fog and shroud), so no location is fogged when SpySat is active
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive)
	{
		R->EAX<bool>(false);
		return 0;
	}

	MapRevealer revealer(*pCoord);
	CellClass* pCell = MapClass::Instance.GetCellAt(revealer.Base());

	R->EAX<bool>(pCell->Flags & CellFlags::EdgeRevealed ?
		false :
		!(pCell->GetNeighbourCell(static_cast<FacingType>(3))->Flags & CellFlags::EdgeRevealed));

	return 0;
}

DEFINE_HOOK(0x6D7A4F, TacticalClass_DrawPixelEffects_FullFogged, 0x6)
{
	GET(CellClass*, pCell, ESI);

	// SpySat reveals entire map, so no cell is fogged when SpySat is active
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive)
		return 0;

	return pCell->IsFogged() ? 0x6D7BB8 : 0;
}


DEFINE_HOOK(0x4ACE3C, MapClass_TryReshroudCell_SetCopyFlag, 0x6)
{
	GET(CellClass*, pCell, EAX);

	bool bNoFog = static_cast<bool>(pCell->AltFlags & AltCellFlags::NoFog);
	pCell->AltFlags &= ~AltCellFlags::NoFog;
	int Index = TacticalClass::Instance->GetOcclusion(pCell->MapCoords, false);

	if ((bNoFog || pCell->Visibility != Index) && Index >= 0 && pCell->Visibility >= -1)
	{
		pCell->AltFlags |= AltCellFlags::Mapped;
		pCell->Visibility = static_cast<char>(Index);
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

	if (bShouldCleanFog) {
		++g_RevealedCellsThisFrame;
		// Heuristic: 64 cells ~ a modest burst. Tweak if needed.
		if (g_RevealedCellsThisFrame >= 64) {
			g_NeedViewportRefresh = true;
		}
		
		pCell->CleanFog();
		
		// Just set buildings in this cell to redraw - let the fog gate handle visibility
		for (ObjectClass* pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject) {
			if (auto pBuilding = abstract_cast<BuildingClass*>(pObject)) {
				pBuilding->NeedsRedraw = true;
			}
		}
	}

	R->EAX(bRevealed);

	return 0x4A9DC6;
}

DEFINE_HOOK(0x486C50, CellClass_ClearFoggedObjects, 0x6)
{
	// No deletion work here; avoid re-entrancy / bad ECX.
	// Let our Ext-free GC handle deletions safely from the draw pass.
	return 0x486D8A;
}

DEFINE_HOOK(0x486A70, CellClass_FogCell, 0x5)
{
	GET(CellClass*, pThis, ECX);

	// SpySat => do not fog cells at all
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive) {
		return 0x486BE6; // skip fogging loop
	}

	if (ScenarioClass::Instance->SpecialFlags.FogOfWar)
	{
		auto location = pThis->MapCoords;
		for (int i = 1; i < 15; i += 2)
		{
			CellClass* pCell = MapClass::Instance.GetCellAt(location);
			int nLevel = pCell->Level;

			if (nLevel >= i - 2 && nLevel <= i)
			{
				if (!(pCell->Flags & CellFlags::Fogged))
				{
					pCell->Flags |= CellFlags::Fogged;

					for (ObjectClass* pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
					{
						switch (pObject->WhatAmI())
						{
						case AbstractType::Unit:
						case AbstractType::Infantry:
						case AbstractType::Aircraft:
							pObject->Deselect();
							break;

						case AbstractType::Building:
							if (BuildingClass* pBld = abstract_cast<BuildingClass*>(pObject))
							{
								if (pBld->IsAllFogged())
									pBld->FreezeInFog(nullptr, pCell, !pBld->IsStrange() && pBld->Translucency != 15);
							}
							break;

						case AbstractType::Terrain:
							if (TerrainClass* pTerrain = abstract_cast<TerrainClass*>(pObject))
							{
								// pTerrain
								FoggedObject* pFoggedTer = GameCreate<FoggedObject>(pTerrain);
								CellExt::ExtMap.Find(pCell)->FoggedObjects.AddItem(pFoggedTer);
							}
							break;

						default:
							continue;
						}
					}
					if (pCell->OverlayTypeIndex != -1)
					{
						FoggedObject* pFoggedOvl = GameCreate<FoggedObject>(pCell, true);
						CellExt::ExtMap.Find(pCell)->FoggedObjects.AddItem(pFoggedOvl);
					}
					if (pCell->SmudgeTypeIndex != -1)
					{
						FoggedObject* pFoggedSmu = GameCreate<FoggedObject>(pCell, false);
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

	// 1) Do not freeze buildings the player should always see
	if (pThis->Owner && (
		 pThis->Owner->IsControlledByCurrentPlayer() ||
		 (RulesClass::Instance->AllyReveal && pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer))
	)) {
		return 0x457C80; // skip engine freeze
	}

	// 2) Do not freeze anything while SpySat is active (global reveal)
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive) {
		return 0x457C80; // skip engine freeze
	}

	if (!pCell)
		pCell = pThis->GetCell();

	pThis->Deselect();

	// Per-cell fog proxies (one FoggedObject per footprint cell)
	auto addToCell = [&](CellClass* dst) {
		auto* fo = GameCreate<FoggedObject>(pThis, IsVisible);
		CellExt::ExtMap.Find(dst)->FoggedObjects.AddItem(fo);
	};

	// owner cell
	addToCell(pCell);

	// every other foundation cell gets its own FoggedObject instance
	auto MapCoords = pThis->GetMapCoords();
	for (auto pFoundation = pThis->Type->GetFoundationData(false);
		pFoundation->X != 0x7FFF || pFoundation->Y != 0x7FFF;
		++pFoundation)
	{
		CellStruct cs { static_cast<short>(MapCoords.X + pFoundation->X),
						static_cast<short>(MapCoords.Y + pFoundation->Y) };
		if (CellClass* other = MapClass::Instance.GetCellAt(cs)) {
			if (other != pCell) {
				addToCell(other);
			}
		}
	}

	return 0x457C80;
}

// 486C50 = CellClass_ClearFoggedObjects, 6 Dumplcate

DEFINE_HOOK(0x70076E, TechnoClass_GetCursorOverCell_OverFog, 0x5)
{
	GET(CellClass*, pCell, EBP);
	auto const pExt = CellExt::ExtMap.Find(pCell);

	int nOvlIdx = -1;
	if (pExt && pExt->FoggedObjects.Count > 0) {
		for (auto const pObject : pExt->FoggedObjects)
		{
			// Safety check: ensure pObject is valid
			if (!pObject) continue;

			if (pObject->Visible)
			{
				if (pObject->CoveredType == FoggedObject::CoveredType::Overlay)
					nOvlIdx = pObject->OverlayData.Overlay;
				else if (pObject->CoveredType == FoggedObject::CoveredType::Building)
				{
					// Enhanced safety: wrap all potentially dangerous accesses
					__try {
						// Owner-free, visibility-only approach to avoid crashes
						if (HouseClass::CurrentPlayer &&
							pObject->BuildingData.Type &&
							pObject->BuildingData.Type->LegalTarget) {

							if (HouseClass::CurrentPlayer->SpySatActive) {
								R->Stack<bool>(STACK_OFFSET(0x2C, 0x19), true);
							} else {
								// Use current visibility, not "ever seen"
								if (!Fog::IsFogged(pObject->Location)) {
									R->Stack<bool>(STACK_OFFSET(0x2C, 0x19), true);
								} else {
									R->Stack<bool>(STACK_OFFSET(0x2C, 0x19), false); // make intent explicit
									static int logCount = 0;
									if (logCount++ < 5) {
										Debug::Log("DEBUG: Blocked cursor targeting - building under fog\n");
									}
								}
							}
						}
					}
					__except (EXCEPTION_EXECUTE_HANDLER) {
						// Any access violation - just skip this fogged object entirely
						static int crashLog = 0;
						if (crashLog++ < 3) {
							Debug::Log("DEBUG: Caught cursor access violation - skipping fogged building\n");
						}
					}
				}
			}
		}
	}

	if (nOvlIdx != -1)
		R->Stack<OverlayTypeClass*>(STACK_OFFSET(0x2C, 0x18), OverlayTypeClass::Array.GetItem(nOvlIdx));

	return 0x700815;
}

// TODO: Clean up invalid fog proxies - disabled due to wrong address/register causing crashes
/*
DEFINE_HOOK(0x486C50, CellClass_ClearFoggedObjects_CleanupInvalid, 0x6)
{
	GET(CellClass*, pCell, ESI);
	auto const pExt = CellExt::ExtMap.Find(pCell);

	if (pExt && pExt->FoggedObjects.Count > 0) {
		// Clean up invalid building proxies (when real building no longer exists)
		for (int i = pExt->FoggedObjects.Count - 1; i >= 0; i--) {
			auto* pObject = pExt->FoggedObjects[i];
			if (pObject && pObject->CoveredType == FoggedObject::CoveredType::Building) {
				// Check if the real building still exists at this location
				auto* pRealBuilding = pCell->GetBuilding();
				if (!pRealBuilding ||
					pRealBuilding->Type != pObject->BuildingData.Type ||
					pRealBuilding->Owner != pObject->BuildingData.Owner) {
					// Real building is gone or different - delete the proxy
					pExt->FoggedObjects.RemoveItem(i);
					GameDelete(pObject);
				}
			}
		}
	}

	return 0; // Continue normal execution
}
*/

// Universal particle gate causing crashes - disabling
/*
DEFINE_HOOK(0x62CEC0, ParticleClass_Draw_FoWGate, 0x5)
{
	GET(ParticleClass*, pThis, ECX);

	// show-all bypass (existing behaviour)
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive) {
		return 0; // run original draw
	}

	if (ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar && HouseClass::CurrentPlayer) {
		const auto cs = CellClass::Coord2Cell(pThis->GetCoords());
		const auto* c = MapClass::Instance.TryGetCellAt(cs);

		// If this cell was never revealed, skip drawing this particle
		if (!c || !(c->Flags & CellFlags::EdgeRevealed)) {
			return 0x62D295; // safe tail of ParticleClass::Draw (from gamemd.json)
		}
	}
	return 0; // run original
}
*/

// Disabling all new hooks - back to stable approach only
/*
DEFINE_HOOK(0x41C000, TechnoClass_UpdateRefinerySmokeSystems_FoWGate, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive) {
		return 0; // allow normal smoke if spysat is on
	}

	if (ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar && HouseClass::CurrentPlayer) {
		const auto cs = CellClass::Coord2Cell(pThis->GetCoords());
		const auto* c = MapClass::Instance.TryGetCellAt(cs);
		if (!c || !(c->Flags & CellFlags::EdgeRevealed)) {
			// TODO: Need to find the actual epilogue address for this function
			// For now, let's try a minimal skip
			return 0x41C001; // Temporary - need actual function end address
		}
	}
	return 0; // run original
}
*/

// Gate refinery smoke at call-site - disabling for stability testing
/*
DEFINE_HOOK(0x7E3C94, Skip_RefinerySmoke_Call_UnitUnload, 0x5)
{
	GET(TechnoClass*, pThis, ECX); // Assuming ECX holds the techno at this call site

	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive) {
		return 0; // allow normal smoke if spysat is on
	}

	if (ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar && HouseClass::CurrentPlayer) {
		const auto cs = CellClass::Coord2Cell(pThis->GetCoords());
		const auto* c = MapClass::Instance.TryGetCellAt(cs);
		if (!c || !(c->Flags & CellFlags::EdgeRevealed)) {
			return 0x7E3C99; // Skip past the call (5 bytes = call instruction size)
		}
	}
	return 0; // perform the original call
}
*/

// Refinery smoke gate with proper visibility checking
DEFINE_HOOK(0x73E37E, UnitClass_Unload_RefinerySmoke_FoWGate, 0x6)
{
	if (!(ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar))
		return 0;

	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive)
		return 0;

	// Best-effort visibility test: use the unit doing the unload.
	// (On this path the harvester is at/inside the refinery cell.)
#if defined(REF_SMOKE_USE_EDI)
	GET(UnitClass*, pUnit, EDI);
#else
	GET(UnitClass*, pUnit, ESI); // Default: try ESI first
#endif

	if (pUnit && Fog::IsFogged(pUnit->GetCoords())) {
		static int logCount = 0;
		if (logCount++ < 10) {
			Debug::Log("DEBUG: Blocking RefinerySmokeParticleSystem for %s under fog\n", pUnit->GetTechnoType()->ID);
		}
		return 0x73E38E; // skip the virtual smoke update when the unload cell is fogged
	}

	// Optional: log allows to prove visibility path works
	static int allowLog = 0;
	if (pUnit && allowLog++ < 5) {
		Debug::Log("DEBUG: Allow RefinerySmokeParticleSystem for %s (visible now)\n", pUnit->GetTechnoType()->ID);
	}

	return 0; // run original call
}

// Infantry refinery smoke: DISABLED - incorrect hook location/parameters
// Leaving this disabled to avoid breaking pathfinding or other infantry behavior
/*
DEFINE_HOOK(0x522D75, InfantryClass_UnloadAt_Building_RefinerySmoke_FoWGate, 0x6)
{
	// This hook was not working correctly - disabling for stability
	return 0;
}
*/

// 0x62F310 address causes crashes - disabling particle update hooks
// Falling back to stable two-layer producer-side + OnEarlyUpdate approach

// TODO: Refinery smoke hook at 0x7E4324 - disabled due to invalid memory access
// Need to investigate correct registers and skip address
/*
DEFINE_HOOK(0x7E4324, RefinerySmoke_FogGate, 0x6)
{
	// SpySat bypass - if SpySat is active, show everything
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive)
		return 0;

	// Check if we're in fog of war mode
	if (ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar && HouseClass::CurrentPlayer)
	{
		// This will need adjustment based on what registers/stack are available
		// Making educated guesses about common refinery smoke function parameters

		// Try to get building/techno from common registers
		GET(TechnoClass*, pTechno, ESI);  // Common for techno operations
		if (!pTechno) {
			GET(BuildingClass*, pBuilding, ESI);
			pTechno = pBuilding;
		}

		if (pTechno && FoW::EnemyTechnoUnderFog(pTechno)) {
			// Skip refinery smoke creation/update for enemy under fog
			return 0x7E4380; // Estimated skip address - may need adjustment
		}
	}

	return 0; // Continue normal execution
}
*/

DEFINE_HOOK(0x51F95F, InfantryClass_GetCursorOverCell_OverFog, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	GET(CellClass*, pCell, EAX);
	GET_STACK(const bool, bFog, STACK_OFFSET(0x1C, -0x8));

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

				if (pThis->Type->Engineer && pThis->Owner->IsControlledByCurrentPlayer())
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

	// Clean up orphans BEFORE computing finalRect or rendering
	SafeCleanupOrphanedFoggedObjects();

	// SpySat edge-triggered thaw
	const bool spyNow = (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive);
	if (spyNow && !g_SpySatWasActive) {
		ForceUnfreezeAllFogProxiesForSpySat();
		g_NeedViewportRefresh = true; // force same-frame repaint
	}
	g_SpySatWasActive = spyNow;

	// One-shot viewport repaint if we had a burst
	if (g_NeedViewportRefresh) {
		// Mark the whole tactical viewport dirty once
		RectangleStruct vb = DSurface::ViewBounds;
		TacticalClass::Instance->RegisterDirtyArea(vb, /*force*/true);

		g_NeedViewportRefresh = false;
	}

	// Reset per-frame counter
	g_RevealedCellsThisFrame = 0;

	// SpySat reveals everything, so don't draw fogged objects when SpySat is active
	if (HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive)
		return 0x6D3650; // Skip fogged object rendering

	RectangleStruct finalRect { 0,0,0,0 };
	if (bForceViewBounds && DSurface::ViewBounds.Width > 0 && DSurface::ViewBounds.Height > 0)
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
				auto [point, visible] = TacticalClass::Instance->CoordsToClient(location);
				buffer.X = DSurface::ViewBounds.X + point.X - 30;
				buffer.Y = DSurface::ViewBounds.Y + point.Y;
				finalRect = std::move(FoggedObject::Union(finalRect, buffer));
			}
		}

		// TODO: Fix Drawing::DirtyAreas() access - may not exist in current Phobos
		//for (int i = 0; i < Drawing::DirtyAreas().Count; i++)
		//{
		//	RectangleStruct buffer = Drawing::DirtyAreas()[i].Rect;
		//	buffer.Y += DSurface::ViewBounds.Y;
		//	if (buffer.Width > 0 && buffer.Height > 0)
		//		finalRect = std::move(FoggedObject::Union(finalRect, buffer));
		//}
	}

	if (finalRect.Width > 0 && finalRect.Height > 0)
	{
		finalRect.X = std::max(finalRect.X, 0);
		finalRect.Y = std::max(finalRect.Y, 0);
		finalRect.Width = std::min(finalRect.Width, DSurface::ViewBounds.Width - finalRect.X);
		finalRect.Height = std::min(finalRect.Height, DSurface::ViewBounds.Height - finalRect.Y);

		for (const auto pObject : FoggedObject::FoggedObjects)
			pObject->Render(finalRect);
	}

	return 0x6D3650;
}

DEFINE_HOOK(0x4ADFF0, MapClass_RevealMapShroud, 0x5)
{
	GET_STACK(bool, bHideBuilding, 0x4);
	GET_STACK(bool, bFog, 0x8);

	for (int i = 0; i < TechnoClass::Array.Count; i++)
	{
		auto const pTechno = TechnoClass::Array.GetItem(i);

		if (pTechno->WhatAmI() != AbstractType::Building || !bHideBuilding)
		{
			if (pTechno->GetTechnoType()->RevealToAll ||
				pTechno->DiscoveredByCurrentPlayer && pTechno->Owner->IsControlledByCurrentPlayer() ||
				RulesClass::Instance->AllyReveal && pTechno->Owner->IsAlliedWith(HouseClass::CurrentPlayer))
			{
				pTechno->See(0, bFog);
				if (pTechno->IsInAir())
					MapClass::Instance.RevealArea3(&pTechno->Location,
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
	++g_RevealedCellsThisFrame;
	// Heuristic: 64 cells ~ a modest burst. Tweak if needed.
	if (g_RevealedCellsThisFrame >= 64) {
		g_NeedViewportRefresh = true;
	}
	
	pCell->CleanFog();
	
	// Just set buildings in this cell to redraw - let the fog gate handle visibility
	for (ObjectClass* pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject) {
		if (auto pBuilding = abstract_cast<BuildingClass*>(pObject)) {
			pBuilding->NeedsRedraw = true;
		}
	}

	return 0x577EE9;
}

DEFINE_HOOK(0x586683, CellClass_DiscoverTechno, 0x5)
{
	GET(TechnoClass*, pTechno, EAX);
	GET(CellClass*, pThis, ESI);
	GET_STACK(HouseClass*, pHouse, STACK_OFFSET(0x18, -0x8));

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
	MapClass::Instance.Reveal(pHouse);

	return 0x4FC214;
}

// Check if building placement should be allowed at location (respects fog like shroud)
bool CanPlaceBuildingInFog(const CoordStruct& coords, BuildingTypeClass* pBuildingType) {
	// If fog of war is disabled, allow placement
	if (!ScenarioClass::Instance->SpecialFlags.FogOfWar) {
		return true;
	}

	// Check if any part of the building foundation is fogged
	if (pBuildingType) {
		CellStruct baseCell = CellClass::Coord2Cell(coords);

		// Check each foundation cell using the new safe helpers
		for (int x = 0; x < pBuildingType->GetFoundationWidth(); x++) {
			for (int y = 0; y < pBuildingType->GetFoundationHeight(false); y++) {
				CellStruct foundationCell = {
					static_cast<short>(baseCell.X + x),
					static_cast<short>(baseCell.Y + y)
				};

				CoordStruct cellCoords = CellClass::Cell2Coord(foundationCell);

				// If any foundation cell is fogged, disallow placement
				if (!Fog::ShouldShowActiveAt(cellCoords)) {
					return false;
				}
			}
		}
	}

	return true; // All foundation cells are clear
}