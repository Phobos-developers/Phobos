#include "Body.h"
#include <LocomotionClass.h>
#include <TeleportLocomotionClass.h>
#include <Conversions.h>

#define GET_LOCO(reg_Loco) \
	GET(ILocomotion *, Loco, reg_Loco); \
	TeleportLocomotionClass *pLocomotor = static_cast<TeleportLocomotionClass*>(Loco); \
	TechnoTypeClass *pType = pLocomotor->LinkedTo->GetTechnoType(); \
	TechnoTypeExt::ExtData *pExt = TechnoTypeExt::ExtMap.Find(pType);

namespace TempHelper
{
	AnimTypeClass* GetAnimTypeFromFacing(FootClass* pFoot,std::vector<AnimTypeClass*>& nAnimVec, AnimTypeClass* pDefault = nullptr)
	{
		if (!pFoot)
			return pDefault;

		if (nAnimVec.size() > 0)
		{
			auto highest = Conversions::Int2Highest((int)nAnimVec.size());

			// 2^highest is the frame count, 3 means 8 frames
			if (highest >= 3)
			{
				auto offset = 1u << (highest - 3);
				auto index = TranslateFixedPoint(16, highest, static_cast<WORD>(pFoot->GetRealFacing().value()), offset);
				pDefault = nAnimVec.at(index);
			}
			else
			{
				pDefault = nAnimVec.at(0);
			}
		}
	
		return pDefault;
	}
}

DEFINE_HOOK(0x719439, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 0x6)
{
	GET_LOCO(ESI);

	R->EDX<AnimTypeClass*>(TempHelper::GetAnimTypeFromFacing(pLocomotor->LinkedTo, pExt->WarpOut, RulesClass::Instance->WarpOut));

	return 0x71943F;
}

DEFINE_HOOK(0x719788, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 0x6)
{
	GET_LOCO(ESI);

	R->EDX<AnimTypeClass*>(TempHelper::GetAnimTypeFromFacing(pLocomotor->LinkedTo, pExt->WarpIn, RulesClass::Instance->WarpOut));

	return 0x71978E;
}

DEFINE_HOOK(0x71986A, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 0x6)
{
	GET_LOCO(ESI);

	R->ECX<AnimTypeClass*>(TempHelper::GetAnimTypeFromFacing(pLocomotor->LinkedTo, pExt->WarpAway, RulesClass::Instance->WarpOut));

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
	factor = Math::min(1, factor); //fix factor 0 crash by force it to 1 (Vanilla bug)

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