#include "Body.h"

#include "../TechnoType/Body.h"
#include "../Techno/Body.h"
#include "../Bullet/Body.h"

DEFINE_HOOK(6FF660, TechnoClass_FireBullet, 6)
{
	GET(TechnoClass* const, pSource, ESI);
	GET_BASE(AbstractClass* const, pTarget, 0x8);
	//GET(WeaponTypeClass* const, pWeaponType, EBX);

	// Interceptor
	auto const pSourceTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
	if (pSourceTypeExt->Interceptor) {
		if (auto const pTargetObject = specific_cast<BulletClass* const>(pTarget))
			if (auto const pSourceExt = TechnoExt::ExtMap.Find(pSource))
				if (pSourceExt->InterceptedBullet && pSourceExt->InterceptedBullet->IsAlive) {
					auto const pBulletExt = BulletExt::ExtMap.Find(pSourceExt->InterceptedBullet);
					pBulletExt->Intercepted = true;
					pSourceExt->InterceptedBullet = nullptr;
				}
	}

	return 0;
}
