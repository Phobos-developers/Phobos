#include "Body.h"
#include <SpecificStructures.h>

#include <Utilities/GeneralUtils.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

// #issue 88 : shield logic
DEFINE_HOOK(701900, TechnoClass_ReceiveDamage_Shield, 6)
{
	GET(TechnoClass*, pThis, ECX);
	LEA_STACK(args_ReceiveDamage*, args, 0x4);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (auto pShieldData = pExt->ShieldData.get())
	{
		if (!pShieldData->IsAvailable())
			return 0;

		auto nDamageLeft = pShieldData->ReceiveDamage(args);
		if (nDamageLeft >= 0)
			*args->Damage = nDamageLeft;
	}

	return 0;
}

DEFINE_HOOK(7019D8, TechnoClass_ReceiveDamage_SkipLowDamageCheck, 5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int*, Damage, EBX);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (auto pShieldData = pExt->ShieldData.get())
	{
		if (pShieldData->IsAvailable() && pShieldData->GetHP())
			return 0x7019E3;
	}

	return *Damage >= 0 ? 0x7019E3 : 0x7019DD;
}

DEFINE_HOOK_AGAIN(70CF39, TechnoClass_ReplaceArmorWithShields, 6) //TechnoClass_EvalThreatRating_Shield
DEFINE_HOOK_AGAIN(6F7D31, TechnoClass_ReplaceArmorWithShields, 6) //TechnoClass_CanAutoTargetObject_Shield
DEFINE_HOOK_AGAIN(6FCB64, TechnoClass_ReplaceArmorWithShields, 6) //TechnoClass_CanFire_Shield
DEFINE_HOOK(708AEB, TechnoClass_ReplaceArmorWithShields, 6) //TechnoClass_ShouldRetaliate_Shield
{
	WeaponTypeClass* pWeapon = nullptr;
	if (R->Origin() == 0x708AEB)
		pWeapon = R->ESI<WeaponTypeClass*>();
	else if (R->Origin() == 0x6F7D31)
		pWeapon = R->EBP<WeaponTypeClass*>();
	else
		pWeapon = R->EBX<WeaponTypeClass*>();

	if (auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead))
		if (pWHExt->PenetratesShield)
			return 0;
	
	TechnoClass* pTarget = nullptr;
	if (R->Origin() == 0x6F7D31 || R->Origin() == 0x70CF39)
		pTarget = R->ESI<TechnoClass*>();
	else
		pTarget = R->EBP<TechnoClass*>();

	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		if (auto pShieldData = pExt->ShieldData.get())
		{
			if (pShieldData->IsAvailable() && pShieldData->GetHP())
			{
				R->EAX(TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType())->Shield->Armor);
				return R->Origin() + 6;
			}
		}
	}

	return 0;
}

//Abandoned because of Ares!!!! - Uranusian
/*
DEFINE_HOOK_AGAIN(6F3725, TechnoClass_WhatWeaponShouldIUse_Shield, 6)
DEFINE_HOOK(6F36F2, TechnoClass_WhatWeaponShouldIUse_Shield, 6)
{
	GET(TechnoClass*, pTarget, EBP);
	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		if (auto pShieldData = pExt->ShieldData.get())
		{
			if (pShieldData->GetHP())
			{
				auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());

				if (R->Origin() == 0x6F36F2)
					R->ECX(pTypeExt->Shield_Armor);
				else
					R->EAX(pTypeExt->Shield_Armor);

				return R->Origin() + 6;
			}
		}
	}
	return 0;
}
*/

DEFINE_HOOK(6F36DB, TechnoClass_WhatWeaponShouldIUse_Shield, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EBP);
	enum { Primary = 0x6F37AD, Secondary = 0x6F3745, FurtherCheck = 0x6F3754, OriginalCheck = 0x6F36E3 };

	if (!pTarget)
		return 0x6F37AD; //Target invalid, skip. (test ebp,ebp  jz loc_6F37AD)

	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		if (auto pShieldData = pExt->ShieldData.get())
		{
			if (pShieldData->IsAvailable() && pShieldData->GetHP())
			{
				if (pThis->GetWeapon(1))
				{
					if (!pShieldData->CanBeTargeted(pThis->GetWeapon(0)->WeaponType))
						return Secondary;

					return FurtherCheck;
				}

				return Primary;
			}
		}
	}
	return OriginalCheck;
}

DEFINE_HOOK(6F9E50, TechnoClass_AI_Shield, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeData->Shield->Strength && !pExt->ShieldData)
		pExt->ShieldData = std::make_unique<ShieldClass>(pThis);

	if (pExt->ShieldData)
		pExt->ShieldData->AI();

	return 0;
}

DEFINE_HOOK(6F6AC4, TechnoClass_Remove_Shield, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->ShieldData &&
		!(pThis->WhatAmI() == AbstractType::Building && pThis->GetTechnoType()->UndeploysInto && pThis->CurrentMission == Mission::Selling))
	{
		pExt->ShieldData = nullptr;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(44A03C, DeploysInto_UndeploysInto_SyncShieldStatus, 6) //BuildingClass_Mi_Selling_SyncShieldStatus
DEFINE_HOOK(739956, DeploysInto_UndeploysInto_SyncShieldStatus, 6) //UnitClass_Deploy_SyncShieldStatus
{
	GET(TechnoClass*, pThis, EBP);
	GET(TechnoClass*, pInto, EBX);
	auto pThisExt = TechnoExt::ExtMap.Find(pThis);
	auto pIntoTypeExt = TechnoTypeExt::ExtMap.Find(pInto->GetTechnoType());

	if (pThisExt->ShieldData && pIntoTypeExt->Shield->Strength)
	{
		ShieldClass::SyncShieldToAnother(pThis, pInto);
	}

	if (pThis->WhatAmI() == AbstractType::Building && pThisExt->ShieldData)
		pThisExt->ShieldData = nullptr;

	return 0;
}


DEFINE_HOOK(6F65D1, TechnoClass_DrawHealthBar_DrawBuildingShieldBar, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, iLength, EBX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x4C, -0x4));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFS(0x4C, -0x8));
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->ShieldData && pExt->ShieldData->IsAvailable())
		pExt->ShieldData->DrawShieldBar(iLength, pLocation, pBound);

	return 0;
}

DEFINE_HOOK(6F683C, TechnoClass_DrawHealthBar_DrawOtherShieldBar, 7)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x4C, -0x4));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFS(0x4C, -0x8));
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->ShieldData && pExt->ShieldData->IsAvailable())
	{
		int iLength = pThis->WhatAmI() == AbstractType::Infantry ? 8 : 17;
		pExt->ShieldData->DrawShieldBar(iLength, pLocation, pBound);
	}

	return 0;
}