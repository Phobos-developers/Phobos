#include "Body.h"
#include <Helpers/Macro.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <TechnoClass.h>

#include "../BulletType/Body.h"

DEFINE_HOOK(4666F7, BulletClass_Update, 6)
{
	GET(BulletClass*, pThis, EBP);

	auto pExt = BulletExt::ExtMap.Find(pThis);
	if (pExt->ShouldIntercept)
	{
		auto& pTechno = pThis->Owner;
		auto damage = pTechno->Health * 2;
		bool bShouldKillTechno = pThis->WeaponType->LimboLaunch;

		pThis->SetTarget(nullptr);
		pThis->Detonate(pThis->GetCoords());
		pThis->Remove();
		pThis->UnInit();

		if (bShouldKillTechno) {
			pTechno->SetLocation(pThis->GetCoords());
			pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}

	if (pExt->Intercepted)
		pExt->ShouldIntercept = true;

	return 0;
}
