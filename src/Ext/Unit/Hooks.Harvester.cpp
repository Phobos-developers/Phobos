#include <Ext/Techno/Body.h>

#pragma region EnterRefineryFix

DEFINE_HOOK(0x74312A, UnitClass_SetDestination_ReplaceWithHarvestMission, 0x5)
{
	enum { SkipGameCode = 0x742F48 };

	GET(UnitClass*, pThis, EBP);

	// Jumpjet will overlap when entering buildings,
	// which can cause errors in the connection between jumpjet harvester and refinery building,
	// leading to game crashes in drawing
	// Here change the Mission::Enter to Mission::Harvest
	pThis->QueueMission(Mission::Harvest, false);
	pThis->NextMission();
	pThis->MissionStatus = 2; // Status: returning to refinery
	pThis->IsHarvesting = false;
	// Note: jumpjet harvester should not be allowed to comply with this behavior alone, otherwise
	// it may still overlap with other types and crash

	return SkipGameCode;
}

DEFINE_HOOK(0x73E739, UnitClass_Mission_Harvest_SkipUselessArchiveTarget, 0x5)
{
	enum { SkipGameCode = 0x73E755 };

	GET(UnitClass*, pThis, EBP);
	GET(AbstractClass*, pFocus, EAX); // pThis->ArchiveTarget

	// Removing unnecessary set destination
	// This can effectively reduce the ineffective actions when Harvester automatically returning
	// to work after be manually operated to return to Refinery.
	if (pFocus->WhatAmI() != AbstractType::Building || pThis->GetCell()->GetBuilding() != pFocus)
		return 0;

	// Clear ArchiveTarget to avoid checking again next time
	pThis->ArchiveTarget = nullptr;

	return SkipGameCode;
}

#pragma endregion

#pragma region JumpjetHarvesters

DEFINE_HOOK(0x74613C, UnitClass_INoticeSink_CheckJumpjetHarvester, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pType = pThis->Type;

	// Let jumpjet harvesters automatically go mining when leaving the factory
	if (pType->Harvester || pType->Weeder)
	{
		// Have checked pThis->HasAnyLink()
		if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pThis->GetNthLink()))
		{
			// Only need to check WeaponsFactory
			if (pBuilding->Type->WeaponsFactory)
				pThis->QueueMission(Mission::Harvest, true);
		}
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x73E411, UnitClass_Mission_Unload_DumpAmount, 0x7)
{
	enum { SkipGameCode = 0x73E41D };

	GET(UnitClass*, pThis, ESI);
	GET(const int, tiberiumIdx, EBP);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	const float totalAmount = pThis->Tiberium.GetAmount(tiberiumIdx);
	float dumpAmount = pTypeExt->HarvesterDumpAmount.Get(RulesExt::Global()->HarvesterDumpAmount);

	if (dumpAmount <= 0.0f || totalAmount < dumpAmount)
		dumpAmount = totalAmount;

	__asm fld dumpAmount;

	return SkipGameCode;
}

DEFINE_HOOK(0x4D6D34, FootClass_MissionAreaGuard_Miner, 0x5)
{
	enum { GoGuardArea = 0x4D6D69 };

	GET(FootClass*, pThis, ESI);

	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (pTypeExt->Harvester_CanGuardArea && pThis->Owner->IsControlledByHuman())
	{
		if (!pTypeExt->Harvester_CanGuardArea_RequireTarget || pThis->TargetAndEstimateDamage(pThis->Location, ThreatType::Area))
			return GoGuardArea;
	}

	return 0;
}

#pragma region HarvesterScanAfterUnload

DEFINE_HOOK(0x73E730, UnitClass_MissionHarvest_HarvesterScanAfterUnload, 0x5)
{
	GET(UnitClass* const, pThis, EBP);
	GET(AbstractClass* const, pFocus, EAX);

	const auto pType = pThis->Type;
	// Focus is set when the harvester is fully loaded and go home.
	if (pFocus && !pType->Weeder && TechnoTypeExt::ExtMap.Find(pType)->HarvesterScanAfterUnload.Get(RulesExt::Global()->HarvesterScanAfterUnload))
	{
		auto cellBuffer = CellStruct::Empty;
		const auto pCellStru = pThis->ScanForTiberium(&cellBuffer, RulesClass::Instance->TiberiumLongScan / Unsorted::LeptonsPerCell, 0);

		if (*pCellStru != CellStruct::Empty)
		{
			const auto pCell = MapClass::Instance.TryGetCellAt(*pCellStru);
			const auto distFromTiberium = pCell ? pThis->DistanceFrom(pCell) : -1;
			const auto distFromFocus = pThis->DistanceFrom(pFocus);

			// Check if pCell is better than focus.
			if (distFromTiberium > 0 && distFromTiberium < distFromFocus)
				R->EAX(pCell);
		}
	}

	return 0;
}

#pragma endregion

// Hooks that allow harvesters / weeders to work correctly with MovementZone=Subterannean (sic) - Starkku
#pragma region SubterraneanHarvesters

// Allow scanning for docks in all map zones.
DEFINE_HOOK(0x4DEFC6, FootClass_FindDock_SubterraneanHarvester, 0x5)
{
	GET(TechnoTypeClass*, pTechnoType, EAX);

	if (auto const pUnitType = abstract_cast<UnitTypeClass*>(pTechnoType))
	{
		if ((pUnitType->Harvester || pUnitType->Weeder) && pUnitType->MovementZone == MovementZone::Subterrannean)
			R->ECX(MovementZone::Fly);
	}

	return 0;
}

// Allow scanning for ore in all map zones.
DEFINE_HOOK(0x4DCF86, FootClass_FindTiberium_SubterraneanHarvester, 0x5)
{
	enum { SkipGameCode = 0x4DCF9B };

	GET(MovementZone, mZone, ECX);

	if (mZone == MovementZone::Subterrannean)
		R->ECX(MovementZone::Fly);

	return 0;
}

// Allow scanning for weeds in all map zones.
DEFINE_HOOK(0x4DDB23, FootClass_FindWeeds_SubterraneanHarvester, 0x5)
{
	enum { SkipGameCode = 0x4DCF9B };

	GET(MovementZone, mZone, EAX);

	if (mZone == MovementZone::Subterrannean)
		R->EAX(MovementZone::Fly);

	return 0;
}

// Set flag on factory exit to mark the harvester as having just exited factory.
DEFINE_HOOK(0x44459A, BuildingClass_ExitObject_SubterraneanHarvester, 0x5)
{
	GET(TechnoClass*, pThis, EDI);

	if (auto const pUnit = abstract_cast<UnitClass*>(pThis))
	{
		auto const pType = pUnit->Type;

		if ((pType->Harvester || pType->Weeder) && pType->MovementZone == MovementZone::Subterrannean)
		{
			auto const pExt = TechnoExt::ExtMap.Find(pUnit);
			pExt->SubterraneanHarvStatus = 1;
			pExt->SubterraneanHarvRallyPoint = pThis->ArchiveTarget;
			pThis->ArchiveTarget = nullptr;
		}
	}

	return 0;
}

// Apply same special rules on idle to player-owned subterranean harvesters as to Teleporter=yes ones.
DEFINE_HOOK(0x740949, UnitClass_Mission_Guard_SubterraneanHarvester, 0x6)
{
	enum { Continue = 0x740957 };

	GET(UnitTypeClass*, pType, ECX);

	if (pType->MovementZone == MovementZone::Subterrannean)
		return Continue;

	return 0;
}

// Fix an edge case issue stemming from subterranean unit nav queue handling leading
// to harvesters becoming idle randomly while harvesting.
DEFINE_HOOK(0x738A3E, UnitClass_EnterIdleMode_SubterraneanHarvester, 0x5)
{
	enum { ReturnFromFunction = 0x738D21 };

	GET(UnitClass*, pThis, ESI);

	if (auto const pUnit = abstract_cast<UnitClass*>(pThis))
	{
		auto const pType = pUnit->Type;

		if ((pType->Harvester || pType->Weeder) && pType->MovementZone == MovementZone::Subterrannean)
		{
			auto const mission = pUnit->CurrentMission;

			if (mission == Mission::Unload || mission == Mission::Harvest)
				return ReturnFromFunction;
		}
	}

	return 0;
}

#pragma endregion
