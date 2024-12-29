#include "Body.h"
#include <functional>

#include <Ext/Anim/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>

#include <AircraftClass.h>
#include <TacticalClass.h>

DEFINE_HOOK(0x4690D4, BulletClass_Logics_ScreenShake, 0x6)
{
	enum { SkipShaking = 0x469130 };

	GET(WarheadTypeClass*, pWarhead, EAX);
	GET_BASE(CoordStruct*, pCoords, 0x8);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	auto&& [_, visible] = TacticalClass::Instance->CoordsToClient(*pCoords);

	if (pWHExt->ShakeIsLocal && !visible)
		return SkipShaking;

	return 0;
}

DEFINE_HOOK(0x4692BD, BulletClass_Logics_ApplyMindControl, 0x6)
{
	enum { SkipGameCode = 0x4692D5 };

	GET(BulletClass*, pThis, ESI);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto const pControlledAnimType = pWHExt->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);
	R->AL(CaptureManagerExt::CaptureUnit(pThis->Owner->CaptureManager, pThis->Target, pControlledAnimType));

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

	if (pWHExt->DetonateOnAllMapObjects && !pWHExt->WasDetonatedOnAllMapObjects &&
		pWHExt->DetonateOnAllMapObjects_AffectTargets != AffectedTarget::None &&
		pWHExt->DetonateOnAllMapObjects_AffectHouses != AffectedHouse::None)
	{
		pWHExt->WasDetonatedOnAllMapObjects = true;
		auto const originalLocation = pThis->Location;
		auto const pOriginalTarget = pThis->Target;
		auto const pExt = BulletExt::ExtMap.Find(pThis);
		auto pOwner = pThis->Owner ? pThis->Owner->Owner : pExt->FirerHouse;

		auto copy_dvc = []<typename T>(const DynamicVectorClass<T>&dvc)
		{
			std::vector<T> vec(dvc.Count);
			std::copy(dvc.begin(), dvc.end(), vec.begin());
			return vec;
		};

		std::function<void(TechnoClass*)> tryDetonate = [pThis, pWHExt, pOwner](TechnoClass* pTechno)
			{
				if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner))
				{
					pThis->Target = pTechno;
					pThis->Location = pTechno->GetCoords();
					pThis->Detonate(pTechno->GetCoords());
				}
			};

		if (!pWHExt->DetonateOnAllMapObjects_Full)
			tryDetonate = [pThis, pWHExt, pOwner](TechnoClass* pTechno)
			{
				if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner))
				{
					int damage = (pThis->Health * pThis->DamageMultiplier) >> 8;
					pWHExt->DamageAreaWithTarget(pTechno->GetCoords(), damage, pThis->Owner, pThis->WH, true, pOwner, pTechno);
				}
			};

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Aircraft) != AffectedTarget::None)
		{
			auto const aircraft = copy_dvc(*AircraftClass::Array);

			for (auto pAircraft : aircraft)
				tryDetonate(pAircraft);
		}

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Building) != AffectedTarget::None)
		{
			auto const buildings = copy_dvc(*BuildingClass::Array);

			for (auto pBuilding : buildings)
				tryDetonate(pBuilding);
		}

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Infantry) != AffectedTarget::None)
		{
			auto const infantry = copy_dvc(*InfantryClass::Array);

			for (auto pInf : infantry)
				tryDetonate(pInf);
		}

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Unit) != AffectedTarget::None)
		{
			auto const units = copy_dvc(*UnitClass::Array);

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

DEFINE_HOOK(0x469D1A, BulletClass_Logics_Debris_Checks, 0x6)
{
	enum { SkipGameCode = 0x469EBA, SetDebrisCount = 0x469D36 };

	GET(BulletClass*, pThis, ESI);

	bool isLand = pThis->GetCell()->LandType != LandType::Water;

	if (!isLand && WarheadTypeExt::ExtMap.Find(pThis->WH)->Debris_Conventional)
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

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto const debrisAnims = pWHExt->DebrisAnims.GetElements(RulesClass::Instance->MetallicDebris);

	if (debrisAnims.size() < 1)
		return SkipGameCode;

	while (debrisCount > 0)
	{
		int debrisIndex = ScenarioClass::Instance->Random.RandomRanged(0, debrisAnims.size() - 1);
		auto const pAnim = GameCreate<AnimClass>(debrisAnims[debrisIndex], pThis->GetCoords());

		if (pThis->Owner)
			pAnim->Owner = pThis->Owner->Owner;

		debrisCount--;
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
		int creationInterval = pWHExt->Splashed ? pWHExt->SplashList_CreationInterval : pWHExt->AnimList_CreationInterval;
		int* remainingInterval = &pWHExt->RemainingAnimCreationInterval;
		int scatterMin = pWHExt->Splashed ? pWHExt->SplashList_ScatterMin.Get() : pWHExt->AnimList_ScatterMin.Get();
		int scatterMax = pWHExt->Splashed ? pWHExt->SplashList_ScatterMax.Get() : pWHExt->AnimList_ScatterMax.Get();
		bool allowScatter = scatterMax != 0 || scatterMin != 0;

		if (creationInterval > 0 && pThis->Owner)
			remainingInterval = &TechnoExt::ExtMap.Find(pThis->Owner)->WHAnimRemainingCreationInterval;

		if (creationInterval < 1 || *remainingInterval <= 0)
		{
			*remainingInterval = creationInterval;

			HouseClass* pInvoker = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
			HouseClass* pVictim = nullptr;

			if (TechnoClass* Target = generic_cast<TechnoClass*>(pThis->Target))
				pVictim = Target->Owner;

			auto types = make_iterator_single(pAnimType);

			if (pWHExt->SplashList_CreateAll && pWHExt->Splashed)
			{
				types = pWHExt->SplashList.GetElements(RulesClass::Instance->SplashList);
			}
			else if (!pWHExt->Splashed)
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

			for (auto const& pType : types)
			{
				if (!pType)
					continue;

				auto animCoords = *coords;

				if (allowScatter)
				{
					int distance = ScenarioClass::Instance->Random.RandomRanged(scatterMin, scatterMax);
					animCoords = MapClass::GetRandomCoordsNear(animCoords, distance, false);
				}

				auto const pAnim = GameCreate<AnimClass>(pType, animCoords, 0, 1, 0x2600, -15, false);
				createdAnim = true;
				AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);

				if (!pAnim->Owner)
					pAnim->Owner = pInvoker;

				if (pThis->Owner)
				{
					auto pExt = AnimExt::ExtMap.Find(pAnim);
					pExt->SetInvoker(pThis->Owner);
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
	GET_BASE(CoordStruct*, coords, 0x8);

	auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;

	// Extra warheads
	if (pThis->WeaponType)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->WeaponType);
		int defaultDamage = pThis->WeaponType->Damage;

		for (size_t i = 0; i < pWeaponExt->ExtraWarheads.size(); i++)
		{
			auto const pWH = pWeaponExt->ExtraWarheads[i];
			int damage = defaultDamage;
			size_t size = pWeaponExt->ExtraWarheads_DamageOverrides.size();

			if (size > i)
				damage = pWeaponExt->ExtraWarheads_DamageOverrides[i];
			else if (size > 0)
				damage = pWeaponExt->ExtraWarheads_DamageOverrides[size - 1];

			bool detonate = true;
			size = pWeaponExt->ExtraWarheads_DetonationChances.size();

			if (size > i)
				detonate = pWeaponExt->ExtraWarheads_DetonationChances[i] >= ScenarioClass::Instance->Random.RandomDouble();
			if (size > 0)
				detonate = pWeaponExt->ExtraWarheads_DetonationChances[size - 1] >= ScenarioClass::Instance->Random.RandomDouble();

			bool isFull = true;
			size = pWeaponExt->ExtraWarheads_FullDetonation.size();

			if (size > i)
				isFull = pWeaponExt->ExtraWarheads_FullDetonation[i];
			if (size > 0)
				isFull = pWeaponExt->ExtraWarheads_FullDetonation[size - 1];

			if (detonate)
			{
				if (isFull)
				{
					WarheadTypeExt::DetonateAt(pWH, *coords, pThis->Owner, damage, pOwner, pThis->Target);
				}
				else
				{
					auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
					pWHExt->DamageAreaWithTarget(*coords, damage, pThis->Owner, pWH, true, pOwner, abstract_cast<TechnoClass*>(pThis->Target));
				}
			}
		}
	}

	// Return to sender
	if (pThis->Type && pThis->Owner)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

		if (auto const pWeapon = pTypeExt->ReturnWeapon)
		{
			if (BulletClass* pBullet = pWeapon->Projectile->CreateBullet(pThis->Owner, pThis->Owner,
				pWeapon->Damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
			{
				pBullet->WeaponType = pWeapon;
				pBullet->MoveTo(pThis->Location, BulletVelocity::Empty);
				BulletExt::ExtMap.Find(pBullet)->FirerHouse = pOwner;
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

		if (!pType->LegalTarget || GeneralUtils::GetWarheadVersusArmor(pWH, pType->Armor) == 0.0)
			return false;

		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

		if (!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, pTarget->Owner)
			|| !EnumFunctions::IsCellEligible(pTarget->GetCell(), pWeaponExt->CanTarget, true, true)
			|| !EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget))
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

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

	R->EAX(pThis->Type);
	return !(pThis->Type->Airburst || pTypeExt->Splits) ? Continue : Skip;
}

DEFINE_HOOK(0x468FF4, BulletClass_Explodes_AirburstCheck2, 0x6)
{
	enum { Continue = 0x46909A, Skip = 0x469008 };

	GET(BulletClass*, pThis, ESI);

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

	R->EAX(pThis->Type);
	return (pThis->Type->Airburst || pTypeExt->Splits) ? Continue : Skip;
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
		auto const pOwner = pSource ? pSource->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;

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
					int targetCount = targets.Count;

					while (count < clusterCount)
					{
						int index = ScenarioClass::Instance->Random.RandomRanged(0, targetCount);
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
					double stepSize = (targets.Count - 1.0) / (clusterCount - 1.0);

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
			for (auto const pTechno : *TechnoClass::Array)
			{
				if (pTechno->IsInPlayfield && pTechno->IsOnMap && pTechno->Health > 0 && (pTypeExt->RetargetSelf || pTechno != pThis->Owner))
				{
					auto const coords = pTechno->GetCoords();

					if (coordsTarget.DistanceFrom(coords) < pTypeExt->Splits_TargetingDistance.Get()
						&& (pType->AA || !pTechno->IsInAir())
						&& IsAllowedSplitsTarget(pSource, pOwner, pWeapon, pTechno, pTypeExt->Splits_UseWeaponTargeting))
					{
						targets.AddItem(pTechno);
					}
				}
			}

			int range = pTypeExt->Splits_TargetCellRange;

			while (targets.Count < clusterCount)
			{
				int x = random.RandomRanged(-range, range);
				int y = random.RandomRanged(-range, range);

				CellStruct cell = { static_cast<short>(cellTarget.X + x), static_cast<short>(cellTarget.Y + y) };
				auto const pCell = MapClass::Instance->GetCellAt(cell);

				targets.AddItem(pCell);
			}
		}

		int projectileRange = WeaponTypeExt::ExtMap.Find(pWeapon)->ProjectileRange.Get();
		auto const pTypeSplits = pWeapon->Projectile;
		int damage = pWeapon->Damage;

		if (pTypeExt->AirburstWeapon_ApplyFirepowerMult && pThis->Owner)
			damage = static_cast<int>(damage * pThis->Owner->FirepowerMultiplier * TechnoExt::ExtMap.Find(pThis->Owner)->AE.FirepowerMultiplier);

		for (int i = 0; i < clusterCount; ++i)
		{
			auto pTarget = pThis->Target;

			if (!pTypeExt->Splits)
			{
				pTarget = targets.GetItem(i);
			}
			else if (!pTarget || pTypeExt->RetargetAccuracy < random.RandomDouble())
			{
				int index = random.RandomRanged(0, targets.Count - 1);
				pTarget = targets.GetItem(index);

				if (pTarget == pThis->Owner)
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

				if (auto const pBullet = pTypeSplits->CreateBullet(pTarget, pThis->Owner, damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
				{
					BulletExt::ExtMap.Find(pBullet)->FirerHouse = pOwner;
					pBullet->WeaponType = pWeapon;
					pBullet->Range = projectileRange;

					DirStruct dir;
					dir.SetValue<5>(random.RandomRanged(0, 31));

					auto const radians = dir.GetRadian<32>();
					auto const sin_rad = Math::sin(radians);
					auto const cos_rad = Math::cos(radians);
					auto const cos_factor = -2.44921270764e-16; // cos(1.5 * Math::Pi * 1.00001)
					auto const flatSpeed = cos_factor * pBullet->Speed;

					BulletVelocity velocity;
					velocity.X = cos_rad * flatSpeed;
					velocity.Y = sin_rad * flatSpeed;
					velocity.Z = -pBullet->Speed;

					pBullet->MoveTo(pThis->Location, velocity);
				}
			}
		}
	}

	return SkipGameCode;
}

#pragma endregion
