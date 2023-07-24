#include "Body.h"
#include <Ext/Anim/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Misc/CaptureManager.h>

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <TechnoClass.h>
#include <TacticalClass.h>

// has everything inited except SpawnNextAnim at this point
DEFINE_HOOK(0x466556, BulletClass_Init, 0x6)
{
	GET(BulletClass*, pThis, ECX);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		pExt->FirerHouse = pThis->Owner ? pThis->Owner->Owner : nullptr;
		pExt->CurrentStrength = pThis->Type->Strength;
		pExt->TypeExtData = BulletTypeExt::ExtMap.Find(pThis->Type);

		if (!pThis->Type->Inviso)
			pExt->InitializeLaserTrails();
	}

	return 0;
}

// Set in BulletClass::AI and guaranteed to be valid within it.
namespace BulletAITemp
{
	BulletExt::ExtData* ExtData;
	BulletTypeExt::ExtData* TypeExtData;
}

DEFINE_HOOK(0x4666F7, BulletClass_AI, 0x6)
{
	GET(BulletClass*, pThis, EBP);

	auto pBulletExt = BulletExt::ExtMap.Find(pThis);
	BulletAITemp::ExtData = pBulletExt;
	BulletAITemp::TypeExtData = pBulletExt->TypeExtData;

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

	if (auto const pExt = BulletAITemp::ExtData)
	{
		if (auto const pAnimExt = AnimExt::ExtMap.Find(pAnim))
		{
			pAnim->Owner = pThis->Owner ? pThis->Owner->Owner : pExt->FirerHouse;
			pAnimExt->SetInvoker(pThis->Owner);
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

	if (auto const pExt = BulletAITemp::ExtData)
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

DEFINE_HOOK(0x4690C1, BulletClass_Logics_DetonateOnAllMapObjects, 0x8)
{
	enum { ReturnFromFunction = 0x46A2FB };

	GET(BulletClass*, pThis, ESI);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH))
	{
		if (pWHExt->DetonateOnAllMapObjects && !pWHExt->WasDetonatedOnAllMapObjects &&
			pWHExt->DetonateOnAllMapObjects_AffectTargets != AffectedTarget::None &&
			pWHExt->DetonateOnAllMapObjects_AffectHouses != AffectedHouse::None)
		{
			pWHExt->WasDetonatedOnAllMapObjects = true;
			auto const pExt = BulletExt::ExtMap.Find(pThis);
			auto pOwner = pThis->Owner ? pThis->Owner->Owner : pExt->FirerHouse;

			auto tryDetonate = [pThis, pWHExt, pOwner](TechnoClass* pTechno)
			{
				if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner))
				{
					pThis->Target = pTechno;
					auto coords = CoordStruct::Empty;
					coords = *pTechno->GetCoords(&coords);
					pThis->Detonate(coords);
				}
			};

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Aircraft) != AffectedTarget::None)
			{
				for (auto pTechno : *AircraftClass::Array)
					tryDetonate(pTechno);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Building) != AffectedTarget::None)
			{
				for (auto pTechno : *BuildingClass::Array)
					tryDetonate(pTechno);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Infantry) != AffectedTarget::None)
			{
				for (auto pTechno : *InfantryClass::Array)
					tryDetonate(pTechno);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Unit) != AffectedTarget::None)
			{
				for (auto pTechno : *UnitClass::Array)
					tryDetonate(pTechno);
			}

			pWHExt->WasDetonatedOnAllMapObjects = false;

			return ReturnFromFunction;
		}
	}

	return 0;
}

DEFINE_HOOK(0x467CCA, BulletClass_AI_TargetSnapChecks, 0x6)
{
	enum { SkipChecks = 0x467CDE };

	GET(BulletClass*, pThis, EBP);

	// Do not require Airburst=no to check target snapping for Inviso / Trajectory=Straight projectiles
	if (pThis->Type->Inviso)
	{
		R->EAX(pThis->Type);
		return SkipChecks;
	}
	else if (auto const pExt = BulletAITemp::ExtData)
	{
		if (pExt->Trajectory)
		{
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight)
			{
				R->EAX(pThis->Type);
				return SkipChecks;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x468E61, BulletClass_Explode_TargetSnapChecks1, 0x6)
{
	enum { SkipChecks = 0x468E7B };

	GET(BulletClass*, pThis, ESI);

	// Do not require Airburst=no to check target snapping for Inviso / Trajectory=Straight projectiles
	if (pThis->Type->Inviso)
	{
		R->EAX(pThis->Type);
		return SkipChecks;
	}
	else if (pThis->Type->Arcing || pThis->Type->ROT > 0)
	{
		return 0;
	}
	else if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory)
		{
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight)
			{
				R->EAX(pThis->Type);
				return SkipChecks;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x468E9F, BulletClass_Explode_TargetSnapChecks2, 0x6)
{
	enum { SkipInitialChecks = 0x468EC7, SkipSetCoordinate = 0x468F23 };

	GET(BulletClass*, pThis, ESI);

	// Do not require EMEffect=no & Airburst=no to check target coordinate snapping for Inviso projectiles.
	if (pThis->Type->Inviso)
	{
		R->EAX(pThis->Type);
		return SkipInitialChecks;
	}
	else if (pThis->Type->Arcing || pThis->Type->ROT > 0)
	{
		return 0;
	}

	// Do not force Trajectory=Straight projectiles to detonate at target coordinates under certain circumstances.
	// Fixes issues with walls etc.
	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory)
		{
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight)
				return SkipSetCoordinate;
		}
	}

	return 0;
}
