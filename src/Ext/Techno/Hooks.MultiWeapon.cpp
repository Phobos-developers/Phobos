#include "Body.h"

// There was no way to do it, so I decided to leave it at that.
/*
bool _fastcall CanElectricAssault(FootClass* pThis)
{
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	bool allowAssault = !pThis->GetTechnoType()->IsGattling &&
		(pThis->WhatAmI() != AbstractType::Unit || !pThis->GetTechnoType()->Gunner);

	if (pTypeExt->MultiWeapon.Get() && allowAssault)
	{
		for (int index = 0; index < pThis->GetTechnoType()->WeaponCount; index++)
		{
			const auto pWeaponType = pThis->Veterancy.IsElite() ?
				pType->GetEliteWeapon(index)->WeaponType : pType->GetWeapon(index)->WeaponType;

			if (!pWeaponType || !pWeaponType->Warhead ||
				!pWeaponType->Warhead->ElectricAssault)
				continue;

			return true;
		}
	}
	else if (allowAssault)
	{
		const auto secondary = pThis->GetWeapon(1)->WeaponType;

		if (secondary && secondary->Warhead &&
			secondary->Warhead->ElectricAssault)
			return true;
	}

	return false;
}

DEFINE_HOOK(0x4D50E1, FootClass_Mission_Guard_ElectricAssult, 0xA)
{
	GET(FootClass*, pThis, ESI);
	enum { ElectricAssult = 0x4D5116, Skip = 0x4D51D4, SkipAll = 0x4D52F5 };

	if (!pThis)
		return SkipAll;

	return CanElectricAssault(pThis) ? ElectricAssult : Skip;
}

DEFINE_HOOK(0x4D6F44, FootClass_Mission_AreaGuard_ElectricAssult, 0x6)
{
	GET(FootClass*, pThis, ESI);
	enum { ElectricAssult = 0x4D6F78, Skip = 0x4D7025, SkipAll = 0x4D715A };

	if (!pThis)
		return SkipAll;

	return CanElectricAssault(pThis) ? ElectricAssult : Skip;
}
*/

// Do you think the infantry's way of determining that weapons are secondary is stupid ?
// I think it's kind of stupid.
DEFINE_HOOK(0x520888, InfantryClass_UpdateFiring_IsSecondary, 0x8)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, weaponIdx, EDI);
	enum { Primary = 0x5208D6, Secondary = 0x520890 };

	R->AL(pThis->Crawling);
	return TechnoExt::IsSecondary(pThis, weaponIdx) ? Secondary : Primary;
}
