#include <AircraftClass.h>
#include <FlyLocomotionClass.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/Macro.h>

#pragma region Mission_Attack

DEFINE_HOOK(0x417FF1, AircraftClass_Mission_Attack_StrafeShots, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	int weaponIndex = pExt->CurrentAircraftWeaponIndex;

	if (weaponIndex < 0)
	{
		weaponIndex = pThis->SelectWeapon(pThis->Target);
		pExt->CurrentAircraftWeaponIndex = weaponIndex;
	}

	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
		|| pThis->MissionStatus >(int)AirAttackStatus::FireAtTarget5_Strafe)
	{
		pExt->Strafe_BombsDroppedThisRound = 0;
		return 0;
	}

	if (pThis->Is_Strafe() && pWeaponExt->Strafing_UseAmmoPerShot && pExt->Strafe_BombsDroppedThisRound)
	{
		pThis->Ammo--;
		pThis->ShouldLoseAmmo = false;

		if (!pThis->Ammo)
		{
			pThis->IsLocked = false;
			pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;

			return 0;
		}
	}

	int fireCount = pThis->MissionStatus - 4;
	int strafingShots = pWeaponExt->Strafing_Shots.Get(5);

	if (strafingShots > 5)
	{
		if (pThis->MissionStatus == (int)AirAttackStatus::FireAtTarget3_Strafe)
		{
			int remainingShots = strafingShots - 3 - pExt->Strafe_BombsDroppedThisRound;

			if (remainingShots > 0)
				pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
		}
	}
	else if (fireCount > 1 && strafingShots < fireCount)
	{
		if (!pThis->Ammo)
			pThis->IsLocked = false;

		pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
	}

	return 0;
}

// If strafing weapon target is in air, consider the cell it is on as the firing position instead of the object itself if can fire at it.
DEFINE_HOOK(0x4197F3, AircraftClass_GetFireLocation_Strafing, 0x5)
{
	GET(AircraftClass*, pThis, EDI);
	GET(AbstractClass*, pTarget, EAX);

	if (!pTarget)
		return 0;

	auto const pObject = abstract_cast<ObjectClass*>(pTarget);

	if (!pObject || !pObject->IsInAir())
		return 0;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	int weaponIndex = pExt->CurrentAircraftWeaponIndex;

	if (weaponIndex < 0)
		weaponIndex = pThis->SelectWeapon(pTarget);

	auto fireError = pThis->GetFireError(pTarget, weaponIndex, false);

	if (fireError == FireError::ILLEGAL || fireError == FireError::CANT)
		return 0;

	R->EAX(MapClass::Instance->GetCellAt(pObject->GetCoords()));

	return 0;
}

long __stdcall AircraftClass_IFlyControl_IsStrafe(IFlyControl const* ifly)
{
	__assume(ifly != nullptr);

	auto const pThis = static_cast<AircraftClass const*>(ifly);
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	WeaponTypeClass* pWeapon = nullptr;

	if (pExt->CurrentAircraftWeaponIndex >= 0)
		pWeapon = pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
	else if (pThis->Target)
		pWeapon = pThis->GetWeapon(pThis->SelectWeapon(pThis->Target))->WeaponType;

	if (pWeapon)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		return pWeaponExt->Strafing.Get(pWeapon->Projectile->ROT <= 1 && !pWeapon->Projectile->Inviso);
	}

	return false;
}

DEFINE_JUMP(VTABLE, 0x7E2268, GET_OFFSET(AircraftClass_IFlyControl_IsStrafe));

DEFINE_HOOK(0x418403, AircraftClass_Mission_Attack_FireAtTarget_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->ShouldLoseAmmo = true;

	AircraftExt::FireWeapon(pThis, pThis->Target, 0);

	return 0x418478;
}

DEFINE_HOOK(0x4186B6, AircraftClass_Mission_Attack_FireAtTarget2_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireWeapon(pThis, pThis->Target, 0);

	return 0x4186D7;
}

DEFINE_HOOK(0x418805, AircraftClass_Mission_Attack_FireAtTarget2Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireWeapon(pThis, pThis->Target, 1);

	return 0x418826;
}

DEFINE_HOOK(0x418914, AircraftClass_Mission_Attack_FireAtTarget3Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireWeapon(pThis, pThis->Target, 2);

	return 0x418935;
}

DEFINE_HOOK(0x418A23, AircraftClass_Mission_Attack_FireAtTarget4Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireWeapon(pThis, pThis->Target, 3);

	return 0x418A44;
}

DEFINE_HOOK(0x418B1F, AircraftClass_Mission_Attack_FireAtTarget5Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireWeapon(pThis, pThis->Target, 4);

	return 0x418B40;
}

WeaponStruct* __fastcall AircraftClass_GetWeapon_Wrapper(AircraftClass* pThis, void* _, int weaponIndex)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->CurrentAircraftWeaponIndex >= 0)
		return pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex);
	else
		return pThis->GetWeapon(pThis->SelectWeapon(pThis->Target));
}

DEFINE_JUMP(CALL6, 0x4180F9, GET_OFFSET(AircraftClass_GetWeapon_Wrapper));
DEFINE_JUMP(CALL6, 0x4184E3, GET_OFFSET(AircraftClass_GetWeapon_Wrapper));
DEFINE_JUMP(CALL6, 0x41852B, GET_OFFSET(AircraftClass_GetWeapon_Wrapper));
DEFINE_JUMP(CALL6, 0x418893, GET_OFFSET(AircraftClass_GetWeapon_Wrapper));
DEFINE_JUMP(CALL6, 0x4189A2, GET_OFFSET(AircraftClass_GetWeapon_Wrapper));
DEFINE_JUMP(CALL6, 0x418AB1, GET_OFFSET(AircraftClass_GetWeapon_Wrapper));
DEFINE_JUMP(CALL6, 0x418B9A, GET_OFFSET(AircraftClass_GetWeapon_Wrapper));

void __fastcall AircraftClass_SetTarget_Wrapper(AircraftClass* pThis, void* _, AbstractClass* pTarget)
{
	pThis->TechnoClass::SetTarget(pTarget);
	TechnoExt::ExtMap.Find(pThis)->CurrentAircraftWeaponIndex = -1;
}

DEFINE_JUMP(VTABLE, 0x7E266C, GET_OFFSET(AircraftClass_SetTarget_Wrapper));

#pragma endregion

DEFINE_HOOK(0x414F10, AircraftClass_AI_Trailer, 0x5)
{
	enum { SkipGameCode = 0x414F47 };

	GET(AircraftClass*, pThis, ESI);
	GET_STACK(CoordStruct, coords, STACK_OFFSET(0x40, -0xC));

	auto const pTrailerAnim = GameCreate<AnimClass>(pThis->Type->Trailer, coords, 1, 1);
	auto const pTrailerAnimExt = AnimExt::ExtMap.Find(pTrailerAnim);
	AnimExt::SetAnimOwnerHouseKind(pTrailerAnim, pThis->Owner, nullptr, false, true);
	pTrailerAnimExt->SetInvoker(pThis);
	pTrailerAnimExt->IsTechnoTrailerAnim = true;

	return SkipGameCode;
}

DEFINE_HOOK(0x414C0B, AircraftClass_ChronoSparkleDelay, 0x5)
{
	R->ECX(RulesExt::Global()->ChronoSparkleDisplayDelay);
	return 0x414C10;
}


#pragma region LandingDir

DEFINE_HOOK(0x4CF31C, FlyLocomotionClass_FlightUpdate_LandingDir, 0x9)
{
	enum { SkipGameCode = 0x4CF351 };

	GET(FootClass**, pLinkedToPtr, ESI);
	REF_STACK(unsigned int, dir, STACK_OFFSET(0x48, 0x8));

	auto const pLinkedTo = *pLinkedToPtr;
	dir = 0;

	if (pLinkedTo->CurrentMission == Mission::Enter || pLinkedTo->GetMapCoords() == CellClass::Coord2Cell(pLinkedTo->Locomotor->Destination()))
	{
		if (auto const pAircraft = abstract_cast<AircraftClass*>(pLinkedTo))
			dir = DirStruct(AircraftExt::GetLandingDir(pAircraft)).Raw;
	}

	return SkipGameCode;
}

namespace SeparateAircraftTemp
{
	BuildingClass* pBuilding = nullptr;
}

DEFINE_HOOK(0x446F57, BuildingClass_GrandOpening_PoseDir_SetContext, 0x6)
{
	GET(BuildingClass*, pThis, EBP);

	SeparateAircraftTemp::pBuilding = pThis;

	return 0;
}

DirType _fastcall AircraftClass_PoseDir_Wrapper(AircraftClass* pThis)
{
	return AircraftExt::GetLandingDir(pThis, SeparateAircraftTemp::pBuilding);
}

DEFINE_JUMP(CALL, 0x446F67, GET_OFFSET(AircraftClass_PoseDir_Wrapper)); // BuildingClass_GrandOpening

DEFINE_HOOK(0x443FC7, BuildingClass_ExitObject_PoseDir1, 0x8)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAircraft, EBP);

	R->EAX(AircraftExt::GetLandingDir(pAircraft, pThis));

	return 0;
}

DEFINE_HOOK(0x44402E, BuildingClass_ExitObject_PoseDir2, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAircraft, EBP);

	auto dir = DirStruct(AircraftExt::GetLandingDir(pAircraft, pThis));
	// pAircraft->PrimaryFacing.SetCurrent(dir);
	pAircraft->SecondaryFacing.SetCurrent(dir);

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x415EEE, AircraftClass_Fire_KickOutPassengers, 0x6)
{
	enum { SkipKickOutPassengers = 0x415F08 };

	GET(AircraftClass*, pThis, EDI);
	GET_BASE(int, weaponIdx, 0xC);

	auto const pWeapon = pThis->GetWeapon(weaponIdx)->WeaponType;

	if (!pWeapon)
		return 0;

	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeaponExt->KickOutPassengers)
		return 0;

	return SkipKickOutPassengers;
}

// Aircraft mission hard code are all disposable that no ammo, target died or arrived destination all will call the aircraft return airbase
#pragma region AircraftMissionExpand

// Waypoint: enable and smooth moving action
bool __fastcall AircraftTypeClass_CanUseWaypoint(AircraftTypeClass* pThis)
{
	return RulesExt::Global()->ExtendedAircraftMissions.Get();
}
DEFINE_JUMP(VTABLE, 0x7E2908, GET_OFFSET(AircraftTypeClass_CanUseWaypoint))

// KickOut: skip useless tether
DEFINE_JUMP(LJMP, 0x444021, 0x44402E)

// Move: smooth the planning paths and returning route
DEFINE_HOOK_AGAIN(0x4168C7, AircraftClass_Mission_Move_SmoothMoving, 0x5)
DEFINE_HOOK(0x416A0A, AircraftClass_Mission_Move_SmoothMoving, 0x5)
{
	enum { EnterIdleAndReturn = 0x416AC0, ContinueMoving1 = 0x416908, ContinueMoving2 = 0x416A47 };

	GET(AircraftClass* const, pThis, ESI);
	GET(CoordStruct* const, pCoords, EAX);

	if (!RulesExt::Global()->ExtendedAircraftMissions)
		return 0;

	const auto pType = pThis->Type;

	if (!pType->AirportBound || pThis->Team || pThis->Airstrike || pThis->Spawned)
		return 0;

	const int distance = Game::F2I(Point2D { pCoords->X, pCoords->Y }.DistanceFrom(Point2D { pThis->Location.X, pThis->Location.Y }));

	if (distance > std::max((pType->SlowdownDistance >> 1), (2048 / pType->ROT)))
		return (R->Origin() == 0x4168C7 ? ContinueMoving1 : ContinueMoving2);

	pThis->EnterIdleMode(false, true);

	return EnterIdleAndReturn;
}

// AreaGuard: return when no ammo or first target died
DEFINE_HOOK_AGAIN(0x41A982, AircraftClass_Mission_AreaGuard, 0x6)
DEFINE_HOOK(0x41A96C, AircraftClass_Mission_AreaGuard, 0x6)
{
	enum { SkipGameCode = 0x41A97A };

	GET(AircraftClass* const, pThis, ESI);

	if (RulesExt::Global()->ExtendedAircraftMissions && !pThis->Team && pThis->Ammo && pThis->IsArmed())
	{
		auto coords = pThis->GetCoords();

		if (pThis->TargetAndEstimateDamage(coords, ThreatType::Area))
		{
			pThis->QueueMission(Mission::Attack, false);
			return SkipGameCode;
		}
	}

	return 0;
}

// AttackMove: return when no ammo or arrived destination
bool __fastcall AircraftTypeClass_CanAttackMove(AircraftTypeClass* pThis)
{
	return RulesExt::Global()->ExtendedAircraftMissions.Get();
}
DEFINE_JUMP(VTABLE, 0x7E290C, GET_OFFSET(AircraftTypeClass_CanAttackMove))

DEFINE_HOOK(0x6FA68B, TechnoClass_Update_AttackMovePaused, 0xA) // To make aircrafts not search for targets while resting at the airport, this is designed to adapt to loop waypoint
{
	enum { SkipGameCode = 0x6FA6F5 };

	GET(TechnoClass* const, pThis, ESI);

	return (RulesExt::Global()->ExtendedAircraftMissions && pThis->WhatAmI() == AbstractType::Aircraft && (!pThis->Ammo || pThis->GetHeight() < Unsorted::CellHeight)) ? SkipGameCode : 0;
}

DEFINE_HOOK(0x4DF3BA, FootClass_UpdateAttackMove_AircraftHoldAttackMoveTarget1, 0x6)
{
	enum { LoseTarget = 0x4DF3D3, HoldTarget = 0x4DF4AB };

	GET(FootClass* const, pThis, ESI);

	return ((RulesExt::Global()->ExtendedAircraftMissions && pThis->WhatAmI() == AbstractType::Aircraft) || pThis->vt_entry_3B4(reinterpret_cast<DWORD>(pThis->Target))) ? HoldTarget : LoseTarget; // pThis->InAuxiliarySearchRange(pThis->Target)
}

DEFINE_HOOK(0x4DF42A, FootClass_UpdateAttackMove_AircraftHoldAttackMoveTarget2, 0x6)
{
	enum { ContinueCheck = 0x4DF462, HoldTarget = 0x4DF4AB };

	GET(FootClass* const, pThis, ESI);

	return (RulesExt::Global()->ExtendedAircraftMissions && pThis->WhatAmI() == AbstractType::Aircraft) ? HoldTarget : ContinueCheck;
}

DEFINE_HOOK(0x418CD1, AircraftClass_Mission_Attack_ContinueFlyToDestination, 0x6)
{
	enum { Continue = 0x418C43, Return = 0x418CE8 };

	GET(AircraftClass* const, pThis, ESI);

	if (!pThis->Target)
	{
		if (!RulesExt::Global()->ExtendedAircraftMissions || !pThis->vt_entry_4C4() || !pThis->unknown_5C8) // (!pThis->MegaMissionIsAttackMove() || !pThis->MegaDestination)
			return Continue;

		pThis->SetDestination(reinterpret_cast<AbstractClass*>(pThis->unknown_5C8), false); // pThis->MegaDestination
		pThis->QueueMission(Mission::Move, true);
		pThis->unknown_bool_5D1 = false; // pThis->HaveAttackMoveTarget
	}
	else
	{
		pThis->MissionStatus = 1;
	}

	R->EAX(1);
	return Return;
}

// Idle: clear the target if no ammo
DEFINE_HOOK(0x414D4D, AircraftClass_Update_ClearTargetIfNoAmmo, 0x6)
{
	enum { ClearTarget = 0x414D3F };

	GET(AircraftClass* const, pThis, ESI);

	if (RulesExt::Global()->ExtendedAircraftMissions && !pThis->Ammo && !pThis->Airstrike && !pThis->Spawned)
	{
		if (!SessionClass::IsCampaign()) // To avoid AI's aircrafts team repeatedly attempting to attack the target when no ammo
		{
			if (const auto pTeam = pThis->Team)
				pTeam->LiberateMember(pThis);
		}

		return ClearTarget;
	}

	return 0;
}
/*
// This allows the aircraft to receive stop commands even at low altitudes, but if used, receiving a move command
// then quickly receiving a stop command can result in the aircraft landing in an incorrect direction
DEFINE_HOOK(0x417944, AircraftClass_EnterIdleMode_DockCheck, 0x7)
{
	enum { Continue = 0x417953, Invalid = 0x417AD4 };

	GET(AircraftClass* const, pThis, ESI);

	return (pThis->GetHeight() > 0) ? Continue : Invalid; // Replace IsInAir()
}
*/

// Stop: clear the mega mission and return to airbase immediately (Temporary)
// TODO The complete and accurate fix is in #1449
DEFINE_HOOK(0x4C75DA, EventClass_RespondToEvent_Stop, 0x6)
{
	enum { SkipGameCode = 0x4C762A };

	GET(TechnoClass* const, pTechno, ESI);

	const bool expand = RulesExt::Global()->ExtendedAircraftMissions.Get();

	// Check aircrafts
	const auto pAircraft = abstract_cast<AircraftClass*>(pTechno);
	const bool commonAircraft = pAircraft && !pAircraft->Airstrike && !pAircraft->Spawned;
	const auto mission = pTechno->CurrentMission;

	// To avoid aircrafts overlap by keep link if is returning or is in airport now.
	if (!expand || !commonAircraft || (mission != Mission::Sleep && mission != Mission::Guard && mission != Mission::Enter) || !pAircraft->DockNowHeadingTo || (pAircraft->DockNowHeadingTo != pAircraft->GetNthLink()))
		pTechno->SendToEachLink(RadioCommand::NotifyUnlink);

	pTechno->SetTarget(nullptr);

	if (expand && commonAircraft)
	{
		if (pAircraft->Type->AirportBound)
		{
			// To avoid `AirportBound=yes` aircrafts pausing in the air and let they returning to air base immediately.
			if (!pAircraft->DockNowHeadingTo || (pAircraft->DockNowHeadingTo != pAircraft->GetNthLink())) // If the aircraft have no valid dock, try to find a new one
				pAircraft->EnterIdleMode(false, true);
		}
		else if (pAircraft->Ammo)
		{
			// To avoid `AirportBound=no` aircrafts ignoring the stop task or directly return to the airport.
			if (pAircraft->Destination && static_cast<int>(CellClass::Coord2Cell(pAircraft->Destination->GetCoords()).DistanceFromSquared(pAircraft->GetMapCoords())) > 2) // If the aircraft is moving, find the forward cell then stop in it
				pAircraft->SetDestination(pAircraft->GetCell()->GetNeighbourCell(static_cast<FacingType>(pAircraft->PrimaryFacing.Current().GetValue<3>())), true);
		}
		else if (!pAircraft->DockNowHeadingTo || (pAircraft->DockNowHeadingTo != pAircraft->GetNthLink()))
		{
			pAircraft->EnterIdleMode(false, true);
		}
		// Otherwise landing or idling normally without answering the stop command
	}
	else
	{
		pTechno->SetDestination(nullptr, true);
	}

	return SkipGameCode;
}

// GreatestThreat: for all the mission that should let the aircraft auto select a target
AbstractClass* __fastcall AircraftClass_GreatestThreat(AircraftClass* pThis, void* _, ThreatType threatType, CoordStruct* pSelectCoords, bool onlyTargetHouseEnemy)
{
	if (RulesExt::Global()->ExtendedAircraftMissions)
	{
		if (const auto pPrimaryWeapon = pThis->GetWeapon(0)->WeaponType)
			threatType |= pPrimaryWeapon->AllowedThreats();

		if (const auto pSecondaryWeapon = pThis->GetWeapon(1)->WeaponType)
			threatType |= pSecondaryWeapon->AllowedThreats();
	}

	// return pThis->FootClass::GreatestThreat(threatType, pSelectCoords, onlyTargetHouseEnemy);
	return reinterpret_cast<AbstractClass*(__thiscall*)(FootClass*, ThreatType, CoordStruct*, bool)>(0x4D9920)(pThis, threatType, pSelectCoords, onlyTargetHouseEnemy);
}
DEFINE_JUMP(VTABLE, 0x7E2668, GET_OFFSET(AircraftClass_GreatestThreat))

#pragma endregion

static __forceinline bool CheckSpyPlaneCameraCount(AircraftClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType);

	if (!pWeaponExt->Strafing_Shots.isset())
		return true;

	if (pExt->Strafe_BombsDroppedThisRound >= pWeaponExt->Strafing_Shots)
		return false;

	pExt->Strafe_BombsDroppedThisRound++;

	return true;
}

DEFINE_HOOK(0x415666, AircraftClass_Mission_SpyPlaneApproach_MaxCount, 0x6)
{
	enum { Skip = 0x41570C };

	GET(AircraftClass*, pThis, ESI);

	if (!CheckSpyPlaneCameraCount(pThis))
		return Skip;

	return 0;
}

DEFINE_HOOK(0x4157EB, AircraftClass_Mission_SpyPlaneOverfly_MaxCount, 0x6)
{
	enum { Skip = 0x415863 };

	GET(AircraftClass*, pThis, ESI);

	if (!CheckSpyPlaneCameraCount(pThis))
		return Skip;

	return 0;
}

DEFINE_HOOK(0x708FC0, TechnoClass_ResponseMove_Pickup, 0x5)
{
	enum { SkipResponse = 0x709015 };

	GET(TechnoClass*, pThis, ECX);

	if (auto const pAircraft = abstract_cast<AircraftClass*>(pThis))
	{
		if (pAircraft->Type->Carryall && pAircraft->HasAnyLink() &&
			generic_cast<FootClass*>(pAircraft->Destination))
		{
			auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pAircraft->Type);

			if (pTypeExt->VoicePickup.isset())
			{
				pThis->QueueVoice(pTypeExt->VoicePickup.Get());

				R->EAX(1);
				return SkipResponse;
			}
		}
	}

	return 0;
}
