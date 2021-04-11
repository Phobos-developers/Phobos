#include "Body.h"
#include <SpecificStructures.h>

#include "../TechnoType/Body.h"

// #issue 88 : shield logic
DEFINE_HOOK(701900, TechnoClass_ReceiveDamage_Shield, 6)
{
	GET(TechnoClass*, pThis, ECX);
	LEA_STACK(args_ReceiveDamage*, args, 0x4);
	//GET_STACK(int*, pDamage, 0x4);
	//GET_STACK(WarheadTypeClass*, pWH, 0xC);
	//GET_STACK(HouseClass*, pSourceHouse, -0x1C);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (auto pShieldData = pExt->ShieldData.get())
	{
		auto nDamageLeft = pShieldData->ReceiveDamage(args);

		if (nDamageLeft >= 0)
		{
			*args->Damage = nDamageLeft;
		}
	}

	return 0;
}

DEFINE_HOOK(6FC339, TechnoClass_CanFire_Shield, 6)
{
	GET_STACK(TechnoClass*, pTarget, STACK_OFFS(0x20, -0x4));
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		if (auto pShieldData = pExt->ShieldData.get())
		{
			if (!pShieldData->CanBeTargeted(pWeapon, pThis))
			{
				return 0x6FCB7E;
			}
		}
	}
	return 0;
}

DEFINE_HOOK(6F36F2, TechnoClass_WhatWeaponShouldIUse1, 6)
{
	GET(TechnoClass*, pTarget, EBP);
	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		if (auto pShieldData = pExt->ShieldData.get())
		{
			if (!pShieldData->IsShieldBroken())
			{
				auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());
				R->ECX(pTypeExt->Shield_Armor);
				return R->Origin() + 6;
			}
		}
	}
	return 0;
}
DEFINE_HOOK(6F3725, TechnoClass_WhatWeaponShouldIUse2, 6)
{
	GET(TechnoClass*, pTarget, EBP);
	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		if (auto pShieldData = pExt->ShieldData.get())
		{
			if (!pShieldData->IsShieldBroken())
			{
				auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());
				R->EAX(pTypeExt->Shield_Armor);
				return R->Origin() + 6;
			}
		}
	}
	return 0;
}

DEFINE_HOOK(6F9E50, TechnoClass_AI_Shield, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeData->Shield_Strength)
	{
		if (!pExt->ShieldData)
		{
			pExt->ShieldData = std::make_unique<ShieldTechnoClass>(pThis);
		}

        pExt->ShieldData->AI();
    }
    return 0;
}

DEFINE_HOOK(6F6AC4, TechnoClass_Remove_Shield, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->ShieldData)
	{
		pExt->ShieldData = nullptr;
	}

	return 0;
}

DEFINE_HOOK(6F65D1, TechnoClass_DrawHealthBar_DrawBuildingShieldBar, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, iLength, EBX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x4C, -0x4));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFS(0x4C, -0x8));
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->ShieldData)
	{
		pExt->ShieldData->DrawShieldBar(iLength, pLocation, pBound);
	}

	return 0;
}

DEFINE_HOOK(6F683C, TechnoClass_DrawHealthBar_DrawOtherShieldBar, 7)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x4C, -0x4));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFS(0x4C, -0x8));
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->ShieldData)
	{
		int iLength = pThis->WhatAmI() == AbstractType::Infantry ? 8 : 17;
		pExt->ShieldData->DrawShieldBar(iLength, pLocation, pBound);
	}

	return 0;
}