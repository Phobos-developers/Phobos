#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/Script/Body.h>

bool TechnoExt::UpdateRandomTarget(TechnoClass* pThis)
{
	if (!pThis)
		return false;

	int weaponIndex = pThis->SelectWeapon(pThis->Target);
	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	if (!pWeapon)
		return false;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return false;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return false;

	if (pThis->CurrentMission == Mission::Move)
	{
		pExt->CurrentRandomTarget = nullptr;
		pExt->OriginalTarget = nullptr;

		return false;
	}

	if (!IsValidTechno(pExt->CurrentRandomTarget))
		pExt->CurrentRandomTarget = nullptr;

	if (pExt->OriginalTarget && !IsValidTechno(static_cast<TechnoClass*>(pExt->OriginalTarget)))
		pExt->OriginalTarget = nullptr;

	if (pThis->Target && !IsValidTechno(static_cast<TechnoClass*>(pThis->Target)))
		pThis->SetTarget(nullptr);

	if (pThis->Target && pExt->CurrentRandomTarget && pThis->CurrentMission == Mission::Guard)
		pThis->ForceMission(Mission::Attack);

	if (pExt->CurrentRandomTarget && pThis->SpawnManager)
		return false;

	if (!pThis->Target && !pExt->OriginalTarget)
		return false;

	if (!pExt->OriginalTarget)
	{
		pExt->CurrentRandomTarget = nullptr;
		pExt->OriginalTarget = nullptr;
	}

	if (pExt->CurrentRandomTarget && pThis->GetCurrentMission() != Mission::Attack)
	{
		pExt->CurrentRandomTarget = nullptr;
		pThis->SetTarget(pExt->OriginalTarget ? pExt->OriginalTarget : nullptr);
		pExt->OriginalTarget = nullptr;

		return false;
	}

	if (!pThis->Target)
		return false;

	if (pThis->DistanceFrom(pExt->OriginalTarget) > pWeapon->Range || pThis->DistanceFrom(pThis->Target) > pWeapon->Range)
	{
		if (pThis->WhatAmI() == AbstractType::Building)
		{
			pThis->SetTarget(nullptr);
			pExt->CurrentRandomTarget = nullptr;
			pExt->OriginalTarget = nullptr;

			return false;
		}

		pThis->SetTarget(pExt->OriginalTarget);
	}

	auto pRandomTarget = FindRandomTarget(pThis);

	if (!pRandomTarget)
		return false;

	pExt->OriginalTarget = !pExt->OriginalTarget ? pThis->Target : pExt->OriginalTarget;
	pExt->CurrentRandomTarget = pRandomTarget;
	pThis->Target = pRandomTarget;

	if (pThis->SpawnManager)
	{
		bool isFirstSpawn = true;

		for (auto pSpawn : pThis->SpawnManager->SpawnedNodes)
		{
			if (!pSpawn->Unit)
				continue;

			TechnoClass* pSpawnTarget = nullptr;

			auto pSpawnExt = TechnoExt::ExtMap.Find(pSpawn->Unit);
			if (!pSpawnExt)
				continue;

			if (isFirstSpawn)
			{
				pSpawnTarget = pExt->CurrentRandomTarget;

				if (pWeaponExt->RandomTarget_Spawners_MultipleTargets)
					isFirstSpawn = false;
			}
			else
			{
				pSpawnTarget = FindRandomTarget(pThis);

				if (!pSpawnTarget)
					pSpawnTarget = static_cast<TechnoClass*>(pExt->OriginalTarget);
			}

			pSpawnExt->CurrentRandomTarget = pSpawnTarget;
			pSpawnExt->OriginalTarget = pExt->OriginalTarget;
		}
	}

	return true;
}

// Find all valid targets in the attacker's area and from the valid ones picks one randomly
TechnoClass* TechnoExt::FindRandomTarget(TechnoClass* pThis)
{
	TechnoClass* selection = nullptr;

	if (!pThis || !pThis->Target)
		return selection;

	int weaponIndex = pThis->SelectWeapon(pThis->Target);
	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	if (!pWeapon)
		return selection;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (!pExt || !pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return selection;

	auto const originalTarget = static_cast<TechnoClass*>(pExt->OriginalTarget ? pExt->OriginalTarget : pThis->Target);
	if (!IsValidTechno(originalTarget))
		return selection;

	int retargetProbability = std::min((int)round(pWeaponExt->RandomTarget * 100), 100);
	int dice = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	if (retargetProbability < dice)
		return originalTarget;

	bool friendlyFire = pThis->Owner->IsAlliedWith(originalTarget->Owner);
	auto pThisType = pThis->GetTechnoType();
	int minimumRange = pWeapon->MinimumRange;
	int range = pWeapon->Range;
	int airRange = pWeapon->Range + pThisType->AirRangeBonus;
	std::vector<TechnoClass*> candidates;
	candidates.push_back(originalTarget);

	// Looking for all valid targeting candidates
	for (auto pCandidate : *TechnoClass::Array)
	{
		auto const pCandidateType = pCandidate->GetTechnoType();

		if (pCandidate == pThis
			|| pCandidate == originalTarget
			|| !IsValidTechno(pCandidate)
			|| pCandidateType->Immune
			|| !EnumFunctions::IsTechnoEligible(pCandidate, pWeaponExt->CanTarget, true)
			|| (!pWeapon->Projectile->AA && pCandidate->IsInAir())
			|| (!pWeapon->Projectile->AG && !pCandidate->IsInAir())
			|| (!friendlyFire && (pThis->Owner->IsAlliedWith(pCandidate) || ScriptExt::IsUnitMindControlledFriendly(pThis->Owner, pCandidate)))
			|| pCandidate->TemporalTargetingMe
			|| pCandidate->BeingWarpedOut
			|| (pCandidateType->Underwater && pCandidateType->NavalTargeting == NavalTargetingType::Underwater_Never)
			|| (pCandidateType->Naval && pCandidateType->NavalTargeting == NavalTargetingType::Naval_None)
			|| (pCandidate->CloakState == CloakState::Cloaked && !pCandidateType->Naval)
			|| (pCandidate->InWhichLayer() == Layer::Underground))
		{
			continue;
		}

		int distanceFromAttacker = pThis->DistanceFrom(pCandidate);
		if (distanceFromAttacker < minimumRange)
			continue;

		if (pWeapon->OmniFire)
		{
			if (pCandidate->IsInAir())
				range = airRange;

			if (distanceFromAttacker <= range)
				candidates.push_back(pCandidate);
		}
		else
		{
			int distanceFromOriginalTargetToCandidate = pCandidate->DistanceFrom(pThis->Target);
			int distanceFromOriginalTarget = pThis->DistanceFrom(pThis->Target);

			if (pCandidate->IsInAir())
				range = airRange;

			if (distanceFromAttacker <= range && distanceFromOriginalTargetToCandidate <= distanceFromOriginalTarget)
				candidates.push_back(pCandidate);
		}
	}

	if (candidates.size() == 0)
		return selection;

	// Pick one new target from the list of targets inside the weapon range
	dice = ScenarioClass::Instance->Random.RandomRanged(0, candidates.size() - 1);
	selection = candidates.at(dice);

	return selection;
}

void TechnoExt::SendStopRandomTargetTarNav(TechnoClass* pThis)
{
	auto pFoot = abstract_cast<FootClass*>(pThis);

	EventExt event;
	event.Type = EventTypeExt::SyncStopRandomTargetTarNav;
	event.HouseIndex = (char)HouseClass::CurrentPlayer->ArrayIndex;
	event.Frame = Unsorted::CurrentFrame;

	event.AddEvent();
}

void TechnoExt::HandleStopRandomTargetTarNav(EventExt* event)
{
	int technoUniqueID = event->SyncStopRandomTargetTarNav.TechnoUniqueID;

	for (auto pTechno : *TechnoClass::Array)
	{
		if (pTechno && pTechno->UniqueID == technoUniqueID)
		{
			auto const pExt = TechnoExt::ExtMap.Find(pTechno);

			pExt->CurrentRandomTarget = nullptr;
			pExt->OriginalTarget = nullptr;
			pTechno->ForceMission(Mission::Guard);

			auto pFoot = abstract_cast<FootClass*>(pTechno);

			if (pFoot->Locomotor->Is_Moving_Now())
				pFoot->StopMoving();

			break;
		}
	}
}
