#include "Body.h"
#include <Ext/Anim/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Utilities/Macro.h>

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <ScenarioClass.h>
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

	if (auto const pTrailerAnim = GameCreate<AnimClass>(pThis->Type->Trailer, coords, 1, 1))
	{
		auto const pTrailerAnimExt = AnimExt::ExtMap.Find(pTrailerAnim);
		pTrailerAnim->Owner = pThis->Owner ? pThis->Owner->Owner : BulletAITemp::ExtData->FirerHouse;
		pTrailerAnimExt->SetInvoker(pThis->Owner);
	}

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

DEFINE_HOOK(0x4692BD, BulletClass_Logics_ApplyMindControl, 0x6)
{
	GET(BulletClass*, pThis, ESI);

	auto pTypeExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto pControlledAnimType = pTypeExt->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);

	R->AL(CaptureManagerExt::CaptureUnit(pThis->Owner->CaptureManager, pThis->Target, pControlledAnimType));

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

DEFINE_HOOK(0x44D074, BuildingClass_Mission_Missile_ApplyGravity1, 0x6)
{
	GET(WeaponTypeClass* const, pWeapon, EBP);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pWeapon->Projectile);
	__asm { fld nGravity };

	return 0x44D07A;
}

DEFINE_HOOK(0x44D264, BuildingClass_Mission_Missile_ApplyGravity2, 0x6)
{
	GET(WeaponTypeClass* const, pWeapon, EBP);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pWeapon->Projectile);
	__asm { fld nGravity };

	return 0x44D26A;
}

DEFINE_HOOK(0x44D2AE, BuildingClass_Mission_Missile_ApplyGravity3, 0x6)
{
	GET(WeaponTypeClass* const, pWeapon, EBP);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pWeapon->Projectile);
	__asm { fld nGravity };

	return 0x44D2B4;
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

DEFINE_HOOK(0x469008, BulletClass_Explode_Cluster, 0x8)
{
	enum { SkipGameCode = 0x469091 };

	GET(BulletClass*, pThis, ESI);
	GET_STACK(CoordStruct, origCoords, STACK_OFFSET(0x3C, -0x30));

	if (pThis->Type->Cluster > 0)
	{
		if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
		{
			int min = pTypeExt->ClusterScatter_Min.Get(Leptons(256));
			int max = pTypeExt->ClusterScatter_Max.Get(Leptons(512));
			auto coords = origCoords;

			for (int i = 0; i < pThis->Type->Cluster; i++)
			{
				pThis->Detonate(coords);

				if (!pThis->IsAlive)
					break;

				int distance = ScenarioClass::Instance->Random.RandomRanged(min, max);
				coords = MapClass::GetRandomCoordsNear(origCoords, distance, false);
			}
		}
	}

	return SkipGameCode;
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
					pThis->Detonate(pTechno->GetCoords());
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

DEFINE_HOOK(0x469D1A, BulletClass_Logics_Debris_Checks, 0x6)
{
	enum { SkipGameCode = 0x469EBA, SetDebrisCount=0x469D36 };

	GET(BulletClass*, pThis, ESI);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	bool isLand = pThis->GetCell()->LandType != LandType::Water;

	if (!isLand && pWHExt->Debris_Conventional)
		return SkipGameCode;

	// Fix the debris count to be in range of Min, Max instead of Min, Max-1.
	R->EBX(ScenarioClass::Instance->Random.RandomRanged(pThis->WH->MinDebris, pThis->WH->MaxDebris));

	return SetDebrisCount;
}

DEFINE_HOOK(0x469E34, BulletClass_Logics_DebrisAnims, 0x5)
{
	enum { SkipGameCode = 0x469EBA };

	GET(BulletClass*, pThis, ESI);
	GET(int, debrisCount, EBX);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto debrisAnims = pWHExt->DebrisAnims.GetElements(RulesClass::Instance->MetallicDebris);

	if (debrisAnims.size() < 1)
		return SkipGameCode;

	while (debrisCount > 0)
	{
		int debrisIndex = ScenarioClass::Instance->Random.RandomRanged(0, debrisAnims.size() - 1);

		auto anim = GameCreate<AnimClass>(debrisAnims[debrisIndex], pThis->GetCoords());

		if (anim && pThis->Owner)
			anim->Owner = pThis->Owner->Owner;

		debrisCount--;
	}

	return SkipGameCode;
}

// Skip a forced detonation check for Level=true projectiles that is now handled in Hooks.Obstacles.cpp.
DEFINE_JUMP(LJMP, 0x468D08, 0x468D2F);

DEFINE_HOOK(0x46A290, BulletClass_Logics_ExtraWarheads, 0x5)
{
	GET(BulletClass*, pThis, ESI);
	GET_BASE(CoordStruct*, coords, 0x8);

	if (pThis->WeaponType)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->WeaponType);
		int defaultDamage = pThis->WeaponType->Damage;

		for (size_t i = 0; i < pWeaponExt->ExtraWarheads.size(); i++)
		{
			auto const pWH = pWeaponExt->ExtraWarheads[i];
			auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
			int damage = defaultDamage;

			if (pWeaponExt->ExtraWarheads_DamageOverrides.size() > i)
				damage = pWeaponExt->ExtraWarheads_DamageOverrides[i];

			WarheadTypeExt::DetonateAt(pWH, *coords, pThis->Owner, damage, pOwner);
		}
	}

	return 0;
}

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
