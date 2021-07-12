#include "Body.h"
#include <Ext/WeaponType/Body.h>
#include "..\Scenario\Body.h"
#include "..\IsometricTileType\Body.h"

// TODO: Implement proper extended AircraftClass.

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, int shotNumber = 0)
{
	int weaponIndex = pThis->SelectWeapon(pTarget);
	auto weaponType = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(weaponType);

	if (weaponType->Burst > 0)
	{
		while (pThis->CurrentBurstIndex < weaponType->Burst)
		{
			if (weaponType->Burst < 2 && pWeaponTypeExt->Strafing_SimulateBurst)
				pThis->CurrentBurstIndex = shotNumber;

			pThis->Fire(pThis->Target, weaponIndex);

			if (pThis->CurrentBurstIndex == 0)
				break;
		}
	}
}