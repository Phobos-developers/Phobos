#include <AircraftClass.h>
#include <Utilities/Macro.h>
#include <Utilities/Enum.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/WeaponType/Body.h>

DEFINE_HOOK(417FF1, AircraftClass_Mission_Attack_StrafeShots, 6)
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

	if (fireCount > 1 && pWeaponExt->Strafing_Shots < fireCount) {

		if (!pThis->Ammo) {
			pThis->unknown_bool_6D2 = false;
		}

		pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
		return 0;
	}

	return 0;
}

DEFINE_HOOK(418409, AircraftClass_Mission_Attack_FireAtTarget_BurstFix, 0)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->unknown_bool_6C8 = true;

	AircraftExt::FireBurst(pThis, pThis->Target, 0);

	return 0x418478;
}

DEFINE_HOOK(4186B6, AircraftClass_Mission_Attack_FireAtTarget2_BurstFix, 0)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, 0);

	return 0x4186D7;
}

DEFINE_HOOK(418805, AircraftClass_Mission_Attack_FireAtTarget2Strafe_BurstFix, 0)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, 1);

	return 0x418826;
}

DEFINE_HOOK(418914, AircraftClass_Mission_Attack_FireAtTarget3Strafe_BurstFix, 0)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, 2);

	return 0x418935;
}

DEFINE_HOOK(418A23, AircraftClass_Mission_Attack_FireAtTarget4Strafe_BurstFix, 0)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, 3);

	return 0x418A44;
}

DEFINE_HOOK(418B25, AircraftClass_Mission_Attack_FireAtTarget5Strafe_BurstFix, 0)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, 4);

	return 0x418B40;
}