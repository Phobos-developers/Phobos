// Anim-to--Unit
// Author: Otamaa, revisions by Starkku

#include "Body.h"

#include <AircraftTrackerClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <JumpjetLocomotionClass.h>
#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/AnimType/Body.h>

DEFINE_HOOK(0x737F6D, UnitClass_TakeDamage_Destroy, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, Receivedamageargs, STACK_OFFSET(0x44, 0x4));

	R->ECX(R->ESI());
	TechnoExt::ExtMap.Find(pThis)->ReceiveDamage = true;
	AnimTypeExt::ProcessDestroyAnims(pThis, Receivedamageargs.Attacker);
	pThis->Destroy();

	return 0x737F74;
}

DEFINE_HOOK(0x738807, UnitClass_Destroy_DestroyAnim, 0x8)
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExt::ExtMap.Find(pThis);

	if (!Extension->ReceiveDamage)
		AnimTypeExt::ProcessDestroyAnims(pThis);

	return 0x73887E;
}

// Performance tweak, mark once instead of every frame.
// DEFINE_HOOK(0x423BC8, AnimClass_AI_CreateUnit_MarkOccupationBits, 0x6)
DEFINE_HOOK(0x4226F0, AnimClass_CTOR_CreateUnit_MarkOccupationBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->CreateUnit.Get())
	{
		auto location = pThis->GetCoords();

		if (auto pCell = pThis->GetCell())
			location = pCell->GetCoordsWithBridge();
		else
			location.Z = MapClass::Instance->GetCellFloorHeight(location);

		pThis->MarkAllOccupationBits(location);
	}

	return 0; //return (pThis->Type->MakeInfantry != -1) ? 0x423BD6 : 0x423C03;
}

DEFINE_HOOK(0x424932, AnimClass_AI_CreateUnit_ActualAffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (auto unit = pTypeExt->CreateUnit.Get())
	{
		HouseClass* decidedOwner = pThis->Owner && !pThis->Owner->Defeated
			? pThis->Owner : HouseClass::FindCivilianSide();

		auto pCell = pThis->GetCell();
		CoordStruct location = pThis->GetCoords();
		auto origLocation = location;

		if (pCell)
			origLocation = pCell->GetCoordsWithBridge();
		else
			origLocation.Z = MapClass::Instance->GetCellFloorHeight(location);

		pThis->UnmarkAllOccupationBits(origLocation);

		bool allowBridges = GroundType::Array[static_cast<int>(LandType::Clear)].Cost[static_cast<int>(unit->SpeedType)] > 0.0;
		bool isBridge = allowBridges && pCell->ContainsBridge();

		if (pTypeExt->CreateUnit_ConsiderPathfinding && (!pCell || !pCell->IsClearToMove(unit->SpeedType, false, false, -1, unit->MovementZone, -1, isBridge)) )
		{
			auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(location),
				unit->SpeedType, -1, unit->MovementZone, isBridge, 1, 1, true,
				false, false, isBridge, CellStruct::Empty, false, false);

			pCell = MapClass::Instance->TryGetCellAt(nCell);
			location = pCell->GetCoords();
		}

		if (pCell)
		{
			isBridge = allowBridges && pCell->ContainsBridge();
			int bridgeZ = isBridge ? CellClass::BridgeHeight : 0;
			int baseHeight = pThis->GetCoords().Z;
			int zCoord = pTypeExt->CreateUnit_AlwaysSpawnOnGround ? INT32_MIN : baseHeight;
			int cellFloorHeight = MapClass::Instance->GetCellFloorHeight(location) + bridgeZ;

			if (!pTypeExt->CreateUnit_AlwaysSpawnOnGround && pTypeExt->CreateUnit_SpawnHeight.isset())
				location.Z = cellFloorHeight + pTypeExt->CreateUnit_SpawnHeight;
			else
				location.Z = Math::max(cellFloorHeight, zCoord);

			if (auto pTechno = static_cast<FootClass*>(unit->CreateObject(decidedOwner)))
			{
				bool success = false;
				auto const pExt = AnimExt::ExtMap.Find(pThis);
				TechnoClass* pInvoker = pExt->Invoker;
				HouseClass* pInvokerHouse = pExt->InvokerHouse;

				auto facing = pTypeExt->CreateUnit_RandomFacing
					? static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255)) : pTypeExt->CreateUnit_Facing;

				auto resultingFacing = pTypeExt->CreateUnit_InheritDeathFacings && pExt->FromDeathUnit ? pExt->DeathUnitFacing : facing;
				pTechno->OnBridge = isBridge;

				bool inAir = pThis->IsOnMap && location.Z >= Unsorted::CellHeight * 2;
				bool parachuted = false;

				if (pTypeExt->CreateUnit_SpawnParachutedInAir && !pTypeExt->CreateUnit_AlwaysSpawnOnGround && inAir)
				{
					parachuted = true;
					success = pTechno->SpawnParachuted(location);
				}
				else if (!pCell->GetBuilding() || !pTypeExt->CreateUnit_ConsiderPathfinding)
				{
					++Unsorted::IKnowWhatImDoing;
					success = pTechno->Unlimbo(location, resultingFacing);
					--Unsorted::IKnowWhatImDoing;
				}
				else
				{
					success = pTechno->Unlimbo(location, resultingFacing);
				}

				if (success)
				{
					if (auto const pAnimType = pTypeExt->CreateUnit_SpawnAnim)
					{
						if (auto const pAnim = GameCreate<AnimClass>(pAnimType, location))
						{
							AnimExt::SetAnimOwnerHouseKind(pAnim, pInvokerHouse, nullptr, false, true);

							if (auto const pAnimExt = AnimExt::ExtMap.Find(pAnim))
								pAnimExt->SetInvoker(pInvoker, pInvokerHouse);
						}
					}

					if (pTechno->HasTurret() && pExt->FromDeathUnit && pExt->DeathUnitHasTurret && pTypeExt->CreateUnit_InheritTurretFacings)
					{
						pTechno->SecondaryFacing.SetCurrent(pExt->DeathUnitTurretFacing);
						Debug::Log("CreateUnit: Using stored turret facing %d\n", pExt->DeathUnitTurretFacing.GetFacing<256>());
					}

					if (!pTechno->InLimbo)
					{
						if (!pTypeExt->CreateUnit_AlwaysSpawnOnGround)
						{
							if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pTechno->Locomotor))
							{
								auto const pType = pTechno->GetTechnoType();
								pJJLoco->LocomotionFacing.SetCurrent(DirStruct(static_cast<DirType>(resultingFacing)));

								if (pType->BalloonHover)
								{
									// Makes the jumpjet think it is hovering without actually moving.
									pJJLoco->State = JumpjetLocomotionClass::State::Hovering;
									pJJLoco->IsMoving = true;
									pJJLoco->DestinationCoords = location;
									pJJLoco->CurrentHeight = pType->JumpjetHeight;

									if (!inAir)
										AircraftTrackerClass::Instance->Add(pTechno);
								}
								else if (inAir)
								{
									// Order non-BalloonHover jumpjets to land.
									pJJLoco->Move_To(location);
								}
							}
							else if (inAir && !parachuted)
							{
								pTechno->IsFallingDown = true;
							}
						}

						auto mission = pTypeExt->CreateUnit_Mission;

						if (!decidedOwner->IsControlledByHuman())
							mission = pTypeExt->CreateUnit_AIMission.Get(mission);

						pTechno->QueueMission(mission, false);
					}

					if (!decidedOwner->Type->MultiplayPassive)
						decidedOwner->RecheckTechTree = true;
				}
				else
				{
					if (pTechno)
						pTechno->UnInit();
				}
			}
		}
	}

	return (pThis->Type->MakeInfantry != -1) ? 0x42493E : 0x424B31;
}
