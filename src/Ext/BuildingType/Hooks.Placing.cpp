#include "Body.h"
#include <EventClass.h>
#include <TerrainClass.h>
#include <AircraftClass.h>
#include <TacticalClass.h>
#include <IsometricTileTypeClass.h>
#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/TerrainType/Body.h>
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
		ObjectClass* pCellObject = pCell->FirstObject;

		while (pCellObject)
		{
			const AbstractType absType = pCellObject->WhatAmI();

			if (absType == AbstractType::Infantry || absType == AbstractType::Unit || absType == AbstractType::Aircraft || absType == AbstractType::Building)
			{
				TechnoClass* const pTechno = static_cast<TechnoClass*>(pCellObject);
				TechnoTypeClass* const pType = pTechno->GetTechnoType();
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
				{
					pTechno->KillPassengers(nullptr);
					pTechno->Stun();
					pTechno->Limbo();
					pTechno->UnInit();
				}
			}
			else if (TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pCellObject))
			{
				auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

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

	ObjectClass* pCellObject = pCell->FirstObject;

	while (pCellObject)
	{
		if (TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pCellObject))
		{
			auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

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

	const bool differentColor = RulesExt::Global()->CheckExpandPlaceGrid;
	bool isInAdjacent = false;
	auto& vec = HouseExt::ExtMap.Find(HouseClass::CurrentPlayer)->BaseNormalCells;

	if (differentColor)
		vec.clear();

	if (Game::IsActive)
	{
		const short foundationWidth = pBuildBldType->GetFoundationWidth();
		const short foundationHeight = pBuildBldType->GetFoundationHeight(false);
		const short topLeftX = pFoundationTopLeft->X;
		const short topLeftY = pFoundationTopLeft->Y;
		const short bottomRightX = topLeftX + foundationWidth;
		const short bottomRightY = topLeftY + foundationHeight;

		BuildingTypeExt::ExtData* const pBuildBldTypeExt = BuildingTypeExt::ExtMap.Find(pBuildBldType);
		const short buildingAdjacent = static_cast<short>(pBuildBldType->Adjacent + 1);
		const short leftX = topLeftX - buildingAdjacent;
		const short topY = topLeftY - buildingAdjacent;
		const short rightX = bottomRightX + buildingAdjacent;
		const short bottomY = bottomRightY + buildingAdjacent;

		for (short curX = leftX; curX < rightX; ++curX)
		{
			for (short curY = topY; curY < bottomY; ++curY)
			{
				if (CellClass* const pCell = MapClass::Instance->GetCellAt(CellStruct{curX, curY}))
				{
					ObjectClass* pObject = pCell->FirstObject;
					bool baseNormal = false;

					while (pObject)
					{
						const AbstractType absType = pObject->WhatAmI();

						if (absType == AbstractType::Building)
						{
							if (curX < topLeftX || curX >= bottomRightX || curY < topLeftY || curY >= bottomRightY)
							{
								BuildingClass* const pCellBld = static_cast<BuildingClass*>(pObject);
								BuildingTypeClass* const pCellBldType = pCellBld->Type;
								const Mission mission = pCellBld->CurrentMission;

								if (mission != Mission::Construction && mission != Mission::Selling && !BuildingTypeExt::ExtMap.Find(pCellBldType)->NoBuildAreaOnBuildup)
								{
									auto const& pBuildingsAllowed = pBuildBldTypeExt->Adjacent_Allowed;

									if (!pBuildingsAllowed.size() || pBuildingsAllowed.Contains(pCellBldType))
									{
										auto const& pBuildingsDisallowed = pBuildBldTypeExt->Adjacent_Disallowed;

										if (!pBuildingsDisallowed.size() || !pBuildingsDisallowed.Contains(pCellBldType))
										{
											if (HouseClass* const pOwner = pCellBld->Owner)
											{
												if (pOwner->ArrayIndex == idxHouse && pCellBldType->BaseNormal)
												{
													if (CAN_USE_ARES && AresHelper::CanUseAres) // Restore Ares MapClass_CanBuildingTypeBePlacedHere_Ignore
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

														if (const DummyAresBuildingExt* pAresBuildingExt = reinterpret_cast<DummyBuildingClass*>(pCellBld)->align_71C)
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
						else if (RulesExt::Global()->CheckUnitBaseNormal && absType == AbstractType::Unit)
						{
							UnitClass* const pUnit = static_cast<UnitClass*>(pObject);

							if (HouseClass* const pOwner = pUnit->Owner)
							{
								if (TechnoTypeExt::ExtData* const pTypeExt = TechnoTypeExt::ExtMap.Find(static_cast<UnitClass*>(pObject)->Type))
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

	const bool expand = RulesExt::Global()->ExpandBuildingPlace;
	bool landFootOnly = false;

	if (pBuildingType->LaserFence)
	{
		ObjectClass* pObject = pCell->FirstObject;

		while (pObject)
		{
			const AbstractType absType = pObject->WhatAmI();

			if (absType == AbstractType::Building)
			{
				BuildingClass* const pBuilding = static_cast<BuildingClass*>(pObject);
				BuildingTypeClass* const pType = pBuilding->Type;
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}
			else if (TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}

			pObject = pObject->NextObject;
		}
	}
	else if (pBuildingType->LaserFencePost || pBuildingType->Gate)
	{
		bool builtOnTechno = false;
		ObjectClass* pObject = pCell->FirstObject;

		while (pObject)
		{
			const AbstractType absType = pObject->WhatAmI();

			if (absType == AbstractType::Aircraft)
			{
				AircraftClass* const pAircraft = static_cast<AircraftClass*>(pObject);
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pAircraft->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Building)
			{
				BuildingClass* const pBuilding = static_cast<BuildingClass*>(pObject);
				BuildingTypeClass* const pType = pBuilding->Type;
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else if (pOwner != pBuilding->Owner || !pType->LaserFence)
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
			{
				TechnoClass* const pTechno = static_cast<TechnoClass*>(pObject);
				TechnoTypeClass* const pTechnoType = pTechno->GetTechnoType();
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else if (!expand || pTechnoType->Speed <= 0 || !BuildingTypeExt::CheckOccupierCanLeave(pOwner, pTechno->Owner))
					return CanNotExistHere;
				else
					landFootOnly = true;
			}
			else if (TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else
					return CanNotExistHere;
			}

			pObject = pObject->NextObject;
		}

		if (!landFootOnly && !builtOnTechno && (pCell->OccupationFlags & 0xFF))
			landFootOnly = true;
	}
	else if (pBuildingType->ToTile)
	{
		const int isoTileTypeIndex = pCell->IsoTileTypeIndex;

		if (isoTileTypeIndex >= 0 && isoTileTypeIndex < IsometricTileTypeClass::Array->Count && !IsometricTileTypeClass::Array->Items[isoTileTypeIndex]->Morphable)
			return CanNotExistHere;

		ObjectClass* pObject = pCell->FirstObject;

		while (pObject)
		{
			const AbstractType absType = pObject->WhatAmI();

			if (absType == AbstractType::Building)
			{
				BuildingClass* const pBuilding = static_cast<BuildingClass*>(pObject);
				BuildingTypeClass* const pType = pBuilding->Type;
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}
			else if (TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}

			pObject = pObject->NextObject;
		}
	}
	else
	{
		bool builtOnTechno = false;
		ObjectClass* pObject = pCell->FirstObject;

		while (pObject)
		{
			const AbstractType absType = pObject->WhatAmI();

			if (absType == AbstractType::Aircraft || absType == AbstractType::Building)
			{
				TechnoClass* const pTechno = static_cast<TechnoClass*>(pObject);
				TechnoTypeClass* const pTechnoType = pTechno->GetTechnoType();
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
			{
				TechnoClass* const pTechno = static_cast<TechnoClass*>(pObject);
				TechnoTypeClass* const pTechnoType = pTechno->GetTechnoType();
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else if (!expand || pTechnoType->Speed <= 0 || !BuildingTypeExt::CheckOccupierCanLeave(pOwner, pTechno->Owner))
					return CanNotExistHere;
				else
					landFootOnly = true;
			}
			else if (TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnTechno = true;
				else
					return CanNotExistHere;
			}

			pObject = pObject->NextObject;
		}

		if (!landFootOnly && !builtOnTechno && (pCell->OccupationFlags & 0xFF))
			landFootOnly = true;
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

	RulesExt::ExtData* const pRules = RulesExt::Global();
	BlitterFlags flags = BlitterFlags::Centered | BlitterFlags::bf_400;

	if (pRules->CheckExpandPlaceGrid)
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
	RulesExt::ExtData* const pRules = RulesExt::Global();

	if (!pRules->CheckExpandPlaceGrid)
		return 0;

	CellClass* const pCell = BuildOnOccupiersHelpers::CurrentCell;

	if (!pCell)
		return 0;

	const CellStruct cell = pCell->MapCoords;
	auto const pObj = DisplayClass::Instance->CurrentBuildingType;
	const short range = static_cast<short>((pObj && pObj->WhatAmI() == AbstractType::BuildingType) ? static_cast<BuildingTypeClass*>(pObj)->Adjacent + 1 : 0);

	const short maxX = cell.X + range;
	const short maxY = cell.Y + range;
	const short minX = cell.X - range;
	const short minY = cell.Y - range;

	bool green = false;
	auto const cells = HouseExt::ExtMap.Find(HouseClass::CurrentPlayer)->BaseNormalCells;

	for (auto const& baseCell : cells)
	{
		if (baseCell.X >= minX && baseCell.Y >= minY && baseCell.X <= maxX && baseCell.Y <= maxY)
		{
			green = true;
			break;
		}
	}

	const bool foot = BuildOnOccupiersHelpers::Exist;
	BuildOnOccupiersHelpers::Exist = false;
	const bool land = pCell->LandType != LandType::Water;
	const CoordStruct frames = land ? pRules->ExpandLandGridFrames : pRules->ExpandWaterGridFrames;
	int frame = foot ? frames.X : (green ? frames.Z : frames.Y);

	if (frame >= Make_Global<SHPStruct*>(0x8A03FC)->Frames)
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
		BuildingClass* const pBuilding = static_cast<BuildingClass*>(pTechno);
		BuildingTypeClass* const pBuildingType = pBuilding->Type;
		BuildingTypeExt::ExtData* const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType);

		if (pTypeExt->LimboBuild)
		{
			BuildingTypeExt::CreateLimboBuilding(pBuilding, pBuildingType, pHouse, pTypeExt->LimboBuildID);

			if (DisplayClass::Instance->CurrentBuilding == pBuilding && HouseClass::CurrentPlayer == pHouse)
			{
				DisplayClass::Instance->SetActiveFoundation(nullptr);
				DisplayClass::Instance->CurrentBuilding = nullptr;
				DisplayClass::Instance->CurrentBuildingType = nullptr;
				DisplayClass::Instance->unknown_11AC = 0xFFFFFFFF;

				if (!Unsorted::ArmageddonMode)
				{
					reinterpret_cast<void(__thiscall*)(DisplayClass*, CellStruct*)>(0x4A8D50)(DisplayClass::Instance, nullptr); // Clear CurrentFoundationCopy_Data
					DisplayClass::Instance->unknown_1190 = 0;
					DisplayClass::Instance->unknown_1194 = 0;
				}
			}

			BuildingTypeClass* const pFactoryType = pFactory->Type;

			if (pFactoryType->ConstructionYard)
			{
				VocClass::PlayGlobal(RulesClass::Instance->BuildingSlam, 0x2000, 1.0);

				pFactory->DestroyNthAnim(BuildingAnimSlot::PreProduction);
				pFactory->DestroyNthAnim(BuildingAnimSlot::Idle);

				const bool damaged = pFactory->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
				const char* const pAnimName = damaged ? pFactoryType->BuildingAnim[8].Damaged : pFactoryType->BuildingAnim[8].Anim;

				if (pAnimName && *pAnimName)
					pFactory->PlayAnim(pAnimName, BuildingAnimSlot::Production, damaged, false);
			}

			return BuildSucceeded;
		}
		else
		{
			HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse);

			if (!pBuildingType->PlaceAnywhere && !pBuildingType->PowersUpBuilding[0])
			{
				bool canBuild = true;
				bool noOccupy = true;

				for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
				{
					CellStruct currentCoord = topLeftCell + *pFoundation;
					CellClass* const pCell = MapClass::Instance->GetCellAt(currentCoord);

					if (!pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pHouse))
						canBuild = false;
					else if (BuildOnOccupiersHelpers::Exist)
						noOccupy = false;
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
								DisplayClass::Instance->SetActiveFoundation(nullptr);
								DisplayClass::Instance->CurrentBuilding = nullptr;
								DisplayClass::Instance->CurrentBuildingType = nullptr;
								DisplayClass::Instance->unknown_11AC = 0xFFFFFFFF;

							/*	if (!Unsorted::ArmageddonMode) // To reserve for drawing grids
								{
									reinterpret_cast<void(__thiscall*)(DisplayClass*, CellStruct*)>(0x4A8D50)(DisplayClass::Instance, nullptr); // Clear CurrentFoundationCopy_Data
									DisplayClass::Instance->unknown_1190 = 0;
									DisplayClass::Instance->unknown_1194 = 0;
								}*/
							}

							pFactory->SendToFirstLink(RadioCommand::NotifyUnlink);
							--pHouseExt->CurrentBuildingTimes;
							pHouseExt->CurrentBuildingTimer.Start(8);

							return TemporarilyCanNotBuild;
						}
						while (false);
					}
					else if (pHouseExt->CurrentBuildingTopLeft == CellStruct::Empty)
					{
						BuildOnOccupiersHelpers::Mouse = true;
					}

					pHouseExt->CurrentBuildingType = nullptr;
					pHouseExt->CurrentBuildingTimes = 0;
					pHouseExt->CurrentBuildingTopLeft = CellStruct::Empty;
					pHouseExt->CurrentBuildingTimer.Stop();
					return CanNotBuild;
				}
				while (false);
			}

			pHouseExt->CurrentBuildingType = nullptr;
			pHouseExt->CurrentBuildingTimes = 0;
			pHouseExt->CurrentBuildingTopLeft = CellStruct::Empty;
			pHouseExt->CurrentBuildingTimer.Stop();
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

	if (BuildingClass* const pBuilding = abstract_cast<BuildingClass*>(pTechno))
	{
		BuildingTypeExt::ExtData* const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

		if (pTypeExt->AutoUpgrade && DisplayClass::Instance->CurrentBuilding != pBuilding)
			return SkipGameCode;
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #4-3 -> sub_4FB0E0 - Check whether need to skip the clear command
DEFINE_HOOK(0x4FAB83, HouseClass_AbandonProductionOf_SkipMouseClear, 0x7)
{
	enum { SkipGameCode = 0x4FABA4 };

	GET(const int, index, EBX);

	if (index >= 0)
	{
		BuildingTypeClass* const pType = BuildingTypeClass::Array->Items[index];
		BuildingTypeExt::ExtData* const pTypeExt = BuildingTypeExt::ExtMap.Find(pType);

		if (pTypeExt->AutoUpgrade && DisplayClass::Instance->CurrentBuildingType != pType)
			return SkipGameCode;
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #5 -> sub_4C9FF0 - Restart timer and clear buffer when abandon building production
DEFINE_HOOK(0x4CA05B, FactoryClass_AbandonProduction_AbandonCurrentBuilding, 0x5)
{
	GET(FactoryClass*, pFactory, ESI);

	if (RulesExt::Global()->ExpandBuildingPlace)
	{
		HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pFactory->Owner);

		if (!pHouseExt || !pHouseExt->CurrentBuildingType)
			return 0;

		TechnoClass* const pTechno = pFactory->Object;

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

	BuildingTypeClass* const pBuildingType = pBuilding->Type;

	if (topLeftCell != CellStruct::Empty && !pBuildingType->PlaceAnywhere)
	{
		HouseClass* const pHouse = pFactory->Owner;
		BuildingTypeExt::ExtData* const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType);

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

			BuildingTypeClass* const pFactoryType = pFactory->Type;

			if (pFactoryType->ConstructionYard)
			{
				VocClass::PlayGlobal(RulesClass::Instance->BuildingSlam, 0x2000, 1.0);

				pFactory->DestroyNthAnim(BuildingAnimSlot::PreProduction);
				pFactory->DestroyNthAnim(BuildingAnimSlot::Idle);

				const bool damaged = pFactory->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
				const char* const pAnimName = damaged ? pFactoryType->BuildingAnim[8].Damaged : pFactoryType->BuildingAnim[8].Anim;

				if (pAnimName && *pAnimName)
					pFactory->PlayAnim(pAnimName, BuildingAnimSlot::Production, damaged, false);
			}

			return BuildSucceeded;
		}
		else if (!pBuildingType->PowersUpBuilding[0])
		{
			HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse);
			bool canBuild = true;
			bool noOccupy = true;

			for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
			{
				CellStruct currentCoord = topLeftCell + *pFoundation;
				CellClass* const pCell = MapClass::Instance->GetCellAt(currentCoord);

				if (!pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pHouse))
					canBuild = false;
				else if (BuildOnOccupiersHelpers::Exist)
					noOccupy = false;
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
		else if (CellClass* const pCell = MapClass::Instance->GetCellAt(topLeftCell))
		{
			BuildingClass* const pCellBuilding = pCell->GetBuilding();

			if (!pCellBuilding || !reinterpret_cast<bool(__thiscall*)(BuildingClass*, BuildingTypeClass*, HouseClass*)>(0x452670)(pCellBuilding, pBuildingType, pHouse)) // CanUpgradeBuilding
				return CanNotBuild;
		}
	}

	if (pBuilding->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
	{
		BuildingTypeClass* const pFactoryType = pFactory->Type;

		if (pFactoryType->ConstructionYard)
		{
			pFactory->DestroyNthAnim(BuildingAnimSlot::PreProduction);
			pFactory->DestroyNthAnim(BuildingAnimSlot::Idle);

			const bool damaged = pFactory->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
			const char* const pAnimName = damaged ? pFactoryType->BuildingAnim[8].Damaged : pFactoryType->BuildingAnim[8].Anim;

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

	TechnoExt::ExtData* const pTechnoExt = TechnoExt::ExtMap.Find(pUnit);
	BuildingTypeClass* const pBuildingType = pUnit->Type->DeploysInto;
	HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pUnit->Owner);
	auto& vec = pHouseExt->OwnedDeployingUnits;
	CellStruct topLeftCell = CellClass::Coord2Cell(pUnit->GetCoords()); // pUnit->GetMapCoords() -> desync

	if (pBuildingType->GetFoundationWidth() > 2 || pBuildingType->GetFoundationHeight(false) > 2)
		topLeftCell -= CellStruct { 1, 1 };

	if (!pBuildingType->PlaceAnywhere)
	{
		bool canBuild = true;
		bool noOccupy = true;

		for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
		{
			CellStruct currentCoord = topLeftCell + *pFoundation;
			CellClass* const pCell = MapClass::Instance->GetCellAt(currentCoord);

			if (!pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pUnit->Owner))
				canBuild = false;
			else if (BuildOnOccupiersHelpers::Exist)
				noOccupy = false;
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
			const Mission mission = pUnit->CurrentMission;

			if (mission == Mission::Guard || mission == Mission::Unload)
			{
				if (HouseClass* const pHouse = pUnit->Owner)
				{
					if (HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse))
					{
						auto& vec = pHouseExt->OwnedDeployingUnits;

						if (vec.size() > 0)
							vec.erase(std::remove(vec.begin(), vec.end(), pUnit), vec.end());
					}
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
		if (FactoryClass* const pFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::DontCare))
		{
			if (pFactory->IsDone())
			{
				if (BuildingClass* const pBuilding = abstract_cast<BuildingClass*>(pFactory->Object))
					BuildingTypeExt::AutoUpgradeBuilding(pBuilding);
			}
		}

		if (FactoryClass* const pFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::Combat))
		{
			if (pFactory->IsDone())
			{
				if (BuildingClass* const pBuilding = abstract_cast<BuildingClass*>(pFactory->Object))
					BuildingTypeExt::AutoUpgradeBuilding(pBuilding);
			}
		}
	}

	if (HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse))
	{
		if (pHouse == HouseClass::CurrentPlayer) // Prevent unexpected wrong event
		{
			const BuildingTypeClass* const pBuildingType = pHouseExt->CurrentBuildingType;

			if (pHouseExt->CurrentBuildingTimer.Completed())
			{
				pHouseExt->CurrentBuildingTimer.Stop();
				EventClass event
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
				UnitClass* const pUnit = *it;

				if (!pUnit->InLimbo && pUnit->IsOnMap && !pUnit->IsSinking && pUnit->Owner == pHouse && !pUnit->Destination && pUnit->CurrentMission == Mission::Guard && !pUnit->ParasiteEatingMe && !pUnit->TemporalTargetingMe)
				{
					if (UnitTypeClass* const pType = pUnit->Type)
					{
						if (pType->DeploysInto)
						{
							if (TechnoExt::ExtData* const pExt = TechnoExt::ExtMap.Find(pUnit))
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
	const HouseClass* const pPlayer = HouseClass::CurrentPlayer;

	for (auto const& pHouse : *HouseClass::Array)
	{
		if (pPlayer->IsObserver() || pPlayer->IsAlliedWith(pHouse))
		{
			const HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse);
			const bool isPlayer = pHouse == pPlayer;

			if (BuildingTypeClass* const pType = isPlayer ? abstract_cast<BuildingTypeClass*>(reinterpret_cast<ObjectTypeClass*>(DisplayClass::Instance->unknown_1194)) : pHouseExt->CurrentBuildingType)
			{
				if (const CellClass* const pCell = MapClass::Instance->TryGetCellAt(isPlayer ? DisplayClass::Instance->CurrentFoundationCopy_TopLeftOffset + DisplayClass::Instance->CurrentFoundationCopy_CenterCell : pHouseExt->CurrentBuildingTopLeft))
				{
					SHPStruct* pImage = pType->LoadBuildup();
					int imageFrame = 0;

					if (pImage)
						imageFrame = ((pImage->Frames / 2) - 1);
					else
						pImage = pType->GetImage();

					if (pImage)
					{
						BlitterFlags blitFlags = BlitterFlags::TransLucent50 | BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;
						RectangleStruct rect = DSurface::Temp->GetRect();
						rect.Height -= 32;
						Point2D point = TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, (1 + pCell->GetFloorHeight(Point2D::Empty)))).first;
						point.Y -= 15;

						const int ColorSchemeIndex = pHouse->ColorSchemeIndex;
						auto const Palettes = pType->Palette;
						ColorScheme* const pColor = Palettes ? Palettes->Items[ColorSchemeIndex] : ColorScheme::Array->Items[ColorSchemeIndex];

						DSurface::Temp->DrawSHP(pColor->LightConvert, pImage, imageFrame, &point, &rect, blitFlags, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
					}
				}
			}

			if (pHouseExt->OwnedDeployingUnits.size() > 0)
			{
				for (auto const& pUnit : pHouseExt->OwnedDeployingUnits)
				{
					if (pUnit && pUnit->Type)
					{
						if (BuildingTypeClass* const pType = pUnit->Type->DeploysInto)
						{
							CellStruct displayCell = CellClass::Coord2Cell(pUnit->GetCoords()); // pUnit->GetMapCoords();

							if (pType->GetFoundationWidth() > 2 || pType->GetFoundationHeight(false) > 2)
								displayCell -= CellStruct { 1, 1 };

							if (const CellClass* const pCell = MapClass::Instance->TryGetCellAt(displayCell))
							{
								int imageFrame = 0;
								SHPStruct* pImage = pType->LoadBuildup();

								if (pImage)
									imageFrame = ((pImage->Frames / 2) - 1);
								else
									pImage = pType->GetImage();

								if (pImage)
								{
									BlitterFlags blitFlags = BlitterFlags::TransLucent50 | BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;
									RectangleStruct rect = DSurface::Temp->GetRect();
									rect.Height -= 32;
									Point2D point = TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, (1 + pCell->GetFloorHeight(Point2D::Empty)))).first;
									point.Y -= 15;

									const int ColorSchemeIndex = pUnit->Owner->ColorSchemeIndex;
									auto const Palettes = pType->Palette;
									ColorScheme* const pColor = Palettes ? Palettes->Items[ColorSchemeIndex] : ColorScheme::Array->Items[ColorSchemeIndex];

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

	return (RulesExt::Global()->ExpandBuildingPlace && (BuildingTypeExt::BuildLimboBuilding(pBuilding) || (pBuilding->Type->PowersUpBuilding[0] && BuildingTypeExt::AutoUpgradeBuilding(pBuilding)))) ? SkipSetStripShortCut : 0;
}

// Limbo Build Hook -> sub_42EB50 - Check Base Node
DEFINE_HOOK(0x42EB8E, BaseClass_GetBaseNodeIndex_CheckValidBaseNode, 0x6)
{
	enum { Valid = 0x42EBC3, Invalid = 0x42EBAE };

	GET(BaseClass* const, pBase, ESI);
	GET(BaseNodeClass* const, pBaseNode, EAX);

	if (pBaseNode->Placed)
	{
		const int index = pBaseNode->BuildingTypeIndex;

		if (index >= 0 && index < BuildingTypeClass::Array->Count && BuildingTypeExt::ExtMap.Find(BuildingTypeClass::Array->Items[index])->LimboBuild)
			return Invalid;
	}

	return reinterpret_cast<bool(__thiscall*)(HouseClass*, BaseNodeClass*)>(0x50CAD0)(pBase->Owner, pBaseNode) ? Valid : Invalid;
}
