#include <Helpers/Macro.h>
#include <InfantryClass.h>

DEFINE_HOOK(0x520AD2, InfantryClass_UpdateFiring_NoTarget, 0x7)
{
	GET(InfantryClass*, pThis, EBP);

	if (pThis->Type->IsGattling)
	{
		pThis->GattlingRateDown(1);
	}

	return 0;
}

// the function is being called repeatedly. Maybe it doesn't have to do that.
namespace UpdateFiringTemp
{
	int WeaponIndex;
	FireError fireError;
}

DEFINE_HOOK(0x5206D2, InfantryClass_UpdateFiring_GetContext, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, WeaponIndex, EDI);

	const auto pTarget = pThis->Target;
	UpdateFiringTemp::WeaponIndex = WeaponIndex;
	UpdateFiringTemp::fireError = pThis->GetFireError(pTarget, WeaponIndex, true);

	if (pThis->Type->IsGattling)
	{
		switch (UpdateFiringTemp::fireError)
		{
		case FireError::OK:
		case FireError::REARM:
		case FireError::FACING:
		case FireError::ROTATING:
			pThis->GattlingRateUp(1);
			break;
		default:
			pThis->GattlingRateDown(1);
			break;
		}
	}

	return 0;
}

// avoid repeatedly calling SelectWeapon().
DEFINE_HOOK(0x5209BB, InfantryClass_UpdateFiring_SetWeaponIndex, 0x6)
{
	R->EAX(UpdateFiringTemp::WeaponIndex);
	return 0x5209CD;
}

// avoid repeatedly calling GetFireError().
DEFINE_HOOK_AGAIN(0x5209D2, InfantryClass_UpdateFiring_SetFireError, 0x6)
DEFINE_HOOK(0x5206E4, InfantryClass_UpdateFiring_SetFireError, 0x6)
{
	R->EAX(UpdateFiringTemp::fireError);
	return R->Origin() == 0x5206E4 ? 0x5206F9 : 0x5209E4;
}
