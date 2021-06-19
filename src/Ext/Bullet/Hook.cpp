#include "Body.h"
#include <Ext/WarheadType/Body.h>
#include <Misc/CaptureManager.h>


#include <TechnoClass.h>

// has everything inited except SpawnNextAnim at this point
DEFINE_HOOK(466556, BulletClass_Init_SetLaserTrail, 6)
{
	GET(BulletClass*, pThis, ECX);
	
	if (!pThis->Type->Inviso)
		BulletExt::InitializeLaserTrail(pThis);

	return 0;
}

DEFINE_HOOK(4666F7, BulletClass_AI, 6)
{
	GET(BulletClass*, pThis, EBP);
	auto pBulletExt = BulletExt::ExtMap.Find(pThis);

	if (!pBulletExt)
		return 0;

	if (pBulletExt->ShouldIntercept)
	{
		pThis->Detonate(pThis->GetCoords());
		pThis->Remove();
		pThis->UnInit();

		const auto pTechno = pThis->Owner;
		const bool isLimbo =
			pTechno &&
			pTechno->InLimbo &&
			pThis->WeaponType &&
			pThis->WeaponType->LimboLaunch;

		if (isLimbo) {
			pThis->SetTarget(nullptr);
			auto damage = pTechno->Health * 2;
			pTechno->SetLocation(pThis->GetCoords());
			pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}

	if (pBulletExt->Intercepted)
		pBulletExt->ShouldIntercept = true;

	return 0;
}

DEFINE_HOOK(4680E2, BulletClass_DrawIt_LaserTrail, 6)
{
	GET(BulletClass*, pThis, ESI);
	auto pBulletExt = BulletExt::ExtMap.Find(pThis);

	if (pBulletExt && pBulletExt->LaserTrail)
	{
		CoordStruct location = pThis->GetCoords();
        BulletVelocity velocity = pThis->Velocity;

        CoordStruct drawnCoords = CoordStruct
		{
			(int)(location.X + velocity.X),
			(int)(location.Y + velocity.Y),
			(int)(location.Z + velocity.Z)
		};

		pBulletExt->LaserTrail->Draw(drawnCoords);
	}

	return 0;
}

DEFINE_HOOK(4692BD, BulletClass_Logics_ApplyMindControl, 6)
{
	GET(BulletClass*, pThis, ESI);

	auto pTypeExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto pControlledAnimType = pTypeExt->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);
	auto pTechno = generic_cast<TechnoClass*>(pThis->Target);

	R->AL(CaptureManager::CaptureUnit(pThis->Owner->CaptureManager, pTechno, pControlledAnimType));

	return 0x4692D5;
}
