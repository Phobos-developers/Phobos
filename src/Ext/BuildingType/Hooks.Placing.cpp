#include "Body.h"

#include <EventClass.h>
#include <TerrainClass.h>
#include <AircraftClass.h>
#include <TacticalClass.h>
#include <IsometricTileTypeClass.h>

#include <Ext/Scenario/Body.h>
#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/TerrainType/Body.h>

// Buildable-upon TerrainTypes Hook #2 -> sub_6D5730 - Draw laser fence placement even if they are on the way.
DEFINE_HOOK(0x6D57C1, TacticalClass_DrawLaserFencePlacement_BuildableTerrain, 0x9)
{
	enum { ContinueChecks = 0x6D57D2, DontDraw = 0x6D59A6 };

	GET(CellClass*, pCell, ESI);

	if (auto const pTerrain = pCell->GetTerrain(false))
		return TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn ? ContinueChecks : DontDraw;

	return ContinueChecks;
}

// Buildable-upon TerrainTypes Hook #3 -> sub_5683C0 - Remove them when buildings are placed on them.
// Buildable-upon TechnoTypes Hook #7 -> sub_5683C0 - Remove some of them when buildings are placed on them.
DEFINE_HOOK(0x5684B1, MapClass_PlaceDown_BuildableUponTypes, 0x6)
{
	GET(ObjectClass*, pPlaceObject, EDI);
	GET(CellClass*, pCell, EAX);

	if (pPlaceObject->WhatAmI() == AbstractType::Building)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (const auto pTechno = abstract_cast<TechnoClass*, true>(pObject))
			{
				if (TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType())->CanBeBuiltOn)
				{
					pTechno->KillPassengers(nullptr);
					pTechno->Stun();
					pTechno->Limbo();
					pTechno->UnInit();
				}
			}
			else if (const auto pTerrain = abstract_cast<TerrainClass*, true>(pObject))
			{
				if (TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn)
				{
					pCell->RemoveContent(pTerrain, false);
					TerrainTypeExt::Remove(pTerrain);
				}
			}
		}
	}

	return 0;
}

// Buildable-upon TerrainTypes Hook #4 -> sub_5FD270 - Allow placing buildings on top of them
DEFINE_HOOK(0x5FD2B6, OverlayClass_Unlimbo_SkipTerrainCheck, 0x9)
{
	enum { Unlimbo = 0x5FD2CA, NoUnlimbo = 0x5FD2C3 };

	GET(CellClass* const, pCell, EAX);

	if (!Game::IsActive)
		return Unlimbo;

	if (auto const pTerrain = pCell->GetTerrain(false))
	{
		if (!TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn)
			return NoUnlimbo;

		pCell->RemoveContent(pTerrain, false);
		TerrainTypeExt::Remove(pTerrain);
	}

	return Unlimbo;
}

// Buildable Proximity Helper
namespace ProximityTemp
{
	bool Exist = false;
	bool Mouse = false;
	CellClass* CurrentCell = nullptr;
	BuildingTypeClass* BuildType = nullptr;
}

// BaseNormal extra checking Hook #1-1 -> sub_4A8EB0 - Set context and clear up data
DEFINE_HOOK(0x4A8F20, DisplayClass_BuildingProximityCheck_SetContext, 0x5)
{
	GET(BuildingTypeClass*, pType, ESI);

	ProximityTemp::BuildType = pType;

	return 0;
}

// BaseNormal extra checking Hook #1-2 -> sub_4A8EB0 - Check allowed building
DEFINE_HOOK(0x4A8FD7, DisplayClass_BuildingProximityCheck_BuildArea, 0x6)
{
	enum { SkipBuilding = 0x4A902C };

	GET(BuildingClass*, pCellBuilding, ESI);

	if (BuildingTypeExt::ExtMap.Find(pCellBuilding->Type)->NoBuildAreaOnBuildup && pCellBuilding->CurrentMission == Mission::Construction)
		return SkipBuilding;

	auto const& pBuildingsAllowed = BuildingTypeExt::ExtMap.Find(ProximityTemp::BuildType)->Adjacent_Allowed;

	if (pBuildingsAllowed.size() > 0 && !pBuildingsAllowed.Contains(pCellBuilding->Type))
		return SkipBuilding;

	auto const& pBuildingsDisallowed = BuildingTypeExt::ExtMap.Find(ProximityTemp::BuildType)->Adjacent_Disallowed;

	if (pBuildingsDisallowed.size() > 0 && pBuildingsDisallowed.Contains(pCellBuilding->Type))
		return SkipBuilding;

	return 0;
}

// Let the game do the proximity and shroud check when the cell which mouse is pointing at has not changed
DEFINE_JUMP(LJMP, 0x4AACD9, 0x4AACF5);
DEFINE_JUMP(LJMP, 0x4A9361, 0x4A9371);

static inline bool IsSameFenceType(const BuildingTypeClass* const pPostType, const BuildingTypeClass* const pFenceType)
{
	if (const auto pSpecificType = BuildingTypeExt::ExtMap.Find(pPostType)->LaserFencePost_Fence.Get())
	{
		if (pSpecificType != pFenceType)
			return false;
	}
	else
	{
		const auto count = BuildingTypeClass::Array.Count;

		for (int i = 0; i < count; ++i)
		{
			const auto pSearchType = BuildingTypeClass::Array.Items[i];

			if (pSearchType->LaserFence)
			{
				if (pSearchType != pFenceType)
					return false;

				break;
			}
		}
	}

	return true;
}

static inline bool CheckCanNotExistHere(FootClass* const pTechno, HouseClass* const pOwner,
	bool expand, bool& skipFlag, bool& builtOnCanBeBuiltOn, bool& landFootOnly, bool canBuildUnderUnits)
{
	if (pTechno == TechnoExt::Deployer)
	{
		skipFlag = true;
		return false;
	}

	const auto pTechnoType = pTechno->GetTechnoType();

	if (canBuildUnderUnits || TechnoTypeExt::ExtMap.Find(pTechnoType)->CanBeBuiltOn)
		builtOnCanBeBuiltOn = true;
	else if (!expand || pTechnoType->Speed <= 0 || !BuildingTypeExt::CheckOccupierCanLeave(pOwner, pTechno->Owner))
		return true;
	else
		landFootOnly = true;

	return false;
}

// Buildable-upon TerrainTypes Hook #1 -> sub_47C620 - Allow placing buildings on top of them
// Buildable-upon TechnoTypes Hook #1 -> sub_47C620 - Rewrite and check whether allow placing buildings on top of them
// Customized Laser Fence Hook #1 -> sub_47C620 - Forbid placing laser fence post on inappropriate laser fence
// Fix DeploysInto Desync core Hook -> sub_47C620 - Exclude the specific unit who want to deploy
DEFINE_HOOK(0x47C640, CellClass_CanThisExistHere_IgnoreSomething, 0x6)
{
	enum { CanNotExistHere = 0x47C6D1, CanExistHere = 0x47C6A0 };

	GET(const CellClass* const, pCell, EDI);
	GET(const BuildingTypeClass* const, pBuildingType, EAX);
	GET_STACK(HouseClass* const, pOwner, STACK_OFFSET(0x18, 0xC));

	ProximityTemp::Exist = false;

	if (!Game::IsActive)
		return CanExistHere;

	const bool expand = RulesExt::Global()->ExtendedBuildingPlacing.Get();
	const bool canBuildUnderUnits = BuildingTypeExt::ExtMap.Find(pBuildingType)->CanBuildUnderUnits.Get();
	bool landFootOnly = false;

	if (pBuildingType->LaserFence)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			switch (pObject->WhatAmI())
			{
				case AbstractType::Building:
				{
					if (!TechnoTypeExt::ExtMap.Find(static_cast<BuildingClass*>(pObject)->Type)->CanBeBuiltOn)
						return CanNotExistHere;

					break;
				}

				case AbstractType::Terrain:
				{
					if (!TerrainTypeExt::ExtMap.Find(static_cast<TerrainClass*>(pObject)->Type)->CanBeBuiltOn)
						return CanNotExistHere;

					break;
				}

				default:
				{
					break;
				}
			}
		}
	}
	else if (pBuildingType->LaserFencePost || pBuildingType->Gate)
	{
		bool skipFlag = TechnoExt::Deployer ? TechnoExt::Deployer->CurrentMapCoords == pCell->MapCoords : false;
		bool builtOnCanBeBuiltOn = false;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			switch (pObject->WhatAmI())
			{
				case AbstractType::Aircraft:
				{
					if (!canBuildUnderUnits && !TechnoTypeExt::ExtMap.Find(static_cast<AircraftClass*>(pObject)->Type)->CanBeBuiltOn)
						return CanNotExistHere;

					builtOnCanBeBuiltOn = true;
					break;
				}

				case AbstractType::Building:
				{
					const auto pBuilding = static_cast<BuildingClass*>(pObject);
					const auto pType = pBuilding->Type;

					if (TechnoTypeExt::ExtMap.Find(pType)->CanBeBuiltOn)
						builtOnCanBeBuiltOn = true;
					else if (pOwner != pBuilding->Owner || !pType->LaserFence)
						return CanNotExistHere;
					else if (pBuildingType->LaserFencePost && !IsSameFenceType(pBuildingType, pType))
						return CanNotExistHere;

					break;
				}

				case AbstractType::Infantry:
				case AbstractType::Unit:
				{
					if (CheckCanNotExistHere(static_cast<FootClass*>(pObject), pOwner, expand, skipFlag, builtOnCanBeBuiltOn, landFootOnly, canBuildUnderUnits))
						return CanNotExistHere;

					break;
				}

				case AbstractType::Terrain:
				{
					if (!TerrainTypeExt::ExtMap.Find(static_cast<TerrainClass*>(pObject)->Type)->CanBeBuiltOn)
						return CanNotExistHere;

					builtOnCanBeBuiltOn = true;
					break;
				}

				default:
				{
					break;
				}
			}
		}

		if (!landFootOnly && !builtOnCanBeBuiltOn && (pCell->OccupationFlags & (skipFlag ? 0x1F : 0x3F)))
		{
			if (expand)
				landFootOnly = true;
			else
				return CanNotExistHere;
		}
	}
	else if (pBuildingType->ToTile)
	{
		const auto isoTileTypeIndex = pCell->IsoTileTypeIndex;

		if (isoTileTypeIndex >= 0 && isoTileTypeIndex < IsometricTileTypeClass::Array.Count && !IsometricTileTypeClass::Array.Items[isoTileTypeIndex]->Morphable)
			return CanNotExistHere;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pObject))
			{
				if (!TechnoTypeExt::ExtMap.Find(pBuilding->Type)->CanBeBuiltOn)
					return CanNotExistHere;
			}
		}
	}
	else
	{
		bool skipFlag = TechnoExt::Deployer ? TechnoExt::Deployer->CurrentMapCoords == pCell->MapCoords : false;
		bool builtOnCanBeBuiltOn = false;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			switch (pObject->WhatAmI())
			{
				case AbstractType::Aircraft:
				{
					if (canBuildUnderUnits)
					{
						builtOnCanBeBuiltOn = true;
						break;
					}

					// No break
				}
				case AbstractType::Building:
				{
					if (!TechnoTypeExt::ExtMap.Find(pObject->GetTechnoType())->CanBeBuiltOn)
						return CanNotExistHere;

					builtOnCanBeBuiltOn = true;
					break;
				}

				case AbstractType::Infantry:
				case AbstractType::Unit:
				{
					if (CheckCanNotExistHere(static_cast<FootClass*>(pObject), pOwner, expand, skipFlag, builtOnCanBeBuiltOn, landFootOnly, canBuildUnderUnits))
						return CanNotExistHere;

					break;
				}

				case AbstractType::Terrain:
				{
					if (!TerrainTypeExt::ExtMap.Find(static_cast<TerrainClass*>(pObject)->Type)->CanBeBuiltOn)
						return CanNotExistHere;

					builtOnCanBeBuiltOn = true;
					break;
				}

				default:
				{
					break;
				}
			}
		}

		if (!landFootOnly && !builtOnCanBeBuiltOn && (pCell->OccupationFlags & (skipFlag ? 0x1F : 0x3F)))
		{
			if (expand)
				landFootOnly = true;
			else
				return CanNotExistHere;
		}
	}

	if (landFootOnly)
		ProximityTemp::Exist = true;

	return CanExistHere; // Continue check the overlays .etc
}

// Buildable-upon TechnoTypes Hook #2-1 -> sub_47EC90 - Record cell before draw it then skip vanilla AltFlags check
DEFINE_HOOK(0x47EEBC, CellClass_DrawPlaceGrid_RecordCell, 0x6)
{
	enum { DontDrawAlt = 0x47EF1A, DrawVanillaAlt = 0x47EED6 };

	GET(CellClass* const, pCell, ESI);
	GET(const bool, zero, EDX);

	const BlitterFlags flags = BlitterFlags::Centered | BlitterFlags::bf_400;
	ProximityTemp::CurrentCell = pCell;

	if (!(pCell->AltFlags & AltCellFlags::ContainsBuilding))
	{
		if (!RulesExt::Global()->ExtendedBuildingPlacing)
		{
			R->EDX<BlitterFlags>(flags | (zero ? BlitterFlags::Zero : BlitterFlags::Nonzero));
			return DrawVanillaAlt;
		}
		else if (BuildingTypeClass* const pType = abstract_cast<BuildingTypeClass*>(DisplayClass::Instance.CurrentBuildingTypeCopy))
		{
			R->Stack<bool>(STACK_OFFSET(0x30, -0x1D), pCell->CanThisExistHere(pType->SpeedType, pType, HouseClass::CurrentPlayer));
			R->EDX<BlitterFlags>(flags | BlitterFlags::TransLucent75);
			return DontDrawAlt;
		}
	}

	R->EDX<BlitterFlags>(flags | (zero ? BlitterFlags::Zero : BlitterFlags::Nonzero));
	return DontDrawAlt;
}

static inline void ClearPlacingBuildingData(PlacingBuildingStruct* const pPlace)
{
	pPlace->Type = nullptr;
	pPlace->DrawType = nullptr;
	pPlace->Times = 0;
	pPlace->Timer.Stop();
	pPlace->TopLeft = CellStruct::Empty;
	pPlace->PlaceType = 0;
}

static inline void ClearCurrentBuildingData(DisplayClass* const pDisplay)
{
	pDisplay->SetActiveFoundation(nullptr);
	pDisplay->CurrentBuilding = nullptr;
	pDisplay->CurrentBuildingType = nullptr;
	pDisplay->CurrentBuildingOwnerArrayIndexCopy = -1;

	if (!Unsorted::ArmageddonMode)
	{
		reinterpret_cast<void(__thiscall*)(DisplayClass*, CellStruct*)>(0x4A8D50)(pDisplay, nullptr); // Clear CurrentFoundationCopy_Data
		pDisplay->CurrentBuildingCopy = nullptr;
		pDisplay->CurrentBuildingTypeCopy = nullptr;
	}
}

template <bool slam = false>
static inline void PlayConstructionYardAnim(BuildingClass* const pFactory)
{
	const auto pFactoryType = pFactory->Type;

	if (pFactoryType->ConstructionYard)
	{
		if constexpr (slam)
			VocClass::PlayGlobal(RulesClass::Instance->BuildingSlam, 0x2000, 1.0);

		pFactory->DestroyNthAnim(BuildingAnimSlot::PreProduction);
		pFactory->DestroyNthAnim(BuildingAnimSlot::Idle);

		const bool damaged = pFactory->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
		const auto pAnimName = damaged ? pFactoryType->BuildingAnim[8].Damaged : pFactoryType->BuildingAnim[8].Anim;

		if (pAnimName && *pAnimName)
			pFactory->PlayAnim(pAnimName, BuildingAnimSlot::Production, damaged, false);
	}
}

static inline bool CheckBuildingFoundation(BuildingTypeClass* const pBuildingType, const CellStruct topLeftCell, HouseClass* const pHouse, bool& noOccupy)
{
	for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
	{
		if (const auto pCell = MapClass::Instance.TryGetCellAt(topLeftCell + *pFoundation))
		{
			if (!pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pHouse))
				return false;
			else if (ProximityTemp::Exist)
				noOccupy = false;
		}
	}

	ProximityTemp::Exist = false;
	return true;
}

// Place Another Type Helper
namespace PlaceTypeTemp
{
	size_t PlaceType = 0;
}

// Place Another Type Hook #1 -> sub_4FB0E0 - Replace the factory product
// Buildable-upon TechnoTypes Hook #3 -> sub_4FB0E0 - Hang up place event if there is only infantries and units on the cell
DEFINE_HOOK(0x4FB1EA, HouseClass_UnitFromFactory_HangUpPlaceEvent, 0x5)
{
	enum { CanBuild = 0x4FB23C, TemporarilyCanNotBuild = 0x4FB5BA, CanNotBuild = 0x4FB35F, BuildSucceeded = 0x4FB649 };

	GET(HouseClass* const, pHouse, EBP);
	GET(TechnoClass* const, pTechno, ESI);
	GET(FactoryClass* const, pPrimary, EBX);
	GET(BuildingClass* const, pFactory, EDI);
	GET_STACK(const CellStruct, topLeftCell, STACK_OFFSET(0x3C, 0x10));

	if (pTechno->WhatAmI() != AbstractType::Building)
	{
		pFactory->SendCommand(RadioCommand::RequestLink, pTechno);

		if (pTechno->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
			return CanBuild;

		ProximityTemp::Mouse = true;
		return CanNotBuild;
	}

	const auto pDisplay = &DisplayClass::Instance;
	auto pBuilding = static_cast<BuildingClass*>(pTechno);
	auto pBuildingType = pBuilding->Type;
	const auto pBufferBuilding = pBuilding;
	const auto pBufferType = pBuildingType;
	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType);

	if (pTypeExt->LimboBuild)
	{
		BuildingTypeExt::CreateLimboBuilding(pBuilding, pBuildingType, pHouse, pTypeExt->LimboBuildID);

		if (pDisplay->CurrentBuilding == pBuilding && HouseClass::CurrentPlayer == pHouse)
			ClearCurrentBuildingData(pDisplay);

		PlayConstructionYardAnim<true>(pFactory);
		return BuildSucceeded;
	}

	const bool upgrade = pBuildingType->PowersUpBuilding[0];
	const bool expand = RulesExt::Global()->ExtendedBuildingPlacing && !pBuildingType->PlaceAnywhere && !upgrade;
	const size_t placeType = PlaceTypeTemp::PlaceType;

	if (pTypeExt->PlaceBuilding_Extra)
	{
		if (const auto pSelectType = pTypeExt->GetAnotherPlacingType(((placeType >> 1u) & 0x1Fu), (placeType & 1u)))
			pBuildingType = pSelectType;
	}

	bool revert = upgrade;

	if (expand)
	{
		bool noOccupy = true;
		bool canBuild = CheckBuildingFoundation(pBuildingType, topLeftCell, pHouse, noOccupy);
		const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
		auto& place = pBufferType->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;

		do
		{
			if (canBuild)
			{
				if (noOccupy)
					break; // Can Build

				do
				{
					if (topLeftCell != place.TopLeft || pBufferType != place.Type) // New command
					{
						place.Type = pBufferType;
						place.DrawType = pBuildingType;
						place.Times = 30;
						place.TopLeft = topLeftCell;
						place.PlaceType = placeType;
					}
					else if (place.Times <= 0)
					{
						break; // Time out
					}

					if (!(place.Times % 5) && BuildingTypeExt::CleanUpBuildingSpace(pBuildingType, topLeftCell, pHouse))
						break; // No place for cleaning

					if (pHouse == HouseClass::CurrentPlayer && place.Times == 30)
						ClearCurrentBuildingData(pDisplay);

					--place.Times;
					place.Timer.Start(8);

					return TemporarilyCanNotBuild;
				}
				while (false);
			}

			revert = place.Times == 30 || !place.Type;
			ClearPlacingBuildingData(&place);

			if (revert)
				ProximityTemp::Mouse = true;

			return CanNotBuild;
		}
		while (false);

		revert = !place.Type;
		ClearPlacingBuildingData(&place);
	}

	if (pBufferType != pBuildingType)
		pBuilding = static_cast<BuildingClass*>(pBuildingType->CreateObject(pHouse));

	pFactory->SendCommand(RadioCommand::RequestLink, pBuilding);

	if (pBuilding->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
	{
		if (pBufferBuilding != pBuilding)
		{
			if (HouseClass::CurrentPlayer == pHouse)
			{
				if (!pDisplay->CurrentBuilding)
				{
					Phobos::Config::CurrentPlacingDirection = Phobos::Config::DefaultPlacingDirection;
				}
				else if (pDisplay->CurrentBuilding == pBufferBuilding)
				{
					pDisplay->CurrentBuilding = pBuilding;

					Phobos::Config::CurrentPlacingDirection = Phobos::Config::DefaultPlacingDirection;
				}

				if (pDisplay->CurrentBuildingType == pBufferType)
					pDisplay->CurrentBuildingType = pBuildingType;

				if (pDisplay->CurrentBuildingCopy == pBufferBuilding)
					pDisplay->CurrentBuildingCopy = pBuilding;

				if (pDisplay->CurrentBuildingTypeCopy == pBufferType)
					pDisplay->CurrentBuildingTypeCopy = pBuildingType;

				if (Make_Global<BuildingClass*>(0xB0FE5C) == pBufferBuilding) // Q
					Make_Global<BuildingClass*>(0xB0FE5C) = pBuilding;

				if (Make_Global<BuildingClass*>(0xB0FE60) == pBufferBuilding) // W
					Make_Global<BuildingClass*>(0xB0FE60) = pBuilding;
			}

			pBufferBuilding->UnInit();
			pPrimary->Object = pBuilding;
			R->ESI(pBuilding);
		}
		else if (HouseClass::CurrentPlayer == pHouse
			&& (!pDisplay->CurrentBuilding
				|| pDisplay->CurrentBuilding == pBuilding))
		{
			Phobos::Config::CurrentPlacingDirection = Phobos::Config::DefaultPlacingDirection;
		}

		return CanBuild;
	}

	if (pBufferBuilding != pBuilding)
		pBuilding->UnInit();

	if (revert)
		ProximityTemp::Mouse = true;

	return CanNotBuild;
}

// Place Another Type Hook #2 -> sub_4A91B0 - Replace current building type for check
DEFINE_HOOK(0x4A937D, DisplayClass_CallBuildingPlaceCheck_ReplaceBuildingType, 0x8)
{
	enum { SkipGameCode = 0x4A943A };

	GET(const CellStruct, cell, EBP);

	const auto pDisplay = &DisplayClass::Instance;

	auto updateCurrentFoundation = [pDisplay, cell]()
		{
			if (pDisplay->CurrentFoundation_CenterCell != cell)
			{
				auto oldCell = pDisplay->CurrentFoundation_CenterCell;

				if (oldCell != CellStruct::Empty)
				{
					oldCell += pDisplay->CurrentFoundation_TopLeftOffset;
					pDisplay->MarkFoundation(&oldCell, false);
				}

				auto newCell = cell;

				if (newCell != CellStruct::Empty)
				{
					newCell += pDisplay->CurrentFoundation_TopLeftOffset;
					pDisplay->MarkFoundation(&newCell, true);
				}

				pDisplay->CurrentFoundation_CenterCell = cell;
			}
		};

	const auto pCurrentBuilding = abstract_cast<BuildingClass*>(pDisplay->CurrentBuilding);
	const auto pTypeExt = pCurrentBuilding ? BuildingTypeExt::ExtMap.Find(pCurrentBuilding->Type) : nullptr;

	if (pTypeExt && pTypeExt->PlaceBuilding_Extra)
	{
		if (!ScrollClass::Instance.unknown_byte_554A) // 555A: AnyMouseButtonDown
			updateCurrentFoundation();
		else // bp
			R->EBP(pDisplay->CurrentFoundation_CenterCell.X);

		const auto delta = cell - pDisplay->CurrentFoundation_CenterCell;

		if (delta.Y || delta.X)
			Phobos::Config::CurrentPlacingDirection = DirStruct(Math::atan2(-delta.Y, delta.X)).GetFacing<32>();

		auto getAnotherPlacingType = [pDisplay, pTypeExt]() -> BuildingTypeClass*
			{
				if (pTypeExt)
				{
					const auto pCenterCell = MapClass::Instance.GetCellAt(pDisplay->CurrentFoundation_CenterCell);
					const bool onWater = pCenterCell->LandType == LandType::Water;
					return pTypeExt->GetAnotherPlacingType(Phobos::Config::CurrentPlacingDirection, onWater);
				}

				return nullptr;
			};

		if (const auto pAnotherType = getAnotherPlacingType())
		{
			if (pDisplay->CurrentBuildingType && pDisplay->CurrentBuildingType != pAnotherType)
			{
				pDisplay->CurrentBuildingType = pAnotherType;
				pDisplay->SetActiveFoundation(pAnotherType->GetFoundationData(true));
			}
		}
		else if (pCurrentBuilding)
		{
			if (pDisplay->CurrentBuildingType && pDisplay->CurrentBuildingType != pCurrentBuilding->Type)
			{
				pDisplay->CurrentBuildingType = pCurrentBuilding->Type;
				pDisplay->SetActiveFoundation(pCurrentBuilding->Type->GetFoundationData(true));
			}
		}
	}
	else
	{
		updateCurrentFoundation();
	}

	return SkipGameCode;
}

// Place Another Type Hook #3 -> sub_4AB9B0 - Keep current foundation center cell
DEFINE_HOOK(0x4AB9FF, DisplayClass_LeftMouseButtonUp_MaintainCell, 0x6)
{
	enum { ContinueCheckUpgrade = 0x4ABA84, SkipCheckUpgrade = 0x4ABAA4 };

	const auto pCellStruct = &DisplayClass::Instance.CurrentFoundation_CenterCell;

	R->EBX(pCellStruct);

	const auto pType = DisplayClass::Instance.CurrentBuildingType;

	if (pType->WhatAmI() != AbstractType::BuildingType || !static_cast<BuildingTypeClass*>(pType)->PowersUpBuilding[0])
		return SkipCheckUpgrade;

	const auto pCellBuilding = MapClass::Instance.GetCellAt(*pCellStruct)->GetBuilding();

	if (!pCellBuilding)
		return SkipCheckUpgrade;
	else
		R->ESI(pCellBuilding);

	return ContinueCheckUpgrade;
}

// Place Another Type Hook #4 -> sub_4AB9B0 - Replace event
DEFINE_HOOK(0x4ABAC0, DisplayClass_LeftMouseButtonUp_ReplaceBuildingType, 0x6)
{
	enum { SkipGameCode = 0x4ABBB3 };

	const auto pDisplay = &DisplayClass::Instance;

	const auto centerCell = pDisplay->CurrentFoundation_CenterCell;
	const auto placeCell = centerCell + pDisplay->CurrentFoundation_TopLeftOffset;

	const auto pPlace = pDisplay->CurrentBuilding;
	const auto pType = pPlace->GetType(); // Should not use CurrentBuildingType
	const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType);

	int placeType = 0;

	if (pBuildingType && BuildingTypeExt::ExtMap.Find(pBuildingType)->PlaceBuilding_Extra)
	{
		const auto pCenterCell = MapClass::Instance.GetCellAt(centerCell);
		placeType |= pCenterCell->LandType == LandType::Water;
		placeType |= (Phobos::Config::CurrentPlacingDirection << 1);
	}
	else
	{
		const auto pTechnoType = TechnoTypeExt::GetTechnoType(pType);
		placeType |= (pTechnoType && pTechnoType->Naval);
	}

	const int arrayIndex = pType->GetArrayIndex();
	const auto absType = pPlace->WhatAmI();

	const EventClass event (HouseClass::CurrentPlayer->ArrayIndex, EventType::Place, absType, arrayIndex, placeType, placeCell);
	EventClass::AddEvent(event);

	return SkipGameCode;
}

// Place Another Type Hook #5 -> sub_4C6CB0 - Replace placing action
DEFINE_HOOK(0x4C70E1, EventClass_RespondToEvent_SetPlaceType, 0x8)
{
	enum { SkipGameCode = 0x4C7110 };

	GET(EventClass* const, pThis, ESI);
	GET(HouseClass* const, pHouse, EDI);

	const auto cell = pThis->Place.Location;
	const int flags = pThis->Place.IsNaval;
	const bool isNaval = flags & 1;
	const int arrayIndex = pThis->Place.HeapID;
	const auto absType = pThis->Place.RTTIType;

	if (absType == AbstractType::Building || absType == AbstractType::BuildingType)
		PlaceTypeTemp::PlaceType = static_cast<size_t>(flags);

	reinterpret_cast<void(__thiscall*)(HouseClass*, AbstractType, int, bool, const CellStruct*)>
		(0x4FB0E0)(pHouse, absType, arrayIndex, isNaval, &cell); // UnitFromFactory

	PlaceTypeTemp::PlaceType = 0;

	return SkipGameCode;
}

// Buildable-upon TechnoTypes Hook #4-1 -> sub_4FB0E0 - Check whether need to skip the replace command
DEFINE_HOOK(0x4FB395, HouseClass_UnitFromFactory_SkipMouseReturn, 0x6)
{
	enum { SkipGameCode = 0x4FB489, CheckMouseCoords = 0x4FB3E3 };

	if (!RulesExt::Global()->ExtendedBuildingPlacing)
		return 0;

	if (ProximityTemp::Mouse)
	{
		ProximityTemp::Mouse = false;
		return 0;
	}

	R->EBX(0);

	if (!DisplayClass::Instance.CurrentBuildingTypeCopy)
		return SkipGameCode;

	R->ECX(DisplayClass::Instance.CurrentBuildingTypeCopy);
	return CheckMouseCoords;
}

// Buildable-upon TechnoTypes Hook #4-2 -> sub_4FB0E0 - Check whether need to skip the clear command
DEFINE_HOOK(0x4FB339, HouseClass_UnitFromFactory_SkipMouseClear, 0x6)
{
	enum { SkipGameCode = 0x4FB4A0 };

	GET(TechnoClass* const, pTechno, ESI);

	if (RulesExt::Global()->ExtendedBuildingPlacing)
	{
		if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pTechno))
		{
			if (const auto pCurrentType = abstract_cast<BuildingTypeClass*>(DisplayClass::Instance.CurrentBuildingType))
			{
				if (!BuildingTypeExt::IsSameBuildingType(pBuilding->Type, pCurrentType))
					return SkipGameCode;
			}
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #4-3 -> sub_4FB0E0 - Check whether need to skip the clear command
DEFINE_HOOK(0x4FAB83, HouseClass_AbandonProductionOf_SkipMouseClear, 0x7)
{
	enum { SkipGameCode = 0x4FABA4 };

	GET(const int, index, EBX);

	if (RulesExt::Global()->ExtendedBuildingPlacing && index >= 0)
	{
		if (const auto pCurrentBuildingType = abstract_cast<BuildingTypeClass*>(DisplayClass::Instance.CurrentBuildingType))
		{
			if (!BuildingTypeExt::IsSameBuildingType(BuildingTypeClass::Array.Items[index], pCurrentBuildingType))
				return SkipGameCode;
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #5 -> sub_4C9FF0 - Restart timer and clear buffer when abandon building production
DEFINE_HOOK(0x4CA05B, FactoryClass_AbandonProduction_AbandonCurrentBuilding, 0x5)
{
	GET(FactoryClass*, pFactory, ESI);

	if (RulesExt::Global()->ExtendedBuildingPlacing)
	{
		const auto pBuilding = abstract_cast<BuildingClass*>(pFactory->Object);

		if (!pBuilding)
			return 0;

		const auto pHouseExt = HouseExt::ExtMap.Find(pFactory->Owner);
		auto& place = pBuilding->Type->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;
		ClearPlacingBuildingData(&place);
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #6 -> sub_443C60 - Try to clean up the building space when AI is building
DEFINE_HOOK(0x4451F8, BuildingClass_KickOutUnit_CleanUpAIBuildingSpace, 0x6)
{
	enum { CanBuild = 0x4452F0, TemporarilyCanNotBuild = 0x445237, CanNotBuild = 0x4454E6, BuildSucceeded = 0x4454D4, BuildFailed = 0x445696 };

	GET(BaseNodeClass* const, pBaseNode, EBX);
	GET(BuildingClass* const, pBuilding, EDI);
	GET(BuildingClass* const, pFactory, ESI);
	GET(const CellStruct, topLeftCell, EDX);

	const auto pBuildingType = pBuilding->Type;

/*	if (RulesExt::Global()->AIForbidConYard && pBuildingType->ConstructionYard) // TODO If merge #1470
	{
		if (pBaseNode)
		{
			pBaseNode->Placed = true;
			pBaseNode->Attempts = 0;
		}

		return BuildFailed;
	}*/

	const auto pHouse = pFactory->Owner;
	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType);

	if (pTypeExt->LimboBuild)
	{
		BuildingTypeExt::CreateLimboBuilding(pBuilding, pBuildingType, pHouse, pTypeExt->LimboBuildID);

		if (pBaseNode)
		{
			pBaseNode->Placed = true;
			pBaseNode->Attempts = 0;

			if (pHouse->ProducingBuildingTypeIndex == pBuildingType->ArrayIndex)
				pHouse->ProducingBuildingTypeIndex = -1;
		}

		PlayConstructionYardAnim<true>(pFactory);
		return BuildSucceeded;
	}

	if (!RulesExt::Global()->ExtendedBuildingPlacing)
		return 0;

	if (topLeftCell != CellStruct::Empty && !pBuildingType->PlaceAnywhere)
	{
		if (!pBuildingType->PowersUpBuilding[0])
		{
			bool noOccupy = true;
			bool canBuild = CheckBuildingFoundation(pBuildingType, topLeftCell, pHouse, noOccupy);
			const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
			auto& place = pBuildingType->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;

			do
			{
				if (canBuild)
				{
					if (noOccupy)
						break; // Can Build

					do
					{
						if (topLeftCell != place.TopLeft || pBuildingType != place.Type) // New command
						{
							place.Type = pBuildingType;
							place.DrawType = pBuildingType;
							place.TopLeft = topLeftCell;
						}

						if (!place.Timer.HasTimeLeft())
						{
							place.Timer.Start(40);

							if (BuildingTypeExt::CleanUpBuildingSpace(pBuildingType, topLeftCell, pHouse))
								break; // No place for cleaning
						}

						return TemporarilyCanNotBuild;
					}
					while (false);
				}

				ClearPlacingBuildingData(&place);
				return CanNotBuild;
			}
			while (false);

			ClearPlacingBuildingData(&place);
		}
		else
		{
			const auto pCell = MapClass::Instance.GetCellAt(topLeftCell);
			const auto pCellBuilding = pCell->GetBuilding();

			if (!pCellBuilding || !reinterpret_cast<bool(__thiscall*)(BuildingClass*, BuildingTypeClass*, HouseClass*)>(0x452670)(pCellBuilding, pBuildingType, pHouse)) // CanUpgradeBuilding
				return CanNotBuild;
		}
	}

	if (pBuilding->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
	{
		PlayConstructionYardAnim(pFactory);
		return CanBuild;
	}

	return CanNotBuild;
}

static inline bool CanDrawGrid(bool draw)
{
	if (ProximityTemp::Exist)
	{
		ProximityTemp::Exist = false;
		return false;
	}

	return draw;
}

// Laser fence use GetBuilding to check whether can build and draw, so no need to change
// Buildable-upon TechnoTypes Hook #8-1 -> sub_6D5C50 - Don't draw overlay wall grid when have occupiers
DEFINE_HOOK(0x6D5D38, TacticalClass_DrawOverlayWallGrid_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x6D5D40, Invalid = 0x6D5F0F };

	GET(bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #8-2 -> sub_6D59D0 - Don't draw firestorm wall grid when have occupiers
DEFINE_HOOK(0x6D5A9D, TacticalClass_DrawFirestormWallGrid_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x6D5AA5, Invalid = 0x6D5C2F };

	GET(bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #8-3 -> sub_588750 - Don't place overlay wall when have occupiers
DEFINE_HOOK(0x588873, MapClass_BuildingToWall_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x58887B, Invalid = 0x588935 };

	GET(bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #8-4 -> sub_588570 - Don't place firestorm wall when have occupiers
DEFINE_HOOK(0x588664, MapClass_BuildingToFirestormWall_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x58866C, Invalid = 0x588730 };

	GET(bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #9-1 -> sub_7393C0 - Try to clean up the building space when is deploying
DEFINE_HOOK(0x73946C, UnitClass_TryToDeploy_CleanUpDeploySpace, 0x6)
{
	enum { CanDeploy = 0x73958A, TemporarilyCanNotDeploy = 0x73953B, CanNotDeploy = 0x7394E0 };

	GET(UnitClass* const, pUnit, EBP);
	GET(CellStruct, topLeftCell, ESI);

	if (!RulesExt::Global()->ExtendedBuildingPlacing)
		return 0;

	const auto pTechnoExt = TechnoExt::ExtMap.Find(pUnit);
	const auto pBuildingType = pUnit->Type->DeploysInto;
	const auto pHouseExt = HouseExt::ExtMap.Find(pUnit->Owner);
	auto& vec = pHouseExt->OwnedDeployingUnits;

	if (pBuildingType->GetFoundationWidth() > 2 || pBuildingType->GetFoundationHeight(false) > 2)
		topLeftCell -= CellStruct { 1, 1 };

	R->Stack<CellStruct>(STACK_OFFSET(0x28, -0x14), topLeftCell);

	if (!pBuildingType->PlaceAnywhere)
	{
		bool noOccupy = true;
		bool canBuild = CheckBuildingFoundation(pBuildingType, topLeftCell, pUnit->Owner, noOccupy);

		do
		{
			if (canBuild)
			{
				if (noOccupy)
					break; // Can build

				do
				{
					if (pTechnoExt && !pTechnoExt->UnitAutoDeployTimer.InProgress())
					{
						if (BuildingTypeExt::CleanUpBuildingSpace(pBuildingType, topLeftCell, pUnit->Owner, pUnit))
							break; // No place for cleaning

						if (vec.size() == 0 || std::find(vec.begin(), vec.end(), pUnit) == vec.end())
							vec.push_back(pUnit);

						pTechnoExt->UnitAutoDeployTimer.Start(40);
					}

					return TemporarilyCanNotDeploy;
				}
				while (false);
			}

			if (vec.size() > 0)
				vec.erase(std::remove(vec.begin(), vec.end(), pUnit), vec.end());

			if (pTechnoExt)
				pTechnoExt->UnitAutoDeployTimer.Stop();

			return CanNotDeploy;
		}
		while (false);
	}

	if (vec.size() > 0)
		vec.erase(std::remove(vec.begin(), vec.end(), pUnit), vec.end());

	if (pTechnoExt)
		pTechnoExt->UnitAutoDeployTimer.Stop();

	return CanDeploy;
}

// Buildable-upon TechnoTypes Hook #9-2 -> sub_73FD50 - Push the owner house into deploy check
DEFINE_HOOK(0x73FF8F, UnitClass_MouseOverObject_ShowDeployCursor, 0x6)
{
	if (RulesExt::Global()->ExtendedBuildingPlacing) // This IF check is not so necessary
	{
		GET(const UnitClass* const, pUnit, ESI);
		LEA_STACK(HouseClass**, pHousePtr, STACK_OFFSET(0x20, -0x20));
		*pHousePtr = pUnit->Owner;
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #10 -> sub_4C6CB0 - Stop deploy when get stop command
DEFINE_HOOK(0x4C7665, EventClass_RespondToEvent_StopDeployInIdleEvent, 0x6)
{
	if (RulesExt::Global()->ExtendedBuildingPlacing) // This IF check is not so necessary
	{
		GET(const UnitClass* const, pUnit, ESI);

		if (pUnit->Type->DeploysInto)
		{
			const auto mission = pUnit->CurrentMission;

			if (mission == Mission::Guard || mission == Mission::Unload)
			{
				if (const auto pHouseExt = HouseExt::ExtMap.Find(pUnit->Owner))
				{
					auto& vec = pHouseExt->OwnedDeployingUnits;

					if (vec.size() > 0)
						vec.erase(std::remove(vec.begin(), vec.end(), pUnit), vec.end());
				}
			}
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #11 -> sub_4F8440 - Check whether can place again in each house
DEFINE_HOOK(0x4F8DB1, HouseClass_Update_CheckHangUpBuilding, 0x6)
{
	GET(HouseClass* const, pHouse, ESI);

	if (!pHouse->IsControlledByHuman())
		return 0;

	if (!RulesExt::Global()->ExtendedBuildingPlacing)
		return 0;

	const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
	auto buildCurrent = [&pHouse, &pHouseExt](BuildingTypeClass* pType, CellStruct cell, size_t placeType)
		{
			if (!pType)
				return;

			auto currentCanBuild = [&pHouse, &pType]() -> const bool
				{
					auto const bitsOwners = pType->GetOwners();

					for(auto const& pConYard : pHouse->ConYards)
					{
						if (pConYard->InLimbo || !pConYard->HasPower || pConYard->Deactivated)
							continue;

						if (pConYard->CurrentMission == Mission::Selling || pConYard->QueuedMission == Mission::Selling)
							continue;

						const auto pConYardType = pConYard->Type;

						if (pConYardType->Factory != AbstractType::BuildingType || !pConYardType->InOwners(bitsOwners))
							continue;

						return true;
					}

					return false;
				};

			if (!currentCanBuild())
			{
				ClearPlacingBuildingData(pType->BuildCat != BuildCat::Combat ? &pHouseExt->Common : &pHouseExt->Combat);

				if (pHouse == HouseClass::CurrentPlayer)
					VoxClass::Play(GameStrings::EVA_CannotDeployHere);
			}
			else if (pHouse == HouseClass::CurrentPlayer) // Prevent unexpected wrong event
			{
				const int place = static_cast<int>(placeType);
				const auto arrayIndex = pType->GetArrayIndex();
				const EventClass event (pHouse->ArrayIndex, EventType::Place, AbstractType::Building, arrayIndex, place, cell);
				EventClass::AddEvent(event);
			}
		};

	auto& commonPlace = pHouseExt->Common;

	if (commonPlace.Timer.Completed())
	{
		commonPlace.Timer.Stop();
		buildCurrent(commonPlace.Type, commonPlace.TopLeft, commonPlace.PlaceType);
	}

	auto& combatPlace = pHouseExt->Combat;

	if (combatPlace.Timer.Completed())
	{
		combatPlace.Timer.Stop();
		buildCurrent(combatPlace.Type, combatPlace.TopLeft, combatPlace.PlaceType);
	}

	if (pHouseExt->OwnedDeployingUnits.size() > 0)
	{
		auto& vec = pHouseExt->OwnedDeployingUnits;

		for (auto it = vec.begin(); it != vec.end(); )
		{
			const auto pUnit = *it;

			if (!pUnit->InLimbo && pUnit->IsOnMap && !pUnit->IsSinking && pUnit->Owner == pHouse && !pUnit->Destination && pUnit->CurrentMission == Mission::Guard
				&& !pUnit->ParasiteEatingMe && !pUnit->TemporalTargetingMe && pUnit->Type->DeploysInto)
			{
				if (const auto pExt = TechnoExt::ExtMap.Find(pUnit))
				{
					if (!(pExt->UnitAutoDeployTimer.GetTimeLeft() % 8))
						pUnit->QueueMission(Mission::Unload, true);

					++it;
					continue;
				}
			}

			it = vec.erase(it);
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #12 -> sub_6D5030 - Draw the placing building preview
DEFINE_HOOK(0x6D504C, TacticalClass_DrawPlacement_DrawPlacingPreview, 0x6)
{
	if (!RulesExt::Global()->ExtendedBuildingPlacing)
		return 0;

	const auto pPlayer = HouseClass::CurrentPlayer;
	const auto pDisplay = &DisplayClass::Instance;

	auto drawImage = [&pDisplay](BuildingTypeClass* const pType, HouseClass* const pHouse, const CellStruct cell)
	{
		const auto pCell = pDisplay->TryGetCellAt(cell);

		if (!pCell || cell == CellStruct::Empty || pType->PowersUpBuilding[0])
			return;

		auto pImage = pType->LoadBuildup();
		int imageFrame = 0;

		if (pImage)
			imageFrame = ((pImage->Frames / 2) - 1);
		else
			pImage = pType->GetImage();

		if (!pImage)
			return;

		BlitterFlags blitFlags = BlitterFlags::TransLucent75 | BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;
		auto rect = DSurface::Temp->GetRect();
		rect.Height -= 32;
		auto point = TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(cell, (1 + pCell->GetFloorHeight(Point2D::Empty)))).first;
		point.Y -= 15;

		const auto ColorSchemeIndex = pHouse->ColorSchemeIndex;
		const auto Palettes = pType->Palette;
		const auto pColor = Palettes ? Palettes->Items[ColorSchemeIndex] : ColorScheme::Array.Items[ColorSchemeIndex];
		DSurface::Temp->DrawSHP(pColor->LightConvert, pImage, imageFrame, &point, &rect, blitFlags, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	};

	for (const auto& pHouse : HouseClass::Array)
	{
		if (pPlayer->IsObserver() || pHouse->IsAlliedWith(pPlayer))
		{
			const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);

			if (const auto pType = abstract_cast<BuildingTypeClass*>(pDisplay->CurrentBuildingTypeCopy))
				drawImage(pType, pHouse, (pDisplay->CurrentFoundationCopy_TopLeftOffset + pDisplay->CurrentFoundationCopy_CenterCell));

			if (const auto pType = pHouseExt->Common.DrawType)
				drawImage(pType, pHouse, pHouseExt->Common.TopLeft);

			if (const auto pType = pHouseExt->Combat.DrawType)
				drawImage(pType, pHouse, pHouseExt->Combat.TopLeft);

			if (pHouseExt->OwnedDeployingUnits.size() <= 0)
				continue;

			for (const auto& pUnit : pHouseExt->OwnedDeployingUnits)
			{
				const auto pType = pUnit->Type->DeploysInto;

				if (!pType)
					continue;

				auto displayCell = CellClass::Coord2Cell(pUnit->GetCoords()); // pUnit->GetMapCoords();

				if (pType->GetFoundationWidth() > 2 || pType->GetFoundationHeight(false) > 2)
					displayCell -= CellStruct { 1, 1 };

				drawImage(pType, pHouse, displayCell);
			}
		}
	}

	return 0;
}

// Auto Build Hook -> sub_6A8B30 - Auto Build Buildings
DEFINE_HOOK(0x6A8E34, StripClass_Update_AutoBuildBuildings, 0x7)
{
	enum { SkipGameCode = 0x6A8F49 };

	GET(BuildingClass* const, pBuilding, ESI);

	if (BuildingTypeExt::BuildLimboBuilding(pBuilding))
		return SkipGameCode;

	return 0;
}

// Limbo Build Hook -> sub_42EB50 - Check Base Node
DEFINE_HOOK(0x42EB8E, BaseClass_GetBaseNodeIndex_CheckValidBaseNode, 0x6)
{
	enum { Valid = 0x42EBC3, Invalid = 0x42EBAE };

	GET(BaseClass* const, pBase, ESI);
	GET(BaseNodeClass* const, pBaseNode, EAX);

	if (pBaseNode->Placed)
	{
		const auto index = pBaseNode->BuildingTypeIndex;

		if (index >= 0 && index < BuildingTypeClass::Array.Count)
		{
			const auto pType = BuildingTypeClass::Array.Items[index];

//			if ((pType->ConstructionYard && RulesExt::Global()->AIForbidConYard) || BuildingTypeExt::ExtMap.Find(pType)->LimboBuild) // TODO If merge #1470
			if (BuildingTypeExt::ExtMap.Find(pType)->LimboBuild)
				return Invalid;
		}
	}

	return reinterpret_cast<bool(__thiscall*)(HouseClass*, BaseNodeClass*)>(0x50CAD0)(pBase->Owner, pBaseNode) ? Valid : Invalid;
}

// Customized Laser Fence Hook #2 -> sub_453060 - Select the specific laser fence type
DEFINE_HOOK(0x452E2C, BuildingClass_CreateLaserFence_FindSpecificIndex, 0x5)
{
	enum { SkipGameCode = 0x452E50 };

	GET(BuildingClass* const, pThis, EDI);

	if (const auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type))
	{
		if (const auto pFenceType = pExt->LaserFencePost_Fence.Get())
		{
			if (pFenceType->LaserFence)
			{
				R->EBP(pFenceType->ArrayIndex);
				R->EAX(BuildingTypeClass::Array.Count);
				return SkipGameCode;
			}
		}
	}

	return 0;
}

// Customized Laser Fence Hook #3 -> sub_440580 - Skip uninit inappropriate laser fence
DEFINE_HOOK(0x440AE9, BuildingClass_Unlimbo_SkipUninitFence, 0x7)
{
	enum { SkipGameCode = 0x440B07 };

	GET(BuildingClass* const, pFence, EDI);
	GET(BuildingClass* const, pThis, ESI);

	return IsSameFenceType(pThis->Type, pFence->Type) ? 0 : SkipGameCode;
}

static inline bool IsMatchedPostType(const BuildingTypeClass* const pThisType, const BuildingTypeClass* const pPostType)
{
	if (const auto pThisTypeExt = BuildingTypeExt::ExtMap.Find(pThisType))
	{
		if (const auto pPostTypeExt = BuildingTypeExt::ExtMap.Find(pPostType))
		{
			if (pThisTypeExt->LaserFencePost_Fence.Get() != pPostTypeExt->LaserFencePost_Fence.Get())
				return false;
		}
	}

	return true;
}

// Customized Laser Fence Hook #4 -> sub_452BB0 - Only accept specific fence post
DEFINE_HOOK(0x452CB4, BuildingClass_FindLaserFencePost_CheckLaserFencePost, 0x7)
{
	enum { SkipGameCode = 0x452D2C };

	GET(BuildingClass* const, pPost, ESI);
	GET(BuildingClass* const, pThis, EDI);

	return IsMatchedPostType(pThis->Type, pPost->Type) ? 0 : SkipGameCode;
}

// Customized Laser Fence Hook #5 -> sub_6D5730 - Break draw inappropriate laser fence grids
DEFINE_HOOK(0x6D5815, TacticalClass_DrawLaserFenceGrid_SkipDrawLaserFence, 0x6)
{
	enum { SkipGameCode = 0x6D59A6 };

	GET(BuildingTypeClass* const, pPostType, ECX);

	// Have used CurrentBuilding->Type yet, so simply use static_cast
	return IsMatchedPostType(static_cast<BuildingClass*>(DisplayClass::Instance.CurrentBuilding)->Type, pPostType) ? 0 : SkipGameCode;
}
