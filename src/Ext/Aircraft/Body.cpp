#include "Body.h"
#include <Ext/WeaponType/Body.h>
#include "..\Scenario\Body.h"

// TODO: Implement proper extended AircraftClass.

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, int shotNumber = 0)
{
	if (!pTarget) return;
	int weaponIndex = pThis->SelectWeapon(pTarget);
	auto weaponType = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(weaponType);

	if (weaponType->Burst > 0)
	{
		for (int i = 0; i < weaponType->Burst; i++)
		{
			if (weaponType->Burst < 2 && pWeaponTypeExt->Strafing_SimulateBurst)
				pThis->CurrentBurstIndex = shotNumber;

			pThis->Fire(pTarget, weaponIndex);
		}
	}
}
