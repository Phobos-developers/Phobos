#include "Body.h"
#include <Ext/Anim/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Misc/CaptureManager.h>

#include <TechnoClass.h>
#include <TacticalClass.h>
#include <ScenarioClass.h>

// has everything inited except SpawnNextAnim at this point
DEFINE_HOOK(0x466556, BulletClass_Init, 0x6)
{
	GET(BulletClass*, pThis, ECX);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		pExt->FirerHouse = pThis->Owner ? pThis->Owner->Owner : nullptr;
		pExt->CurrentStrength = pThis->Type->Strength;
	}

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

	if (pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted)
	{
		if (pBulletExt->DetonateOnInterception)
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

DEFINE_HOOK(0x4668BD, BulletClass_AI_TrailerInheritOwner, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET(AnimClass*, pAnim, EAX);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (auto const pAnimExt = AnimExt::ExtMap.Find(pAnim))
		{
			pAnim->Owner = pThis->Owner ? pThis->Owner->Owner : pExt->FirerHouse;
			pAnimExt->Invoker = pThis->Owner;
		}
	}

	return 0;
}

// Inviso bullets behave differently in BulletClass::AI when their target is bullet and
// seemingly (at least partially) adopt characteristics of a vertical projectile.
// This is a potentially slightly hacky solution to that, as proper solution
// would likely require making sense of BulletClass::AI and ain't nobody got time for that.
DEFINE_HOOK(0x4668BD, BulletClass_AI_Interceptor_InvisoSkip, 0x6)
{
	enum { DetonateBullet = 0x467F9B };

	GET(BulletClass*, pThis, EBP);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pThis->Type->Inviso && pExt->IsInterceptor)
			return DetonateBullet;
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

DEFINE_HOOK(0x469211, BulletClass_Logics_MindControlAlternative1, 0x6)
{
	GET(BulletClass*, pBullet, ESI);

	if (!pBullet->Target)
		return 0;

	auto pTarget = generic_cast<TechnoClass*>(pBullet->Target);
	if (!pTarget)
		return 0;

	auto pBulletWH = pBullet->WH;
	if (!pBulletWH)
		return 0;

	if (pBullet->Owner 
		&& pBulletWH->MindControl)
	{
		auto pTargetType = pTarget->GetTechnoType();
		if (!pTargetType)
			return 0;

		auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH);
		if (!pWarheadExt)
			return 0;

		double currentHealthPerc = pTarget->GetHealthPercentage();
		bool flipComparations = pWarheadExt->MindControl_Threshold_Inverse;

		if (pWarheadExt->MindControl_Threshold < 0.0 || pWarheadExt->MindControl_Threshold > 1.0)
			pWarheadExt->MindControl_Threshold = flipComparations ? 0.0 : 1.0;

		bool skipMindControl = flipComparations ? (pWarheadExt->MindControl_Threshold > 0.0) : (pWarheadExt->MindControl_Threshold < 1.0);
		bool healthComparation = flipComparations ? (currentHealthPerc <= pWarheadExt->MindControl_Threshold) : (currentHealthPerc >= pWarheadExt->MindControl_Threshold);

		if (skipMindControl
			&& healthComparation
			&& pWarheadExt->MindControl_AlternateDamage.isset()
			&& pWarheadExt->MindControl_AlternateWarhead.isset())
		{
			int damage = pWarheadExt->MindControl_AlternateDamage.Get();
			WarheadTypeClass* pAltWarhead = pWarheadExt->MindControl_AlternateWarhead.Get();
			int realDamage = MapClass::GetTotalDamage(damage, pAltWarhead, pTargetType->Armor, 0);

			if (!pWarheadExt->MindControl_CanKill && pTarget->Health <= realDamage && realDamage > 1)
				pTarget->Health = realDamage;

			return 0x469343;
		}
	}

	return 0;
}

DEFINE_HOOK(0x469BD6, BulletClass_Logics_MindControlAlternative2, 0x6)
{
	GET(BulletClass*, pBullet, ESI);
	GET(AnimTypeClass*, pAnimType, EBX);

	if (!pBullet->Target)
		return 0;

	auto pTarget = generic_cast<TechnoClass*>(pBullet->Target);
	if (!pTarget)
		return 0;

	auto pBulletWH = pBullet->WH;
	if (!pBulletWH)
		return 0;

	if (pBullet->Owner
		&& pBulletWH->MindControl)
	{
		auto pTargetType = pTarget->GetTechnoType();
		if (!pTargetType)
			return 0;

		auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH);
		if (!pWarheadExt)
			return 0;

		double currentHealthPerc = pTarget->GetHealthPercentage();
		bool flipComparations = pWarheadExt->MindControl_Threshold_Inverse;

		bool skipMindControl = flipComparations ? (pWarheadExt->MindControl_Threshold > 0.0) : (pWarheadExt->MindControl_Threshold < 1.0);
		bool healthComparation = flipComparations ? (currentHealthPerc <= pWarheadExt->MindControl_Threshold) : (currentHealthPerc >= pWarheadExt->MindControl_Threshold);

		if (skipMindControl
			&& healthComparation
			&& pWarheadExt->MindControl_AlternateDamage.isset()
			&& pWarheadExt->MindControl_AlternateWarhead.isset())
		{
			int damage = pWarheadExt->MindControl_AlternateDamage;
			WarheadTypeClass* pAltWarhead = pWarheadExt->MindControl_AlternateWarhead;
			auto pAttacker = pBullet->Owner;
			auto pAttackingHouse = pBullet->Owner->Owner;
			int realDamage = MapClass::GetTotalDamage(damage, pAltWarhead, pTargetType->Armor, 0);

			if (!pWarheadExt->MindControl_CanKill && pTarget->Health <= realDamage)
			{
				pTarget->Health += abs(realDamage);
				realDamage = 1;
				pTarget->ReceiveDamage(&realDamage, 0, pAltWarhead, pAttacker, true, false, pAttackingHouse);
				pTarget->Health = 1;
			}
			else
			{
				pTarget->ReceiveDamage(&damage, 0, pAltWarhead, pAttacker, true, false, pAttackingHouse);
			}

			pAnimType = nullptr;

			// If the alternative Warhead have AnimList tag declared then use it
			if (pWarheadExt->MindControl_AlternateWarhead->AnimList.Count > 0)
			{
				if (CellClass* pCell = MapClass::Instance->TryGetCellAt(pTarget->Location))
					pAnimType = MapClass::SelectDamageAnimation(damage, pAltWarhead, pCell->LandType, pTarget->Location);
			}

			R->EBX(pAnimType);
		}
	}

	return 0;
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

DEFINE_HOOK(0x469A75, BulletClass_Logics_DamageHouse, 0x7)
{
	GET(BulletClass*, pThis, ESI);
	GET(HouseClass*, pHouse, ECX);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (!pHouse)
			R->ECX(pExt->FirerHouse);
	}

	return 0;
}
