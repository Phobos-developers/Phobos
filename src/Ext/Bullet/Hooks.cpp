#include "Body.h"
#include <Ext/Anim/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

#include <ScenarioClass.h>

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

	//Because the laser trails will be drawn before the calculation of changing the velocity direction in each frame.
	//This will cause the laser trails to be drawn in the wrong position too early, resulting in a visual appearance resembling a "bouncing".
	//Let trajectories draw their own laser trails after the Trajectory's OnAI() to avoid predicting incorrect positions or pass through targets.
	if (!pBulletExt->Trajectory && pBulletExt->LaserTrails.size())
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

		for (auto& trail : pBulletExt->LaserTrails)
		{
			// We insert initial position so the first frame of trail doesn't get skipped - Kerbiter
			// TODO move hack to BulletClass creation
			if (!trail.LastLocation.isset())
				trail.LastLocation = location;

			trail.Update(drawnCoords);
		}

	}

	return 0;
}

DEFINE_HOOK(0x466897, BulletClass_AI_Trailer, 0x6)
{
	enum { SkipGameCode = 0x4668BD };

	GET(BulletClass*, pThis, EBP);
	GET_STACK(CoordStruct, coords, STACK_OFFSET(0x1A8, -0x184));

	auto const pTrailerAnim = GameCreate<AnimClass>(pThis->Type->Trailer, coords, 1, 1);

	auto const pTrailerAnimExt = AnimExt::ExtMap.Find(pTrailerAnim);
	auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletAITemp::ExtData->FirerHouse;
	AnimExt::SetAnimOwnerHouseKind(pTrailerAnim, pOwner, nullptr, false, true);
	pTrailerAnimExt->SetInvoker(pThis->Owner);

	return SkipGameCode;
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

#pragma region Gravity

#define APPLYGRAVITY(pType)\
auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);\
__asm { fld nGravity };\

DEFINE_HOOK(0x4671B9, BulletClass_AI_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	APPLYGRAVITY(pType);

	return 0x4671BF;
}

DEFINE_HOOK(0x6F7481, TechnoClass_Targeting_ApplyGravity, 0x6)
{
	GET(WeaponTypeClass* const, pWeaponType, EDX);

	APPLYGRAVITY(pWeaponType->Projectile);

	return 0x6F74A4;
}

DEFINE_HOOK(0x6FDAA6, TechnoClass_FireAngle_6FDA00_ApplyGravity, 0x5)
{
	GET(WeaponTypeClass* const, pWeaponType, EDI);

	APPLYGRAVITY(pWeaponType->Projectile);

	return 0x6FDACE;
}

DEFINE_HOOK(0x6FECB2, TechnoClass_FireAt_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	APPLYGRAVITY(pType);

	return 0x6FECD1;
}

DEFINE_HOOK_AGAIN(0x44D2AE, BuildingClass_Mission_Missile_ApplyGravity, 0x6)
DEFINE_HOOK_AGAIN(0x44D264, BuildingClass_Mission_Missile_ApplyGravity, 0x6)
DEFINE_HOOK(0x44D074, BuildingClass_Mission_Missile_ApplyGravity, 0x6)
{
	GET(WeaponTypeClass* const, pWeaponType, EBP);

	APPLYGRAVITY(pWeaponType->Projectile);

	switch (R->Origin())
	{
	case 0x44D074:
		return 0x44D07A;
		break;
	case 0x44D264:
		return 0x44D26A;
		break;
	case 0x44D2AE:
		return 0x44D2B4;
		break;
	default:
		__assume(0);
	}
}

#pragma endregion

DEFINE_HOOK(0x46A3D6, BulletClass_Shrapnel_Forced, 0xA)
{
	enum { Shrapnel = 0x46A40C, Skip = 0x46ADCD };

	GET(BulletClass*, pThis, EDI);

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

	if (auto const pObject = pThis->GetCell()->FirstObject)
	{
		if (pObject->WhatAmI() != AbstractType::Building || pTypeExt->Shrapnel_AffectsBuildings)
			return Shrapnel;
	}
	else if (pTypeExt->Shrapnel_AffectsGround)
	{
		return Shrapnel;
	}

	return Skip;
}

DEFINE_HOOK(0x46A4FB, BulletClass_Shrapnel_Targeting, 0x6)
{
	enum { SkipObject = 0x46A8EA, Continue = 0x46A50F };

	GET(BulletClass*, pThis, EDI);
	GET(ObjectClass*, pObject, EBP);
	GET(TechnoClass*, pSource, EAX);
	GET(WeaponTypeClass*, pShrapnelWeapon, ESI);

	auto const pOwner = pSource->Owner;
	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->Shrapnel_UseWeaponTargeting)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pShrapnelWeapon);
		auto const pType = pObject->GetType();

		if (!pType->LegalTarget || GeneralUtils::GetWarheadVersusArmor(pShrapnelWeapon->Warhead, pType->Armor) == 0.0)
			return SkipObject;
		else if (!EnumFunctions::IsCellEligible(pObject->GetCell(), pWeaponExt->CanTarget, true, true))
			return SkipObject;

		if (auto const pTechno = abstract_cast<TechnoClass*>(pObject))
		{
			if (!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, pTechno->Owner))
				return SkipObject;

			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget))
				return SkipObject;

			if (!pWeaponExt->HasRequiredAttachedEffects(pTechno, pSource))
				return SkipObject;
		}

	}
	else if (pOwner->IsAlliedWith(pObject))
	{
		return SkipObject;
	}

	return Continue;
}

DEFINE_HOOK(0x46902C, BulletClass_Explode_Cluster, 0x6)
{
	enum { SkipGameCode = 0x469091 };

	GET(BulletClass*, pThis, ESI);
	GET_STACK(CoordStruct, origCoords, STACK_OFFSET(0x3C, -0x30));

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
	int min = pTypeExt->ClusterScatter_Min.Get();
	int max = pTypeExt->ClusterScatter_Max.Get();
	auto coords = origCoords;

	for (int i = 0; i < pThis->Type->Cluster; i++)
	{
		pThis->Detonate(coords);

		if (!pThis->IsAlive)
			break;

		int distance = ScenarioClass::Instance->Random.RandomRanged(min, max);
		coords = MapClass::GetRandomCoordsNear(origCoords, distance, false);
	}

	return SkipGameCode;
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
		if (auto pTraj = pExt->Trajectory.get())
		{
			if (pTraj->Flag() == TrajectoryFlag::Straight)
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
		if (pExt->Trajectory && pExt->Trajectory->Flag() == TrajectoryFlag::Straight && !pExt->SnappedToTarget)
		{
			R->EAX(pThis->Type);
			return SkipChecks;
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
		if (pExt->Trajectory && pExt->Trajectory->Flag() == TrajectoryFlag::Straight && !pExt->SnappedToTarget)
			return SkipSetCoordinate;
	}

	return 0;
}

DEFINE_HOOK(0x468D3F, BulletClass_ShouldExplode_AirTarget, 0x6)
{
	enum { SkipCheck = 0x468D73 };

	GET(BulletClass*, pThis, ESI);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory && pExt->Trajectory->Flag() == TrajectoryFlag::Straight)
			return SkipCheck;
	}

	return 0;
}

DEFINE_HOOK(0x4687F8, BulletClass_Unlimbo_FlakScatter, 0x6)
{
	GET(BulletClass*, pThis, EBX);
	GET_STACK(float, mult, STACK_OFFSET(0x5C, -0x44));

	if (pThis->WeaponType)
	{
		if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
		{
			int defaultValue = RulesClass::Instance->BallisticScatter;
			int min = pTypeExt->BallisticScatter_Min.Get(Leptons(0));
			int max = pTypeExt->BallisticScatter_Max.Get(Leptons(defaultValue));

			int result = (int)((mult * ScenarioClass::Instance->Random.RandomRanged(2 * min, 2 * max)) / pThis->WeaponType->Range);
			R->EAX(result);
		}
	}

	return 0;
}

// Skip a forced detonation check for Level=true projectiles that is now handled in Hooks.Obstacles.cpp.
DEFINE_JUMP(LJMP, 0x468D08, 0x468D2F);

DEFINE_HOOK(0x6FE657, TechnoClass_FireAt_ArcingFix, 0x6)
{
	GET_STACK(BulletTypeClass*, pBulletType, STACK_OFFSET(0xB0, -0x48));
	GET(int, targetHeight, EDI);
	GET(int, fireHeight, EAX);

	if (pBulletType->Arcing && targetHeight > fireHeight)
	{
		auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBulletType);

		if (!pBulletTypeExt->Arcing_AllowElevationInaccuracy)
			R->EAX(targetHeight);
	}

	return 0;
}

DEFINE_HOOK(0x44D23C, BuildingClass_Mission_Missile_ArcingFix, 0x7)
{
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET(int, targetHeight, EBX);
	GET(int, fireHeight, EAX);

	auto const pBulletType = pWeapon->Projectile;

	if (pBulletType->Arcing && targetHeight > fireHeight)
	{
		auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBulletType);

		if (!pBulletTypeExt->Arcing_AllowElevationInaccuracy)
			R->EAX(targetHeight);
	}

	return 0;
}
