#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/Script/Body.h>

DEFINE_HOOK(0x4D4E8A, FootClass_FiringAI_BurstRandomTarget, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->RearmTimer.TimeLeft <= 0 && TechnoExt::IsValidTechno(pThis->Target))
		TechnoExt::NewRandomTarget(pThis);

	return 0;
}

DEFINE_HOOK(0x44AFF8, BuildingClass_FireAt_BurstRandomTarget, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto pOriginalTarget = pThis->Target;

	if (pThis->RearmTimer.TimeLeft <= 0 && TechnoExt::IsValidTechno(pThis->Target))
		TechnoExt::NewRandomTarget(pThis);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return 0;

	int weaponIndex = pExt->OriginalTarget && TechnoExt::IsValidTechno(pExt->OriginalTarget) ? pExt->OriginalTargetWeaponIndex : pThis->SelectWeapon(pThis->Target);
	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	if (!pWeapon || pWeapon->IsLaser || pWeapon->Spawner)
		return 0;

	if (pThis->Target)
		pThis->Target = pOriginalTarget;

	return 0;
}

DEFINE_HOOK(0x736F61, UnitClass_FiringAI_RandomTarget, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->RearmTimer.TimeLeft <= 0 && TechnoExt::IsValidTechno(pThis->Target))
		TechnoExt::NewRandomTarget(pThis);

	return 0;
}

DEFINE_HOOK(0x6B7AE3, SpawnerManagerClassAI_RandomTarget_AssignTargetToAircraft, 0x6)
{
	GET(AircraftClass*, pThis, ECX);

	SpawnManagerStatus pSpawnManagerStatus = pThis->SpawnOwner->SpawnManager->Status;
	AirAttackStatus missionStatus = (AirAttackStatus)pThis->MissionStatus;

	if (pSpawnManagerStatus != SpawnManagerStatus::Launching && missionStatus != AirAttackStatus::ValidateAZ)
		return 0;

	const auto pSpawnExt = TechnoExt::ExtMap.Find(pThis);
	if (!pSpawnExt)
		return 0;

	if (!TechnoExt::IsValidTechno(pSpawnExt->OriginalTarget))
		return 0;

	auto pSpawner = pThis->SpawnOwner;

	const auto pSpawnerExt = TechnoExt::ExtMap.Find(pSpawner);
	if (!pSpawnerExt)
		return 0;

	if (!pSpawner->Target || pSpawnerExt->OriginalTarget != pSpawnExt->OriginalTarget || !TechnoExt::IsValidTechno(pSpawnerExt->CurrentRandomTarget))
	{
		pSpawnExt->CurrentRandomTarget = nullptr;
		pSpawnExt->OriginalTarget = nullptr;
		pSpawnExt->OriginalTargetWeaponIndex = -1; // Really not needed in spawns, right?
		pThis->SetTarget(nullptr);

		return 0;
	}

	auto pWeapon = pSpawner->GetWeapon(pSpawnerExt->OriginalTargetWeaponIndex)->WeaponType;
	if (!pWeapon)
		return 0;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return 0;

	if (!TechnoExt::IsValidTechno(pSpawnExt->CurrentRandomTarget))
	{
		pSpawnExt->CurrentRandomTarget = pWeaponExt->RandomTarget_Spawners_MultipleTargets ?
			TechnoExt::FindRandomTarget(pSpawner) : pSpawnerExt->CurrentRandomTarget;
	}

	pThis->SetTarget(pSpawnExt->CurrentRandomTarget);
	pSpawnExt->OriginalTargetWeaponIndex = pSpawnerExt->OriginalTargetWeaponIndex;

	return 0x6B7AEF;
}

DEFINE_HOOK(0x6B76E3, SpawnerManagerClassAI_RandomTarget_AssignTargetToAircraft2, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(int, index, EBX);

	auto pSpawn = pThis->SpawnedNodes[index];

	if (!pSpawn->Unit || pSpawn->IsSpawnMissile || pSpawn->Unit->GetCurrentMission() != Mission::Attack)
		return 0;

	const auto pSpawnExt = TechnoExt::ExtMap.Find(pSpawn->Unit);
	if (!pSpawnExt)
		return 0;

	if (!TechnoExt::IsValidTechno(pSpawnExt->OriginalTarget))
		return 0;

	auto pSpawner = pSpawn->Unit->SpawnOwner;

	const auto pSpawnerExt = TechnoExt::ExtMap.Find(pSpawner);
	if (!pSpawnerExt)
		return 0;

	if (!pSpawner->Target || pSpawnerExt->OriginalTarget != pSpawnExt->OriginalTarget || !TechnoExt::IsValidTechno(pSpawnerExt->CurrentRandomTarget))
	{
		pSpawnExt->CurrentRandomTarget = nullptr;
		pSpawnExt->OriginalTarget = nullptr;
		pSpawnExt->OriginalTargetWeaponIndex = -1;

		return 0;
	}

	auto pWeapon = pSpawner->GetWeapon(pSpawnerExt->OriginalTargetWeaponIndex)->WeaponType;
	if (!pWeapon)
		return 0;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return 0;

	if (!TechnoExt::IsValidTechno(pSpawnExt->CurrentRandomTarget))
	{
		pSpawnExt->CurrentRandomTarget = pWeaponExt->RandomTarget_Spawners_MultipleTargets ?
			TechnoExt::FindRandomTarget(pSpawner) : pSpawnerExt->CurrentRandomTarget;
		pSpawnExt->OriginalTargetWeaponIndex = pSpawnerExt->OriginalTargetWeaponIndex;
	}

	if (pSpawnExt->CurrentRandomTarget)
	{
		R->EAX(pSpawnExt->CurrentRandomTarget);
		return 0x6B76EA;
	}

	return 0;
}

DEFINE_HOOK(0x730F00, AIMissionClassUAEXXZ_StopSelected_RandomTarget_ClearRetargets, 0x5)
{
	// Makes technos with random targets stop targeting
	for (auto pObj : ObjectClass::CurrentObjects())
	{
		auto pTechno = abstract_cast<TechnoClass*>(pObj);
		if (!pTechno)
			continue;

		auto pExt = TechnoExt::ExtMap.Find(pTechno);
		if (!pExt)
			continue;

		if (pExt->CurrentRandomTarget || pExt->OriginalTarget)
		{
			if (SessionClass::IsMultiplayer())
			{
				TechnoExt::SendStopRandomTargetTarNav(pTechno); // Prevent desyncs!
			}
			else
			{
				pExt->CurrentRandomTarget = nullptr;
				pExt->OriginalTarget = nullptr;
				pExt->OriginalTargetWeaponIndex = -1;
				pTechno->ForceMission(Mission::Guard);
				pTechno->SetTarget(nullptr);

				if (pTechno->SpawnManager)
					pTechno->SpawnManager->ResetTarget();
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4D4256, MissionMove_RandomTarget_ClearRetargets, 0x9)
{
	GET(FootClass*, pThis, ESI);

	if (!pThis)
		return 0;

	auto pTechno = abstract_cast<TechnoClass*>(pThis);
	if (!pTechno)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pTechno);
	if (!pExt)
		return 0;

	if ((pExt->CurrentRandomTarget || pExt->OriginalTarget) && pThis->CurrentMission == Mission::Move)
	{
		pExt->ResetRandomTarget = false;
		pExt->CurrentRandomTarget = nullptr;
		pExt->OriginalTarget = nullptr;
		pExt->OriginalTargetWeaponIndex = -1;

		if (pThis->SpawnManager)
			pThis->SpawnManager->ResetTarget();
	}

	return 0;
}

DEFINE_HOOK(0x6FE562, TechnoClass_FireAt_RandomTarget_BulletWithNewTarget, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(BulletClass*, pBullet, EAX);

	if (!pBullet)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt || !pExt->CurrentRandomTarget)
		return 0;

	auto const pCurrRandTarget = pExt->CurrentRandomTarget;
	bool isValidRandTechno = TechnoExt::IsValidTechno(pCurrRandTarget);
	bool isValidTechno = TechnoExt::IsValidTechno(pThis->Target);

	if (!isValidTechno)
		pThis->SetTarget(nullptr);

	if (!isValidRandTechno || !isValidTechno)
	{
		pThis->SetTarget(pExt->OriginalTarget);
		pExt->ResetRandomTarget = false;
		pExt->CurrentRandomTarget = nullptr;
		pExt->OriginalTarget = nullptr;
		pExt->OriginalTargetWeaponIndex = -1;

		pBullet->Detonate(pBullet->GetCoords());
		pBullet->Limbo();
		pBullet->UnInit();

		if (pThis->SpawnManager)
			pThis->SpawnManager->ResetTarget();

		return 0;
	}

	pBullet->Target = pExt->CurrentRandomTarget;

	if (pExt->OriginalTargetWeaponIndex < 0)
		return 0;

	const auto pWeaponType = pThis->GetWeapon(pExt->OriginalTargetWeaponIndex)->WeaponType;
	const auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeaponType);

	if (!pWeaponTypeExt)
		return 0;

	return 0;
}

DEFINE_HOOK(0x6FF8F1, TechnoClass_FireAt_RandomTarget_ResetTargets, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return 0;

	if (pExt->OriginalTargetWeaponIndex < 0)
		return 0;

	// Distance & weapon checks
	auto const pWeapon = pThis->GetWeapon(pExt->OriginalTargetWeaponIndex)->WeaponType;
	if (!pWeapon)
		return 0;

	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return 0;

	if (pExt->ResetRandomTarget || (pExt->CurrentRandomTarget && !TechnoExt::IsValidTechno(pExt->CurrentRandomTarget)))
	{
		pExt->ResetRandomTarget = false;
		pExt->CurrentRandomTarget = TechnoExt::FindRandomTarget(pThis);

		pThis->Target = pExt->CurrentRandomTarget;
	}

	return 0;
}
