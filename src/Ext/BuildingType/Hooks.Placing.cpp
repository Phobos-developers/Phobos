#include "Body.h"

#include <EventClass.h>
#include <TerrainClass.h>
#include <AircraftClass.h>
#include <TacticalClass.h>
#include <IsometricTileTypeClass.h>

#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/AresHelper.h>

// Buildable-upon TerrainTypes Hook #2 -> sub_6D5730 - Draw laser fence placement even if they are on the way.
DEFINE_HOOK(0x6D57C1, TacticalClass_DrawLaserFencePlacement_BuildableTerrain, 0x9)
{
	enum { ContinueChecks = 0x6D57D2, DontDraw = 0x6D59A6 };

	GET(CellClass*, pCell, ESI);

	if (auto const pTerrain = pCell->GetTerrain(false))
	{
		auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

		if (pTypeExt->CanBeBuiltOn)
			return ContinueChecks;

		return DontDraw;
	}

	return ContinueChecks;
}

// Buildable-upon TerrainTypes Hook #3 -> sub_5683C0 - Remove them when buildings are placed on them.
// Buildable-upon TechnoTypes Hook #7 -> sub_5683C0 - Remove some of them when buildings are placed on them.
DEFINE_HOOK(0x5684B1, MapClass_PlaceDown_BuildableUponTypes, 0x6)
{
	GET(ObjectClass*, pObject, EDI);
	GET(CellClass*, pCell, EAX);

	if (pObject->WhatAmI() == AbstractType::Building)
	{
		auto pCellObject = pCell->FirstObject;

		while (pCellObject)
		{
			const auto absType = pCellObject->WhatAmI();

			if (absType == AbstractType::Infantry || absType == AbstractType::Unit || absType == AbstractType::Aircraft || absType == AbstractType::Building)
			{
				const auto pTechno = static_cast<TechnoClass*>(pCellObject);
				const auto pType = pTechno->GetTechnoType();
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
				{
					pTechno->KillPassengers(nullptr);
					pTechno->Stun();
					pTechno->Limbo();
					pTechno->UnInit();
				}
			}
			else if (const auto pTerrain = abstract_cast<TerrainClass*>(pCellObject))
			{
				const auto pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
				{
					pCell->RemoveContent(pTerrain, false);
					TerrainTypeExt::Remove(pTerrain);
				}
			}

			pCellObject = pCellObject->NextObject;
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

	auto pCellObject = pCell->FirstObject;

	while (pCellObject)
	{
		if (const auto pTerrain = abstract_cast<TerrainClass*>(pCellObject))
		{
			const auto pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

			if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
				return NoUnlimbo;

			pCell->RemoveContent(pTerrain, false);
			TerrainTypeExt::Remove(pTerrain);
		}

		pCellObject = pCellObject->NextObject;
	}

	return Unlimbo;
}

// BaseNormal for units Hook #1 -> sub_4A8EB0 - Rewrite and add functions in
DEFINE_HOOK(0x4A8F21, MapClass_PassesProximityCheck_BaseNormalExtra, 0x9)
{
	enum { CheckCompleted = 0x4A904E };

	GET(const CellStruct* const, pFoundationTopLeft, EDI);
	GET(const BuildingTypeClass* const, pBuildBldType, ESI);
	GET_STACK(const int, idxHouse, STACK_OFFSET(0x30, 0x8));

	const auto differentColor = RulesExt::Global()->PlacementGrid_Expand.Get();
	bool isInAdjacent = false;
	auto& vec = ScenarioExt::Global()->BaseNormalCells;

	if (differentColor)
		vec.clear();

	if (Game::IsActive)
	{
		const auto foundationWidth = pBuildBldType->GetFoundationWidth();
		const auto foundationHeight = pBuildBldType->GetFoundationHeight(false);
		const auto topLeftX = pFoundationTopLeft->X;
		const auto topLeftY = pFoundationTopLeft->Y;
		const auto bottomRightX = static_cast<short>(topLeftX + foundationWidth);
		const auto bottomRightY = static_cast<short>(topLeftY + foundationHeight);

		const auto pBuildBldTypeExt = BuildingTypeExt::ExtMap.Find(pBuildBldType);
		const auto buildingAdjacent = static_cast<short>(pBuildBldType->Adjacent + 1);
		const auto leftX = static_cast<short>(topLeftX - buildingAdjacent);
		const auto topY = static_cast<short>(topLeftY - buildingAdjacent);
		const auto rightX = static_cast<short>(bottomRightX + buildingAdjacent);
		const auto bottomY = static_cast<short>(bottomRightY + buildingAdjacent);
		const auto unitBase = RulesExt::Global()->CheckUnitBaseNormal.Get();

		for (auto curX = leftX; curX < rightX; ++curX)
		{
			for (auto curY = topY; curY < bottomY; ++curY)
			{
				if (const auto pCell = MapClass::Instance->GetCellAt(CellStruct{curX, curY}))
				{
					auto pObject = pCell->FirstObject;
					bool baseNormal = false;

					while (pObject)
					{
						const auto absType = pObject->WhatAmI();

						if (absType == AbstractType::Building)
						{
							if (curX < topLeftX || curX >= bottomRightX || curY < topLeftY || curY >= bottomRightY)
							{
								const auto pCellBld = static_cast<BuildingClass*>(pObject);
								const auto pCellBldType = pCellBld->Type;
								const auto mission = pCellBld->CurrentMission;

								if (mission != Mission::Construction || !BuildingTypeExt::ExtMap.Find(pCellBldType)->NoBuildAreaOnBuildup)
								{
									const auto& pBuildingsAllowed = pBuildBldTypeExt->Adjacent_Allowed;

									if (!pBuildingsAllowed.size() || pBuildingsAllowed.Contains(pCellBldType))
									{
										const auto& pBuildingsDisallowed = pBuildBldTypeExt->Adjacent_Disallowed;

										if (!pBuildingsDisallowed.size() || !pBuildingsDisallowed.Contains(pCellBldType))
										{
											if (const auto pOwner = pCellBld->Owner)
											{
												if (pOwner->ArrayIndex == idxHouse && pCellBldType->BaseNormal)
												{
													if (AresHelper::CanUseAres) // Restore Ares MapClass_CanBuildingTypeBePlacedHere_Ignore
													{
														struct DummyAresBuildingExt // Temp Ares Building Ext
														{
															char _[0xE];
															bool unknownExtBool;
														};

														struct DummyBuildingClass // Temp Building Class
														{
															char _[0x71C];
															DummyAresBuildingExt* align_71C;
														};

														if (const auto pAresBuildingExt = reinterpret_cast<DummyBuildingClass*>(pCellBld)->align_71C)
															baseNormal = !pAresBuildingExt->unknownExtBool;
														else
															baseNormal = true;
													}
													else
													{
														baseNormal = true;
													}
												}
												else if (RulesClass::Instance->BuildOffAlly && pOwner->IsAlliedWith(HouseClass::Array->Items[idxHouse]) && pCellBldType->EligibileForAllyBuilding)
												{
													baseNormal = true;
												}
											}
										}
									}
								}
							}
						}
						else if (unitBase && absType == AbstractType::Unit)
						{
							const auto pUnit = static_cast<UnitClass*>(pObject);

							if (const auto pOwner = pUnit->Owner)
							{
								if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(static_cast<UnitClass*>(pObject)->Type))
								{
									if (pOwner->ArrayIndex == idxHouse && pTypeExt->UnitBaseNormal)
										baseNormal = true;
									else if (RulesClass::Instance->BuildOffAlly && pOwner->IsAlliedWith(HouseClass::Array->Items[idxHouse]) && pTypeExt->UnitBaseForAllyBuilding)
										baseNormal = true;
								}
							}
						}

						if (baseNormal)
						{
							if (differentColor)
							{
								isInAdjacent = true;
								vec.push_back(CellStruct{curX, curY});
								break; // Next cell
							}
							else
							{
								R->Stack<bool>(STACK_OFFSET(0x30, 0xC), true);
								return CheckCompleted; // No need to check any more
							}
						}

						pObject = pObject->NextObject;
					}
				}
			}
		}
	}

	R->Stack<bool>(STACK_OFFSET(0x30, 0xC), isInAdjacent);
	return CheckCompleted;
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

// Buildable-upon TechnoTypes Helper
namespace BuildOnOccupiersHelpers
{
	bool Exist = false;
	bool Mouse = false;
	CellClass* CurrentCell = nullptr;
}

// Buildable-upon TerrainTypes Hook #1 -> sub_47C620 - Allow placing buildings on top of them
// Buildable-upon TechnoTypes Hook #1 -> sub_47C620 - Rewrite and check whether allow placing buildings on top of them
DEFINE_HOOK(0x47C640, CellClass_CanThisExistHere_IgnoreSomething, 0x6)
{
	enum { CanNotExistHere = 0x47C6D1, CanExistHere = 0x47C6A0 };

	GET(const CellClass* const, pCell, EDI);
	GET(const BuildingTypeClass* const, pBuildingType, EAX);
	GET_STACK(HouseClass* const, pOwner, STACK_OFFSET(0x18, 0xC));

	BuildOnOccupiersHelpers::Exist = false;

	if (!Game::IsActive)
		return CanExistHere;

	const auto expand = RulesExt::Global()->ExpandBuildingPlace.Get();
	bool landFootOnly = false;

	if (pBuildingType->LaserFence)
	{
		auto pObject = pCell->FirstObject;

		while (pObject)
		{
			const auto absType = pObject->WhatAmI();

			if (absType == AbstractType::Building)
			{
				const auto pBuilding = static_cast<BuildingClass*>(pObject);
				const auto pType = pBuilding->Type;
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}
			else if (const auto pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				const auto pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}

			pObject = pObject->NextObject;
		}
	}
	else if (pBuildingType->LaserFencePost || pBuildingType->Gate)
	{
		bool builtOnTechno = false;
		auto pObject = pCell->FirstObject;

		while (pObject)
		{
			const auto absType = pObject->WhatAmI();

			if (absType == AbstractType::Aircraft)
			{
				const auto pAircraft = static_cast<AircraftClass*>(pObject);
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pAircraft->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Building)
			{
				const auto pBuilding = static_cast<BuildingClass*>(pObject);
				const auto pType = pBuilding->Type;
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else if (pOwner != pBuilding->Owner || !pType->LaserFence)
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
			{
				const auto pTechno = static_cast<TechnoClass*>(pObject);
				const auto pTechnoType = pTechno->GetTechnoType();
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else if (!expand || pTechnoType->Speed <= 0 || !BuildingTypeExt::CheckOccupierCanLeave(pOwner, pTechno->Owner))
					return CanNotExistHere;
				else
					landFootOnly = true;
			}
			else if (const auto pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				const auto pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else
					return CanNotExistHere;
			}

			pObject = pObject->NextObject;
		}

		if (!landFootOnly && !builtOnTechno && (pCell->OccupationFlags & 0x3F))
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

		if (isoTileTypeIndex >= 0 && isoTileTypeIndex < IsometricTileTypeClass::Array->Count && !IsometricTileTypeClass::Array->Items[isoTileTypeIndex]->Morphable)
			return CanNotExistHere;

		auto pObject = pCell->FirstObject;

		while (pObject)
		{
			const auto absType = pObject->WhatAmI();

			if (absType == AbstractType::Building)
			{
				const auto pBuilding = static_cast<BuildingClass*>(pObject);
				const auto pType = pBuilding->Type;
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}

			pObject = pObject->NextObject;
		}
	}
	else
	{
		bool builtOnTechno = false;
		auto pObject = pCell->FirstObject;

		while (pObject)
		{
			const auto absType = pObject->WhatAmI();

			if (absType == AbstractType::Aircraft || absType == AbstractType::Building)
			{
				const auto pTechno = static_cast<TechnoClass*>(pObject);
				const auto pTechnoType = pTechno->GetTechnoType();
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
			{
				const auto pTechno = static_cast<TechnoClass*>(pObject);
				const auto pTechnoType = pTechno->GetTechnoType();
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else if (!expand || pTechnoType->Speed <= 0 || !BuildingTypeExt::CheckOccupierCanLeave(pOwner, pTechno->Owner))
					return CanNotExistHere;
				else
					landFootOnly = true;
			}
			else if (const auto pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				const auto pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else
					return CanNotExistHere;
			}

			pObject = pObject->NextObject;
		}

		if (!landFootOnly && !builtOnTechno && (pCell->OccupationFlags & 0x3F))
		{
			if (expand)
				landFootOnly = true;
			else
				return CanNotExistHere;
		}
	}

	if (landFootOnly)
		BuildOnOccupiersHelpers::Exist = true;

	return CanExistHere; // Continue check the overlays .etc
}

// Buildable-upon TechnoTypes Hook #2-1 -> sub_47EC90 - Record cell before draw it then skip vanilla AltFlags check
DEFINE_HOOK(0x47EEBC, CellClass_DrawPlaceGrid_RecordCell, 0x6)
{
	enum { DontDrawAlt = 0x47EF1A, DrawVanillaAlt = 0x47EED6 };

	GET(CellClass* const, pCell, ESI);
	GET(const bool, zero, EDX);

	const auto pRules = RulesExt::Global();
	BlitterFlags flags = BlitterFlags::Centered | BlitterFlags::bf_400;

	if (pRules->PlacementGrid_Expand)
		BuildOnOccupiersHelpers::CurrentCell = pCell;

	if (!(pCell->AltFlags & AltCellFlags::ContainsBuilding))
	{
		if (!pRules->ExpandBuildingPlace)
		{
			R->EDX<BlitterFlags>(flags | (zero ? BlitterFlags::Zero : BlitterFlags::Nonzero));
			return DrawVanillaAlt;
		}
		else if (BuildingTypeClass* const pType = abstract_cast<BuildingTypeClass*>(reinterpret_cast<ObjectTypeClass*>(DisplayClass::Instance->unknown_1194)))
		{
			R->Stack<bool>(STACK_OFFSET(0x30, -0x1D), pCell->CanThisExistHere(pType->SpeedType, pType, HouseClass::CurrentPlayer));
			R->EDX<BlitterFlags>(flags | BlitterFlags::TransLucent75);
			return DontDrawAlt;
		}
	}

	R->EDX<BlitterFlags>(flags | (zero ? BlitterFlags::Zero : BlitterFlags::Nonzero));
	return DontDrawAlt;
}

// Buildable-upon TechnoTypes Hook #2-2 -> sub_47EC90 - Draw different color grid
DEFINE_HOOK(0x47EF52, CellClass_DrawPlaceGrid_DrawGrids, 0x6)
{
	const auto pRules = RulesExt::Global();

	if (!pRules->PlacementGrid_Expand)
		return 0;

	const auto pCell = BuildOnOccupiersHelpers::CurrentCell;

	if (!pCell)
		return 0;

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

	const auto foot = BuildOnOccupiersHelpers::Exist;
	BuildOnOccupiersHelpers::Exist = false;
	const bool land = pCell->LandType != LandType::Water;
	const auto frames = land ? pRules->PlacementGrid_LandFrames.Get() : pRules->PlacementGrid_WaterFrames.Get();
	auto frame = foot ? frames.X : (green ? frames.Z : frames.Y);

	if (frame >= Make_Global<SHPStruct*>(0x8A03FC)->Frames) // place.shp
		frame = 0;

	R->EDI(frame);
	return 0;
}

// Buildable-upon TechnoTypes Hook #3 -> sub_4FB0E0 - Hang up place event if there is only infantries and units on the cell
DEFINE_HOOK(0x4FB1EA, HouseClass_UnitFromFactory_HangUpPlaceEvent, 0x5)
{
	enum { CanBuild = 0x4FB23C, TemporarilyCanNotBuild = 0x4FB5BA, CanNotBuild = 0x4FB35F, BuildSucceeded = 0x4FB649 };

	GET(HouseClass* const, pHouse, EBP);
	GET(TechnoClass* const, pTechno, ESI);
	GET(BuildingClass* const, pFactory, EDI);
	GET_STACK(const CellStruct, topLeftCell, STACK_OFFSET(0x3C, 0x10));

	if (pTechno->WhatAmI() == AbstractType::Building && RulesExt::Global()->ExpandBuildingPlace)
	{
		const auto pBuilding = static_cast<BuildingClass*>(pTechno);
		const auto pBuildingType = pBuilding->Type;
		const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType);
		const auto pDisplay = DisplayClass::Instance();

		if (pTypeExt->LimboBuild)
		{
			BuildingTypeExt::CreateLimboBuilding(pBuilding, pBuildingType, pHouse, pTypeExt->LimboBuildID);

			if (pDisplay->CurrentBuilding == pBuilding && HouseClass::CurrentPlayer == pHouse)
			{
				pDisplay->SetActiveFoundation(nullptr);
				pDisplay->CurrentBuilding = nullptr;
				pDisplay->CurrentBuildingType = nullptr;
				pDisplay->unknown_11AC = 0xFFFFFFFF;

				if (!Unsorted::ArmageddonMode)
				{
					reinterpret_cast<void(__thiscall*)(DisplayClass*, CellStruct*)>(0x4A8D50)(pDisplay, nullptr); // Clear CurrentFoundationCopy_Data
					pDisplay->unknown_1190 = 0;
					pDisplay->unknown_1194 = 0;
				}
			}

			const auto pFactoryType = pFactory->Type;

			if (pFactoryType->ConstructionYard)
			{
				VocClass::PlayGlobal(RulesClass::Instance->BuildingSlam, 0x2000, 1.0);

				pFactory->DestroyNthAnim(BuildingAnimSlot::PreProduction);
				pFactory->DestroyNthAnim(BuildingAnimSlot::Idle);

				const bool damaged = pFactory->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
				const auto pAnimName = damaged ? pFactoryType->BuildingAnim[8].Damaged : pFactoryType->BuildingAnim[8].Anim;

				if (pAnimName && *pAnimName)
					pFactory->PlayAnim(pAnimName, BuildingAnimSlot::Production, damaged, false);
			}

			return BuildSucceeded;
		}
		else
		{
			if (!pBuildingType->PlaceAnywhere && !pBuildingType->PowersUpBuilding[0])
			{
				const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
				bool canBuild = true;
				bool noOccupy = true;

				for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
				{
					const auto currentCoord = topLeftCell + *pFoundation;
					const auto pCell = pDisplay->GetCellAt(currentCoord);

					if (!pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pHouse))
					{
						canBuild = false;
						break;
					}
					else if (BuildOnOccupiersHelpers::Exist)
					{
						noOccupy = false;
					}
				}

				BuildOnOccupiersHelpers::Exist = false;

				do
				{
					if (canBuild)
					{
						if (noOccupy)
							break; // Can Build

						do
						{
							if (topLeftCell != pHouseExt->CurrentBuildingTopLeft || pBuildingType != pHouseExt->CurrentBuildingType) // New command
							{
								pHouseExt->CurrentBuildingType = pBuildingType;
								pHouseExt->CurrentBuildingTimes = 30;
								pHouseExt->CurrentBuildingTopLeft = topLeftCell;
							}
							else if (pHouseExt->CurrentBuildingTimes <= 0)
							{
								break; // Time out
							}

							if (!(pHouseExt->CurrentBuildingTimes % 5) && BuildingTypeExt::CleanUpBuildingSpace(pBuildingType, topLeftCell, pHouse))
								break; // No place for cleaning

							if (pHouse == HouseClass::CurrentPlayer && pHouseExt->CurrentBuildingTimes == 30)
							{
								pDisplay->SetActiveFoundation(nullptr);
								pDisplay->CurrentBuilding = nullptr;
								pDisplay->CurrentBuildingType = nullptr;
								pDisplay->unknown_11AC = 0xFFFFFFFF;

								if (!Unsorted::ArmageddonMode)
								{
									reinterpret_cast<void(__thiscall*)(DisplayClass*, CellStruct*)>(0x4A8D50)(pDisplay, nullptr); // Clear CurrentFoundationCopy_Data
									pDisplay->unknown_1190 = 0;
									pDisplay->unknown_1194 = 0;
								}
							}

							--pHouseExt->CurrentBuildingTimes;
							pHouseExt->CurrentBuildingTimer.Start(8);

							return TemporarilyCanNotBuild;
						}
						while (false);
					}

					if (pHouseExt->CurrentBuildingType == pBuildingType)
					{
						pHouseExt->CurrentBuildingType = nullptr;
						pHouseExt->CurrentBuildingTimes = 0;
						pHouseExt->CurrentBuildingTopLeft = CellStruct::Empty;
						pHouseExt->CurrentBuildingTimer.Stop();

						if (pHouseExt->CurrentBuildingTimes != 30)
							return CanNotBuild;
					}

					BuildOnOccupiersHelpers::Mouse = true;
					return CanNotBuild;
				}
				while (false);

				if (pHouseExt->CurrentBuildingType == pBuildingType)
				{
					pHouseExt->CurrentBuildingType = nullptr;
					pHouseExt->CurrentBuildingTimes = 0;
					pHouseExt->CurrentBuildingTopLeft = CellStruct::Empty;
					pHouseExt->CurrentBuildingTimer.Stop();
				}
			}
		}
	}

	pFactory->SendCommand(RadioCommand::RequestLink, pTechno);

	if (pTechno->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
		return CanBuild;

	BuildOnOccupiersHelpers::Mouse = true;
	return CanNotBuild;
}

// Buildable-upon TechnoTypes Hook #4-1 -> sub_4FB0E0 - Check whether need to skip the replace command
DEFINE_HOOK(0x4FB395, HouseClass_UnitFromFactory_SkipMouseReturn, 0x6)
{
	enum { SkipGameCode = 0x4FB489 };

	if (!RulesExt::Global()->ExpandBuildingPlace)
		return 0;

	if (BuildOnOccupiersHelpers::Mouse)
	{
		BuildOnOccupiersHelpers::Mouse = false;
		return 0;
	}

	R->EBX(0);
	return SkipGameCode;
}

// Buildable-upon TechnoTypes Hook #4-2 -> sub_4FB0E0 - Check whether need to skip the clear command
DEFINE_HOOK(0x4FB319, HouseClass_UnitFromFactory_SkipMouseClear, 0x5)
{
	enum { SkipGameCode = 0x4FB4A0 };

	GET(TechnoClass* const, pTechno, ESI);

	if (const auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
	{
		if (RulesExt::Global()->ExpandBuildingPlace && reinterpret_cast<ObjectTypeClass*>(DisplayClass::Instance->unknown_1194) != pBuilding->Type)
			return SkipGameCode;
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #4-3 -> sub_4FB0E0 - Check whether need to skip the clear command
DEFINE_HOOK(0x4FAB83, HouseClass_AbandonProductionOf_SkipMouseClear, 0x7)
{
	enum { SkipGameCode = 0x4FABA4 };

	GET(const int, index, EBX);

	return (RulesExt::Global()->ExpandBuildingPlace && index >= 0 && BuildingTypeClass::Array->Items[index] != DisplayClass::Instance->CurrentBuildingType) ? SkipGameCode : 0;
}

// Buildable-upon TechnoTypes Hook #5 -> sub_4C9FF0 - Restart timer and clear buffer when abandon building production
DEFINE_HOOK(0x4CA05B, FactoryClass_AbandonProduction_AbandonCurrentBuilding, 0x5)
{
	GET(FactoryClass*, pFactory, ESI);

	if (RulesExt::Global()->ExpandBuildingPlace)
	{
		const auto pHouseExt = HouseExt::ExtMap.Find(pFactory->Owner);

		if (!pHouseExt || !pHouseExt->CurrentBuildingType)
			return 0;

		const auto pTechno = pFactory->Object;

		if (pTechno->WhatAmI() != AbstractType::Building || pHouseExt->CurrentBuildingType != static_cast<BuildingClass*>(pTechno)->Type)
			return 0;

		pHouseExt->CurrentBuildingType = nullptr;
		pHouseExt->CurrentBuildingTimes = 0;
		pHouseExt->CurrentBuildingTopLeft = CellStruct::Empty;
		pHouseExt->CurrentBuildingTimer.Stop();
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #6 -> sub_443C60 - Try to clean up the building space when AI is building
DEFINE_HOOK(0x4451F8, BuildingClass_KickOutUnit_CleanUpAIBuildingSpace, 0x6)
{
	enum { CanBuild = 0x4452F0, TemporarilyCanNotBuild = 0x445237, CanNotBuild = 0x4454E6, BuildSucceeded = 0x4454D4 };

	GET(BaseNodeClass* const, pBaseNode, EBX);
	GET(BuildingClass* const, pBuilding, EDI);
	GET(BuildingClass* const, pFactory, ESI);
	GET(const CellStruct, topLeftCell, EDX);

	if (!RulesExt::Global()->ExpandBuildingPlace)
		return 0;

	const auto pBuildingType = pBuilding->Type;

	if (topLeftCell != CellStruct::Empty && !pBuildingType->PlaceAnywhere)
	{
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

			const auto pFactoryType = pFactory->Type;

			if (pFactoryType->ConstructionYard)
			{
				VocClass::PlayGlobal(RulesClass::Instance->BuildingSlam, 0x2000, 1.0);

				pFactory->DestroyNthAnim(BuildingAnimSlot::PreProduction);
				pFactory->DestroyNthAnim(BuildingAnimSlot::Idle);

				const bool damaged = pFactory->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
				const auto pAnimName = damaged ? pFactoryType->BuildingAnim[8].Damaged : pFactoryType->BuildingAnim[8].Anim;

				if (pAnimName && *pAnimName)
					pFactory->PlayAnim(pAnimName, BuildingAnimSlot::Production, damaged, false);
			}

			return BuildSucceeded;
		}
		else if (!pBuildingType->PowersUpBuilding[0])
		{
			const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
			bool canBuild = true;
			bool noOccupy = true;

			for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
			{
				const auto currentCoord = topLeftCell + *pFoundation;
				const auto pCell = MapClass::Instance->GetCellAt(currentCoord);

				if (!pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pHouse))
				{
					canBuild = false;
					break;
				}
				else if (BuildOnOccupiersHelpers::Exist)
				{
					noOccupy = false;
				}
			}

			BuildOnOccupiersHelpers::Exist = false;

			do
			{
				if (canBuild)
				{
					if (noOccupy)
						break; // Can Build

					do
					{
						if (topLeftCell != pHouseExt->CurrentBuildingTopLeft || pBuildingType != pHouseExt->CurrentBuildingType) // New command
						{
							pHouseExt->CurrentBuildingType = pBuildingType;
							pHouseExt->CurrentBuildingTimes = 30;
							pHouseExt->CurrentBuildingTopLeft = topLeftCell;
						}
						else if (pHouseExt->CurrentBuildingTimes <= 0)
						{
							// This will be different from what vanilla do, but the vanilla way will still be used if in campaign
							if (!SessionClass::IsCampaign())
								break; // Time out

							pHouseExt->CurrentBuildingTimes = 30;
						}

						if (!pHouseExt->CurrentBuildingTimer.HasTimeLeft())
						{
							pHouseExt->CurrentBuildingTimes -= 5;
							pHouseExt->CurrentBuildingTimer.Start(40);

							if (BuildingTypeExt::CleanUpBuildingSpace(pBuildingType, topLeftCell, pHouse))
								break; // No place for cleaning
						}

						return TemporarilyCanNotBuild;
					}
					while (false);
				}

				pHouseExt->CurrentBuildingType = nullptr;
				pHouseExt->CurrentBuildingTimes = 0;
				pHouseExt->CurrentBuildingTopLeft = CellStruct::Empty;
				pHouseExt->CurrentBuildingTimer.Stop();

				return CanNotBuild;
			}
			while (false);

			pHouseExt->CurrentBuildingType = nullptr;
			pHouseExt->CurrentBuildingTimes = 0;
			pHouseExt->CurrentBuildingTopLeft = CellStruct::Empty;
			pHouseExt->CurrentBuildingTimer.Stop();
		}
		else if (const auto pCell = MapClass::Instance->GetCellAt(topLeftCell))
		{
			const auto pCellBuilding = pCell->GetBuilding();

			if (!pCellBuilding || !reinterpret_cast<bool(__thiscall*)(BuildingClass*, BuildingTypeClass*, HouseClass*)>(0x452670)(pCellBuilding, pBuildingType, pHouse)) // CanUpgradeBuilding
				return CanNotBuild;
		}
	}

	if (pBuilding->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
	{
		const auto pFactoryType = pFactory->Type;

		if (pFactoryType->ConstructionYard)
		{
			pFactory->DestroyNthAnim(BuildingAnimSlot::PreProduction);
			pFactory->DestroyNthAnim(BuildingAnimSlot::Idle);

			const bool damaged = pFactory->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
			const auto pAnimName = damaged ? pFactoryType->BuildingAnim[8].Damaged : pFactoryType->BuildingAnim[8].Anim;

			if (pAnimName && *pAnimName)
				pFactory->PlayAnim(pAnimName, BuildingAnimSlot::Production, damaged, false);
		}

		return CanBuild;
	}

	return CanNotBuild;
}

// Laser fence use GetBuilding to check whether can build and draw, so no need to change
// Buildable-upon TechnoTypes Hook #8-1 -> sub_6D5C50 - Don't draw overlay wall grid when have occupiers
DEFINE_HOOK(0x6D5D38, TacticalClass_DrawOverlayWallGrid_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x6D5D40, Invalid = 0x6D5F0F };

	GET(bool, valid, EAX);

	if (BuildOnOccupiersHelpers::Exist)
	{
		BuildOnOccupiersHelpers::Exist = false;
		return Invalid;
	}

	return valid ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #8-2 -> sub_6D59D0 - Don't draw firestorm wall grid when have occupiers
DEFINE_HOOK(0x6D5A9D, TacticalClass_DrawFirestormWallGrid_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x6D5AA5, Invalid = 0x6D5C2F };

	GET(bool, valid, EAX);

	if (BuildOnOccupiersHelpers::Exist)
	{
		BuildOnOccupiersHelpers::Exist = false;
		return Invalid;
	}

	return valid ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #8-3 -> sub_588750 - Don't place overlay wall when have occupiers
DEFINE_HOOK(0x588873, MapClass_BuildingToWall_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x58887B, Invalid = 0x588935 };

	GET(bool, valid, EAX);

	if (BuildOnOccupiersHelpers::Exist)
	{
		BuildOnOccupiersHelpers::Exist = false;
		return Invalid;
	}

	return valid ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #8-4 -> sub_588570 - Don't place firestorm wall when have occupiers
DEFINE_HOOK(0x588664, MapClass_BuildingToFirestormWall_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x58866C, Invalid = 0x588730 };

	GET(bool, valid, EAX);

	if (BuildOnOccupiersHelpers::Exist)
	{
		BuildOnOccupiersHelpers::Exist = false;
		return Invalid;
	}

	return valid ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #9-1 -> sub_7393C0 - Try to clean up the building space when is deploying
DEFINE_HOOK(0x73946C, UnitClass_TryToDeploy_CleanUpDeploySpace, 0x6)
{
	enum { CanDeploy = 0x73958A, TemporarilyCanNotDeploy = 0x73953B, CanNotDeploy = 0x7394E0 };

	GET(UnitClass* const, pUnit, EBP);

	if (!RulesExt::Global()->ExpandBuildingPlace)
		return 0;

	const auto pTechnoExt = TechnoExt::ExtMap.Find(pUnit);
	const auto pBuildingType = pUnit->Type->DeploysInto;
	const auto pHouseExt = HouseExt::ExtMap.Find(pUnit->Owner);
	auto& vec = pHouseExt->OwnedDeployingUnits;
	auto topLeftCell = CellClass::Coord2Cell(pUnit->GetCoords());

	if (pBuildingType->GetFoundationWidth() > 2 || pBuildingType->GetFoundationHeight(false) > 2)
		topLeftCell -= CellStruct { 1, 1 };

	if (!pBuildingType->PlaceAnywhere)
	{
		bool canBuild = true;
		bool noOccupy = true;

		for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
		{
			const auto currentCoord = topLeftCell + *pFoundation;
			const auto pCell = MapClass::Instance->GetCellAt(currentCoord);

			if (!pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pUnit->Owner))
			{
				canBuild = false;
				break;
			}
			else if (BuildOnOccupiersHelpers::Exist)
			{
				noOccupy = false;
			}
		}

		BuildOnOccupiersHelpers::Exist = false;

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

	LEA_STACK(CellStruct*, pTopLeftCell, STACK_OFFSET(0x28, -0x14));
	*pTopLeftCell = topLeftCell;
	return CanDeploy;
}

// Buildable-upon TechnoTypes Hook #9-2 -> sub_73FD50 - Push the owner house into deploy check
DEFINE_HOOK(0x73FF8F, UnitClass_MouseOverObject_ShowDeployCursor, 0x6)
{
	if (RulesExt::Global()->ExpandBuildingPlace) // This IF check is not so necessary
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
	if (RulesExt::Global()->ExpandBuildingPlace) // This IF check is not so necessary
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
	GET(const HouseClass* const, pHouse, ESI);

	if (!pHouse->IsControlledByHuman() || !RulesExt::Global()->ExpandBuildingPlace)
		return 0;

	if (pHouse->RecheckTechTree)
	{
		if (const auto pFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::DontCare))
		{
			if (pFactory->IsDone())
			{
				if (const auto pBuilding = abstract_cast<BuildingClass*>(pFactory->Object))
					BuildingTypeExt::AutoUpgradeBuilding(pBuilding);
			}
		}

		if (const auto pFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::Combat))
		{
			if (pFactory->IsDone())
			{
				if (const auto pBuilding = abstract_cast<BuildingClass*>(pFactory->Object))
					BuildingTypeExt::AutoUpgradeBuilding(pBuilding);
			}
		}
	}

	if (const auto pHouseExt = HouseExt::ExtMap.Find(pHouse))
	{
		if (pHouse == HouseClass::CurrentPlayer) // Prevent unexpected wrong event
		{
			const auto pBuildingType = pHouseExt->CurrentBuildingType;

			if (pHouseExt->CurrentBuildingTimer.Completed())
			{
				pHouseExt->CurrentBuildingTimer.Stop();
				const EventClass event
				(
					pHouse->ArrayIndex,
					EventType::Place,
					AbstractType::Building,
					pBuildingType->GetArrayIndex(),
					pBuildingType->Naval,
					pHouseExt->CurrentBuildingTopLeft
				);
				EventClass::AddEvent(event);
			}
		}

		if (pHouseExt->OwnedDeployingUnits.size() > 0)
		{
			auto& vec = pHouseExt->OwnedDeployingUnits;

			for (auto it = vec.begin(); it != vec.end(); )
			{
				const auto pUnit = *it;

				if (!pUnit->InLimbo && pUnit->IsOnMap && !pUnit->IsSinking && pUnit->Owner == pHouse && !pUnit->Destination && pUnit->CurrentMission == Mission::Guard && !pUnit->ParasiteEatingMe && !pUnit->TemporalTargetingMe)
				{
					if (const auto pType = pUnit->Type)
					{
						if (pType->DeploysInto)
						{
							if (const auto pExt = TechnoExt::ExtMap.Find(pUnit))
							{
								if (!(pExt->UnitAutoDeployTimer.GetTimeLeft() % 8))
									pUnit->QueueMission(Mission::Unload, true);

								++it;
								continue;
							}
						}
					}
				}

				it = vec.erase(it);
			}
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #12 -> sub_6D5030 - Draw the placing building preview
DEFINE_HOOK(0x6D504C, TacticalClass_DrawPlacement_DrawPlacingPreview, 0x6)
{
	const auto pPlayer = HouseClass::CurrentPlayer();

	for (const auto& pHouse : *HouseClass::Array)
	{
		if (pPlayer->IsObserver() || pPlayer->IsAlliedWith(pHouse))
		{
			const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
			const bool isPlayer = pHouse == pPlayer;
			const auto pDisplay = DisplayClass::Instance();
			{
				auto pType = pHouseExt->CurrentBuildingType;

				if (pType || (isPlayer && (pType = abstract_cast<BuildingTypeClass*>(reinterpret_cast<ObjectTypeClass*>(pDisplay->unknown_1194)), pType)))
				{
					auto pCell = pDisplay->TryGetCellAt(pHouseExt->CurrentBuildingTopLeft);

					if (pCell || (isPlayer && (pCell = pDisplay->TryGetCellAt(pDisplay->CurrentFoundationCopy_TopLeftOffset + pDisplay->CurrentFoundationCopy_CenterCell), pCell)))
					{
						auto pImage = pType->LoadBuildup();
						int imageFrame = 0;

						if (pImage)
							imageFrame = ((pImage->Frames / 2) - 1);
						else
							pImage = pType->GetImage();

						if (pImage)
						{
							BlitterFlags blitFlags = BlitterFlags::TransLucent75 | BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;
							auto rect = DSurface::Temp->GetRect();
							rect.Height -= 32;
							auto point = TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, (1 + pCell->GetFloorHeight(Point2D::Empty)))).first;
							point.Y -= 15;

							const auto ColorSchemeIndex = pHouse->ColorSchemeIndex;
							const auto Palettes = pType->Palette;
							const auto pColor = Palettes ? Palettes->Items[ColorSchemeIndex] : ColorScheme::Array->Items[ColorSchemeIndex];

							DSurface::Temp->DrawSHP(pColor->LightConvert, pImage, imageFrame, &point, &rect, blitFlags, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
						}
					}
				}
			}

			if (pHouseExt->OwnedDeployingUnits.size() > 0)
			{
				for (const auto& pUnit : pHouseExt->OwnedDeployingUnits)
				{
					if (pUnit && pUnit->Type)
					{
						if (const auto pType = pUnit->Type->DeploysInto)
						{
							auto displayCell = CellClass::Coord2Cell(pUnit->GetCoords()); // pUnit->GetMapCoords();

							if (pType->GetFoundationWidth() > 2 || pType->GetFoundationHeight(false) > 2)
								displayCell -= CellStruct { 1, 1 };

							if (const auto pCell = pDisplay->TryGetCellAt(displayCell))
							{
								auto pImage = pType->LoadBuildup();
								int imageFrame = 0;

								if (pImage)
									imageFrame = ((pImage->Frames / 2) - 1);
								else
									pImage = pType->GetImage();

								if (pImage)
								{
									BlitterFlags blitFlags = BlitterFlags::TransLucent75 | BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;
									auto rect = DSurface::Temp->GetRect();
									rect.Height -= 32;
									auto point = TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, (1 + pCell->GetFloorHeight(Point2D::Empty)))).first;
									point.Y -= 15;

									const auto ColorSchemeIndex = pUnit->Owner->ColorSchemeIndex;
									const auto Palettes = pType->Palette;
									const auto pColor = Palettes ? Palettes->Items[ColorSchemeIndex] : ColorScheme::Array->Items[ColorSchemeIndex];

									DSurface::Temp->DrawSHP(pColor->LightConvert, pImage, imageFrame, &point, &rect, blitFlags, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
								}
							}
						}
					}
				}
			}
		}
	}

	return 0;
}

// Auto Build Hook -> sub_6A8B30 - Auto Build Buildings
DEFINE_HOOK(0x6A8E34, StripClass_Update_AutoBuildBuildings, 0x7)
{
	enum { SkipSetStripShortCut = 0x6A8E4D };

	GET(BuildingClass* const, pBuilding, ESI);

	return (RulesExt::Global()->ExpandBuildingPlace && (BuildingTypeExt::BuildLimboBuilding(pBuilding) || BuildingTypeExt::AutoUpgradeBuilding(pBuilding))) ? SkipSetStripShortCut : 0;
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

		if (index >= 0 && index < BuildingTypeClass::Array->Count && BuildingTypeExt::ExtMap.Find(BuildingTypeClass::Array->Items[index])->LimboBuild)
			return Invalid;
	}

	return reinterpret_cast<bool(__thiscall*)(HouseClass*, BaseNodeClass*)>(0x50CAD0)(pBase->Owner, pBaseNode) ? Valid : Invalid;
}
