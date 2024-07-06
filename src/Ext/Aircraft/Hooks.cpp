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

	if (pWeaponExt->Strafing_Shots > 5)
	{
		if (pThis->MissionStatus == (int)AirAttackStatus::FireAtTarget3_Strafe)
		{
			int remainingShots = pWeaponExt->Strafing_Shots - 3 - pExt->Strafe_BombsDroppedThisRound;

			if (remainingShots > 0)
				pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
		}
	}
	else if (fireCount > 1 && pWeaponExt->Strafing_Shots < fireCount)
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

	if (auto const pTrailerAnim = GameCreate<AnimClass>(pThis->Type->Trailer, coords, 1, 1))
	{
		auto const pTrailerAnimExt = AnimExt::ExtMap.Find(pTrailerAnim);
		AnimExt::SetAnimOwnerHouseKind(pTrailerAnim, pThis->Owner, nullptr, false, true);
		pTrailerAnimExt->SetInvoker(pThis);
	}

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

DEFINE_HOOK(0x446F6C, BuildingClass_GrandOpening_PoseDir, 0x9)
{
	GET(BuildingClass*, pThis, EBP);
	GET(AircraftClass*, pAircraft, ESI);

	R->EAX(AircraftExt::GetLandingDir(pAircraft, pThis));

	return 0;
}

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

DEFINE_HOOK(0x4CF68D, FlyLocomotionClass_DrawMatrix_OnAirport, 0x5)
{
	GET(ILocomotion*, iloco, ESI);
	__assume(iloco != nullptr);
	auto loco = static_cast<FlyLocomotionClass*>(iloco);
	auto pThis = static_cast<AircraftClass*>(loco->LinkedTo);
	if (pThis->GetHeight() <= 0)
	{
		float ars = pThis->AngleRotatedSideways;
		float arf = pThis->AngleRotatedForwards;
		if (std::abs(ars) > 0.005 || std::abs(arf) > 0.005)
		{
			LEA_STACK(Matrix3D*, mat, STACK_OFFSET(0x38, -0x30));
			mat->TranslateZ(float(std::abs(Math::sin(ars)) * pThis->Type->VoxelScaleX
				+ std::abs(Math::sin(arf)) * pThis->Type->VoxelScaleY));
			R->ECX(pThis);
			return 0x4CF6AD;
		}
	}

	return 0;
}

DEFINE_HOOK(0x415EEE, AircraftClass_Fire_KickOutPassenger, 0x6)
{
	GET(AircraftClass*, pThis, EDI);
	GET_STACK(int, weaponIdx, STACK_OFFSET(0x7C, 0x8));

	enum { KickOutPassenger = 0x415EF8, Fire = 0x415F08 };

	if (pThis->Passengers.GetFirstPassenger() == nullptr)
		return Fire;

	if (TechnoExt::IsActive(pThis) && pThis->GetWeapon(weaponIdx) != nullptr)
	{
		const WeaponTypeClass* pWeapon = pThis->GetWeapon(weaponIdx)->WeaponType;

		if (const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
		{
			if (!pWeaponExt->KickOutPassenger)
				return Fire;
		}
	}

	return KickOutPassenger;
}