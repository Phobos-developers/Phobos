#include "Body.h"

#include <TacticalClass.h>

#include <Ext/House/Body.h>
#include <Ext/Scenario/Body.h>

// Buildable Proximity Helper
namespace ProximityTemp
{
	bool Build = false;
	bool Exist = false;
	CellClass* CurrentCell = nullptr;
	BuildingTypeClass* BuildType = nullptr;
}

// BaseNormal extra checking Hook #1-1 -> sub_4A8EB0 - Set context and clear up data
DEFINE_HOOK(0x4A8F20, DisplayClass_BuildingProximityCheck_SetContext, 0x5)
{
	GET(BuildingTypeClass*, pType, ESI);

	if (RulesExt::Global()->PlacementGrid_Expand)
		ScenarioExt::Global()->BaseNormalCells.clear();

	ProximityTemp::Build = false;
	ProximityTemp::BuildType = pType;

	return 0;
}

// BaseNormal extra checking Hook #1-2 -> sub_4A8EB0 - Check unit base normal
DEFINE_HOOK(0x4A8FCA, DisplayClass_BuildingProximityCheck_BaseNormalExtra, 0x7)
{
	enum { CanBuild = 0x4A9027, ContinueCheck = 0x4A8FD1 };

	GET(CellClass*, pCell, EAX);
	GET_STACK(const int, idxHouse, STACK_OFFSET(0x30, 0x8));

	ProximityTemp::CurrentCell = pCell;

	if (RulesExt::Global()->CheckUnitBaseNormal)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			const auto pCellUnit = abstract_cast<UnitClass*>(pObject);

			if (!pCellUnit)
				continue;

			const auto pOwner = pCellUnit->Owner;
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pCellUnit->Type);

			auto canBeBaseNormal = [&]()
			{
				if (pOwner->ArrayIndex == idxHouse)
					return pTypeExt->UnitBaseNormal.Get();
				else if (RulesClass::Instance->BuildOffAlly && pOwner->IsAlliedWith(HouseClass::Array->Items[idxHouse]))
					return pTypeExt->UnitBaseForAllyBuilding.Get();

				return false;
			};

			if (canBeBaseNormal())
			{
				const auto& pUnitsAllowed = BuildingTypeExt::ExtMap.Find(ProximityTemp::BuildType)->Adjacent_AllowedUnit;

				if (pUnitsAllowed.size() > 0 && !pUnitsAllowed.Contains(pCellUnit->Type))
					continue;

				const auto& pUnitsDisallowed = BuildingTypeExt::ExtMap.Find(ProximityTemp::BuildType)->Adjacent_DisallowedUnit;

				if (pUnitsDisallowed.size() > 0 && pUnitsDisallowed.Contains(pCellUnit->Type))
					continue;

				return CanBuild;
			}
		}
	}

	R->EAX(pCell->GetBuilding());
	return ContinueCheck;
}

// BaseNormal extra checking Hook #1-3 -> sub_4A8EB0 - Check allowed building
DEFINE_HOOK(0x4A8FD7, DisplayClass_BuildingProximityCheck_BuildArea, 0x6)
{
	enum { SkipBuilding = 0x4A902C };

	GET(BuildingClass*, pCellBuilding, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pCellBuilding->Type);

	if (pTypeExt->NoBuildAreaOnBuildup && pCellBuilding->CurrentMission == Mission::Construction)
		return SkipBuilding;

	auto const& pBuildingsAllowed = BuildingTypeExt::ExtMap.Find(ProximityTemp::BuildType)->Adjacent_Allowed;

	if (pBuildingsAllowed.size() > 0 && !pBuildingsAllowed.Contains(pCellBuilding->Type))
		return SkipBuilding;

	auto const& pBuildingsDisallowed = BuildingTypeExt::ExtMap.Find(ProximityTemp::BuildType)->Adjacent_Disallowed;

	if (pBuildingsDisallowed.size() > 0 && pBuildingsDisallowed.Contains(pCellBuilding->Type))
		return SkipBuilding;

	return 0;
}

// BaseNormal extra checking Hook #1-4 -> sub_4A8EB0 - Break loop or record cell for drawing
DEFINE_HOOK(0x4A902C, MapClass_PassesProximityCheck_BaseNormalExtra, 0x5)
{
	enum { CheckCompleted = 0x4A904E };

	REF_STACK(bool, canBuild, STACK_OFFSET(0x30, 0xC));

	if (canBuild)
	{
		if (!RulesExt::Global()->PlacementGrid_Expand)
			return CheckCompleted;

		canBuild = false;
		ProximityTemp::Build = true;
		ScenarioExt::Global()->BaseNormalCells.push_back(ProximityTemp::CurrentCell->MapCoords);
	}

	return 0;
}

// BaseNormal extra checking Hook #1-5 -> sub_4A8EB0 - Restore the correct result
DEFINE_HOOK(0x4A904E, MapClass_PassesProximityCheck_RestoreResult, 0x5)
{
	if (RulesExt::Global()->PlacementGrid_Expand)
		R->Stack<bool>(STACK_OFFSET(0x30, 0xC), ProximityTemp::Build);

	return 0;
}

// BaseNormal for units Hook #2-1 -> sub_4AAC10 - Let the game do the PassesProximityCheck when the cell which mouse is pointing at has not changed
DEFINE_HOOK(0x4AACD9, MapClass_TacticalAction_BaseNormalRecheck, 0x5)
{
	return (RulesExt::Global()->CheckUnitBaseNormal && !(Unsorted::CurrentFrame % 8)) ? 0x4AACF5 : 0;
}

// BaseNormal for units Hook #2-2 -> sub_4A91B0 - Let the game do the PassesProximityCheck when the cell which mouse is pointing at has not changed
DEFINE_HOOK(0x4A9361, MapClass_CallBuildingPlaceCheck_BaseNormalRecheck, 0x5)
{
	return (RulesExt::Global()->CheckUnitBaseNormal && !(Unsorted::CurrentFrame % 8)) ? 0x4A9371 : 0;
}

// Buildable-upon TechnoTypes Hook #2-1 -> sub_47EC90 - Record cell before draw it then skip vanilla AltFlags check
DEFINE_HOOK(0x47EEBC, CellClass_DrawPlaceGrid_RecordCell, 0x6)
{
	enum { DontDrawAlt = 0x47EF1A, DrawVanillaAlt = 0x47EED6 };

	GET(CellClass* const, pCell, ESI);
	GET(const bool, zero, EDX);

	const BlitterFlags flags = BlitterFlags::Centered | BlitterFlags::bf_400;
	ProximityTemp::CurrentCell = pCell;

	R->EDX<BlitterFlags>(flags | (zero ? BlitterFlags::Zero : BlitterFlags::Nonzero));
	return (pCell->AltFlags & AltCellFlags::ContainsBuilding) ? DontDrawAlt : DrawVanillaAlt;
}

// Buildable-upon TechnoTypes Hook #2-2 -> sub_47EC90 - Draw different color grid
DEFINE_HOOK(0x47EF52, CellClass_DrawPlaceGrid_DrawGrids, 0x6)
{
	const auto pRules = RulesExt::Global();

	if (!pRules->PlacementGrid_Expand)
		return 0;

	const auto pCell = ProximityTemp::CurrentCell;
	const auto cell = pCell->MapCoords;
	const auto pObj = DisplayClass::Instance->CurrentBuildingType;
	const auto range = static_cast<short>((pObj && pObj->WhatAmI() == AbstractType::BuildingType) ? static_cast<BuildingTypeClass*>(pObj)->Adjacent + 1 : 0);

	const auto maxX = static_cast<short>(cell.X + range);
	const auto maxY = static_cast<short>(cell.Y + range);
	const auto minX = static_cast<short>(cell.X - range);
	const auto minY = static_cast<short>(cell.Y - range);

	bool green = false;
	const auto& cells = ScenarioExt::Global()->BaseNormalCells;

	for (const auto& baseCell : cells)
	{
		if (baseCell.X >= minX && baseCell.Y >= minY && baseCell.X <= maxX && baseCell.Y <= maxY)
		{
			green = true;
			break;
		}
	}

	const auto foot = ProximityTemp::Exist;
	ProximityTemp::Exist = false;
	const bool land = pCell->LandType != LandType::Water;
	const auto frames = land ? pRules->PlacementGrid_LandFrames.Get() : pRules->PlacementGrid_WaterFrames.Get();
	auto frame = foot ? frames.X : (green ? frames.Z : frames.Y);

	if (frame >= Make_Global<SHPStruct*>(0x8A03FC)->Frames) // place.shp
		frame = 0;

	R->EDI(frame);
	return 0;
}
