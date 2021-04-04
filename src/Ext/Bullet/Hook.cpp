#include "Body.h"
#include "../WarheadType/Body.h"
#include "../../Misc/CaptureManager.h"


#include <TechnoClass.h>

DEFINE_HOOK(4666F7, BulletClass_Update, 6)
{
	GET(BulletClass*, pBullet, EBP);

	auto pBulletExt = BulletExt::ExtMap.Find(pBullet);
	if (pBulletExt && pBulletExt->ShouldIntercept)
	{
		pBullet->Detonate(pBullet->GetCoords());
		pBullet->Remove();
		pBullet->UnInit();

		const auto pTechno = pBullet->Owner;
		const bool isLimbo =
			pTechno &&
			pTechno->InLimbo &&
			pBullet->WeaponType &&
			pBullet->WeaponType->LimboLaunch;

		if (isLimbo) {
			pBullet->SetTarget(nullptr);
			auto damage = pTechno->Health * 2;
			pTechno->SetLocation(pBullet->GetCoords());
			pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}

	if (pBulletExt && pBulletExt->Intercepted)
		pBulletExt->ShouldIntercept = true;

	return 0;
}

DEFINE_HOOK(4692BD, BulletClass_Fire_ApplyMindControl, 6)
{
	GET(BulletClass*, pBullet, ESI);

	auto pControlledAnimType = RulesClass::Instance->ControlledAnimationType;
	if (const auto pTypeExt = WarheadTypeExt::ExtMap.Find(pBullet->WH))
		if (pTypeExt->ControlledAnimationType)
			pControlledAnimType = pTypeExt->ControlledAnimationType;

	const auto pTechno = generic_cast<TechnoClass*>(pBullet->Target);
	
	R->AL(CaptureManager::CaptureUnit(pBullet->Owner->CaptureManager, pTechno, pControlledAnimType));
	

	return 0x4692D5;
}
