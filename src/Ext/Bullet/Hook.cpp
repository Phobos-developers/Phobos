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
	if (pExt && pExt->ShouldIntercept)
	{
		pThis->Detonate(pThis->GetCoords());
		pThis->Remove();
		pThis->UnInit();

		const auto pTechno = pThis->Owner;
		if (pTechno && pThis->WeaponType->LimboLaunch) {
			pThis->SetTarget(nullptr);
			auto damage = pTechno->Health * 2;
			pTechno->SetLocation(pThis->GetCoords());
			pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}

	if (pExt->Intercepted)
		pExt->ShouldIntercept = true;

	return 0;
}
