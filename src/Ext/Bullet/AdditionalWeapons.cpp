#include "Body.h"

#include <AircraftTrackerClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Utilities/EnumFunctions.h>

void BulletExt::ExtData::GetTechnoFLHCoord()
{
	const auto pBullet = this->OwnerObject();
	const auto pTechno = pBullet->Owner;
	const auto pExt = TechnoExt::ExtMap.TryFind(pTechno);

	// Record the launch location, the building has an additional offset
	if (!pExt || !pExt->LastWeaponType || pExt->LastWeaponType->Projectile != pBullet->Type)
		this->NotMainWeapon = true;
	else
		this->FLHCoord = pExt->LastWeaponFLH;
}

CoordStruct BulletExt::ExtData::GetDisperseWeaponFireCoord(TechnoClass* pTechno)
{
	const auto pBullet = this->OwnerObject();
	const auto pType = this->TypeExtData;
	const auto pTraj = this->Trajectory.get();
	const auto flag = pTraj ? pTraj->Flag() : TrajectoryFlag::Invalid;

	// Fire from the original firer's position
	if (pType->DisperseFromFirer.Get(false))
	{
		// Find the outermost transporter
		pTechno = BulletExt::GetSurfaceFirer(pTechno);

		if (!this->NotMainWeapon && pTechno && !pTechno->InLimbo)
			return TechnoExt::GetFLHAbsoluteCoords(pTechno, this->FLHCoord, pTechno->HasTurret());

		return pBullet->SourceCoords;
	}

	// Fire from the bullet's position
	const auto& weaponCoord = pType->DisperseCoord.Get();

	if (weaponCoord == CoordStruct::Empty)
		return pBullet->Location;

	const double rotateRadian = Math::atan2(pBullet->TargetCoords.Y - pBullet->Location.Y , pBullet->TargetCoords.X - pBullet->Location.X);
	const auto fireOffsetCoord = BulletExt::Vector2Coord(BulletExt::HorizontalRotate(weaponCoord, rotateRadian));
	return pBullet->Location + fireOffsetCoord;
}

bool BulletExt::ExtData::PrepareDisperseWeapon()
{
	const auto pBullet = this->OwnerObject();
	const auto pType = this->TypeExtData;

	if (!this->DisperseCycle)
		return pType->DisperseSuicide;

	if (this->DisperseCount)
	{
		const auto pFirer = pBullet->Owner;
		auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

		// Replace with neutral house when the firer house does not exist
		if (!pOwner || pOwner->Defeated)
		{
			if (const auto pNeutral = HouseClass::FindNeutral())
				pOwner = pNeutral;
			else
				return true;
		}

		const auto fireCoord = this->GetDisperseWeaponFireCoord(pFirer);

		if (!this->FireDisperseWeapon(pFirer, fireCoord, pOwner))
			return false;
	}

	if (const int validDelays = pType->DisperseDelays.size())
	{
		const int delay = pType->DisperseDelays[(this->DisperseIndex < validDelays) ? this->DisperseIndex : (validDelays - 1)];
		this->DisperseTimer.Start((delay > 0) ? delay : 1);
	}

	// Record of Launch Times
	if (this->DisperseCount < 0 || --this->DisperseCount > 0)
		return false;

	const int groupSize = pType->DisperseSeparate ? pType->DisperseWeapons.size() : pType->DisperseCounts.size();

	// Next group
	if (++this->DisperseIndex < groupSize)
	{
		const int validCounts = pType->DisperseCounts.size();
		this->DisperseCount = validCounts > 0 ? pType->DisperseCounts[(this->DisperseIndex < validCounts) ? this->DisperseIndex : (validCounts - 1)] : 0;

		return false;
	}

	// Next cycle
	this->DisperseIndex = 0;
	this->DisperseCount = pType->DisperseCounts.empty() ? 0 : pType->DisperseCounts[0];

	if (this->DisperseCycle < 0 || --this->DisperseCycle > 0)
		return false;

	// Stop
	this->DisperseTimer.Stop();

	// Detonate if the number of attempts is exhausted at the end of the attack
	return pType->DisperseSuicide;
}

bool BulletExt::ExtData::FireDisperseWeapon(TechnoClass* pFirer, const CoordStruct& sourceCoord, HouseClass* pOwner)
{
	const auto pBullet = this->OwnerObject();
	const auto pType = this->TypeExtData;

	// Launch quantity check
	const int validWeapons = pType->DisperseWeapons.size();
	const int validBursts = pType->DisperseBursts.size();

	if (!validWeapons || !validBursts)
		return true;

	auto pTarget = pBullet->Target;

	if (!pType->DisperseForceFire)
	{
		if (!pTarget)
			return false;
	}

	// Set basic target
	if (!pTarget && !this->TargetIsInAir)
		pTarget = MapClass::Instance.TryGetCellAt(pBullet->TargetCoords);

	// Launch weapons in sequence
	for (int weaponNum = 0; weaponNum < validWeapons; ++weaponNum)
	{
		int curIndex = weaponNum;

		// Only launch one group
		if (pType->DisperseSeparate)
		{
			// Set the current weapon number
			curIndex = this->DisperseIndex;

			// End directly after firing this weapon
			weaponNum = validWeapons;
		}

		const auto pWeapon = pType->DisperseWeapons[curIndex];
		const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		const int burstCount = pType->DisperseBursts[(curIndex < validBursts) ? curIndex : (validBursts - 1)];

		if (burstCount <= 0)
			continue;

		// Only attack the bullet itself
		const auto pTraj = this->Trajectory.get();
		const auto flag = pTraj ? pTraj->Flag() : TrajectoryFlag::Invalid;

		if (pType->DisperseFromFirer.Get(false))
		{
			// Launch only when the firer exist
			if (pFirer)
			{
				for (int burstNum = 0; burstNum < burstCount; ++burstNum)
					this->CreateDisperseBullets(pFirer, sourceCoord, pWeapon, pBullet, pOwner, burstNum, burstCount);
			}

			continue;
		}

		// Only attack the original target
		if (!pType->DisperseRetarget)
		{
			// Launch only when the target exist
			if (pTarget)
			{
				for (int burstNum = 0; burstNum < burstCount; ++burstNum)
					this->CreateDisperseBullets(pFirer, sourceCoord, pWeapon, pTarget, pOwner, burstNum, burstCount);
			}

			continue;
		}

		int burstNow = 0;

		// Prioritize attacking the original target once
		if (pType->DisperseTendency && pTarget)
		{
			this->CreateDisperseBullets(pFirer, sourceCoord, pWeapon, pTarget, pOwner, burstNow, burstCount);
			++burstNow;

			if (burstCount <= 1)
				continue;
		}

		// Select new targets:
		// Where to select?
		const auto centerCoords = pType->DisperseLocation ? pBullet->Location : pBullet->TargetCoords;
		const auto centerCell = CellClass::Coord2Cell(centerCoords);

		std::vector<AbstractClass*> validTechnos;
		std::vector<AbstractClass*> validObjects;
		std::vector<AbstractClass*> validCells;

		// Select what?
		const bool checkTechnos = (pWeaponExt->CanTarget & AffectedTarget::AllContents) != AffectedTarget::None;
		const bool checkObjects = pType->DisperseMarginal;
		const bool checkCells = (pWeaponExt->CanTarget & AffectedTarget::AllCells) != AffectedTarget::None;

		const size_t initialSize = pWeapon->Range / Unsorted::LeptonsPerCell + 1;

		// The number of technos cannot be predicted, only estimated
		if (checkTechnos)
			validTechnos.reserve(initialSize * 2);

		// Object type has a small quantity, reduce the pre allocation quantity
		if (checkObjects)
			validObjects.reserve(initialSize);

		// The number of cells is square times the size
		if (checkCells)
			validCells.reserve(initialSize * initialSize);

		// How to select?
		if (pType->DisperseHolistic || !this->TargetIsInAir || checkCells) // On land targets
		{
			// Ensure that the same building is not recorded repeatedly
			std::set<TechnoClass*> inserted;
			const bool checkCellObjects = !this->TargetIsInAir || pType->DisperseHolistic;

			for (CellSpreadEnumerator thisCell(static_cast<size_t>((static_cast<double>(pWeapon->Range) / Unsorted::LeptonsPerCell) + 0.99)); thisCell; ++thisCell)
			{
				if (const auto pCell = MapClass::Instance.TryGetCellAt(*thisCell + centerCell))
				{
					if (checkCells && EnumFunctions::IsCellEligible(pCell, pWeaponExt->CanTarget, true, true))
						validCells.push_back(pCell);

					if (!checkCellObjects)
						continue;

					for (auto pObject = pCell->GetContent(); pObject; pObject = pObject->NextObject)
					{
						const auto pTechno = abstract_cast<TechnoClass*, true>(pObject);

						if (!pTechno)
						{
							if (checkObjects && (!pType->DisperseTendency || pType->DisperseDoRepeat || pObject != pTarget))
							{
								const auto pObjType = pObject->GetType();

								if (pObjType && !pObjType->Immune && centerCoords.DistanceFrom(pObject->GetCoords()) <= pWeapon->Range)
									validObjects.push_back(pObject);
							}
						}
						else if (checkTechnos && !BulletExt::CheckTechnoIsInvalid(pTechno))
						{
							const bool isBuilding = pTechno->WhatAmI() == AbstractType::Building;

							if ((!isBuilding || (!static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame && !inserted.contains(pTechno)))
								&& BulletExt::CheckCanDisperse(pTechno, pOwner, pType, centerCoords, pCell, pWeapon->Range, pTarget, pWeapon, pWeaponExt, pFirer))
							{
								validTechnos.push_back(pTechno);

								if (isBuilding)
									inserted.insert(pTechno);
							}
						}
					}
				}
			}
		}

		if (pType->DisperseHolistic || this->TargetIsInAir) // In air targets
		{
			const int range = pWeapon->Range + (pFirer ? pFirer->GetTechnoType()->AirRangeBonus : 0);

			if (checkTechnos)
			{
				const auto airTracker = &AircraftTrackerClass::Instance;
				airTracker->FillCurrentVector(MapClass::Instance.GetCellAt(centerCoords), Game::F2I(static_cast<double>(range) / Unsorted::LeptonsPerCell));

				for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
				{
					if (!BulletExt::CheckTechnoIsInvalid(pTechno)
						&& BulletExt::CheckCanDisperse(pTechno, pOwner, pType, centerCoords, pTechno->GetCell(), range, pTarget, pWeapon, pWeaponExt, pFirer))
					{
						validTechnos.push_back(pTechno);
					}
				}
			}

			if (checkObjects && BulletExt::ExtMap.Find(pBullet)->InterceptorTechnoType)
			{
				for (const auto& pObject : BulletClass::Array)
				{
					const auto pBulletExt = BulletExt::ExtMap.Find(pObject);
					const auto pBulletTypeExt = pBulletExt->TypeExtData;

					if (pBulletTypeExt->Interceptable
						&& !pObject->SpawnNextAnim
						&& centerCoords.DistanceFrom(pObject->Location) <= range
						&& (!pBulletTypeExt->Armor.isset() || GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pBulletTypeExt->Armor.Get()) != 0.0)
						&& EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, (pObject->Owner ? pObject->Owner->Owner : pBulletExt->FirerHouse))
						&& (!pType->DisperseTendency || pType->DisperseDoRepeat || pObject != pTarget))
					{
						validObjects.push_back(pObject);
					}
				}
			}
		}

		// Arrange the targets
		int burstRemain = burstCount - burstNow;
		std::vector<AbstractClass*> validTargets;
		validTargets.reserve(burstRemain);
		std::vector<AbstractClass*>* vectors[3] = { &validTechnos, &validObjects, &validCells };

		if (pType->DisperseDoRepeat) // Repeatedly attack new targets
		{
			for (const auto pVector : vectors)
			{
				if (pVector->empty())
					continue;

				const int size = pVector->size();
				const int base = burstRemain / size;
				const int remainder = burstRemain % size;

				if (remainder && size > 1) // Shuffle
				{
					for (int i = size - 1; i > 0; --i)
					{
						const int j = ScenarioClass::Instance->Random.RandomRanged(0, i);

						if (i != j)
							std::swap((*pVector)[i], (*pVector)[j]);
					}
				}

				// Fill in multiple items in order
				for (int i = 0; i < size; ++i)
				{
					int count = base + (i < remainder ? 1 : 0);

					for (int j = 0; j < count; ++j)
						validTargets.push_back((*pVector)[i]);
				}

				break;
			}
		}
		else // Missile attacks on all optional targets
		{
			for (const auto pVector : vectors)
			{
				if (burstRemain <= 0)
					break;

				if (pVector->empty())
					continue;

				const int size = pVector->size();
				const int take = Math::min(burstRemain, size);

				if (take != size && size > 1) // Shuffle
				{
					for (int i = size - 1; i > 0; --i)
					{
						const int j = ScenarioClass::Instance->Random.RandomRanged(0, i);

						if (i != j)
							std::swap((*pVector)[i], (*pVector)[j]);
					}
				}

				// Fill in all optional targets in order once
				validTargets.insert(validTargets.end(), pVector->begin(), pVector->begin() + take);
				burstRemain -= take;
			}
		}

		// When WeaponTendency=false, if no suitable target can be found, attempt to attack the original target once
		if (validTargets.empty() && pTarget && !burstNow && pType->DisperseForceFire)
			validTargets.push_back(pTarget);

		for (const auto& pNewTarget : validTargets)
		{
			this->CreateDisperseBullets(pFirer, sourceCoord, pWeapon, pNewTarget, pOwner, burstNow, burstCount);
			++burstNow;
		}
	}

	return true;
}

// Simulate the launch of weapons with burst.
void BulletExt::ExtData::CreateDisperseBullets(TechnoClass* pTechno, const CoordStruct& sourceCoord, WeaponTypeClass* pWeapon, AbstractClass* pTarget, HouseClass* pOwner, int curBurst, int maxBurst)
{
	const auto finalDamage = static_cast<int>(pWeapon->Damage * this->FirepowerMult);

	if (const auto pBullet = pWeapon->Projectile->CreateBullet(pTarget, pTechno, finalDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
	{
		const auto pExt = BulletExt::ExtMap.Find(pBullet);

		// Record basic information
		pExt->DispersedTrajectory = true;
		BulletExt::SimulatedFiringUnlimbo(pBullet, pOwner, pWeapon, sourceCoord, false);
		pExt->DispersedTrajectory = false;

		// Record additional content for trajectory
		pExt->FirepowerMult = this->FirepowerMult;
		pExt->NotMainWeapon = !pExt->TypeExtData->UseDisperseCoord || !pTechno || this->NotMainWeapon;

		if (!pExt->NotMainWeapon)
			pExt->FLHCoord = this->FLHCoord;

		// Calculate TargetCoords before drawing laser, ebolt, etc
		if (const auto pTraj = pExt->Trajectory.get())
		{
			pTraj->CurrentBurst = (this->Trajectory && this->Trajectory->CurrentBurst < 0) ? (-curBurst - 1) : curBurst;
			pTraj->CountOfBurst = maxBurst;
			pTraj->OpenFire();
		}

		// Simulate the actual weapon launch effect
		BulletExt::SimulatedFiringEffects(pBullet, pOwner, nullptr, true, true);

		if (pTarget->WhatAmI() == AbstractType::Bullet)
		{
			if (const auto pTypeExt = BulletExt::ExtMap.Find(this->OwnerObject())->InterceptorTechnoType)
			{
				pExt->InterceptorTechnoType = pTypeExt;
				pExt->InterceptedStatus |= InterceptedStatus::Targeted;

				if (!pTypeExt->InterceptorType->ApplyFirepowerMult)
					pBullet->Health = pWeapon->Damage;
			}
		}
	}
}
