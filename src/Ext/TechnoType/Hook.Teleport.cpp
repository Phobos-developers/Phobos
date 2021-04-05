#include "Body.h"
#include <LocomotionClass.h>
#include <TeleportLocomotionClass.h>


DEFINE_HOOK(719439, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 6)
{
	GET(ILocomotion *, Loco, ESI);
	auto pLocomotor = static_cast<TeleportLocomotionClass*>(Loco);
	auto const pType = pLocomotor->LinkedTo->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);
	
	R->EDX<AnimTypeClass*>(pExt->WarpOut.Get(RulesClass::Instance->WarpOut));

	return 0x71943F;
}


DEFINE_HOOK(719788, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 6)
{
	GET(ILocomotion *, Loco, ESI);
	auto pLocomotor = static_cast<TeleportLocomotionClass*>(Loco);
	auto const pType = pLocomotor->LinkedTo->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	R->EDX<AnimTypeClass*>(pExt->WarpIn.Get(RulesClass::Instance->WarpIn));
			
	
	return 0x71978E;
}

DEFINE_HOOK(71986A, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 6)
{
	
	GET(ILocomotion *, Loco, ESI);
	auto pLocomotor = static_cast<TeleportLocomotionClass*>(Loco);
	auto const pType = pLocomotor->LinkedTo->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	R->EDX<AnimTypeClass*>(pExt->WarpAway.Get(RulesClass::Instance->WarpAway));
	return 0x719870;
}

//ChronoTrigger
DEFINE_HOOK(7194D0, TeleportLocomotionClass_ILocomotion_Process_ChronoTrigger, 6)
{
	GET(ILocomotion *, Loco, ESI);
	auto pLocomotor = static_cast<TeleportLocomotionClass*>(Loco);
	auto const pType = pLocomotor->LinkedTo->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	R->AL(pExt->ChronoTrigger.Get(RulesClass::Instance->ChronoTrigger));
	return 0x7194D6;

}

// ChronoDistanceFactor
DEFINE_HOOK(7194E3, TeleportLocomotionClass_ILocomotion_Process_ChronoDistanceFactor, 6)
{
	GET(ILocomotion *, Loco, ESI);
	auto pLocomotor = static_cast<TeleportLocomotionClass*>(Loco);
	auto const pType = pLocomotor->LinkedTo->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto factor = pExt->ChronoDistanceFactor.Get(RulesClass::Instance->ChronoDistanceFactor);

	__asm xor edx, edx; // fixing integer overflow crash
	__asm idiv factor;
			
	return 0x7194E9;
}

//ChronoMinimumDelay
DEFINE_HOOK(719519, TeleportLocomotionClass_ILocomotion_Process_ChronoMinimumDelay, 6)
{
	GET(ILocomotion *, Loco, ESI);
	auto pLocomotor = static_cast<TeleportLocomotionClass*>(Loco);
	auto const pType = pLocomotor->LinkedTo->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	R->EBX(pExt->ChronoMinimumDelay.Get(RulesClass::Instance->ChronoMinimumDelay));
	return 0x71951F;
}

//719562 , esi , 6
DEFINE_HOOK(719562, TeleportLocomotionClass_ILocomotion_Process_ChronoMinimumDelay2, 6)
{
	GET(ILocomotion *, Loco, ESI);
	auto pLocomotor = static_cast<TeleportLocomotionClass*>(Loco);
	auto const pType = pLocomotor->LinkedTo->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	R->ECX(pExt->ChronoMinimumDelay.Get(RulesClass::Instance->ChronoMinimumDelay));
	return 0x71951F;
}

//ChronoRangeMinimum
//719555 , esi  jump if greater edx , 6
DEFINE_HOOK(719555, TeleportLocomotionClass_ILocomotion_Process_ChronoRangeMinimum, 6)
{
	GET(int, comparator, EDX);
	GET(ILocomotion *, Loco, ESI);
	auto pLocomotor = static_cast<TeleportLocomotionClass*>(Loco);
	auto const pType = pLocomotor->LinkedTo->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto factor = pExt->ChronoRangeMinimum.Get(RulesClass::Instance->ChronoRangeMinimum);

	return comparator < factor ? 0x71955D : 0x719576;
}

//ChronoDelay
//71997B ,esi ,R->ECX ,719981 , 6
DEFINE_HOOK(71997B, TeleportLocomotionClass_ILocomotion_Process_ChronoDelay, 6)
{
	GET(ILocomotion *, Loco, ESI);
	auto pLocomotor = static_cast<TeleportLocomotionClass*>(Loco);
	auto const pType = pLocomotor->LinkedTo->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	R->ECX(pExt->ChronoDelay.Get(RulesClass::Instance->ChronoDelay));

	return 0x719981;
}
