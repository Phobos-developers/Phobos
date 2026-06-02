#include "Body.h"

#include <New/PrismRelay.h>

DEFINE_HOOK(0x6FDDC0, TechnoClass_FireAt_PrismRelay, 0x6){
	enum { SkipFiring = 0x6FDE03 };

	GET(TechnoClass* const, pThis, ESI);
	GET_BASE(AbstractClass* const, pTarget, 0x8);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET_BASE(const int, weaponIndex, 0xC);

	if (PrismRelay::TryHandleFireAt(pThis, pTarget, pWeapon, weaponIndex))
		return SkipFiring;

	return 0;
}

DEFINE_HOOK(0x6FF660, TechnoClass_FireAt_LateLogic_PrismRelay, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	if (pThis->CurrentBurstIndex == 0 && pWeapon->Burst > 0)
	{
		if (auto const pExt = TechnoExt::ExtMap.TryFind(pThis))
		{
			pExt->PrismRelayBurstChainBuilt = false;
			pExt->PrismRelayCachedNetworkId = 0;
			pExt->PrismRelayCachedProviders.clear();
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F9E50, TechnoClass_AI_PrismRelay, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	PrismRelay::UpdateSessionTimeouts(pThis);

	return 0;
}
