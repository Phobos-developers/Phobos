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

			if (absType == AbstractType::Infantry || absType == AbstractType::Unit || absType == AbstractType::Aircraft)
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
			else if (absType == AbstractType::Building)
			{
				BuildingClass* const pBuilding = static_cast<BuildingClass*>(pCellObject);
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
				{
					pBuilding->KillOccupants(nullptr);
					pBuilding->Stun();
					pBuilding->Limbo();
					pBuilding->UnInit();
				}
			}
			else if (absType == AbstractType::Terrain)
			{
				TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pCellObject);
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

/*
DisplayClass:
unknown_1180 -> CurrentFoundation_InAdjacent
When the cell which mouse is pointing at has changed, new command has given or try to click to place the current building

unknown_1181 -> CurrentFoundation_NoShrouded
When the cell which mouse is pointing at has changed or new command has given

CurrentFoundationCopy_CenterCell
When the left mouse button release, move the CurrentFoundation_CenterCell to here and clear the CurrentFoundation_CenterCell, and move itself back to CurrentFoundation_CenterCell if place failed

CurrentFoundationCopy_TopLeftOffset
When the left mouse button release, move the CurrentFoundation_TopLeftOffset to here and clear the CurrentFoundation_TopLeftOffset, and move itself back to CurrentFoundation_TopLeftOffset if place failed

CurrentFoundationCopy_Data
When the left mouse button release, move the CurrentFoundation_Data to here and clear the CurrentFoundation_Data, and move itself back to CurrentFoundation_Data if place failed

unknown_1190 -> CurrentBuilding_Buffer
When the left mouse button release, move the CurrentBuilding to here and clear the CurrentBuilding, and move itself back to CurrentBuilding if place failed, otherwise it will clear itself

unknown_1194 -> CurrentBuildingType_Buffer
When the left mouse button release, move the CurrentBuildingType to here and clear the CurrentBuildingType, and move itself back to CurrentBuildingType if place failed, otherwise it will clear itself

unknown_1198 -> CurrentBuildingTypeArrayIndexCopy
When the left mouse button release, move the unknown_11AC to here and clear the unknown_11AC, and move itself back to unknown_11AC if place failed

unknown_11AC -> CurrentBuildingOwnerHouseArrayIndex
When the building type cameo clicked, this record the ArrayIndex of the owner house of the building product

CellClass:
AltFlags = AltCellFlags::Unknown_4 -> InBuildingProcess
Vanilla only between AddPlaceEvent and RespondToEvent
*/

// BaseNormal for units Hook #1 -> sub_4A8EB0 - Rewrite and add functions in
DEFINE_HOOK(0x4A8F21, MapClass_PassesProximityCheck_BaseNormalExtra, 0x9)
{
	enum { CheckCompleted = 0x4A904E };

	GET(const CellStruct* const, pFoundationTopLeft, EDI);
	GET(const BuildingTypeClass* const, pBuildingType, ESI);
	GET_STACK(const int, idxHouse, STACK_OFFSET(0x30, 0x8));

	const bool differentColor = RulesExt::Global()->CheckExpandPlaceGrid;
	bool isInAdjacent = false;
	auto& vec = HouseExt::ExtMap.Find(HouseClass::CurrentPlayer)->BaseNormalCells;

	if (differentColor)
		vec.clear();

	if (Game::IsActive)
	{
		const short foundationWidth = pBuildingType->GetFoundationWidth();
		const short foundationHeight = pBuildingType->GetFoundationHeight(false);
		const short topLeftX = pFoundationTopLeft->X;
		const short topLeftY = pFoundationTopLeft->Y;
		const short bottomRightX = topLeftX + foundationWidth;
		const short bottomRightY = topLeftY + foundationHeight;

		const short buildingAdjacent = static_cast<short>(pBuildingType->Adjacent + 1);
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
								BuildingClass* const pBuilding = static_cast<BuildingClass*>(pObject);

								if (HouseClass* const pOwner = pBuilding->Owner)
								{
									if (pOwner->ArrayIndex == idxHouse && pBuilding->Type->BaseNormal)
									{
										do
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

												if (const DummyAresBuildingExt* pAresBuildingExt = reinterpret_cast<DummyBuildingClass*>(pBuilding)->align_71C)
												{
													baseNormal = !pAresBuildingExt->unknownExtBool;
													break;
												}
											}

											baseNormal = true;
										}
										while (false);
									}
									else if (RulesClass::Instance->BuildOffAlly && pOwner->IsAlliedWith(HouseClass::Array->Items[idxHouse]) && pBuilding->Type->EligibileForAllyBuilding)
									{
										baseNormal = true;
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
			else if (absType == AbstractType::Terrain)
			{
				if (TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pObject))
				{
					auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

					if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
						return CanNotExistHere;
				}
			}

			pObject = pObject->NextObject;
		}
	}
	else if (pBuildingType->LaserFencePost || pBuildingType->Gate)
	{
		ObjectClass* pObject = pCell->FirstObject;

		while (pObject)
		{
			const AbstractType absType = pObject->WhatAmI();

			if (absType == AbstractType::Aircraft)
			{
				AircraftClass* const pAircraft = static_cast<AircraftClass*>(pObject);
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pAircraft->Type);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Building)
			{
				BuildingClass* const pBuilding = static_cast<BuildingClass*>(pObject);
				BuildingTypeClass* const pType = pBuilding->Type;
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

				if ((!pTypeExt || !pTypeExt->CanBeBuiltOn) && (pOwner != pBuilding->Owner || !pType->LaserFence))
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
			{
				TechnoClass* const pTechno = static_cast<TechnoClass*>(pObject);
				TechnoTypeClass* const pTechnoType = pTechno->GetTechnoType();
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
				{
					if (!expand || pTechnoType->Speed <= 0 || !BuildingTypeExt::CheckOccupierCanLeave(pOwner, pTechno->Owner))
						return CanNotExistHere;
					else
						landFootOnly = true;
				}
			}
			else if (absType == AbstractType::Terrain)
			{
				if (TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pObject))
				{
					auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

					if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
						return CanNotExistHere;
				}
			}

			pObject = pObject->NextObject;
		}
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
			else if (absType == AbstractType::Terrain)
			{
				if (TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pObject))
				{
					auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

					if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
						return CanNotExistHere;
				}
			}

			pObject = pObject->NextObject;
		}
	}
	else
	{
		ObjectClass* pObject = pCell->FirstObject;

		while (pObject)
		{
			const AbstractType absType = pObject->WhatAmI();

			if (absType == AbstractType::Aircraft || absType == AbstractType::Building)
			{
				TechnoClass* const pTechno = static_cast<TechnoClass*>(pObject);
				TechnoTypeClass* const pTechnoType = pTechno->GetTechnoType();
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
			{
				TechnoClass* const pTechno = static_cast<TechnoClass*>(pObject);
				TechnoTypeClass* const pTechnoType = pTechno->GetTechnoType();
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
				{
					if (!expand || pTechnoType->Speed <= 0 || !BuildingTypeExt::CheckOccupierCanLeave(pOwner, pTechno->Owner))
						return CanNotExistHere;
					else
						landFootOnly = true;
				}
			}
			else if (absType == AbstractType::Terrain)
			{
				if (TerrainClass* const pTerrain = abstract_cast<TerrainClass*>(pObject))
				{
					auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

					if (!pTypeExt || !pTypeExt->CanBeBuiltOn)
						return CanNotExistHere;
				}
			}

			pObject = pObject->NextObject;
		}
	}

	if (landFootOnly)
		BuildOnOccupiersHelpers::Exist = true;

	return CanExistHere; // Continue check the overlays .etc
}

// Buildable-upon TechnoTypes Hook #2-1 -> sub_47EC90 - Record cell before draw it
DEFINE_HOOK(0x47EEBC, CellClass_DrawPlaceGrid_RecordCell, 0x6)
{
	GET(CellClass* const, pCell, ESI);

	if (RulesExt::Global()->CheckExpandPlaceGrid)
		BuildOnOccupiersHelpers::CurrentCell = pCell;

	return 0;
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

	for (auto const& baseCell : HouseExt::ExtMap.Find(HouseClass::CurrentPlayer)->BaseNormalCells)
	{
		if (baseCell.X >= minX && baseCell.Y >= minY && baseCell.X <= maxX && baseCell.Y <= maxY)
		{
			green = true;
			break;
		}
	}

	const bool foot = BuildOnOccupiersHelpers::Exist;
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
	enum { CanBuild = 0x4FB23C, TemporarilyCanNotBuild = 0x4FB5BA, CanNotBuild = 0x4FB35F };

	GET(HouseClass* const, pHouse, EBP);
	GET(TechnoClass* const, pTechno, ESI);
	GET(BuildingClass* const, pFactory, EDI);
	GET_STACK(const CellStruct, topLeftCell, STACK_OFFSET(0x3C, 0x10));

	if (pTechno->WhatAmI() == AbstractType::Building && RulesExt::Global()->ExpandBuildingPlace)
	{
		BuildingClass* const pBuilding = static_cast<BuildingClass*>(pTechno);
		BuildingTypeClass* const pBuildingType = pBuilding->Type;
		HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse);

		if (!pBuildingType->PlaceAnywhere && !pBuildingType->PowersUpBuilding[0])
		{
			bool canBuild = true;
			bool noOccupy = true;

			for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
			{
				CellStruct currentCoord = topLeftCell + *pFoundation;
				CellClass* const pCell = MapClass::Instance->GetCellAt(currentCoord);

				if (!pCell || !pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pHouse))
					canBuild = false;
				else if (BuildOnOccupiersHelpers::Exist)
					noOccupy = false;
			}

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

						if (pHouse == HouseClass::CurrentPlayer)
						{
							if (DisplayClass::Instance->unknown_1194)
								DisplayClass::Instance->SetActiveFoundation(nullptr);

							reinterpret_cast<void(__thiscall*)(DisplayClass*, CellStruct*)>(0x4A8D50)(DisplayClass::Instance, nullptr); // Clear CurrentFoundation_Data
							DisplayClass::Instance->unknown_1190 = 0;
							DisplayClass::Instance->unknown_1194 = 0;
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

	pFactory->SendCommand(RadioCommand::RequestLink, pTechno);

	if (pTechno->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
		return CanBuild;

	BuildOnOccupiersHelpers::Mouse = true;
	return CanNotBuild;
}

// Buildable-upon TechnoTypes Hook #4 -> sub_4FB0E0 - Check whether need to skip the replace command
DEFINE_HOOK(0x4FB395, HouseClass_UnitFromFactory_SkipMouseReturn, 0x6)
{
	if (!RulesExt::Global()->ExpandBuildingPlace)
		return 0;

	if (BuildOnOccupiersHelpers::Mouse)
	{
		BuildOnOccupiersHelpers::Mouse = false;
		return 0;
	}

	R->EBX(0);
	return 0x4FB489;
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
	enum { CanBuild = 0x4452F0, TemporarilyCanNotBuild = 0x445237, CanNotBuild = 0x4454E6 };

	GET(BuildingClass* const, pBuilding, EDI);
	GET(BuildingClass* const, pFactory, ESI);
	GET(const CellStruct, topLeftCell, EDX);

	if (!RulesExt::Global()->ExpandBuildingPlace)
		return 0;

	BuildingTypeClass* const pBuildingType = pBuilding->Type;

	if (topLeftCell != Make_Global<CellStruct>(0x89C8B0) && !pBuildingType->PlaceAnywhere)
	{
		HouseClass* const pHouse = pFactory->Owner;

		if (!pBuildingType->PowersUpBuilding[0])
		{
			HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse);
			bool canBuild = true;
			bool noOccupy = true;

			for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
			{
				CellStruct currentCoord = topLeftCell + *pFoundation;
				CellClass* const pCell = MapClass::Instance->GetCellAt(currentCoord);

				if (!pCell || !pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pHouse))
					canBuild = false;
				else if (BuildOnOccupiersHelpers::Exist)
					noOccupy = false;
			}

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

			pHouseExt->CurrentBuildingType = nullptr;
			pHouseExt->CurrentBuildingTimes = 0;
			pHouseExt->CurrentBuildingTopLeft = CellStruct::Empty;
			pHouseExt->CurrentBuildingTimer.Stop();
		}
		else if (CellClass* const pCell = MapClass::Instance->GetCellAt(topLeftCell))
		{
			BuildingClass* const pCellBuilding = pCell->GetBuilding();

			if (!reinterpret_cast<bool(__thiscall*)(BuildingClass*, BuildingTypeClass*, HouseClass*)>(0x452670)(pCellBuilding, pBuildingType, pHouse)) // CanUpgradeBuilding
				return CanNotBuild;
		}
	}

	if (pBuilding->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
		return CanBuild;

	return CanNotBuild;
}

// Laser fence use GetBuilding to check whether can build and draw, so no need to change
// Buildable-upon TechnoTypes Hook #8-1 -> sub_6D5C50 - Don't draw overlay wall grid when have occupiers
DEFINE_HOOK(0x6D5D38, TacticalClass_DrawOverlayWallGrid_DisableWhenHaveTechnos, 0x8)
{
	GET(bool, valid, EAX);
	return (!valid || BuildOnOccupiersHelpers::Exist) ? 0x6D5F0F : 0x6D5D40;
}

// Buildable-upon TechnoTypes Hook #8-2 -> sub_6D59D0 - Don't draw firestorm wall grid when have occupiers
DEFINE_HOOK(0x6D5A9D, TacticalClass_DrawFirestormWallGrid_DisableWhenHaveTechnos, 0x8)
{
	GET(bool, valid, EAX);
	return (!valid || BuildOnOccupiersHelpers::Exist) ? 0x6D5C2F : 0x6D5AA5;
}

// Buildable-upon TechnoTypes Hook #8-3 -> sub_588750 - Don't place overlay wall when have occupiers
DEFINE_HOOK(0x588873, MapClass_BuildingToWall_DisableWhenHaveTechnos, 0x8)
{
	GET(bool, valid, EAX);
	return (!valid || BuildOnOccupiersHelpers::Exist) ? 0x588935 : 0x58887B;
}

// Buildable-upon TechnoTypes Hook #8-4 -> sub_588570 - Don't place firestorm wall when have occupiers
DEFINE_HOOK(0x588664, MapClass_BuildingToFirestormWall_DisableWhenHaveTechnos, 0x8)
{
	GET(bool, valid, EAX);
	return (!valid || BuildOnOccupiersHelpers::Exist) ? 0x588730 : 0x58866C;
}

// Buildable-upon TechnoTypes Hook #9-1 -> sub_7393C0 - Try to clean up the building space when is deploying
DEFINE_HOOK(0x73946C, UnitClass_TryToDeploy_CleanUpDeploySpace, 0x6)
{
	enum { CanDeploy = 0x73958A, TemporarilyCanNotDeploy = 0x73950F, CanNotDeploy = 0x7394E0 };

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

			if (!pCell || !pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pUnit->Owner))
				canBuild = false;
			else if (BuildOnOccupiersHelpers::Exist)
				noOccupy = false;
		}

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

			if (pUnit->CurrentMission == Mission::Guard || pUnit->CurrentMission == Mission::Unload)
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
DEFINE_HOOK(0x4F8DB1, HouseClass_AI_CheckHangUpBuilding, 0x6)
{
	GET(const HouseClass* const, pHouse, ESI);

	if (!pHouse->IsControlledByHuman() || !RulesExt::Global()->ExpandBuildingPlace)
		return 0;

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
			std::vector<TechnoClass*> deleteTechnos;
			auto& vec = pHouseExt->OwnedDeployingUnits;

			for (auto const& pUnit : pHouseExt->OwnedDeployingUnits)
			{
				if (pUnit && !pUnit->InLimbo && pUnit->IsAlive && pUnit->Health && !pUnit->IsSinking && !pUnit->Destination && pUnit->Owner == pHouse
					&& pUnit->Type && pUnit->Type->DeploysInto && pUnit->CurrentMission == Mission::Guard)
				{
					TechnoExt::ExtData* const pTechnoExt = TechnoExt::ExtMap.Find(pUnit);

					if (pTechnoExt && !(pTechnoExt->UnitAutoDeployTimer.GetTimeLeft() % 8))
						pUnit->QueueMission(Mission::Unload, true);
				}
				else
				{
					deleteTechnos.push_back(pUnit);
				}
			}

			for (auto const& pUnit : deleteTechnos)
				vec.erase(std::remove(vec.begin(), vec.end(), pUnit), vec.end());
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

			if (BuildingTypeClass* const pType = pHouseExt->CurrentBuildingType)
			{
				if (const CellClass* const pCell = MapClass::Instance->TryGetCellAt(pHouseExt->CurrentBuildingTopLeft))
				{
					SHPStruct* pImage = pType->LoadBuildup();
					int imageFrame = 0;

					if (pImage)
						imageFrame = ((pImage->Frames / 2) - 1);
					else
						pImage = pType->GetImage();

					if (pImage)
					{
						BlitterFlags blitFlags = TranslucencyLevel(75) | BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;
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
									BlitterFlags blitFlags = TranslucencyLevel(75) | BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;
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
