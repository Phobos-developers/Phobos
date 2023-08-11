// Anim-to--Unit
// Author: Otamaa, revisions by Starkku

#include "Body.h"

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

		pThis->UnmarkAllOccupationBits(location);

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
			int z = pTypeExt->CreateUnit_AlwaysSpawnOnGround ? INT32_MIN : pThis->GetCoords().Z;
			location.Z = Math::max(MapClass::Instance->GetCellFloorHeight(location) + bridgeZ, z);

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

				if (!pCell->GetBuilding())
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
					auto const loc = pTechno->Location;

					if (pTypeExt->CreateUnit_SpawnAnim.isset())
					{
						const auto pAnimType = pTypeExt->CreateUnit_SpawnAnim.Get();

						if (auto const pAnim = GameCreate<AnimClass>(pAnimType, location))
						{
							pAnim->Owner = pThis->Owner;

							if (auto const pAnimExt = AnimExt::ExtMap.Find(pAnim))
							{
								pAnimExt->Invoker = pInvoker;
								pAnimExt->InvokerHouse = pInvokerHouse;
							}
						}
					}

					if (pTechno->HasTurret() && pExt->FromDeathUnit && pExt->DeathUnitHasTurret && pTypeExt->CreateUnit_InheritTurretFacings)
					{
						pTechno->SecondaryFacing.SetCurrent(pExt->DeathUnitTurretFacing);
						Debug::Log("CreateUnit: Using stored turret facing %d\n", pExt->DeathUnitTurretFacing.GetFacing<256>());
					}

					if (!pTechno->InLimbo)
					{
						if (pThis->IsInAir() && !pTypeExt->CreateUnit_AlwaysSpawnOnGround)
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
								}
								else
								{
									// Order non-BalloonHover jumpjets to land.
									pJJLoco->Move_To(location);
								}
							}
							else
							{
								pTechno->IsFallingDown = true;
							}
						}

						pTechno->QueueMission(pTypeExt->CreateUnit_Mission.Get(), false);
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

DEFINE_HOOK(0x469C98, BulletClass_DetonateAt_DamageAnimSelected, 0x0)
{
	enum { Continue = 0x469D06, NukeWarheadExtras = 0x469CAF };

	GET(BulletClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);

	if (pAnim)
	{
		auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);

		HouseClass* pInvoker = (pThis->Owner) ? pThis->Owner->Owner : nullptr;
		HouseClass* pVictim = nullptr;

		if (TechnoClass* Target = generic_cast<TechnoClass*>(pThis->Target))
			pVictim = Target->Owner;

		if (auto unit = pTypeExt->CreateUnit.Get())
		{
			AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);
		}
		else if (!pAnim->Owner)
		{
			auto const pExt = BulletExt::ExtMap.Find(pThis);
			pAnim->Owner = pThis->Owner ? pThis->Owner->Owner : pExt->FirerHouse;
		}

		if (pThis->Owner)
		{
			auto pExt = AnimExt::ExtMap.Find(pAnim);
			pExt->SetInvoker(pThis->Owner);
		}
	}
	else if (pThis->WH == RulesClass::Instance->NukeWarhead)
	{
		return NukeWarheadExtras;
	}

	return Continue;
}

DEFINE_HOOK(0x6E2368, ActionClass_PlayAnimAt, 0x7)
{
	GET(AnimClass*, pAnim, EAX);
	GET_STACK(HouseClass*, pHouse, STACK_OFFSET(0x18, 0x4));

	if (pAnim)
	{
		auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);

		if (auto unit = pTypeExt->CreateUnit.Get())
			AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, pHouse, pHouse);
		else if (!pAnim->Owner && pHouse)
			pAnim->Owner = pHouse;
	}

	return 0;
}
