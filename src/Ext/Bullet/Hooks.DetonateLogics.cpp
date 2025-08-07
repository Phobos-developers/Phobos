#include "Body.h"
#include <functional>

#include <Ext/Anim/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Utilities/Helpers.Alex.h>

#include <VoxelAnimClass.h>
#include <AircraftClass.h>
#include <TacticalClass.h>

DEFINE_HOOK(0x4690D4, BulletClass_Logics_NewChecks, 0x6)
{
	enum { SkipShaking = 0x469130, GoToExtras = 0x469AA4 };

	GET(BulletClass*, pBullet, ESI);
	GET(WarheadTypeClass*, pWarhead, EAX);
	GET_BASE(CoordStruct const* const, pCoords, 0x8);

	auto const pExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (auto const pTarget = abstract_cast<TechnoClass*>(pBullet->Target))
	{
		// Check if the WH should affect the techno target or skip it
		if (!pExt->IsHealthInThreshold(pTarget) || (!pExt->AffectsNeutral && pTarget->Owner->IsNeutral()))
			return GoToExtras;
	}

	// Check for ScreenShake
	auto&& [_, visible] = TacticalClass::Instance->CoordsToClient(*pCoords);

	if (pExt->ShakeIsLocal && !visible)
		return SkipShaking;

	return 0;
}

DEFINE_HOOK(0x4692BD, BulletClass_Logics_ApplyMindControl, 0x6)
{
	enum { SkipGameCode = 0x4692D5 };

	GET(BulletClass*, pThis, ESI);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto const pControlledAnimType = pWHExt->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);
	auto const threatDelay = pWHExt->MindControl_ThreatDelay.Get(RulesExt::Global()->MindControl_ThreatDelay);
	R->AL(CaptureManagerExt::CaptureUnit(pThis->Owner->CaptureManager, pThis->Target, pControlledAnimType, threatDelay));

	return SkipGameCode;
}

DEFINE_HOOK(0x469A75, BulletClass_Logics_DamageHouse, 0x7)
{
	GET(BulletClass*, pThis, ESI);
	GET(HouseClass*, pHouse, ECX);

	if (!pHouse)
		R->ECX(BulletExt::ExtMap.Find(pThis)->FirerHouse);

	return 0;
}

#pragma region DetonateOnAllMapObjects

DEFINE_HOOK(0x4690C1, BulletClass_Logics_DetonateOnAllMapObjects, 0x8)
{
	enum { ReturnFromFunction = 0x46A2FB };

	GET(BulletClass*, pThis, ESI);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	if (pWHExt->DetonateOnAllMapObjects && !pWHExt->WasDetonatedOnAllMapObjects
		&& pWHExt->DetonateOnAllMapObjects_AffectTargets != AffectedTarget::None
		&& pWHExt->DetonateOnAllMapObjects_AffectHouses != AffectedHouse::None)
	{
		pWHExt->WasDetonatedOnAllMapObjects = true;
		auto const originalLocation = pThis->Location;
		auto const pOriginalTarget = pThis->Target;
		auto const isFull = pWHExt->DetonateOnAllMapObjects_Full;
		auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;

		auto copy_dvc = []<typename T>(const DynamicVectorClass<T>&dvc)
		{
			std::vector<T> vec(dvc.Count);
			std::copy(dvc.begin(), dvc.end(), vec.begin());
			return vec;
		};

		auto tryDetonate = [pThis, pWHExt, pOwner, isFull](TechnoClass* pTechno)
			{
				if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner))
				{
					if (isFull)
					{
						pThis->Target = pTechno;
						pThis->Location = pTechno->GetCoords();
						pThis->Detonate(pTechno->GetCoords());
					}
					else
					{
						int damage = (pThis->Health * pThis->DamageMultiplier) >> 8;
						pWHExt->DamageAreaWithTarget(pTechno->GetCoords(), damage, pThis->Owner, pThis->WH, true, pOwner, pTechno);
					}
				}
			};

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Aircraft) != AffectedTarget::None)
		{
			auto const aircraft = copy_dvc(AircraftClass::Array);

			for (auto const pAircraft : aircraft)
				tryDetonate(pAircraft);
		}

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Building) != AffectedTarget::None)
		{
			auto const buildings = copy_dvc(BuildingClass::Array);

			for (auto const pBuilding : buildings)
				tryDetonate(pBuilding);
		}

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Infantry) != AffectedTarget::None)
		{
			auto const infantry = copy_dvc(InfantryClass::Array);

			for (auto const pInf : infantry)
				tryDetonate(pInf);
		}

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Unit) != AffectedTarget::None)
		{
			auto const units = copy_dvc(UnitClass::Array);

			for (auto const pUnit : units)
				tryDetonate(pUnit);
		}

		pThis->Target = pOriginalTarget;
		pThis->Location = originalLocation;
		pWHExt->WasDetonatedOnAllMapObjects = false;

		return ReturnFromFunction;
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x469D1A, BulletClass_Logics_Debris, 0x6)
{
	enum { SkipGameCode = 0x469EBA };

	GET(BulletClass*, pThis, ESI);

	const auto pWH = pThis->WH;
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt->Debris_Conventional && pThis->GetCell()->LandType == LandType::Water)
		return SkipGameCode;

	// Fix the debris count to be in range of Min, Max instead of Min, Max-1.
	int totalSpawnAmount = ScenarioClass::Instance->Random.RandomRanged(pWH->MinDebris, pWH->MaxDebris);

	if (totalSpawnAmount > 0)
	{
		const auto& debrisTypes = pWH->DebrisTypes;
		const auto& debrisMaximums = pWH->DebrisMaximums;

		const auto pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
		auto coord = pThis->GetCoords();

		int count = Math::min(debrisTypes.Count, debrisMaximums.Count);

		// Restore DebrisMaximums logic
		// Make DebrisTypes generate completely in accordance with DebrisMaximums,
		// without continuously looping until it exceeds totalSpawnAmount
		if (count > 0)
		{
			const auto& debrisMinimums = pWHExt->DebrisMinimums;
			const bool limit = pWHExt->DebrisTypes_Limit.Get(count > 1);
			const int minIndex = static_cast<int>(debrisMinimums.size()) - 1;
			int currentIndex = 0;

			while (totalSpawnAmount > 0)
			{
				const int currentMaxDebris = Math::min(1, debrisMaximums[currentIndex]);
				const int currentMinDebris = (minIndex >= 0) ? Math::max(0, debrisMinimums[Math::min(currentIndex, minIndex)]) : 0;
				int amountToSpawn = Math::min(totalSpawnAmount, ScenarioClass::Instance->Random.RandomRanged(currentMinDebris, currentMaxDebris));
				totalSpawnAmount -= amountToSpawn;

				for ( ; amountToSpawn > 0; --amountToSpawn)
					GameCreate<VoxelAnimClass>(debrisTypes[currentIndex], &coord, pOwner);

				if (totalSpawnAmount <= 0)
					break;

				if (++currentIndex < count)
					continue;

				if (limit)
					break;

				currentIndex = 0;
			}
		}
		// Record the ownership of the animation
		// Different from technos, the original judging condition here were mutually exclusive
		else
		{
			const auto debrisAnims = pWHExt->DebrisAnims.GetElements(RulesClass::Instance->MetallicDebris);
			const int maxIndex = static_cast<int>(debrisAnims.size()) - 1;

			if (maxIndex >= 0)
			{
				do
				{
					const int debrisIndex = ScenarioClass::Instance->Random.RandomRanged(0, maxIndex);
					const auto pAnim = GameCreate<AnimClass>(debrisAnims[debrisIndex], coord);
					AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false, true);
				}
				while (--totalSpawnAmount > 0);
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x469B44, BulletClass_Logics_LandTypeCheck, 0x6)
{
	enum { SkipChecks = 0x469BA2 };

	GET(BulletClass*, pThis, ESI);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	if (pWHExt->Conventional_IgnoreUnits)
		return SkipChecks;

	return 0;
}

DEFINE_HOOK(0x469C46, BulletClass_Logics_DamageAnimSelected, 0x8)
{
	enum { SkipGameCode = 0x469C98 };

	GET(BulletClass*, pThis, ESI);
	GET(AnimTypeClass*, pAnimType, EBX);
	LEA_STACK(CoordStruct*, coords, STACK_OFFSET(0xA4, -0x40));

	bool createdAnim = false;

	if (pAnimType)
	{
		auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
		auto const pOwner = pThis->Owner;
		const bool splashed = pWHExt->Splashed;
		const int creationInterval = splashed ? pWHExt->SplashList_CreationInterval : pWHExt->AnimList_CreationInterval;
		int* remainingInterval = &pWHExt->RemainingAnimCreationInterval;
		const int scatterMin = splashed ? pWHExt->SplashList_ScatterMin.Get() : pWHExt->AnimList_ScatterMin.Get();
		const int scatterMax = splashed ? pWHExt->SplashList_ScatterMax.Get() : pWHExt->AnimList_ScatterMax.Get();
		const bool allowScatter = scatterMax != 0 || scatterMin != 0;

		if (creationInterval > 0 && pOwner)
			remainingInterval = &TechnoExt::ExtMap.Find(pOwner)->WHAnimRemainingCreationInterval;

		if (creationInterval < 1 || *remainingInterval <= 0)
		{
			*remainingInterval = creationInterval;

			HouseClass* pInvoker = pOwner ? pOwner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
			HouseClass* pVictim = nullptr;

			if (auto const pTarget = abstract_cast<TechnoClass*>(pThis->Target))
				pVictim = pTarget->Owner;

			auto types = make_iterator_single(pAnimType);

			if (pWHExt->SplashList_CreateAll && splashed)
			{
				types = pWHExt->SplashList.GetElements(RulesClass::Instance->SplashList);
			}
			else if (!splashed)
			{
				bool createAll = pWHExt->AnimList_CreateAll;

				if (pWHExt->Crit_Active && pWHExt->Crit_AnimList.size() > 0 && !pWHExt->Crit_AnimOnAffectedTargets)
				{
					createAll = pWHExt->Crit_AnimList_CreateAll.Get(createAll);

					if (createAll)
						types = pWHExt->Crit_AnimList;
				}
				else if (createAll)
				{
					types = pWHExt->OwnerObject()->AnimList;
				}
			}

			auto& random = ScenarioClass::Instance->Random;

			for (auto const& pType : types)
			{
				if (!pType)
					continue;

				auto animCoords = *coords;

				if (allowScatter)
				{
					const int distance = random.RandomRanged(scatterMin, scatterMax);
					animCoords = MapClass::GetRandomCoordsNear(animCoords, distance, false);
				}

				auto const pAnim = GameCreate<AnimClass>(pType, animCoords, 0, 1, 0x2600, -15, false);
				createdAnim = true;
				AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);

				if (!pAnim->Owner)
					pAnim->Owner = pInvoker;

				if (pOwner)
				{
					auto const pExt = AnimExt::ExtMap.Find(pAnim);
					pExt->SetInvoker(pOwner);
				}
			}
		}
		else
		{
			(*remainingInterval)--;
		}
	}

	R->EAX(createdAnim);
	return SkipGameCode;
}

DEFINE_HOOK(0x469AA4, BulletClass_Logics_Extras, 0x5)
{
	GET(BulletClass*, pThis, ESI);
	GET_BASE(CoordStruct const* const, coords, 0x8);

	auto const pTechno = pThis->Owner;
	auto const pOwner = pTechno ? pTechno->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;

	// Extra warheads
	if (auto const pWeapon = pThis->WeaponType)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		auto const& extraWarheads = pWeaponExt->ExtraWarheads;
		auto const& damageOverrides = pWeaponExt->ExtraWarheads_DamageOverrides;
		auto const& detonationChances = pWeaponExt->ExtraWarheads_DetonationChances;
		auto const& fullDetonation = pWeaponExt->ExtraWarheads_FullDetonation;
		const int defaultDamage = pWeapon->Damage;
		auto& random = ScenarioClass::Instance->Random;

		for (size_t i = 0; i < extraWarheads.size(); i++)
		{
			auto const pWH = extraWarheads[i];
			auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
			auto const pTarget = abstract_cast<TechnoClass*>(pThis->Target);

			if (pTarget && !pWHExt->IsHealthInThreshold(pTarget))
				continue;

			int damage = defaultDamage;
			size_t size = damageOverrides.size();

			if (size > i)
				damage = damageOverrides[i];
			else if (size > 0)
				damage = damageOverrides[size - 1];

			size = detonationChances.size();
			bool detonate = true;

			if (size > i)
				detonate = detonationChances[i] >= random.RandomDouble();
			else if (size > 0)
				detonate = detonationChances[size - 1] >= random.RandomDouble();

			size = fullDetonation.size();
			bool isFull = true;

			if (size > i)
				isFull = fullDetonation[i];
			else if (size > 0)
				isFull = fullDetonation[size - 1];

			if (!detonate)
				continue;

			if (isFull)
				WarheadTypeExt::DetonateAt(pWH, *coords, pTechno, damage, pOwner, pThis->Target);
			else
				pWHExt->DamageAreaWithTarget(*coords, damage, pTechno, pWH, true, pOwner, pTarget);
		}
	}

	// Return to sender
	if (pTechno)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

		if (auto const pWeapon = pTypeExt->ReturnWeapon)
		{
			int damage = pWeapon->Damage;

			if (pTypeExt->ReturnWeapon_ApplyFirepowerMult)
				damage = static_cast<int>(damage * TechnoExt::GetCurrentFirepowerMultiplier(pTechno));

			if (BulletClass* pBullet = pWeapon->Projectile->CreateBullet(pTechno, pTechno,
				damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
			{
				BulletExt::SimulatedFiringUnlimbo(pBullet, pOwner, pWeapon, pThis->Location, false);
				BulletExt::SimulatedFiringEffects(pBullet, pOwner, nullptr, false, true);
			}
		}
	}

	WarheadTypeExt::ExtMap.Find(pThis->WH)->InDamageArea = true;

	return 0;
}

#pragma region Airburst

static bool IsAllowedSplitsTarget(TechnoClass* pSource, HouseClass* pOwner, WeaponTypeClass* pWeapon, TechnoClass* pTarget, bool useWeaponTargeting)
{
	auto const pWH = pWeapon->Warhead;

	if (useWeaponTargeting)
	{
		auto const pType = pTarget->GetTechnoType();

		if (!pType->LegalTarget || GeneralUtils::GetWarheadVersusArmor(pWH, pTarget, pType) == 0.0)
			return false;

		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

		if (pWeaponExt->SkipWeaponPicking)
			return true;

		if (!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, pTarget->Owner)
			|| !EnumFunctions::IsCellEligible(pTarget->GetCell(), pWeaponExt->CanTarget, true, true)
			|| !EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget)
			|| !pWeaponExt->IsHealthInThreshold(pTarget))
		{
			return false;
		}

		if (!pWeaponExt->HasRequiredAttachedEffects(pTarget, pSource))
			return false;
	}
	else
	{
		if (!WarheadTypeExt::ExtMap.Find(pWH)->CanTargetHouse(pOwner, pTarget))
			return false;
	}

	return true;
}

// Disable Ares' Airburst implementation.
DEFINE_PATCH(0x469EBA, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90);

DEFINE_HOOK(0x468EB3, BulletClass_Explodes_AirburstCheck1, 0x6)
{
	enum { Continue = 0x468EC7, Skip = 0x468FF4 };

	GET(BulletClass*, pThis, ESI);

	auto const pType = pThis->Type;

	R->EAX(pType);
	return !(pType->Airburst || BulletTypeExt::ExtMap.Find(pType)->Splits) ? Continue : Skip;
}

DEFINE_HOOK(0x468FF4, BulletClass_Explodes_AirburstCheck2, 0x6)
{
	enum { Continue = 0x46909A, Skip = 0x469008 };

	GET(BulletClass*, pThis, ESI);

	auto const pType = pThis->Type;

	R->EAX(pType);
	return (pType->Airburst || BulletTypeExt::ExtMap.Find(pType)->Splits) ? Continue : Skip;
}

DEFINE_HOOK(0x469EC0, BulletClass_Logics_AirburstWeapon, 0x6)
{
	enum { SkipGameCode = 0x46A290 };

	GET(BulletClass*, pThis, ESI);

	auto const pType = pThis->Type;
	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pType);
	auto const pWeapon = pType->AirburstWeapon;

	if ((pType->Airburst || pTypeExt->Splits) && pWeapon)
	{
		auto const pSource = pThis->Owner;
		auto pOwner = pSource ? pSource->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;

		if (!pOwner || pOwner->Defeated)
		{
			if (const auto pNeutral = HouseClass::FindNeutral())
				pOwner = pNeutral;
			else
				return SkipGameCode;
		}

		auto& random = ScenarioClass::Instance->Random;
		int clusterCount = pType->Cluster;

		auto const coordsTarget = pTypeExt->AroundTarget.Get(pTypeExt->Splits) ? pThis->GetTargetCoords() : pThis->GetCoords();
		auto const cellTarget = CellClass::Coord2Cell(coordsTarget);
		DynamicVectorClass<AbstractClass*> targets;

		if (!pTypeExt->Splits)
		{
			CellRangeIterator<CellClass>{}(cellTarget, pTypeExt->AirburstSpread, [&targets](CellClass* pCell) -> bool
			{
				targets.AddItem(pCell);
				return true;
			});

			if (pTypeExt->Airburst_UseCluster)
			{
				DynamicVectorClass<AbstractClass*> newTargets;

				if (pTypeExt->Airburst_RandomClusters)
				{
					// Do random cells for amount matching Cluster.
					int count = 0;
					const int targetCount = targets.Count;

					while (count < clusterCount)
					{
						const int index = random.RandomRanged(0, targetCount);
						auto const pTarget = targets.GetItem(index);

						if (count > targetCount || newTargets.FindItemIndex(pTarget) < 0)
						{
							newTargets.AddItem(pTarget);
							count++;
						}
					}
				}
				else
				{
					// Do evenly selected cells for amount matching Cluster.
					const double stepSize = (targets.Count - 1.0) / (clusterCount - 1.0);

					for (int i = 0; i < clusterCount; i++)
					{
						newTargets.AddItem(targets.GetItem(static_cast<int>(round(stepSize * i))));
					}
				}

				targets = newTargets;
			}
			else
			{
				clusterCount = targets.Count;
			}
		}
		else
		{
			const double cellSpread = static_cast<double>(pTypeExt->Splits_TargetingDistance.Get()) / (double)Unsorted::LeptonsPerCell;
			const bool isAA = pType->AA;
			const bool retargetSelf = pTypeExt->RetargetSelf;
			const bool useWeaponTargeting = pTypeExt->Splits_UseWeaponTargeting;

			for (auto const pTechno : Helpers::Alex::getCellSpreadItems(coordsTarget, cellSpread, true))
			{
				if (pTechno->IsInPlayfield && pTechno->IsOnMap && pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo
					&& (retargetSelf || pTechno != pSource))
				{
					if ((isAA || !pTechno->IsInAir())
						&& IsAllowedSplitsTarget(pSource, pOwner, pWeapon, pTechno, useWeaponTargeting))
					{
						targets.AddItem(pTechno);
					}
				}
			}

			const int range = pTypeExt->Splits_TargetCellRange;

			while (targets.Count < clusterCount)
			{
				const int x = random.RandomRanged(-range, range);
				const int y = random.RandomRanged(-range, range);

				CellStruct cell = { static_cast<short>(cellTarget.X + x), static_cast<short>(cellTarget.Y + y) };
				auto const pCell = MapClass::Instance.GetCellAt(cell);

				targets.AddItem(pCell);
			}
		}

		auto const pTypeSplits = pWeapon->Projectile;
		int damage = pWeapon->Damage;

		if (pTypeExt->AirburstWeapon_ApplyFirepowerMult && pSource)
			damage = static_cast<int>(damage * TechnoExt::GetCurrentFirepowerMultiplier(pSource));

		// Cache all pointer variables before the loop
		auto const pWH = pWeapon->Warhead;
		auto const location = pThis->Location;
		bool const bright = pWeapon->Bright;
		bool const airburst = pType->Airburst;
		bool const splits = pTypeExt->Splits;
		bool const targetAsSource = pTypeExt->Airburst_TargetAsSource;
		bool const skipHeight = pTypeExt->Airburst_TargetAsSource_SkipHeight;
		double const retargetAccuracy = pTypeExt->RetargetAccuracy;
		int const speed = pWeapon->Speed;
		int const scatterMin = pTypeExt->AirburstWeapon_SourceScatterMin.Get();
		int const scatterMax = pTypeExt->AirburstWeapon_SourceScatterMax.Get();

		for (int i = 0; i < clusterCount; ++i)
		{
			auto pTarget = pThis->Target;

			if (!splits)
			{
				pTarget = targets.GetItem(i);
			}
			else if (!pTarget || retargetAccuracy < random.RandomDouble())
			{
				int index = random.RandomRanged(0, targets.Count - 1);
				pTarget = targets.GetItem(index);

				if (pTarget == pSource)
				{
					if (random.RandomDouble() > pTypeExt->RetargetSelf_Probability)
					{
						index = random.RandomRanged(0, targets.Count - 1);
						pTarget = targets.GetItem(index);
					}
				}

				targets.RemoveItem(index);
			}

			if (pTarget)
			{
				if (auto const pBullet = pTypeSplits->CreateBullet(pTarget, pSource, damage, pWH, speed, bright))
				{
					CoordStruct coords = location;
					const int minScatter = scatterMin;
					const int maxScatter = scatterMax;

					if (airburst && targetAsSource)
					{
						coords = pTarget->GetCoords();

						if (skipHeight)
							coords.Z = location.Z;
					}

					if (minScatter > 0 || maxScatter > 0)
					{
						const int distance = random.RandomRanged(scatterMin, scatterMax);
						coords = MapClass::GetRandomCoordsNear(coords, distance, false);
					}

					BulletExt::SimulatedFiringUnlimbo(pBullet, pOwner, pWeapon, coords, true);
					BulletExt::SimulatedFiringEffects(pBullet, pOwner, nullptr, false, true);
				}
			}
		}
	}

	return SkipGameCode;
}

#pragma endregion
