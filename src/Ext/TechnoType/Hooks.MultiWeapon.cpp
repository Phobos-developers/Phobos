#include "Body.h"

#include <Ext/Techno/Body.h>

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
					if (weaponIndex >= weaponCount || weaponIndex < 0)
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

static inline int GetVoiceAttack(TechnoTypeClass* pType, int weaponIndex, bool isElite, WeaponTypeClass* pWeaponType)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	int voiceAttack = -1;

	if (pWeaponType && pWeaponType->Damage < 0)
	{
		voiceAttack = pTypeExt->VoiceIFVRepair;

		if (voiceAttack == -1)
			voiceAttack = !strcmp(pType->ID, "FV") ? RulesClass::Instance->VoiceIFVRepair : -1; // It's hardcoded like this in vanilla

		if (voiceAttack != -1)
			return voiceAttack;
	}

	if (weaponIndex >= 0 && int(pTypeExt->VoiceWeaponAttacks.size()) > weaponIndex)
		voiceAttack = isElite ? pTypeExt->VoiceEliteWeaponAttacks[weaponIndex] : pTypeExt->VoiceWeaponAttacks[weaponIndex];

	if (voiceAttack == -1)
	{
		if (pTypeExt->IsSecondary(weaponIndex))
		{
			const int eliteVoice = pType->VoiceSecondaryEliteWeaponAttack;
			voiceAttack = isElite && eliteVoice != -1 ? eliteVoice : pType->VoiceSecondaryWeaponAttack;
		}
		else
		{
			const int eliteVoice = pType->VoicePrimaryEliteWeaponAttack;
			voiceAttack = isElite && eliteVoice != -1 ? eliteVoice : pType->VoicePrimaryWeaponAttack;
		}
	}

	return voiceAttack;
}

DEFINE_HOOK(0x7090A0, TechnoClass_VoiceAttack, 0x7)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	const auto pType = pThis->GetTechnoType();
	const int weaponIndex = pThis->SelectWeapon(pTarget);
	const int voiceAttack = GetVoiceAttack(pType, weaponIndex, pThis->Veterancy.IsElite(), pThis->GetWeapon(weaponIndex)->WeaponType);

	if (voiceAttack >= 0)
	{
		pThis->QueueVoice(voiceAttack);
		return 0x7091C7;
	}

	const auto& voiceList = pType->VoiceAttack;

	if (voiceList.Count > 0)
	{
		const int idx = Randomizer::Global.RandomRanged(0, voiceList.Count - 1);
		pThis->QueueVoice(voiceList[idx]);
	}

	return 0x7091C7;
}

bool __forceinline IsDeployed(TechnoClass* const pThis, const AbstractType rtti)
{
	switch (rtti)
	{
	case AbstractType::Infantry:
		return static_cast<InfantryClass*>(pThis)->IsDeployed();
	case AbstractType::Unit:
	{
		auto const pUnit = static_cast<UnitClass*>(pThis);

		return pUnit->Deployed || (pUnit->Type->DeployFire &&
			pThis->CurrentMission == Mission::Unload);
	}
	default:
		return false;
	}
}

static __forceinline ThreatType GetThreatType(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, ThreatType result)
{
	const ThreatType flags = pThis->Veterancy.IsElite() ? pTypeExt->ThreatTypes.Y : pTypeExt->ThreatTypes.X;
	return result | flags;
}

DEFINE_HOOK_AGAIN(0x51E2CF, FootClass_SelectAutoTarget_MultiWeapon, 0x6)	// InfantryClass_SelectAutoTarget
DEFINE_HOOK(0x7431C9, FootClass_SelectAutoTarget_MultiWeapon, 0x7)			// UnitClass_SelectAutoTarget
{
	enum { InfantryReturn = 0x51E31B, UnitReturn = 0x74324F, UnitGunner = 0x7431E4 };

	GET(FootClass*, pThis, ESI);
	GET(const ThreatType, result, EDI);

	const auto pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	const auto pType = pTypeExt->OwnerObject();
	const bool isUnit = R->Origin() == 0x7431C9;

	if (isUnit
		&& !pType->IsGattling && pType->TurretCount > 0
		&& (pType->Gunner || !pTypeExt->MultiWeapon))
	{
		return UnitGunner;
	}

	const SeparateWeaponType weaponTypes = pTypeExt->SeparateWeaponTypes;

	if (weaponTypes & SeparateWeaponType::DeployFire)
	{
		const AbstractType rtti = isUnit ? AbstractType::Unit : AbstractType::Infantry;
		const int deployFireWeapon = pType->DeployFireWeapon;

		if (IsDeployed(pThis, rtti) && pType->DeployFire && deployFireWeapon >= 0)
		{
			ThreatType flags = result;

			if (const auto pWeapon = pThis->GetWeapon(deployFireWeapon)->WeaponType)
				flags |= pWeapon->AllowedThreats();

			R->EDI(flags);
			return isUnit ? UnitReturn : InfantryReturn;
		}
	}

	if (weaponTypes & SeparateWeaponType::OpenTransport)
	{
		const int openTransportWeapon = pType->OpenTransportWeapon;

		if (pThis->InOpenToppedTransport && openTransportWeapon >= 0)
		{
			ThreatType flags = result;

			if (const auto pWeapon = pThis->GetWeapon(openTransportWeapon)->WeaponType)
				flags |= pWeapon->AllowedThreats();

			R->EDI(flags);
			return isUnit ? UnitReturn : InfantryReturn;
		}
	}

	if (weaponTypes & SeparateWeaponType::NoAmmo)
	{
		const int noAmmoWeapon = pTypeExt->NoAmmoWeapon;

		if (pType->Ammo >= 0 && noAmmoWeapon >= 0 && pThis->Ammo <= pTypeExt->NoAmmoAmount)
		{
			ThreatType flags = result;

			if (const auto pWeapon = pThis->GetWeapon(noAmmoWeapon)->WeaponType)
				flags |= pWeapon->AllowedThreats();

			R->EDI(flags);
			return isUnit ? UnitReturn : InfantryReturn;
		}
	}

	R->EDI(GetThreatType(pThis, pTypeExt, result));
	return isUnit ? UnitReturn : InfantryReturn;
}

DEFINE_HOOK(0x445F04, BuildingClass_SelectAutoTarget_MultiWeapon, 0xA)
{
	enum { ReturnThreatType = 0x445F58, Continue = 0x445F0E };

	GET(BuildingClass*, pThis, ESI);
	GET_STACK(const ThreatType, result, STACK_OFFSET(0x8, 0x4));

	if (pThis->UpgradeLevel > 0 || pThis->CanOccupyFire())
	{
		R->EAX(pThis->GetWeapon(0));
		return Continue;
	}

	const auto pType = pThis->Type;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->SeparateWeaponTypes & SeparateWeaponType::NoAmmo)
	{
		const int noAmmoWeapon = pTypeExt->NoAmmoWeapon;

		if (pType->Ammo >= 0 && noAmmoWeapon >= 0 && pThis->Ammo <= pTypeExt->NoAmmoAmount)
		{
			ThreatType flags = result;

			if (const auto pWeapon = pThis->GetWeapon(noAmmoWeapon)->WeaponType)
				flags |= pWeapon->AllowedThreats();

			R->EDI(flags);
			return ReturnThreatType;
		}
	}

	R->EDI(GetThreatType(pThis, pTypeExt, result));
	return ReturnThreatType;
}

DEFINE_HOOK(0x6F398E, TechnoClass_CombatDamage_MultiWeapon, 0x7)
{
	enum { ReturnDamage = 0x6F3ABB, GunnerDamage = 0x6F39AD, Continue = 0x6F39F4 };

	GET(TechnoClass*, pThis, ESI);

	const AbstractType rtti = pThis->WhatAmI();

	if (rtti == AbstractType::Building)
	{
		const auto pBuilding = static_cast<BuildingClass*>(pThis);

		if (pBuilding->UpgradeLevel > 0 || pBuilding->CanOccupyFire())
			return Continue;
	}

	const auto pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	const auto pType = pTypeExt->OwnerObject();

	if (rtti == AbstractType::Unit
		&& !pType->IsGattling && pType->TurretCount > 0
		&& (pType->Gunner || !pTypeExt->MultiWeapon))
	{
		return GunnerDamage;
	}

	const SeparateWeaponType weaponTypes = pTypeExt->SeparateWeaponTypes;

	if (weaponTypes & SeparateWeaponType::DeployFire)
	{
		const int deployFireWeapon = pType->DeployFireWeapon;

		if (IsDeployed(pThis, rtti) && pType->DeployFire && deployFireWeapon >= 0)
		{
			int damage = 0;

			if (auto const pWeapon = pThis->GetWeapon(deployFireWeapon)->WeaponType)
				damage = (pWeapon->Damage + pWeapon->AmbientDamage);

			R->EAX(damage);
			return ReturnDamage;
		}
	}

	if (weaponTypes & SeparateWeaponType::OpenTransport)
	{
		const int openTransportWeapon = pType->OpenTransportWeapon;

		if (pThis->InOpenToppedTransport && openTransportWeapon >= 0)
		{
			int damage = 0;

			if (auto const pWeapon = pThis->GetWeapon(openTransportWeapon)->WeaponType)
				damage = (pWeapon->Damage + pWeapon->AmbientDamage);

			R->EAX(damage);
			return ReturnDamage;
		}
	}

	if (weaponTypes & SeparateWeaponType::NoAmmo)
	{
		const int noAmmoWeapon = pTypeExt->NoAmmoWeapon;

		if (pType->Ammo >= 0 && noAmmoWeapon >= 0 && pThis->Ammo <= pTypeExt->NoAmmoAmount)
		{
			int damage = 0;

			if (auto const pWeapon = pThis->GetWeapon(noAmmoWeapon)->WeaponType)
				damage = (pWeapon->Damage + pWeapon->AmbientDamage);

			R->EAX(damage);
			return ReturnDamage;
		}
	}

	R->EAX(pThis->Veterancy.IsElite() ? pTypeExt->CombatDamages.Y : pTypeExt->CombatDamages.X);
	return ReturnDamage;
}
