#include "Body.h"
#include <Randomizer.h>

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

int GetVoiceAttack(TechnoTypeClass* pType, int WeaponIndex, bool isElite, WeaponTypeClass* pWeaponType)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	int VoiceAttack = -1;

	if (pWeaponType && pWeaponType->Damage < 0)
	{
		VoiceAttack = pTypeExt->VoiceIFVRepair.Get();

		if (VoiceAttack < 0)
			VoiceAttack = !strcmp(pType->ID, "FV") ? RulesClass::Instance->VoiceIFVRepair : -1;

		if (VoiceAttack >= 0)
			return VoiceAttack;
	}

	if (WeaponIndex >= 0 && int(pTypeExt->VoiceWeaponAttacks.size()) > WeaponIndex)
		VoiceAttack = isElite ? pTypeExt->VoiceEliteWeaponAttacks[WeaponIndex] : pTypeExt->VoiceWeaponAttacks[WeaponIndex];

	if (VoiceAttack < 0)
	{
		if (pTypeExt->IsSecondary(WeaponIndex))
			VoiceAttack = isElite ? pType->VoiceSecondaryEliteWeaponAttack : pType->VoiceSecondaryWeaponAttack;
		else
			VoiceAttack = isElite ? pType->VoicePrimaryEliteWeaponAttack : pType->VoicePrimaryWeaponAttack;
	}

	return VoiceAttack;
}

DEFINE_HOOK(0x7090A0, TechnoClass_VoiceAttack, 0x7)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	const auto pType = pThis->GetTechnoType();
	const int WeaponIndex = pThis->SelectWeapon(pTarget);
	int VoiceAttack = GetVoiceAttack(pType, WeaponIndex, pThis->Veterancy.IsElite(), pThis->GetWeapon(WeaponIndex)->WeaponType);

	if (VoiceAttack >= 0)
	{
		pThis->QueueVoice(VoiceAttack);
		return 0x7091C7;
	}

	const auto& Lists = pType->VoiceAttack;

	if (Lists.Count > 0)
	{
		int idx = Randomizer::Global.RandomRanged(0, Lists.Count - 1);
		pThis->QueueVoice(Lists[idx]);
	}

	return 0x7091C7;
}
