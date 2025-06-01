#include <Helpers/Macro.h>
#include <InfantryClass.h>

DEFINE_HOOK(0x5206E4, InfantryClass_UpdateFiring_IsGattling, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, nWeaponIndex, EDI);

	if (pThis->Type->IsGattling)
	{
		auto fireError = pThis->GetFireError(pThis->Target, nWeaponIndex, true);

		switch (fireError)
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

		R->EAX(fireError);
		return 0x5206F9;
	}

	return 0;
}

DEFINE_HOOK(0x5209D2, InfantryClass_UpdateFiring_IsGattling2, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, nWeaponIndex, ESI);

	if (pThis->Type->IsGattling)
	{
		auto fireError = pThis->GetFireError(pThis->Target, nWeaponIndex, true);

		switch (fireError)
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

		R->EAX(fireError);
		return 0x5209E4;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x520A03, InfantryClass_UpdateFiring_IsGattling3, 0x7)
DEFINE_HOOK(0x520AD2, InfantryClass_UpdateFiring_IsGattling3, 0x7)
{
	GET(InfantryClass*, pThis, EBP);

	if (pThis->Type->IsGattling)
		pThis->GattlingRateDown(1);

	return 0;
}
