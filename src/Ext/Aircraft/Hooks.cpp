#include <AircraftClass.h>
#include <FlyLocomotionClass.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
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

	AirAttackStatus const state = (AirAttackStatus)pThis->MissionStatus;
	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (state < AirAttackStatus::FireAtTarget2_Strafe
		|| state > AirAttackStatus::FireAtTarget5_Strafe)
	{
		pExt->Strafe_BombsDroppedThisRound = 0;
		return 0;
	}

	int strafingShots = pWeaponExt->Strafing_Shots.Get(5);

	if (strafingShots > 5)
	{
		if (state == AirAttackStatus::FireAtTarget3_Strafe)
		{
			int remainingShots = strafingShots - 3 - pExt->Strafe_BombsDroppedThisRound;

			if (remainingShots > 0)
				pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
		}
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

	R->EAX(MapClass::Instance.GetCellAt(pObject->GetCoords()));

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
	else
		pWeapon = pThis->GetWeapon(0)->WeaponType;

	if (pWeapon)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		auto const pBulletType = pWeapon->Projectile;
		return pWeaponExt->Strafing.Get(pBulletType->ROT <= 1 && !pBulletType->Inviso && !BulletTypeExt::ExtMap.Find(pBulletType)->TrajectoryType);
	}

	return false;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2268, AircraftClass_IFlyControl_IsStrafe);

DEFINE_HOOK(0x418403, AircraftClass_Mission_Attack_FireAtTarget_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->ShouldLoseAmmo = true;

	AircraftExt::FireWeapon(pThis, pThis->Target);

	return 0x418478;
}

DEFINE_HOOK(0x4186B6, AircraftClass_Mission_Attack_FireAtTarget2_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireWeapon(pThis, pThis->Target);

	return 0x4186D7;
}

DEFINE_HOOK(0x418805, AircraftClass_Mission_Attack_FireAtTarget2Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireWeapon(pThis, pThis->Target);

	return 0x418826;
}

DEFINE_HOOK(0x418914, AircraftClass_Mission_Attack_FireAtTarget3Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireWeapon(pThis, pThis->Target);

	return 0x418935;
}

DEFINE_HOOK(0x418A23, AircraftClass_Mission_Attack_FireAtTarget4Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireWeapon(pThis, pThis->Target);

	return 0x418A44;
}

DEFINE_HOOK(0x418B1F, AircraftClass_Mission_Attack_FireAtTarget5Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireWeapon(pThis, pThis->Target);

	return 0x418B40;
}

#pragma region After_Shot_Delays

static int GetDelay(AircraftClass* pThis, bool isLastShot)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	int weaponIndex = pExt->CurrentAircraftWeaponIndex >= 0 ? pExt->CurrentAircraftWeaponIndex : pThis->SelectWeapon(pThis->Target);
	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	int delay = pWeapon->ROF;

	if (isLastShot || pExt->Strafe_BombsDroppedThisRound == pWeaponExt->Strafing_Shots.Get(5) || (pWeaponExt->Strafing_UseAmmoPerShot && !pThis->Ammo))
	{
		pThis->MissionStatus = (int)AirAttackStatus::FlyToPosition;
		delay = pWeaponExt->Strafing_EndDelay.Get((pWeapon->Range + (Unsorted::LeptonsPerCell * 4)) / pThis->Type->Speed);
	}

	return delay;
}

DEFINE_HOOK(0x4184CC, AircraftClass_Mission_Attack_Delay1A, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->IsLocked = true;
	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x4184F1;
}

DEFINE_HOOK(0x418506, AircraftClass_Mission_Attack_Delay1B, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->IsLocked = true;
	pThis->MissionStatus = pThis->Ammo > 0 ? (int)AirAttackStatus::PickAttackLocation : (int)AirAttackStatus::ReturnToBase;
	R->EAX(GetDelay(pThis, false));

	return 0x418539;
}

DEFINE_HOOK(0x418883, AircraftClass_Mission_Attack_Delay2, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget3_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x4188A1;
}

DEFINE_HOOK(0x418992, AircraftClass_Mission_Attack_Delay3, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget4_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x4189B0;
}

DEFINE_HOOK(0x418AA1, AircraftClass_Mission_Attack_Delay4, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget5_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x418ABF;
}

DEFINE_HOOK(0x418B8A, AircraftClass_Mission_Attack_Delay5, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	R->EAX(GetDelay(pThis, true));

	return 0x418BBA;
}

#pragma endregion

void __fastcall AircraftClass_SetTarget_Wrapper(AircraftClass* pThis, void* _, AbstractClass* pTarget)
{
	pThis->TechnoClass::SetTarget(pTarget);
	TechnoExt::ExtMap.Find(pThis)->CurrentAircraftWeaponIndex = -1;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E266C, AircraftClass_SetTarget_Wrapper);

#pragma endregion

DEFINE_HOOK(0x414F10, AircraftClass_AI_Trailer, 0x5)
{
	enum { SkipGameCode = 0x414F47 };

	GET(AircraftClass*, pThis, ESI);
	REF_STACK(const CoordStruct, coords, STACK_OFFSET(0x40, -0xC));

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

DEFINE_FUNCTION_JUMP(CALL, 0x446F67, AircraftClass_PoseDir_Wrapper); // BuildingClass_GrandOpening

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

DEFINE_HOOK(0x4DDD66, FootClass_IsLandZoneClear_ReplaceHardcode, 0x6) // To avoid that the aircraft cannot fly towards the water surface normally
{
	enum { SkipGameCode = 0x4DDD8A };

	GET(FootClass* const, pThis, EBP);
	GET_STACK(CellStruct, cell, STACK_OFFSET(0x20, 0x4));

	const auto pType = pThis->GetTechnoType();

	// In vanilla, only aircrafts or `foots with fly locomotion` will call this virtual function
	// So I don't know why WW use hard-coded `SpeedType::Track` and `MovementZone::Normal` to check this
	R->AL(MapClass::Instance.GetCellAt(cell)->IsClearToMove(pType->SpeedType, false, false, -1, pType->MovementZone, -1, true));
	return SkipGameCode;
}

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
