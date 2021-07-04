#include "Body.h"
#include <Ext/WarheadType/Body.h>
#include <Misc/CaptureManager.h>

#include <TechnoClass.h>

DEFINE_HOOK(0x4666F7, BulletClass_AI, 0x6)
{
	GET(BulletClass*, pBullet, EBP);

	if (auto pBulletExt = BulletExt::ExtMap.Find(pBullet))
	{
		if (pBulletExt->ShouldIntercept)
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

			if (isLimbo)
			{
				pBullet->SetTarget(nullptr);
				auto damage = pTechno->Health * 2;
				pTechno->SetLocation(pBullet->GetCoords());
				pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			}
		}

		if (pBulletExt->Intercepted)
			pBulletExt->ShouldIntercept = true;

		if (pBullet->Type->Arcing && !pBulletExt->ArcingFixed)
			pBulletExt->ApplyArcingFix();
	}
	return 0;
}

DEFINE_HOOK(0x4692BD, BulletClass_Logics_ApplyMindControl, 0x6)
{
	GET(BulletClass*, pBullet, ESI);
	auto pTypeExt = WarheadTypeExt::ExtMap.Find(pBullet->WH);
	auto pControlledAnimType = pTypeExt->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);
	auto pTechno = generic_cast<TechnoClass*>(pBullet->Target);
	R->AL(CaptureManager::CaptureUnit(pBullet->Owner->CaptureManager, pTechno, pControlledAnimType));
	return 0x4692D5;
}
