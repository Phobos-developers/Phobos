#include "PhobosTrajectory.h"

#include <AircraftTrackerClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Utilities/EnumFunctions.h>

bool PhobosTrajectory::BulletRetargetTechno()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->GetType();
	bool check = false;
	// Will only attempt to search for a new target when the original target is a techno
	if (this->TargetIsTechno)
	{
		if (!pBullet->Target)
			check = true;
		else if (pBullet->Target->AbstractFlags & AbstractFlags::Techno)
			check = PhobosTrajectory::CheckTechnoIsInvalid(static_cast<TechnoClass*>(pBullet->Target));
		// Current target may be a bullet, and will not retarget at this time, in order to adapt to thermal decoys
	}
	// It has not lost its target
	if (!check)
		return false;
	// Check whether need to detonate directly after the target was lost
	if (!pType->TolerantTime || pType->RetargetRadius < 0)
		return true;
	// Check the timer
	if (this->RetargetTimer.HasTimeLeft())
		return false;
	// Next time wait for so long first
	this->RetargetTimer.Start(pType->RetargetInterval);
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
	// The central location and radius for searching for enemies
	const auto retargetCoords = this->GetRetargetCenter();
	const auto retargetRange = pType->RetargetRadius * Unsorted::LeptonsPerCell;
	TechnoClass* pNewTechno = nullptr;
	// Find the first target
	if (!this->TargetInTheAir) // Only get same type (on ground / in air)
	{
		const auto retargetCell = CellClass::Coord2Cell(retargetCoords);

		for (CellSpreadEnumerator thisCell(static_cast<size_t>(pType->RetargetRadius + 0.99)); thisCell; ++thisCell)
		{
			if (const auto pCell = MapClass::Instance.TryGetCellAt(*thisCell + retargetCell))
			{
				for (auto pObject = pCell->GetContent(); pObject; pObject = pObject->NextObject)
				{
					const auto pTechno = abstract_cast<TechnoClass*>(pObject);

					if (!pTechno || PhobosTrajectory::CheckTechnoIsInvalid(pTechno))
						continue;

					const auto pTechnoType = pTechno->GetTechnoType();

					if (!pTechnoType->LegalTarget)
						continue;
					else if (pTechno->WhatAmI() == AbstractType::Building && static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame)
						continue;
					else if (MapClass::GetTotalDamage(100, pBullet->WH, pTechnoType->Armor, 0) == 0)
						continue;
					else if (pTechno->GetCoords().DistanceFrom(retargetCoords) > retargetRange)
						continue;

					if (const auto pWeapon = pBullet->WeaponType)
					{
						if (pTechno->GetCoords().DistanceFrom(pFirer ? pFirer->GetCoords() : pBullet->SourceCoords) > pWeapon->Range)
							continue;

						const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

						if (!PhobosTrajectory::CheckWeaponCanTarget(pWeaponExt, pFirer, pTechno))
							continue;
						if (!PhobosTrajectory::CheckWeaponValidness(pOwner, pTechno, pCell, pWeaponExt->CanTargetHouses))
							continue;
					}

					pNewTechno = pTechno;
					break;
				}
			}

			if (pNewTechno)
				break;
		}
	}
	else
	{
		const auto airTracker = &AircraftTrackerClass::Instance;
		airTracker->FillCurrentVector(MapClass::Instance.GetCellAt(retargetCoords), Game::F2I(pType->RetargetRadius));

		for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
		{
			if (PhobosTrajectory::CheckTechnoIsInvalid(pTechno))
				continue;

			const auto pTechnoType = pTechno->GetTechnoType();

			if (!pTechnoType->LegalTarget)
				continue;
			else if (MapClass::GetTotalDamage(100, pBullet->WH, pTechnoType->Armor, 0) == 0)
				continue;
			else if (pTechno->GetCoords().DistanceFrom(retargetCoords) > retargetRange)
				continue;

			if (const auto pWeapon = pBullet->WeaponType)
			{
				if (pTechno->GetCoords().DistanceFrom(pFirer ? pFirer->GetCoords() : pBullet->SourceCoords) > pWeapon->Range)
					continue;

				const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

				if (!PhobosTrajectory::CheckWeaponCanTarget(pWeaponExt, pFirer, pTechno))
					continue;
				if (!PhobosTrajectory::CheckWeaponValidness(pOwner, pTechno, pTechno->GetCell(), pWeaponExt->CanTargetHouses))
					continue;
			}

			pNewTechno = pTechno;
			break;
		}
	}
	// Replace if there is a new target
	if (pNewTechno)
		this->SetBulletNewTarget(pNewTechno);
	// If not found, in order to minimize the response time, it will continue to check in the next frame, so the performance will be reduced a bit
	return false;
}

void PhobosTrajectory::GetTechnoFLHCoord()
{
	const auto pBullet = this->Bullet;
	const auto pTechno = pBullet->Owner;
	const auto pExt = TechnoExt::ExtMap.Find(pTechno);
	// Record the launch location, the building has an additional offset
	if (!pExt || !pExt->LastWeaponType || pExt->LastWeaponType->Projectile != pBullet->Type)
		this->NotMainWeapon = true;
	else
		this->FLHCoord = pExt->LastWeaponFLH;
}

CoordStruct PhobosTrajectory::GetWeaponFireCoord(TechnoClass* pTechno)
{
	const auto pBullet = this->Bullet;
	const auto pType = this->GetType();
	const auto flag = this->Flag();

	if (pType->DisperseFromFirer.Get(flag == TrajectoryFlag::Engrave || flag == TrajectoryFlag::Tracing))
	{
		// Find the outermost transporter
		pTechno = GetSurfaceFirer(pTechno);

		if (!this->NotMainWeapon && pTechno && !pTechno->InLimbo)
			return TechnoExt::GetFLHAbsoluteCoords(pTechno, this->FLHCoord, pTechno->HasTurret());

		return pBullet->SourceCoords;
	}

	const auto& weaponCoord = pType->DisperseCoord.Get();

	if (weaponCoord == CoordStruct::Empty)
		return pBullet->Location;

	const auto rotateRadian = Math::atan2(pBullet->TargetCoords.Y - pBullet->Location.Y , pBullet->TargetCoords.X - pBullet->Location.X);
	const auto fireOffsetCoord = PhobosTrajectory::Vector2Coord(PhobosTrajectory::HorizontalRotate(weaponCoord, rotateRadian));
	return pBullet->Location + fireOffsetCoord;
}

bool PhobosTrajectory::PrepareDisperseWeapon()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->GetType();

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

		const auto fireCoord = this->GetWeaponFireCoord(pFirer);

		if (!this->FireDisperseWeapon(pFirer, fireCoord, pOwner))
			return false;
	}

	if (const int validDelays = pType->DisperseDelays.size())
	{
		const auto delay = pType->DisperseDelays[(this->DisperseIndex < validDelays) ? this->DisperseIndex : (validDelays - 1)];
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

bool PhobosTrajectory::FireDisperseWeapon(TechnoClass* pFirer, const CoordStruct& sourceCoord, HouseClass* pOwner)
{
	const auto pBullet = this->Bullet;
	const auto pType = this->GetType();
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

		if (pType->Synchronize)
		{
			if (const auto pWeapon = pBullet->WeaponType)
			{
				const auto range = (pType->ApplyRangeModifiers && pFirer ? WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer, pWeapon->Range) : pWeapon->Range) + 32;
				const auto pSource = (pFirer && !this->NotMainWeapon) ? static_cast<ObjectClass*>(pFirer) : pBullet;
				const auto distance = (this->NotMainWeapon || this->TargetInTheAir || (pFirer && pFirer->IsInAir())) ? pSource->DistanceFrom(pTarget) : pSource->DistanceFrom3D(pTarget);

				if (distance >= range)
					return false;
			}
		}
	}
	// Set basic target
	if (!pTarget && !this->TargetInTheAir)
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
		const auto burstCount = pType->DisperseBursts[(curIndex < validBursts) ? curIndex : (validBursts - 1)];

		if (burstCount <= 0)
			continue;
		// Only attack the bullet itself
		const auto flag = this->Flag();

		if (pType->DisperseFromFirer.Get(flag == TrajectoryFlag::Engrave || flag == TrajectoryFlag::Tracing))
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

		const size_t initialSize = pWeapon->Range >> 7;

		if (checkTechnos)
			validTechnos.reserve(initialSize);

		if (checkObjects)
			validObjects.reserve(initialSize >> 1);

		if (checkCells)
			validCells.reserve(initialSize);
		// How to select?
		if (pType->DisperseHolistic || !this->TargetInTheAir || checkCells) // On land targets
		{
			// Ensure that the same building is not recorded repeatedly
			std::set<TechnoClass*> inserted;

			for (CellSpreadEnumerator thisCell(static_cast<size_t>((static_cast<double>(pWeapon->Range) / Unsorted::LeptonsPerCell) + 0.99)); thisCell; ++thisCell)
			{
				if (const auto pCell = MapClass::Instance.TryGetCellAt(*thisCell + centerCell))
				{
					if (checkCells && EnumFunctions::IsCellEligible(pCell, pWeaponExt->CanTarget, true, true))
						validCells.push_back(pCell);

					if (!pType->DisperseHolistic && this->TargetInTheAir)
						continue;

					for (auto pObject = pCell->GetContent(); pObject; pObject = pObject->NextObject)
					{
						const auto pTechno = abstract_cast<TechnoClass*>(pObject);

						if (!pTechno)
						{
							if (checkObjects && (!pType->DisperseTendency || pType->DisperseDoRepeat || pObject != pTarget))
							{
								const auto pObjType = pObject->GetType();

								if (pObjType && !pObjType->Immune && centerCoords.DistanceFrom(pObject->GetCoords()) <= pWeapon->Range)
									validObjects.push_back(pObject);
							}

							continue;
						}

						if (!checkTechnos || PhobosTrajectory::CheckTechnoIsInvalid(pTechno))
							continue;

						const auto pTechnoType = pTechno->GetTechnoType();

						if (!pTechnoType->LegalTarget)
							continue;
						else if (pType->DisperseTendency && !pType->DisperseDoRepeat && pTechno == pTarget)
							continue;

						const auto isBuilding = pTechno->WhatAmI() == AbstractType::Building;

						if (isBuilding && (static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame || inserted.contains(pTechno)))
							continue;
						else if (centerCoords.DistanceFrom(pTechno->GetCoords()) > pWeapon->Range)
							continue;
						else if (MapClass::GetTotalDamage(100, pWeapon->Warhead, pTechnoType->Armor, 0) == 0)
							continue;
						else if (!PhobosTrajectory::CheckWeaponCanTarget(pWeaponExt, pFirer, pTechno))
							continue;
						else if (!PhobosTrajectory::CheckWeaponValidness(pOwner, pTechno, pCell, pWeaponExt->CanTargetHouses))
							continue;

						validTechnos.push_back(pTechno);

						if (isBuilding)
							inserted.insert(pTechno);
					}
				}
			}
		}

		if (pType->DisperseHolistic || this->TargetInTheAir) // In air targets
		{
			if (checkTechnos)
			{
				const auto airTracker = &AircraftTrackerClass::Instance;
				airTracker->FillCurrentVector(MapClass::Instance.GetCellAt(centerCoords), Game::F2I(static_cast<double>(pWeapon->Range) / Unsorted::LeptonsPerCell));

				for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
				{
					if (PhobosTrajectory::CheckTechnoIsInvalid(pTechno))
						continue;

					const auto pTechnoType = pTechno->GetTechnoType();

					if (!pTechnoType->LegalTarget)
						continue;
					else if (pType->DisperseTendency && !pType->DisperseDoRepeat && pTechno == pTarget)
						continue;
					else if (centerCoords.DistanceFrom(pTechno->GetCoords()) > pWeapon->Range)
						continue;
					else if (MapClass::GetTotalDamage(100, pWeapon->Warhead, pTechnoType->Armor, 0) == 0)
						continue;
					else if (!PhobosTrajectory::CheckWeaponCanTarget(pWeaponExt, pFirer, pTechno))
						continue;
					else if (!PhobosTrajectory::CheckWeaponValidness(pOwner, pTechno, pTechno->GetCell(), pWeaponExt->CanTargetHouses))
						continue;

					validTechnos.push_back(pTechno);
				}
			}

			if (checkObjects)
			{
				for (auto const& pObject : BulletClass::Array)
				{
					auto const pBulletExt = BulletExt::ExtMap.Find(pObject);
					auto const pBulletTypeExt = pBulletExt->TypeExtData;

					if (!pBulletTypeExt || !pBulletTypeExt->Interceptable)
						continue;
					else if (centerCoords.DistanceFrom(centerCoords) > pWeapon->Range)
						continue;
					else if (pBulletTypeExt->Armor.isset() && GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pBulletTypeExt->Armor.Get()) == 0.0)
						continue;
					else if (!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, (pObject->Owner ? pObject->Owner->Owner : pBulletExt->FirerHouse)))
						continue;
					else if (pType->DisperseTendency && !pType->DisperseDoRepeat && pObject == pTarget)
						continue;

					validObjects.push_back(pObject);
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
		if (validTargets.empty() && pTarget && !burstNow)
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
void PhobosTrajectory::CreateDisperseBullets(TechnoClass* pTechno, const CoordStruct& sourceCoord, WeaponTypeClass* pWeapon, AbstractClass* pTarget, HouseClass* pOwner, int curBurst, int maxBurst)
{
	const auto finalDamage = static_cast<int>(pWeapon->Damage * this->FirepowerMult);

	if (const auto pBullet = pWeapon->Projectile->CreateBullet(pTarget, pTechno, finalDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
	{
		const auto pExt = BulletExt::ExtMap.Find(pBullet);

		if (pExt->TypeExtData->TrajectoryType)
			pExt->DispersedTrajectory = true;
		// Record basic information
		BulletExt::SimulatedFiringUnlimbo(pBullet, pOwner, pWeapon, sourceCoord, false);
		// Record additional content for trajectory
		if (const auto pTraj = pExt->Trajectory.get())
		{
			pTraj->CurrentBurst = (this->CurrentBurst < 0) ? (-curBurst - 1) : curBurst;
			pTraj->CountOfBurst = maxBurst;
			pTraj->FirepowerMult = this->FirepowerMult;
			pTraj->NotMainWeapon = !pTraj->GetType()->UseDisperseCoord || !pTechno || this->NotMainWeapon;

			if (!pTraj->NotMainWeapon)
				pTraj->FLHCoord = this->FLHCoord;

			pExt->DispersedTrajectory = false;
			// Calculate TargetCoords before drawing laser, ebolt, etc
			pTraj->OpenFire();
		}
		// Simulate the actual weapon launch effect
		BulletExt::SimulatedFiringEffects(pBullet, pOwner, nullptr, true, true);
	}
}
