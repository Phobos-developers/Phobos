#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/Script/Body.h>

void TechnoExt::NewRandomTarget(TechnoClass* pThis)
{
	if (!pThis)
		return;

	bool hasWeapons = ScriptExt::IsUnitArmed(pThis);
	if (!hasWeapons)
		return;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return;

	if (!IsValidTechno(pExt->OriginalTarget))
		pExt->OriginalTarget = nullptr;

	if (!IsValidTechno(pExt->CurrentRandomTarget))
		pExt->CurrentRandomTarget = nullptr;

	if (pExt->ResetRandomTarget
		|| (pExt->CurrentRandomTarget && pExt->CurrentRandomTarget != pThis->Target))
	{
		pExt->ResetRandomTarget = false;
		pExt->CurrentRandomTarget = nullptr;

		if (pExt->OriginalTarget)
			pThis->Target = pExt->OriginalTarget;

		return;
	}

	int originalTargetWeaponIndex = pExt->OriginalTargetWeaponIndex;
	int targetWeaponIndex = pThis->Target ? pThis->SelectWeapon(pThis->Target) : -1;
	int weaponIndex = originalTargetWeaponIndex >= 0 && targetWeaponIndex != originalTargetWeaponIndex ? originalTargetWeaponIndex : targetWeaponIndex;

	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	if (!pWeapon)
		return;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return;

	//if (!IsValidTechno(pThis->Target))
		//pThis->SetTarget(nullptr);

	if (pThis->Target && pExt->CurrentRandomTarget && pThis->CurrentMission == Mission::Guard)
		pThis->ForceMission(Mission::Attack);

	if (pExt->CurrentRandomTarget && pThis->SpawnManager)
		return;

	//if (!pThis->Target)// && !pExt->OriginalTarget)
		//return;

	if (pExt->OriginalTarget)
	{
		pExt->ResetRandomTarget = false;
		pExt->CurrentRandomTarget = nullptr;
		//pExt->OriginalTarget = nullptr;
		//pExt->OriginalTargetWeaponIndex = -1;
	}

	if (pExt->CurrentRandomTarget && pThis->GetCurrentMission() != Mission::Attack)
	{
		pExt->CurrentRandomTarget = nullptr;
		pThis->SetTarget(pExt->OriginalTarget ? pExt->OriginalTarget : nullptr);
		pExt->OriginalTarget = nullptr;

		return;
	}

	// Distances check
	if (pExt->OriginalTarget)
	{
		bool isBuilding = pThis->WhatAmI() != AbstractType::Building;
		int minimumRange = pWeapon->MinimumRange;
		int range = pWeapon->Range;
		range += pExt->OriginalTarget->IsInAir() ? pThis->GetTechnoType()->AirRangeBonus : 0;
		int distanceToOriginalTarget = pThis->DistanceFrom(pExt->OriginalTarget);
		int distanceToCurrentRandomTarget = pExt->CurrentRandomTarget ? pThis->DistanceFrom(pExt->CurrentRandomTarget) : 0;

		if (distanceToOriginalTarget < minimumRange
			|| distanceToOriginalTarget > range
			|| (isBuilding && pExt->CurrentRandomTarget) && (distanceToCurrentRandomTarget > range || distanceToCurrentRandomTarget < minimumRange))
		{
			pThis->SetTarget(pExt->OriginalTarget);
			pExt->ResetRandomTarget = false;
			pExt->CurrentRandomTarget = nullptr;

			return;
		}
	}

	if (pThis->CurrentMission == Mission::Move)
	{
		pExt->CurrentRandomTarget = nullptr;
		pExt->OriginalTarget = nullptr;

		return;
	}

	pExt->OriginalTargetWeaponIndex = weaponIndex;

	auto pRandomTarget = FindRandomTarget(pThis);
	if (!pRandomTarget)
	{
		pThis->SetTarget(pExt->OriginalTarget ? pExt->OriginalTarget : nullptr);
		pExt->CurrentRandomTarget = nullptr;
		pExt->OriginalTarget = nullptr;
		pExt->OriginalTargetWeaponIndex = -1;

		return;
	}

	pExt->OriginalTarget = !pExt->OriginalTarget ? pThis->Target : pExt->OriginalTarget;
	pExt->CurrentRandomTarget = pRandomTarget;
	pThis->Target = pRandomTarget;

	if (pThis->SpawnManager)
	{
		for (auto pSpawn : pThis->SpawnManager->SpawnedNodes)
		{

			if (!pSpawn->Unit)
				continue;

			auto pSpawnExt = TechnoExt::ExtMap.Find(pSpawn->Unit);
			pSpawnExt->OriginalTarget = pExt->OriginalTarget;
		}
	}

	return;
}

// Find all valid targets in the attacker's area and pick randomly one from the valid list
TechnoClass* TechnoExt::FindRandomTarget(TechnoClass* pThis)
{
	TechnoClass* pSelection = nullptr;

	if (!pThis || !IsValidTechno(pThis->Target))
		return pSelection;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return pSelection;

	int weaponIndex = IsValidTechno(pExt->OriginalTarget) || pExt->OriginalTargetWeaponIndex >= 0 ? pExt->OriginalTargetWeaponIndex : pThis->SelectWeapon(pThis->Target);

	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	if (!pWeapon)
		return pSelection;

	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return pSelection;

	auto const originalTarget = static_cast<TechnoClass*>(pExt->OriginalTarget ? pExt->OriginalTarget : pThis->Target);

	if (!IsValidTechno(originalTarget))
		return pSelection;

	int retargetProbability = std::min((int)round(pWeaponExt->RandomTarget * 100), 100);
	int dice = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	if (retargetProbability < dice)
		return originalTarget;

	bool friendlyFire = pThis->Owner->IsAlliedWith(originalTarget->Owner);
	auto pThisType = pThis->GetTechnoType();
	int minimumRange = pWeapon->MinimumRange;
	int groundRange = pWeapon->Range;
	int airRange = pWeapon->Range + pThisType->AirRangeBonus;
	std::vector<TechnoClass*> candidates;
	candidates.push_back(originalTarget);

	// Looking for all valid targeting candidates
	for (auto pCandidate : *TechnoClass::Array)
	{
		auto const pCandidateType = pCandidate->GetTechnoType();
		int candidateNeedsWeaponIdx = pThis->SelectWeapon(pCandidate);
		auto const pBuilding = abstract_cast<BuildingClass*>(pCandidate);

		if (!IsValidTechno(pCandidate)
			|| pCandidate == pThis
			|| pCandidate == originalTarget
			|| (pExt->OriginalTargetWeaponIndex >= 0 && pExt->OriginalTargetWeaponIndex != candidateNeedsWeaponIdx)
			|| pCandidateType->Immune
			|| (pBuilding && pBuilding->Type->InvisibleInGame)
			|| !EnumFunctions::IsTechnoEligible(pCandidate, pWeaponExt->CanTarget, true)
			|| !pWeaponExt->CanOnlyTargetTheseTechnos(pCandidateType)
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

		int range = pCandidate->IsInAir() ? airRange : groundRange;
		int distanceFromAttacker = pThis->DistanceFrom(pCandidate);
		if (distanceFromAttacker < minimumRange)
			continue;

		if (pWeapon->OmniFire)
		{
			if (distanceFromAttacker <= range)
				candidates.push_back(pCandidate);
		}
		else
		{
			int distanceFromOriginalTargetToCandidate = pCandidate->DistanceFrom(pThis->Target);
			int distanceFromOriginalTarget = pThis->DistanceFrom(pThis->Target);

			if (distanceFromAttacker <= range && distanceFromOriginalTargetToCandidate <= distanceFromOriginalTarget)
				candidates.push_back(pCandidate);
		}
	}

	// Pick one new target from the list of targets inside the weapon range
	dice = ScenarioClass::Instance->Random.RandomRanged(0, candidates.size() - 1);
	pSelection = candidates.at(dice);

	return pSelection;
}

void TechnoExt::SendStopRandomTargetTarNav(TechnoClass* pThis)
{
	//auto pFoot = abstract_cast<FootClass*>(pThis);

	EventExt event;
	event.Type = EventTypeExt::SyncStopRandomTargetTarNav;
	event.HouseIndex = (char)HouseClass::CurrentPlayer->ArrayIndex;
	event.Frame = Unsorted::CurrentFrame;

	event.AddEvent();
}

void TechnoExt::HandleStopRandomTargetTarNav(EventExt* event)
{
	DWORD technoUniqueID = event->SyncStopRandomTargetTarNav.TechnoUniqueID;

	for (auto pTechno : *TechnoClass::Array)
	{
		if (pTechno && pTechno->UniqueID == technoUniqueID)
		{
			auto const pExt = TechnoExt::ExtMap.Find(pTechno);

			pExt->CurrentRandomTarget = nullptr;
			pExt->OriginalTarget = nullptr;
			pExt->OriginalTargetWeaponIndex = -1;
			pTechno->ForceMission(Mission::Guard);

			auto pFoot = abstract_cast<FootClass*>(pTechno);

			if (pFoot->Locomotor->Is_Moving_Now())
				pFoot->StopMoving();

			if (pTechno->SpawnManager)
				pTechno->SpawnManager->ResetTarget();

			break;
		}
	}
}
