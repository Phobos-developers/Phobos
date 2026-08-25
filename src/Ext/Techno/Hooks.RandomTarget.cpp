#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <AircraftClass.h>
#include <BulletClass.h>
#include <SpawnManagerClass.h>

static bool IsValidAliveTarget(AbstractClass* pTarget)
{
	if (!pTarget)
		return false;

	// Only real, destructible Techno objects (units/buildings) are remembered across waves.
	// Ground cells from MissChance or force-fire are single-pass coordinates.
	if (auto const pTechno = abstract_cast<TechnoClass*>(pTarget))
	{
		return pTechno->IsAlive && !pTechno->InLimbo && pTechno->Health > 0 && !pTechno->IsSinking && !pTechno->IsCrashing && !pTechno->BeingWarpedOut;
	}

	return false;
}

static bool IsValidTargetInRange(TechnoClass* pFirer, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	if (!IsValidAliveTarget(pTarget))
		return false;

	if (pFirer && pWeapon)
	{
		int maxRange = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer);
		if (auto const pFirerTypeExt = TechnoTypeExt::ExtMap.TryFind(pFirer->GetTechnoType()))
		{
			if (pFirerTypeExt->Spawner_LimitRange && pFirerTypeExt->Spawner_ExtraLimitRange > 0)
			{
				maxRange += pFirerTypeExt->Spawner_ExtraLimitRange * Unsorted::LeptonsPerCell;
			}
		}

		if (maxRange > 0 && pFirer->DistanceFrom(pTarget) > maxRange)
			return false;
	}

	return true;
}

DEFINE_HOOK(0x6FDD7D, TechnoClass_FireAt_RandomTarget, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET_BASE(AbstractClass*, pTarget, 0x8);

	if (pThis && pWeapon && pTarget)
	{
		// Spawner weapons manage aircraft targets in SpawnManagerClass
		if (pWeapon->Spawner)
			return 0;

		if (auto const pWeaponExt = WeaponTypeExt::ExtMap.TryFind(pWeapon))
		{
			if (pWeaponExt->RandomTarget > 0.0)
			{
				auto pNewTarget = TechnoExt::FindRandomTarget(pThis, pTarget, pWeapon);
				if (pNewTarget && pNewTarget != pTarget)
				{
					R->Base(0x8, pNewTarget);
					R->EDI(pNewTarget);
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6B7B90, SpawnManagerClass_SetTarget_RandomTarget, 0x7)
{
	GET(SpawnManagerClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	if (pThis && pThis->Owner && pTarget)
	{
		auto pSpawner = pThis->Owner;
		int weaponIndex = pSpawner->SelectWeapon(pTarget);
		if (weaponIndex >= 0)
		{
			if (auto pWeapon = pSpawner->GetWeapon(weaponIndex)->WeaponType)
			{
				if (auto const pWeaponExt = WeaponTypeExt::ExtMap.TryFind(pWeapon))
				{
					// Maintain player's clicked target as reference anchor for individual aircraft sampling
					if (pWeaponExt->RandomTarget_Spawners_MultipleTargets)
						return 0;

					if (pWeaponExt->RandomTarget > 0.0)
					{
						// Retain surviving target across passes if RememberTargets is enabled
						if (pWeaponExt->RandomTarget_Spawners_RememberTargets && IsValidTargetInRange(pSpawner, pThis->Target, pWeapon))
						{
							R->Stack(0x4, pThis->Target);
							return 0;
						}

						auto pNewTarget = TechnoExt::FindRandomTarget(pSpawner, pTarget, pWeapon);
						if (pNewTarget && pNewTarget != pTarget)
						{
							R->Stack(0x4, pNewTarget);
						}
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6B7AE3, SpawnManagerClass_Launch_RandomTarget, 0x6)
{
	enum { SkipSetTarget = 0x6B7AEF };

	GET(AircraftClass*, pThis, ECX);

	if (!pThis || !pThis->SpawnOwner || !pThis->SpawnOwner->SpawnManager)
		return 0;

	auto pSpawner = pThis->SpawnOwner;
	auto pManager = pSpawner->SpawnManager;

	int weaponIndex = pSpawner->SelectWeapon(pManager->Target);
	if (weaponIndex < 0)
		return 0;

	auto pWeapon = pSpawner->GetWeapon(weaponIndex)->WeaponType;
	if (!pWeapon)
		return 0;

	auto const pWeaponExt = WeaponTypeExt::ExtMap.TryFind(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return 0;

	auto const pSpawnExt = TechnoExt::ExtMap.Find(pThis);
	if (!pSpawnExt)
		return 0;

	if (pWeaponExt->RandomTarget_Spawners_MultipleTargets)
	{
		if (!pSpawnExt->SpawnRandomTarget || !IsValidTargetInRange(pSpawner, pSpawnExt->SpawnRandomTarget, pWeapon))
		{
			pSpawnExt->SpawnRandomTarget = TechnoExt::FindRandomTarget(pSpawner, pManager->Target, pWeapon);
		}

		if (pSpawnExt->SpawnRandomTarget)
		{
			pThis->SetTarget(pSpawnExt->SpawnRandomTarget);
			return SkipSetTarget;
		}
	}
	else
	{
		if (IsValidTargetInRange(pSpawner, pManager->Target, pWeapon))
		{
			pThis->SetTarget(pManager->Target);
			return SkipSetTarget;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6B76E3, SpawnManagerClass_AI_RandomTarget, 0x5)
{
	enum { SkipSetTarget = 0x6B76EA };

	GET(SpawnManagerClass*, pThis, ESI);
	GET(int, index, EBX);

	if (!pThis || !pThis->Owner)
		return 0;

	auto pSpawn = pThis->SpawnedNodes[index];
	if (!pSpawn || !pSpawn->Unit || pSpawn->IsSpawnMissile)
		return 0;

	auto const pSpawnExt = TechnoExt::ExtMap.Find(pSpawn->Unit);
	if (!pSpawnExt)
		return 0;

	auto pSpawner = pThis->Owner;
	int weaponIndex = pSpawner->SelectWeapon(pThis->Target);
	if (weaponIndex < 0)
		return 0;

	auto pWeapon = pSpawner->GetWeapon(weaponIndex)->WeaponType;
	if (!pWeapon)
		return 0;

	auto const pWeaponExt = WeaponTypeExt::ExtMap.TryFind(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return 0;

	// Reset stored target upon docking if RememberTargets is disabled or target is gone
	if (pSpawn->Unit->GetCurrentMission() != Mission::Attack)
	{
		if (!pWeaponExt->RandomTarget_Spawners_RememberTargets || !IsValidTargetInRange(pSpawner, pSpawnExt->SpawnRandomTarget, pWeapon))
		{
			pSpawnExt->SpawnRandomTarget = nullptr;
		}
		return 0;
	}

	if (pWeaponExt->RandomTarget_Spawners_MultipleTargets)
	{
		if (!pSpawnExt->SpawnRandomTarget || !IsValidTargetInRange(pSpawner, pSpawnExt->SpawnRandomTarget, pWeapon))
		{
			pSpawnExt->SpawnRandomTarget = TechnoExt::FindRandomTarget(pSpawner, pThis->Target, pWeapon);
			if (pSpawnExt->SpawnRandomTarget)
			{
				pSpawn->Unit->SetTarget(pSpawnExt->SpawnRandomTarget);
			}
		}

		if (pSpawnExt->SpawnRandomTarget)
		{
			R->EAX(pSpawnExt->SpawnRandomTarget);
			return SkipSetTarget;
		}
	}
	else
	{
		if (IsValidTargetInRange(pSpawner, pThis->Target, pWeapon))
		{
			R->EAX(pThis->Target);
			return SkipSetTarget;
		}
	}

	return 0;
}
