#include "Body.h"

#include <BulletClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

#pragma region DETONATION

bool DetonationInDamageArea = true;

DEFINE_HOOK(0x46920B, BulletClass_Detonate, 0x6)
{
	GET(BulletClass* const, pThis, ESI);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH))
	{
		GET_BASE(const CoordStruct*, pCoords, 0x8);
		auto const pTechno = pThis ? pThis->Owner : nullptr;
		auto const pHouse = pTechno ? pTechno->Owner : nullptr;

		pWHExt->Detonate(pTechno, pHouse, pThis, *pCoords);
	}

	DetonationInDamageArea = false;

	return 0;
}

DEFINE_HOOK(0x46A290, BulletClass_Detonate_Return, 0x5)
{
	DetonationInDamageArea = true;
	return 0;
}

DEFINE_HOOK(0x489286, MapClass_DamageArea, 0x6)
{
	if (DetonationInDamageArea)
	{
		// GET(const int, Damage, EDX);
		// GET_BASE(const bool, AffectsTiberium, 0x10);

		GET_BASE(const WarheadTypeClass*, pWH, 0x0C);

		if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH))
		{
			GET(const CoordStruct*, pCoords, ECX);
			GET_BASE(TechnoClass*, pOwner, 0x08);
			GET_BASE(HouseClass*, pHouse, 0x14);

			auto const pDecidedHouse = !pHouse && pOwner ? pOwner->Owner : pHouse;

			pWHExt->Detonate(pOwner, pDecidedHouse, nullptr, *pCoords);
		}
	}

	return 0;
}
#pragma endregion

DEFINE_HOOK(0x48A551, WarheadTypeClass_AnimList_SplashList, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt && pWHExt->SplashList.size())
	{
		GET(int, nDamage, ECX);
		int idx = pWHExt->SplashList_PickRandom ?
			ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->SplashList.size() - 1) :
			std::min(pWHExt->SplashList.size() * 35 - 1, (size_t)nDamage) / 35;
		R->EAX(pWHExt->SplashList[idx]);
		return 0x48A5AD;
	}

	return 0;
}

DEFINE_HOOK(0x48A5BD, WarheadTypeClass_AnimList_PickRandom, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	return pWHExt && pWHExt->AnimList_PickRandom ? 0x48A5C7 : 0;
}

DEFINE_HOOK(0x48A5B3, WarheadTypeClass_AnimList_CritAnim, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt && pWHExt->HasCrit && pWHExt->Crit_AnimList.size() && !pWHExt->Crit_AnimOnAffectedTargets)
	{
		GET(int, nDamage, ECX);
		int idx = pThis->EMEffect || pWHExt->Crit_AnimList_PickRandom.Get(pWHExt->AnimList_PickRandom) ?
			ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->Crit_AnimList.size() - 1) :
			std::min(pWHExt->Crit_AnimList.size() * 25 - 1, (size_t)nDamage) / 25;
		R->EAX(pWHExt->Crit_AnimList[idx]);
		return 0x48A5AD;
	}

	return 0;
}

DEFINE_HOOK(0x6FC339, TechnoClass_CanFire, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));
	// Checking for nullptr is not required here, since the game has already executed them before calling the hook  -- Belonit
	const auto pWH = pWeapon->Warhead;
	enum { CannotFire = 0x6FCB7E };

	if (const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH))
	{
		const int nMoney = pWHExt->TransactMoney;
		if (nMoney < 0 && pThis->Owner->Available_Money() < -nMoney)
			return CannotFire;
	}
	if (const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		const auto pTechno = abstract_cast<TechnoClass*>(pTarget);

		CellClass* targetCell = nullptr;

		// Ignore target cell for airborne technos.
		if (!pTechno || !pTechno->IsInAir())
		{
			if (const auto pCell = abstract_cast<CellClass*>(pTarget))
				targetCell = pCell;
			else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
				targetCell = pObject->GetCell();
		}

		if (targetCell)
		{
			if (!EnumFunctions::IsCellEligible(targetCell, pWeaponExt->CanTarget, true))
				return CannotFire;
		}

		if (pTechno)
		{
			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTechno->Owner))
			{
				return CannotFire;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F33CD, TechnoClass_WhatWeaponShouldIUse_ForceFire, 0x6)
{
	enum { Secondary = 0x6F3745 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (const auto pCell = abstract_cast<CellClass*>(pTarget))
	{
		if (const auto pPrimaryExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType))
		{
			if (pThis->GetWeapon(1)->WeaponType && !EnumFunctions::IsCellEligible(pCell, pPrimaryExt->CanTarget, true))
				return Secondary;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	enum { Primary = 0x6F37AD, Secondary = 0x6F3745, FurtherCheck = 0x6F3754, OriginalCheck = 0x6F36E3 };

	CellClass* targetCell = nullptr;

	// Ignore target cell for airborne technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (const auto pCell = abstract_cast<CellClass*>(pTarget))
			targetCell = pCell;
		else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
			targetCell = pObject->GetCell();
	}

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (const auto pSecondary = pThis->GetWeapon(1))
		{
			if (const auto pSecondaryExt = WeaponTypeExt::ExtMap.Find(pSecondary->WeaponType))
			{
				if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pSecondaryExt->CanTarget, true)) ||
					(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondaryExt->CanTarget) ||
					!EnumFunctions::CanTargetHouse(pSecondaryExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
				{
					return Primary;
				}

				if (const auto pPrimaryExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType))
				{
					if (pTypeExt->NoSecondaryWeaponFallback && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1))
						return Primary;

					if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pPrimaryExt->CanTarget, true)) ||
						(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pPrimaryExt->CanTarget) ||
						!EnumFunctions::CanTargetHouse(pPrimaryExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
					{
						return Secondary;
					}
				}
			}
		}

		if (!pTargetTechno)
			return Primary;

		if (const auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno))
		{
			if (const auto pShield = pTargetExt->Shield.get())
			{
				if (pShield->IsActive())
				{
					if (pThis->GetWeapon(1) && !(pTypeExt->NoSecondaryWeaponFallback && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1)))
					{
						if (!pShield->CanBeTargeted(pThis->GetWeapon(0)->WeaponType))
							return Secondary;
						else
							return FurtherCheck;
					}

					return Primary;
				}
			}
		}
	}

	return OriginalCheck;
}