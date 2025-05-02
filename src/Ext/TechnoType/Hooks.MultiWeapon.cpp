#include "Body.h"
#include <InfantryClass.h>

DEFINE_HOOK(0x7128B2, TechnoTypeClass_ReadINI_MultiWeapon, 0x6)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, ESI);
	enum { ReadWeaponX = 0x7128C0 };

	INI_EX exINI(pINI);
	const char* pSection = pThis->ID;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);
	pTypeExt->MultiWeapon.Read(exINI, pSection, "MultiWeapon");
	bool multiWeapon = pThis->HasMultipleTurrets() || pTypeExt->MultiWeapon.Get();

	if (pTypeExt->LastMultiWeapon != multiWeapon)
	{
		auto clearWeapon = [pThis](int index)
		{
			auto& pWeapon = pThis->GetWeapon(index, false);
			auto& pEliteWeapon = pThis->GetWeapon(index, true);

			pWeapon = WeaponStruct();
			pEliteWeapon = WeaponStruct();
		};

		clearWeapon(0);
		clearWeapon(1);

		pTypeExt->LastMultiWeapon = multiWeapon;
	}

	if (pTypeExt->MultiWeapon.Get())
	{
		pTypeExt->MultiWeapon_IsSecondary.Read(exINI, pSection, "MultiWeapon.IsSecondary");
		pTypeExt->MultiWeapon_SelectWeapon.Read(exINI, pSection, "MultiWeapon.SelectWeapon");
	}

	return multiWeapon ? ReadWeaponX : 0;
}

DEFINE_HOOK(0x715B10, TechnoTypeClass_ReadINI_MultiWeapon2, 0x7)
{
	GET(TechnoTypeClass*, pThis, EBP);
	enum { ReadWeaponX = 0x715B1F, Continue = 0x715B17 };

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);

	if (pTypeExt->LastMultiWeapon)
		return ReadWeaponX;

	R->AL(pThis->HasMultipleTurrets());
	return Continue;
}

// Do you think the infantry's way of determining that weapons are secondary is stupid ?
// I think it's kind of stupid.
DEFINE_HOOK(0x520888, InfantryClass_UpdateFiring_IsSecondary, 0x8)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, weaponIdx, EDI);
	enum { Primary = 0x5208D6, Secondary = 0x520890 };

	R->AL(pThis->Crawling);
	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->IsSecondary(weaponIdx) ? Secondary : Primary;
}
