#include "Body.h"

DEFINE_HOOK(0x5047D0, HouseClass_UpdateAngerNodes_SetForceEnemy, 0x6)
{
	GET(HouseClass*, pThis, EAX);
	enum { ReturnValue = 0x50483F };

	if (pThis)
	{
		int forceIndex = HouseExt::ExtMap.Find(pThis)->GetForceEnemyIndex();

		if (forceIndex >= 0 || forceIndex == -2)
		{
			R->EDX(forceIndex == -2 ? -1 : forceIndex);
			return ReturnValue;
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x4F9BFC, HouseClass_ClearForceEnemy, 0xA)	// HouseClass_MakeAlly
DEFINE_HOOK(0x4FD772, HouseClass_ClearForceEnemy, 0xA)			// HouseClass_UpdateAI
{
	HouseClass* pThis = nullptr;

	if (R->Origin() == 0x4FD772)
		pThis = R->EBX<HouseClass*>();
	else
		pThis = R->ESI<HouseClass*>();

	if (pThis)
	{
		HouseExt::ExtMap.Find(pThis)->SetForceEnemyIndex(-1);
		pThis->UpdateAngerNodes(0, nullptr);
	}

	return 0;
}
