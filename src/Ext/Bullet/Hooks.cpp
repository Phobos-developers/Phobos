#include "Body.h"
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Misc/CaptureManager.h>

#include <TechnoClass.h>
#include <TacticalClass.h>

// has everything inited except SpawnNextAnim at this point
DEFINE_HOOK(0x466556, BulletClass_Init_SetLaserTrail, 0x6)
{
	GET(BulletClass*, pThis, ECX);

	if (!pThis->Type->Inviso)
		BulletExt::InitializeLaserTrails(pThis);

	return 0;
}

DEFINE_HOOK(0x4666F7, BulletClass_AI, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	auto pBulletExt = BulletExt::ExtMap.Find(pThis);

	if (!pBulletExt)
		return 0;

	if (pBulletExt->ShouldIntercept)
	{
		pThis->Detonate(pThis->GetCoords());
		pThis->Limbo();
		pThis->UnInit();

		const auto pTechno = pThis->Owner;
		const bool isLimbo =
			pTechno &&
			pTechno->InLimbo &&
			pThis->WeaponType &&
			pThis->WeaponType->LimboLaunch;

		if (isLimbo)
		{
			pThis->SetTarget(nullptr);
			auto damage = pTechno->Health * 2;
			pTechno->SetLocation(pThis->GetCoords());
			pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}

	if (pBulletExt->Intercepted)
		pBulletExt->ShouldIntercept = true;

	// LaserTrails update routine is in BulletClass::AI hook because BulletClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	if (pBulletExt && pBulletExt->LaserTrails.size())
	{
		CoordStruct location = pThis->GetCoords();
		const BulletVelocity& velocity = pThis->Velocity;

		// We adjust LaserTrails to account for vanilla bug of drawing stuff one frame ahead.
		// Pretty meh solution but works until we fix the bug - Kerbiter
		CoordStruct drawnCoords
		{
			(int)(location.X + velocity.X),
			(int)(location.Y + velocity.Y),
			(int)(location.Z + velocity.Z)
		};

		for (auto const& trail : pBulletExt->LaserTrails)
		{
			// We insert initial position so the first frame of trail doesn't get skipped - Kerbiter
			// TODO move hack to BulletClass creation
			if (!trail->LastLocation.isset())
				trail->LastLocation = location;

			trail->Update(drawnCoords);
		}

	}

	return 0;
}

DEFINE_HOOK(0x4692BD, BulletClass_Logics_ApplyMindControl, 0x6)
{
	GET(BulletClass*, pThis, ESI);

	auto pTypeExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto pControlledAnimType = pTypeExt->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);
	auto pTechno = generic_cast<TechnoClass*>(pThis->Target);

	R->AL(CaptureManager::CaptureUnit(pThis->Owner->CaptureManager, pTechno, pControlledAnimType));

	return 0x4692D5;
}

DEFINE_HOOK(0x4671B9, BulletClass_AI_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x4671BF;
}

DEFINE_HOOK(0x6F7481, TechnoClass_Targeting_ApplyGravity, 0x6)
{
	GET(WeaponTypeClass* const, pWeaponType, EDX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pWeaponType->Projectile);
	__asm { fld nGravity };

	return 0x6F74A4;
}

DEFINE_HOOK(0x6FDAA6, TechnoClass_FireAngle_6FDA00_ApplyGravity, 0x5)
{
	GET(WeaponTypeClass* const, pWeaponType, EDI);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pWeaponType->Projectile);
	__asm { fld nGravity };

	return 0x6FDACE;
}

DEFINE_HOOK(0x6FECB2, TechnoClass_FireAt_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x6FECD1;
}

DEFINE_HOOK(0x772A0A, WeaponTypeClass_SetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x772A29;
}

DEFINE_HOOK(0x773087, WeaponTypeClass_GetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x7730A3;
}

DEFINE_HOOK(0x46A3D6, BulletClass_Shrapnel_Forced, 0xA)
{
	enum { Shrapnel = 0x46A40C, Skip = 0x46ADCD };

	GET(BulletClass*, pBullet, EDI);

	auto const pData = BulletTypeExt::ExtMap.Find(pBullet->Type);

	if (auto const pObject = pBullet->GetCell()->FirstObject)
	{
		if (pObject->WhatAmI() != AbstractType::Building || pData->Shrapnel_AffectsBuildings)
			return Shrapnel;
	}
	else if (pData->Shrapnel_AffectsGround)
		return Shrapnel;

	return Skip;
}

DEFINE_HOOK(0x4690D4, BulletClass_Logics_ScreenShake, 0x6)
{
	enum { SkipShaking = 0x469130 };

	GET(WarheadTypeClass*, pWarhead, EAX);
	GET_BASE(CoordStruct*, pCoords, 0x8);

	if (auto const pExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		Point2D screenCoords;

		if (pExt->ShakeIsLocal && !TacticalClass::Instance->CoordsToClient(*pCoords, &screenCoords))
			return SkipShaking;
	}

	return 0;
}

DEFINE_HOOK(0x4690C1, BulletClass_Logics_DetachFromOwner, 0x8)
{
	GET(BulletClass*, pThis, ESI);
	
	if (pThis->Owner && pThis->WeaponType)
	{
		auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->WeaponType);

		if (pWeaponExt->DetachedFromOwner)
			pThis->Owner = nullptr;
	}

	return 0;
}