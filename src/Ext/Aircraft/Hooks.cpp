#include <AircraftClass.h>
#include <FlyLocomotionClass.h>


#include <Ext/Aircraft/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x417FF1, AircraftClass_Mission_Attack_StrafeShots, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
		|| pThis->MissionStatus >(int)AirAttackStatus::FireAtTarget5_Strafe)
	{
		return 0;
	}

	int weaponIndex = pThis->SelectWeapon(pThis->Target);
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(weaponIndex)->WeaponType);
	int fireCount = pThis->MissionStatus - 4;

	if (fireCount > 1 && pWeaponExt->Strafing_Shots < fireCount)
	{
		if (!pThis->Ammo)
			pThis->unknown_bool_6D2 = false;

		pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
	}

	return 0;
}

DEFINE_HOOK(0x418403, AircraftClass_Mission_Attack_FireAtTarget_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->unknown_bool_6C8 = true;

	AircraftExt::FireBurst(pThis, pThis->Target, 0);

	return 0x418478;
}

DEFINE_HOOK(0x4186B6, AircraftClass_Mission_Attack_FireAtTarget2_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, 0);

	return 0x4186D7;
}

DEFINE_HOOK(0x418805, AircraftClass_Mission_Attack_FireAtTarget2Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, 1);

	return 0x418826;
}

DEFINE_HOOK(0x418914, AircraftClass_Mission_Attack_FireAtTarget3Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, 2);

	return 0x418935;
}

DEFINE_HOOK(0x418A23, AircraftClass_Mission_Attack_FireAtTarget4Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, 3);

	return 0x418A44;
}

DEFINE_HOOK(0x418B1F, AircraftClass_Mission_Attack_FireAtTarget5Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, 4);

	return 0x418B40;
}

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
