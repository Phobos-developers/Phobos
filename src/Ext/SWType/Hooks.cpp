#include "Body.h"
/*
DEFINE_HOOK(0x6A84B1, StripClass_OperatorLessThan_CameoPriorityForSW, 0x6)
{
	GET(SuperWeaponTypeClass*, pLeft, EAX);
	GET(SuperWeaponTypeClass*, pRight, ECX);
	auto pLeftExt = SWTypeExt::ExtMap.Find(pLeft);
	auto pRightExt = SWTypeExt::ExtMap.Find(pRight);
	auto leftPriority = pLeftExt->CameoPriority;
	auto rightPriority = pRightExt->CameoPriority;
	enum { rTrue = 0x6A8692, rFalse = 0x6A86A0 };
	if (leftPriority > rightPriority)
		return rTrue;
	else if (rightPriority > leftPriority)
		return rFalse;
	return 0;
}
*/