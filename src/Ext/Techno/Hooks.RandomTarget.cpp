#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/Script/Body.h>

DEFINE_HOOK(0x736F61, UnitClass_FiringAI_BurstRandomTarget_Setup, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->RearmTimer.TimeLeft <= 0 && pThis->Target)
		TechnoExt::UpdateRandomTarget(pThis);

	return 0;
}

DEFINE_HOOK(0x4D4E8A, FootClass_FiringAI_BurstRandomTarget_Setup, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->RearmTimer.TimeLeft <= 0 && pThis->Target)
		TechnoExt::UpdateRandomTarget(pThis);

	return 0;
}

DEFINE_HOOK(0x44AFF8, BuildingClass_FireAt_BurstRandomTarget_Setup, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto pOriginalTarget = pThis->Target;

	if (pThis->RearmTimer.TimeLeft <= 0 && pThis->Target)
		TechnoExt::UpdateRandomTarget(pThis);

	int weaponIndex = pThis->SelectWeapon(pThis->Target);
	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	if (!pWeapon || pWeapon->IsLaser || pWeapon->Spawner)
		return 0;

	if (pThis->Target)
		pThis->Target = pOriginalTarget;

	return 0;
}

DEFINE_HOOK(0x6B770D, SpawnerManagerClassAI_SwitchCase3, 0xB)
{
	GET(SpawnManagerClass*, pThis, ESI);

	bool skip = false;

	for (auto pItem : pThis->SpawnedNodes)
	{
		const auto pSpawnExt = TechnoExt::ExtMap.Find(pItem->Unit);
		if (!pSpawnExt)
			continue;

		auto const pCurrRandTarget = pSpawnExt->CurrentRandomTarget;
		bool isValidTechno = pCurrRandTarget
			&& pCurrRandTarget->IsAlive
			&& pCurrRandTarget->Health > 0
			&& ScriptExt::IsUnitAvailable(pCurrRandTarget, true)
			&& (pCurrRandTarget->WhatAmI() == AbstractType::Infantry
				|| pCurrRandTarget->WhatAmI() == AbstractType::Unit
				|| pCurrRandTarget->WhatAmI() == AbstractType::Building
				|| pCurrRandTarget->WhatAmI() == AbstractType::Aircraft);

		if (isValidTechno)
		{
			skip = true;
			pItem->Unit->Target = pSpawnExt->CurrentRandomTarget;
		}
		else
		{
			pSpawnExt->CurrentRandomTarget = nullptr;
		}
	}

	if (skip)
		return 0x6B771E;

	return 0;
}

DEFINE_HOOK(0x6B776E, SpawnerManagerClassAI_SwitchCase4, 0x6) // Note: Not sure in which cases the game enters in this hook, maybe not necessary :-/
{
	GET(AircraftClass*, pThis, ECX);

	const auto pSpawnExt = TechnoExt::ExtMap.Find(pThis);
	if (!pSpawnExt)
		return 0;

	auto const pCurrRandTarget = pSpawnExt->CurrentRandomTarget;
	bool isValidTechno = pCurrRandTarget
		&& pCurrRandTarget->IsAlive
		&& pCurrRandTarget->Health > 0
		&& ScriptExt::IsUnitAvailable(pCurrRandTarget, true)
		&& (pCurrRandTarget->WhatAmI() == AbstractType::Infantry
			|| pCurrRandTarget->WhatAmI() == AbstractType::Unit
			|| pCurrRandTarget->WhatAmI() == AbstractType::Building
			|| pCurrRandTarget->WhatAmI() == AbstractType::Aircraft);

	if (isValidTechno)
	{
		pThis->Target = pSpawnExt->CurrentRandomTarget;
		return 0x6B777A;
	}
	else
	{
		pSpawnExt->CurrentRandomTarget = nullptr;
	}

	return 0;
}

DEFINE_HOOK(0x6B7AE3, SpawnerManagerClassAI_SpawnControlStatus3, 0x6)
{
	GET(AircraftClass*, pThis, ECX);

	const auto pSpawnExt = TechnoExt::ExtMap.Find(pThis);
	if (!pSpawnExt)
		return 0;

	auto const pCurrRandTarget = pSpawnExt->CurrentRandomTarget;
	bool isValidTechno = pCurrRandTarget
		&& pCurrRandTarget->IsAlive
		&& pCurrRandTarget->Health > 0
		&& ScriptExt::IsUnitAvailable(pCurrRandTarget, true)
		&& (pCurrRandTarget->WhatAmI() == AbstractType::Infantry
			|| pCurrRandTarget->WhatAmI() == AbstractType::Unit
			|| pCurrRandTarget->WhatAmI() == AbstractType::Building
			|| pCurrRandTarget->WhatAmI() == AbstractType::Aircraft);

	if (isValidTechno)
	{
		pThis->Target = pSpawnExt->CurrentRandomTarget;
		return 0x6B7AEF;
	}
	else
	{
		pSpawnExt->CurrentRandomTarget = nullptr;
	}

	return 0;
}

DEFINE_HOOK(0x730F00, AIMissionClassUAEXXZ_StopSelected_ClearRetargets, 0x5)
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

		if (pExt->CurrentRandomTarget)
		{
			if (SessionClass::IsMultiplayer())
			{
				TechnoExt::SendStopRandomTargetTarNav(pTechno); // Prevent desyncs!
			}
			else
			{
				pExt->CurrentRandomTarget = nullptr;
				pExt->OriginalTarget = nullptr;
				pTechno->ForceMission(Mission::Guard);
				return 0;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4D4256, Mission_Move_ClearRetargets, 0x9)
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

	if (pExt->CurrentRandomTarget && pThis->CurrentMission == Mission::Move)
	{
		pExt->CurrentRandomTarget = nullptr;
		pExt->OriginalTarget = nullptr;
	}

	return 0;
}

DEFINE_HOOK(0x6FE562, TechnoClass_FireAt_BulletNewTarget, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(BulletClass*, pBullet, EAX);

	if (!pBullet)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return 0;

	if (!pExt->CurrentRandomTarget)
		return 0;

	auto const pCurrRandTarget = pExt->CurrentRandomTarget;
	bool isValidRandTechno = pCurrRandTarget
		&& pCurrRandTarget->IsAlive
		&& pCurrRandTarget->Health > 0
		&& ScriptExt::IsUnitAvailable(pCurrRandTarget, true)
		&& (pCurrRandTarget->WhatAmI() == AbstractType::Infantry
			|| pCurrRandTarget->WhatAmI() == AbstractType::Unit
			|| pCurrRandTarget->WhatAmI() == AbstractType::Building
			|| pCurrRandTarget->WhatAmI() == AbstractType::Aircraft);

	auto const pCurrTarget = static_cast<TechnoClass*>(pThis->Target);
	bool isValidTechno = pCurrTarget
		&& pCurrTarget->IsAlive
		&& pCurrTarget->Health > 0
		&& ScriptExt::IsUnitAvailable(pCurrTarget, true)
		&& (pCurrTarget->WhatAmI() == AbstractType::Infantry
			|| pCurrTarget->WhatAmI() == AbstractType::Unit
			|| pCurrTarget->WhatAmI() == AbstractType::Building
			|| pCurrTarget->WhatAmI() == AbstractType::Aircraft);

	if (!isValidTechno)
		pThis->SetTarget(nullptr);

	if (!isValidTechno || !isValidRandTechno)
	{
		pExt->CurrentRandomTarget = nullptr;
		pThis->SetTarget(pExt->OriginalTarget);
		pExt->OriginalTarget = nullptr;

		pBullet->Detonate(pBullet->GetCoords());
		pBullet->Limbo();
		pBullet->UnInit();

		return 0;
	}

	pBullet->Target = pExt->CurrentRandomTarget;

	int weaponIndex = pThis->SelectWeapon(pThis->Target);
	const auto pWeaponType = pThis->GetWeapon(weaponIndex)->WeaponType;

	const auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeaponType);
	if (!pWeaponTypeExt)
		return 0;

	if (pWeaponTypeExt->RandomTarget_DistributeBurst.Get() && !pThis->SpawnManager)
		pExt->ResetRandomTarget = true;
}

DEFINE_HOOK(0x6FF8F1, TechnoClass_FireAt_ResetRandomTarget, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return 0;

	if (pExt->ResetRandomTarget || (pExt->CurrentRandomTarget && !TechnoExt::IsValidTechno(pExt->CurrentRandomTarget)))
	{
		pExt->ResetRandomTarget = false;
		pExt->CurrentRandomTarget = TechnoExt::FindRandomTarget(pThis);
		pThis->Target = pExt->CurrentRandomTarget;
	}

	return 0;
}
