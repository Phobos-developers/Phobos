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

// BaseNormal extra checking Hook #1-2 -> sub_4A8EB0 - Check allowed building
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

// BaseNormal extra checking Hook #1-3 -> sub_4A8EB0 - Break loop or record cell for drawing
DEFINE_HOOK(0x4A902C, MapClass_PassesProximityCheck_BaseNormalExtra, 0x5)
{
	enum { CheckCompleted = 0x4A904E };

	REF_STACK(bool, canBuild, STACK_OFFSET(0x30, 0xC));

	if (canBuild)
	{
		if (!RulesExt::Global()->PlacementGrid_Expand)
			return CheckCompleted;

		canBuild = false; // Reset to false so that cells with BaseNormal can be correctly identified
		ProximityTemp::Build = true;

		GET_STACK(const CellStruct, currentCell, STACK_OFFSET(0x30, -0x20));
		ScenarioExt::Global()->BaseNormalCells.push_back(currentCell);
	}

	return 0;
}

// BaseNormal extra checking Hook #1-4 -> sub_4A8EB0 - Restore the correct result
DEFINE_HOOK(0x4A904E, MapClass_PassesProximityCheck_RestoreResult, 0x5)
{
	GET_STACK(bool, canBuild, STACK_OFFSET(0x30, 0xC));
	GET_STACK(const int, idxHouse, STACK_OFFSET(0x30, 0x8));

	const bool gridExpand = RulesExt::Global()->PlacementGrid_Expand;

	if (RulesExt::Global()->CheckExtraBaseNormal && (!canBuild || gridExpand))
	{
		const auto& baseNormalTechnos = ScenarioExt::Global()->BaseNormalTechnos;

		if (baseNormalTechnos.size())
		{
			GET(const int, topLeftX, EBP);
			GET_STACK(const int, foundationWidth, STACK_OFFSET(0x30, -0x1C));
			GET_STACK(const int, topLeftY, STACK_OFFSET(0x30, -0xC));
			GET_STACK(const int, foundationHeight, STACK_OFFSET(0x30, 0x4));

			const auto pBuildType = ProximityTemp::BuildType;
			const auto pBuildTypeExt = BuildingTypeExt::ExtMap.Find(ProximityTemp::BuildType);
			const auto range = pBuildType->Adjacent + 1;
			const auto maxX = topLeftX + range + foundationWidth;
			const auto maxY = topLeftY + range + foundationHeight;
			const auto minX = topLeftX - range;
			const auto minY = topLeftY - range;

			for (const auto& pExt : baseNormalTechnos)
			{
				const auto pTechno = pExt->OwnerObject();

				if (!TechnoExt::IsActive(pTechno))
					continue;

				const auto technoMapCell = pTechno->GetMapCoords();

				if (technoMapCell.X < minX || technoMapCell.Y < minY || technoMapCell.X >= maxX || technoMapCell.Y >= maxY)
					continue;

				const auto pTypeExt = pExt->TypeExtData;
				auto canBeBaseNormal = [&]()
				{
					const auto pOwner = pTechno->Owner;

					if (pOwner->ArrayIndex == idxHouse)
						return pTypeExt->ExtraBaseNormal.Get();
					else if (RulesClass::Instance->BuildOffAlly && pOwner->IsAlliedWith(HouseClass::Array->Items[idxHouse]))
						return pTypeExt->ExtraBaseForAllyBuilding.Get();

					return false;
				};

				if (!canBeBaseNormal())
					continue;

				const auto& pExtraAllowed = pBuildTypeExt->Adjacent_AllowedExtra;

				if (pExtraAllowed.size() > 0 && !pExtraAllowed.Contains(pTypeExt->OwnerObject()))
					continue;

				const auto& pExtraDisallowed = pBuildTypeExt->Adjacent_DisallowedExtra;

				if (pExtraDisallowed.size() > 0 && pExtraDisallowed.Contains(pTypeExt->OwnerObject()))
					continue;

				if (gridExpand)
				{
					ProximityTemp::Build = true;
					ScenarioExt::Global()->BaseNormalCells.push_back(technoMapCell);
				}
				else
				{
					R->Stack<bool>(STACK_OFFSET(0x30, 0xC), true);
					break;
				}
			}
		}
	}

	if (gridExpand)
		R->Stack<bool>(STACK_OFFSET(0x30, 0xC), ProximityTemp::Build);

	return 0;
}

// BaseNormal for units Hook #2-1 -> sub_4AAC10 - Let the game do the PassesProximityCheck when the cell which mouse is pointing at has not changed
DEFINE_HOOK(0x4AACD9, MapClass_TacticalAction_BaseNormalRecheck, 0x5)
{
	return (RulesExt::Global()->CheckExtraBaseNormal && !(Unsorted::CurrentFrame % 8)) ? 0x4AACF5 : 0;
}

// BaseNormal for units Hook #2-2 -> sub_4A91B0 - Let the game do the PassesProximityCheck when the cell which mouse is pointing at has not changed
DEFINE_HOOK(0x4A9361, MapClass_CallBuildingPlaceCheck_BaseNormalRecheck, 0x5)
{
	return (RulesExt::Global()->CheckExtraBaseNormal && !(Unsorted::CurrentFrame % 8)) ? 0x4A9371 : 0;
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

	// Brute force
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
