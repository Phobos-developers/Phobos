#include "Body.h"
#include <LocomotionClass.h>
#include <TeleportLocomotionClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

#define GET_LOCO(reg_Loco) \
	GET(ILocomotion *, Loco, reg_Loco); \
	__assume(Loco!=nullptr);\
	TeleportLocomotionClass *pLocomotor = static_cast<TeleportLocomotionClass*>(Loco); \
	TechnoTypeClass *pType = pLocomotor->LinkedTo->GetTechnoType(); \
	TechnoTypeExt::ExtData *pExt = TechnoTypeExt::ExtMap.Find(pType);

DEFINE_HOOK(0x719439, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 0x6)
{
	GET_LOCO(ESI);

	R->EDX<AnimTypeClass*>(pExt->WarpOut.Get(RulesClass::Instance->WarpOut));

	if (pExt->WarpOutWeapon.isset())
		WeaponTypeExt::DetonateAt(pExt->WarpOutWeapon.Get(), pLocomotor->LinkedTo, pLocomotor->LinkedTo);

	return 0x71943F;
}

DEFINE_HOOK(0x719788, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 0x6)
{
	GET_LOCO(ESI);

	R->EDX<AnimTypeClass*>(pExt->WarpIn.Get(RulesClass::Instance->WarpIn));

	auto pTechnoExt = TechnoExt::ExtMap.Find(pLocomotor->LinkedTo);

	bool isInMinRange = pTechnoExt->LastWarpDistance < pExt->ChronoRangeMinimum.Get(RulesClass::Instance->ChronoRangeMinimum);

	auto weaponType = isInMinRange ? pExt->WarpInMinRangeWeapon.Get(pExt->WarpInWeapon.isset() ? pExt->WarpInWeapon.Get() : nullptr) :
		pExt->WarpInWeapon.isset() ? pExt->WarpInWeapon.Get() : nullptr;

	if (weaponType)
	{
		int damage = pExt->WarpInWeapon_UseDistanceAsDamage ? pTechnoExt->LastWarpDistance / Unsorted::LeptonsPerCell : weaponType->Damage;
		WeaponTypeExt::DetonateAt(weaponType, pLocomotor->LinkedTo, pLocomotor->LinkedTo, damage);
	}

	return 0x71978E;
}

DEFINE_HOOK(0x71986A, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 0x6)
{
	GET_LOCO(ESI);

	R->ECX<AnimTypeClass*>(pExt->WarpAway.Get(RulesClass::Instance->WarpOut));

	return 0x719870;
}

DEFINE_HOOK(0x7194D0, TeleportLocomotionClass_ILocomotion_Process_ChronoTrigger, 0x6)
{
	GET_LOCO(ESI);

	R->AL(pExt->ChronoTrigger.Get(RulesClass::Instance->ChronoTrigger));

	return 0x7194D6;
}

DEFINE_HOOK(0x7194E3, TeleportLocomotionClass_ILocomotion_Process_ChronoDistanceFactor, 0x6)
{
	GET_LOCO(ESI);
	GET(int, val, EAX);

	auto factor = pExt->ChronoDistanceFactor.Get(RulesClass::Instance->ChronoDistanceFactor);
	factor = factor == 0 ? 1 : factor; //fix factor 0 crash by force it to 1 (Vanilla bug)

	//IDIV
	R->EAX(val / factor);
	R->EDX(val % factor);

	return 0x7194E9;
}

DEFINE_HOOK(0x719519, TeleportLocomotionClass_ILocomotion_Process_ChronoMinimumDelay, 0x6)
{
	GET_LOCO(ESI);

	R->EBX(pExt->ChronoMinimumDelay.Get(RulesClass::Instance->ChronoMinimumDelay));

	return 0x71951F;
}

DEFINE_HOOK(0x719562, TeleportLocomotionClass_ILocomotion_Process_ChronoMinimumDelay2, 0x6)
{
	GET_LOCO(ESI);

	R->ECX(pExt->ChronoMinimumDelay.Get(RulesClass::Instance->ChronoMinimumDelay));

	return 0x719568;
}

DEFINE_HOOK(0x719555, TeleportLocomotionClass_ILocomotion_Process_ChronoRangeMinimum, 0x6)
{
	GET_LOCO(ESI);
	GET(int, comparator, EDX);

	TechnoExt::ExtData* pTechnoExt = TechnoExt::ExtMap.Find(pLocomotor->LinkedTo);

	pTechnoExt->LastWarpDistance = comparator;

	auto factor = pExt->ChronoRangeMinimum.Get(RulesClass::Instance->ChronoRangeMinimum);

	return comparator < factor ? 0x71955D : 0x719576;
}

DEFINE_HOOK(0x71997B, TeleportLocomotionClass_ILocomotion_Process_ChronoDelay, 0x6)
{
	GET_LOCO(ESI);

	R->ECX(pExt->ChronoDelay.Get(RulesClass::Instance->ChronoDelay));

	return 0x719981;
}

#undef GET_LOCO
