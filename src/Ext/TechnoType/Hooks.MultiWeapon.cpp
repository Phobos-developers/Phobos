#include "Body.h"

DEFINE_HOOK(0x7128B2, TechnoTypeClass_ReadINI_MultiWeapon, 0x6)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, ESI);
	enum { ReadWeaponX = 0x7128C0 };

	INI_EX exINI(pINI);
	const char* pSection = pThis->ID;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);
	pTypeExt->MultiWeapon.Read(exINI, pSection, "MultiWeapon");
	const bool multiWeapon = pThis->HasMultipleTurrets() || pTypeExt->MultiWeapon.Get();

	if (pTypeExt->ReadMultiWeapon != multiWeapon)
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

		pTypeExt->ReadMultiWeapon = multiWeapon;
	}

	auto& SecondaryList = pTypeExt->MultiWeapon_IsSecondary;

	if (pTypeExt->MultiWeapon.Get())
	{
		pTypeExt->MultiWeapon_SelectCount.Read(exINI, pSection, "MultiWeapon.SelectCount");

		const int weaponCount = pThis->WeaponCount;

		if (weaponCount > 0)
		{
			ValueableVector<int> isSecondary;
			isSecondary.Read(exINI, pSection, "MultiWeapon.IsSecondary");

			if (!isSecondary.empty())
			{
				SecondaryList.resize(weaponCount, false);

				for (int weaponIndex : isSecondary)
				{
					if (weaponIndex >= weaponCount)
						continue;

					SecondaryList[weaponIndex] = true;
				}
			}
		}
	}
	else
	{
		SecondaryList.clear();
	}

	return multiWeapon ? ReadWeaponX : 0;
}

DEFINE_HOOK(0x715B10, TechnoTypeClass_ReadINI_MultiWeapon2, 0x7)
{
	GET(TechnoTypeClass*, pThis, EBP);
	enum { ReadWeaponX = 0x715B1F, Continue = 0x715B17 };

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);

	if (pTypeExt->ReadMultiWeapon)
		return ReadWeaponX;

	R->AL(pThis->HasMultipleTurrets());
	return Continue;
}
