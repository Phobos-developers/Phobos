#include "Body.h"
#include <LocomotionClass.h>
#include <TeleportLocomotionClass.h>

#define GET_LOCO(reg_Loco) \
	GET(ILocomotion *, Loco, reg_Loco); \
	TeleportLocomotionClass *pLocomotor = static_cast<TeleportLocomotionClass*>(Loco); \
	TechnoTypeClass *pType = pLocomotor->LinkedTo->GetTechnoType(); \
	TechnoTypeExt::ExtData *pExt = TechnoTypeExt::ExtMap.Find(pType);

DEFINE_HOOK(719439, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 6)
{
	GET_LOCO(ESI);

	R->EDX<AnimTypeClass*>(pExt->WarpOut.Get(RulesClass::Instance->WarpOut));

	return 0x71943F;
}

DEFINE_HOOK(719788, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 6)
{
	GET_LOCO(ESI);

	R->EDX<AnimTypeClass*>(pExt->WarpIn.Get(RulesClass::Instance->WarpOut));

	return 0x71978E;
}

DEFINE_HOOK(71986A, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 6)
{
	GET_LOCO(ESI);

	R->ECX<AnimTypeClass*>(pExt->WarpAway.Get(RulesClass::Instance->WarpOut));

	return 0x719870;
}

DEFINE_HOOK(7194D0, TeleportLocomotionClass_ILocomotion_Process_ChronoTrigger, 6)
{
	GET_LOCO(ESI);

	R->AL(pExt->ChronoTrigger.Get(RulesClass::Instance->ChronoTrigger));

	return 0x7194D6;
}

DEFINE_HOOK(7194E3, TeleportLocomotionClass_ILocomotion_Process_ChronoDistanceFactor, 6)
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

DEFINE_HOOK(719519, TeleportLocomotionClass_ILocomotion_Process_ChronoMinimumDelay, 6)
{
	GET_LOCO(ESI);

	R->EBX(pExt->ChronoMinimumDelay.Get(RulesClass::Instance->ChronoMinimumDelay));

	return 0x71951F;
}

DEFINE_HOOK(719562, TeleportLocomotionClass_ILocomotion_Process_ChronoMinimumDelay2, 6)
{
	GET_LOCO(ESI);

	R->ECX(pExt->ChronoMinimumDelay.Get(RulesClass::Instance->ChronoMinimumDelay));

	return 0x719568;
}

DEFINE_HOOK(719555, TeleportLocomotionClass_ILocomotion_Process_ChronoRangeMinimum, 6)
{
	GET_LOCO(ESI);
	GET(int, comparator, EDX);

	auto factor = pExt->ChronoRangeMinimum.Get(RulesClass::Instance->ChronoRangeMinimum);

	return comparator < factor ? 0x71955D : 0x719576;
}

DEFINE_HOOK(71997B, TeleportLocomotionClass_ILocomotion_Process_ChronoDelay, 6)
{
	GET_LOCO(ESI);

	R->ECX(pExt->ChronoDelay.Get(RulesClass::Instance->ChronoDelay));

	return 0x719981;
}

#undef GET_LOCO