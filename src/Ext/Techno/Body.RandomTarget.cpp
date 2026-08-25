#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/Script/Body.h>
#include <ScenarioClass.h>
#include <BuildingClass.h>
#include <Utilities/EnumFunctions.h>

#include <algorithm>
#include <cmath>
#include <vector>

AbstractClass* TechnoExt::FindRandomTarget(TechnoClass* pFirer, AbstractClass* pOriginalTarget, WeaponTypeClass* pWeapon)
{
	if (!pFirer || !pWeapon)
		return pOriginalTarget;

	const auto pWeaponExt = WeaponTypeExt::TryFetch(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return pOriginalTarget;

	// Overall random target activation roll
	if (pWeaponExt->RandomTarget < 1.0)
	{
		const double roll = ScenarioClass::Instance->Random.RandomDouble();
		if (roll > pWeaponExt->RandomTarget)
			return pOriginalTarget;
	}

	const auto pFirerType = pFirer->GetTechnoType();
	if (!pFirerType)
		return pOriginalTarget;

	const bool isOmniFire = pWeapon->OmniFire;
	const int minRange = pWeapon->MinimumRange;
	const int range = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer);

	if (range <= 0)
		return pOriginalTarget;

	const int distFromOrigToFirer = pOriginalTarget ? pFirer->DistanceFrom(pOriginalTarget) : range;

	// Scatter shot to a random ground cell within the intersection area
	if (pWeaponExt->RandomTarget_MissChance > 0.0)
	{
		const double missRoll = ScenarioClass::Instance->Random.RandomDouble();
		if (missRoll < pWeaponExt->RandomTarget_MissChance)
		{
			const CoordStruct firerCoords = pFirer->GetCoords();
			const CoordStruct origCoords = pOriginalTarget ? pOriginalTarget->GetCoords() : firerCoords;

			for (int attempt = 0; attempt < 30; ++attempt)
			{
				const int scatterDist = ScenarioClass::Instance->Random.RandomRanged(minRange, range);
				const double scatterAngle = ScenarioClass::Instance->Random.RandomDouble() * 6.283185307179586;

				CoordStruct scatterCoords;
				scatterCoords.X = firerCoords.X + static_cast<int>(scatterDist * cos(scatterAngle));
				scatterCoords.Y = firerCoords.Y + static_cast<int>(scatterDist * sin(scatterAngle));
				scatterCoords.Z = firerCoords.Z;

				if (!isOmniFire && pOriginalTarget)
				{
					const double distFromOrig = scatterCoords.DistanceFrom(origCoords);
					if (distFromOrig > distFromOrigToFirer)
						continue;
				}

				CellClass* pCell = MapClass::Instance.TryGetCellAt(scatterCoords);
				if (pCell)
				{
					const int distFirer = pFirer->DistanceFrom(pCell);
					if (distFirer >= minRange && distFirer <= range)
						return pCell;
				}
			}
		}
	}

	// Invert house filter to acquire friendly/allied targets
	bool targetAllies = false;
	if (pWeaponExt->RandomTarget_FriendlyFireChance > 0.0)
	{
		const double allyRoll = ScenarioClass::Instance->Random.RandomDouble();
		if (allyRoll < pWeaponExt->RandomTarget_FriendlyFireChance)
			targetAllies = true;
	}

	auto const pOrigHouse = pOriginalTarget ? pOriginalTarget->GetOwningHouse() : nullptr;

	std::vector<TechnoClass*> candidates;

	if (pOriginalTarget)
	{
		if (auto const pOrigTechno = abstract_cast<TechnoClass*>(pOriginalTarget))
		{
			if (!targetAllies || (pFirer->Owner && (pFirer->Owner->IsAlliedWith(pOrigTechno->Owner) || pOrigTechno->Owner == pFirer->Owner)))
				candidates.push_back(pOrigTechno);
		}
	}

	for (const auto pCandidate : TechnoClass::Array)
	{
		if (!pCandidate || pCandidate == pFirer || pCandidate == pOriginalTarget)
			continue;

		// Fast distance cull before deeper evaluations
		const int distFromFirer = pFirer->DistanceFrom(pCandidate);
		if (distFromFirer < minRange || distFromFirer > range)
			continue;

		// Bounded intersection cone for OmniFire=no
		if (!isOmniFire && pOriginalTarget)
		{
			const int distFromOrigToCandidate = pCandidate->DistanceFrom(pOriginalTarget);
			if (distFromOrigToCandidate > distFromOrigToFirer)
				continue;
		}

		// Liveness and state validity
		if (!pCandidate->IsAlive || pCandidate->InLimbo || pCandidate->Health <= 0 || !pCandidate->Owner
			|| pCandidate->IsSinking || pCandidate->IsCrashing || pCandidate->BeingWarpedOut)
			continue;

		auto const pCandidateType = pCandidate->GetTechnoType();
		if (!pCandidateType || pCandidateType->Immune)
			continue;

		if (pCandidate->CloakState == CloakState::Cloaked && !pCandidateType->Naval)
			continue;

		if (pCandidate->InWhichLayer() == Layer::Underground)
			continue;

		if (auto pBuilding = abstract_cast<BuildingClass*>(pCandidate))
		{
			if (pBuilding->Type->InvisibleInGame)
				continue;
		}

		if (pCandidate->TemporalTargetingMe)
			continue;

		// Terrain layer restrictions
		const auto landType = pCandidate->GetCell() ? pCandidate->GetCell()->LandType : LandType::Clear;
		const bool isOnWater = (landType == LandType::Water || landType == LandType::Beach) && !pCandidate->IsInAir();

		if (pCandidateType->Underwater && pFirerType->NavalTargeting == NavalTargetingType::Underwater_Never)
			continue;
		if (isOnWater && pFirerType->NavalTargeting == NavalTargetingType::Naval_None)
			continue;
		if (!isOnWater && pFirerType->LandTargeting == LandTargetingType::Land_Not_OK)
			continue;

		// Air / Ground capabilities
		if (pWeapon->Projectile)
		{
			const bool inAir = pCandidate->IsInAir();
			if (!pWeapon->Projectile->AA && inAir)
				continue;
			if (!pWeapon->Projectile->AG && !inAir)
				continue;
		}

		// House relationships
		if (targetAllies)
		{
			const bool isFriendly = pFirer->Owner && (pFirer->Owner->IsAlliedWith(pCandidate->Owner) || pCandidate->Owner == pFirer->Owner);
			if (!isFriendly)
				continue;
		}
		else
		{
			const bool isSameHouseAsTarget = pOrigHouse && (pCandidate->Owner == pOrigHouse);
			const bool isEnemy = pFirer->Owner && (!pFirer->Owner->IsAlliedWith(pCandidate->Owner) && !ScriptExt::IsMindControlledByEnemy(pFirer->Owner, pCandidate));

			if (!isSameHouseAsTarget && !isEnemy)
				continue;
		}

		// Phobos weapon eligibility filters
		if (!EnumFunctions::IsTechnoEligible(pCandidate, pWeaponExt->CanTarget, true))
			continue;

		if (!pWeaponExt->CanOnlyTargetTheseTechnos(pCandidateType))
			continue;

		if (!pWeaponExt->IsHealthInThreshold(pCandidate))
			continue;

		if (!pWeaponExt->HasRequiredAttachedEffects(pCandidate, pFirer))
			continue;

		candidates.push_back(pCandidate);
	}

	if (candidates.empty())
		return pOriginalTarget;

	const int pick = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(candidates.size()) - 1);
	return candidates[pick];
}
