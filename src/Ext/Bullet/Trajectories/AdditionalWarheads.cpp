#include "PhobosTrajectory.h"

#include <AircraftTrackerClass.h>

#include <Ext/WarheadType/Body.h>

// A rectangular shape with a custom width from the current frame to the next frame in length.
std::vector<CellClass*> PhobosTrajectory::GetCellsInProximityRadius()
{
	const auto pBullet = this->Bullet;
	// Seems like the y-axis is reversed, but it's okay.
	const CoordStruct walkCoord { static_cast<int>(this->MovingVelocity.X), static_cast<int>(this->MovingVelocity.Y), 0 };
	const auto sideMult = this->GetType()->ProximityRadius.Get() / walkCoord.Magnitude();

	const CoordStruct cor1Coord { static_cast<int>(walkCoord.Y * sideMult), static_cast<int>((-walkCoord.X) * sideMult), 0 };
	const CoordStruct cor4Coord { static_cast<int>((-walkCoord.Y) * sideMult), static_cast<int>(walkCoord.X * sideMult), 0 };
	const auto thisCell = CellClass::Coord2Cell(pBullet->Location);

	auto cor1Cell = CellClass::Coord2Cell((pBullet->Location + cor1Coord));
	auto cor4Cell = CellClass::Coord2Cell((pBullet->Location + cor4Coord));

	const auto off1Cell = cor1Cell - thisCell;
	const auto off4Cell = cor4Cell - thisCell;
	const auto nextCell = CellClass::Coord2Cell((pBullet->Location + walkCoord));

	auto cor2Cell = nextCell + off1Cell;
	auto cor3Cell = nextCell + off4Cell;
	// Arrange the vertices of the rectangle in order from bottom to top.
	int cornerIndex = 0;
	CellStruct corner[4] = { cor1Cell, cor2Cell, cor3Cell, cor4Cell };

	for (int i = 1; i < 4; ++i)
	{
		if (corner[cornerIndex].Y > corner[i].Y)
			cornerIndex = i;
	}

	cor1Cell = corner[cornerIndex];
	++cornerIndex %= 4;
	cor2Cell = corner[cornerIndex];
	++cornerIndex %= 4;
	cor3Cell = corner[cornerIndex];
	++cornerIndex %= 4;
	cor4Cell = corner[cornerIndex];

	std::vector<CellStruct> recCells = PhobosTrajectory::GetCellsInRectangle(cor1Cell, cor4Cell, cor2Cell, cor3Cell);
	std::vector<CellClass*> recCellClass;
	recCellClass.reserve(recCells.size());

	for (const auto& pCells : recCells)
	{
		if (CellClass* pRecCell = MapClass::Instance.TryGetCellAt(pCells))
			recCellClass.push_back(pRecCell);
	}

	return recCellClass;
}

/*!
	Can ONLY fill RECTANGLE.
	Record cells in the order of "draw left boundary, draw right boundary, fill middle, and move up one level".

	\param bottomStaCell Starting point vertex, located at the lowest point of the Y-axis.
	\param leftMidCell The vertex in the middle of the left path.
	\param rightMidCell The vertex in the middle of the right path.
	\param topEndCell The endpoint vertex, located at the highest point on the Y-axis.

	\returns A container that records all the cells inside, rounded outward.

	\author CrimRecya
*/
std::vector<CellStruct> PhobosTrajectory::GetCellsInRectangle(const CellStruct bottomStaCell, const CellStruct leftMidCell, const CellStruct rightMidCell, const CellStruct topEndCell)
{
	std::vector<CellStruct> recCells;
	const auto cellNums = (std::abs(topEndCell.Y - bottomStaCell.Y) + 1) * (std::abs(rightMidCell.X - leftMidCell.X) + 1);
	recCells.reserve(cellNums);
	recCells.push_back(bottomStaCell);

	if (bottomStaCell == leftMidCell || bottomStaCell == rightMidCell) // A straight line
	{
		auto middleCurCell = bottomStaCell;

		const auto middleTheDist = topEndCell - bottomStaCell;
		const CellStruct middleTheUnit { static_cast<short>(Math::sgn(middleTheDist.X)), static_cast<short>(Math::sgn(middleTheDist.Y)) };
		const CellStruct middleThePace { static_cast<short>(middleTheDist.X * middleTheUnit.X), static_cast<short>(middleTheDist.Y * middleTheUnit.Y) };
		auto mTheCurN = static_cast<float>((middleThePace.Y - middleThePace.X) / 2.0);

		while (middleCurCell != topEndCell)
		{
			if (mTheCurN > 0)
			{
				mTheCurN -= middleThePace.X;
				middleCurCell.Y += middleTheUnit.Y;
				recCells.push_back(middleCurCell);
			}
			else if (mTheCurN < 0)
			{
				mTheCurN += middleThePace.Y;
				middleCurCell.X += middleTheUnit.X;
				recCells.push_back(middleCurCell);
			}
			else
			{
				mTheCurN += middleThePace.Y - middleThePace.X;
				middleCurCell.X += middleTheUnit.X;
				recCells.push_back(middleCurCell);
				middleCurCell.X -= middleTheUnit.X;
				middleCurCell.Y += middleTheUnit.Y;
				recCells.push_back(middleCurCell);
				middleCurCell.X += middleTheUnit.X;
				recCells.push_back(middleCurCell);
			}
		}
	}
	else // Complete rectangle
	{
		auto leftCurCell = bottomStaCell;
		auto rightCurCell = bottomStaCell;
		auto middleCurCell = bottomStaCell;

		bool leftNext = false;
		bool rightNext = false;
		bool leftSkip = false;
		bool rightSkip = false;
		bool leftContinue = false;
		bool rightContinue = false;

		const auto left1stDist = leftMidCell - bottomStaCell;
		const CellStruct left1stUnit { static_cast<short>(Math::sgn(left1stDist.X)), static_cast<short>(Math::sgn(left1stDist.Y)) };
		const CellStruct left1stPace { static_cast<short>(left1stDist.X * left1stUnit.X), static_cast<short>(left1stDist.Y * left1stUnit.Y) };
		auto left1stCurN = static_cast<float>((left1stPace.Y - left1stPace.X) / 2.0);

		const auto left2ndDist = topEndCell - leftMidCell;
		const CellStruct left2ndUnit { static_cast<short>(Math::sgn(left2ndDist.X)), static_cast<short>(Math::sgn(left2ndDist.Y)) };
		const CellStruct left2ndPace { static_cast<short>(left2ndDist.X * left2ndUnit.X), static_cast<short>(left2ndDist.Y * left2ndUnit.Y) };
		auto left2ndCurN = static_cast<float>((left2ndPace.Y - left2ndPace.X) / 2.0);

		const auto right1stDist = rightMidCell - bottomStaCell;
		const CellStruct right1stUnit { static_cast<short>(Math::sgn(right1stDist.X)), static_cast<short>(Math::sgn(right1stDist.Y)) };
		const CellStruct right1stPace { static_cast<short>(right1stDist.X * right1stUnit.X), static_cast<short>(right1stDist.Y * right1stUnit.Y) };
		auto right1stCurN = static_cast<float>((right1stPace.Y - right1stPace.X) / 2.0);

		const auto right2ndDist = topEndCell - rightMidCell;
		const CellStruct right2ndUnit { static_cast<short>(Math::sgn(right2ndDist.X)), static_cast<short>(Math::sgn(right2ndDist.Y)) };
		const CellStruct right2ndPace { static_cast<short>(right2ndDist.X * right2ndUnit.X), static_cast<short>(right2ndDist.Y * right2ndUnit.Y) };
		auto right2ndCurN = static_cast<float>((right2ndPace.Y - right2ndPace.X) / 2.0);

		while (leftCurCell != topEndCell || rightCurCell != topEndCell)
		{
			while (leftCurCell != topEndCell) // Left
			{
				if (!leftNext) // Bottom Left Side
				{
					if (left1stCurN > 0)
					{
						left1stCurN -= left1stPace.X;
						leftCurCell.Y += left1stUnit.Y;

						if (leftCurCell == leftMidCell)
						{
							leftNext = true;
						}
						else
						{
							recCells.push_back(leftCurCell);
							break;
						}
					}
					else
					{
						left1stCurN += left1stPace.Y;
						leftCurCell.X += left1stUnit.X;

						if (leftCurCell == leftMidCell)
						{
							leftNext = true;
							leftSkip = true;
						}
					}
				}
				else // Top Left Side
				{
					if (left2ndCurN >= 0)
					{
						if (leftSkip)
						{
							leftSkip = false;
							left2ndCurN -= left2ndPace.X;
							leftCurCell.Y += left2ndUnit.Y;
						}
						else
						{
							leftContinue = true;
							break;
						}
					}
					else
					{
						left2ndCurN += left2ndPace.Y;
						leftCurCell.X += left2ndUnit.X;
					}
				}

				if (leftCurCell != rightCurCell) // Avoid double counting cells.
					recCells.push_back(leftCurCell);
			}

			while (rightCurCell != topEndCell) // Right
			{
				if (!rightNext) // Bottom Right Side
				{
					if (right1stCurN > 0)
					{
						right1stCurN -= right1stPace.X;
						rightCurCell.Y += right1stUnit.Y;

						if (rightCurCell == rightMidCell)
						{
							rightNext = true;
						}
						else
						{
							recCells.push_back(rightCurCell);
							break;
						}
					}
					else
					{
						right1stCurN += right1stPace.Y;
						rightCurCell.X += right1stUnit.X;

						if (rightCurCell == rightMidCell)
						{
							rightNext = true;
							rightSkip = true;
						}
					}
				}
				else // Top Right Side
				{
					if (right2ndCurN >= 0)
					{
						if (rightSkip)
						{
							rightSkip = false;
							right2ndCurN -= right2ndPace.X;
							rightCurCell.Y += right2ndUnit.Y;
						}
						else
						{
							rightContinue = true;
							break;
						}
					}
					else
					{
						right2ndCurN += right2ndPace.Y;
						rightCurCell.X += right2ndUnit.X;
					}
				}

				if (rightCurCell != leftCurCell) // Avoid double counting cells.
					recCells.push_back(rightCurCell);
			}

			middleCurCell = leftCurCell;
			middleCurCell.X += 1;

			while (middleCurCell.X < rightCurCell.X) // Center
			{
				recCells.push_back(middleCurCell);
				middleCurCell.X += 1;
			}

			if (leftContinue) // Continue Top Left Side
			{
				leftContinue = false;
				left2ndCurN -= left2ndPace.X;
				leftCurCell.Y += left2ndUnit.Y;
				recCells.push_back(leftCurCell);
			}

			if (rightContinue) // Continue Top Right Side
			{
				rightContinue = false;
				right2ndCurN -= right2ndPace.X;
				rightCurCell.Y += right2ndUnit.Y;
				recCells.push_back(rightCurCell);
			}
		}
	}

	return recCells;
}

bool PhobosTrajectory::CheckThroughAndSubjectInCell(CellClass* pCell, HouseClass* pOwner)
{
	const auto pBullet = this->Bullet;
	const auto pType = this->GetType();

	for (auto pObject = pCell->GetContent(); pObject; pObject = pObject->NextObject)
	{
		const auto pTechno = abstract_cast<TechnoClass*>(pObject);
		// Non technos and not target friendly forces will be excluded
		if (!pTechno || (pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pBullet->Target))
			continue;

		const auto absType = pTechno->WhatAmI();
		// Check building obstacles
		if (absType == AbstractType::Building)
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);

			if (pBuilding->Type->InvisibleInGame)
				continue;

			if (pBuilding->IsStrange() ? !pType->ThroughVehicles : !pType->ThroughBuilding)
			{
				this->ExtraCheck = pTechno;
				return true;
			}
		}
		// Check unit obstacles
		if (!pType->ThroughVehicles && (absType == AbstractType::Unit || absType == AbstractType::Aircraft))
		{
			this->ExtraCheck = pTechno;
			return true;
		}
	}

	return false;
}

void PhobosTrajectory::CalculateNewDamage()
{
	const auto pBullet = this->Bullet;
	const auto ratio = this->GetType()->DamageCountAttenuation.Get();
	// Calculate the attenuation damage under three different scenarios
	if (ratio != 1.0)
	{
		// If the ratio is not 0, the lowest damage will be retained
		if (ratio)
		{
			PhobosTrajectory::SetNewDamage(pBullet->Health, ratio);
			PhobosTrajectory::SetNewDamage(this->ProximityDamage, ratio);
			PhobosTrajectory::SetNewDamage(this->PassDetonateDamage, ratio);
		}
		else
		{
			pBullet->Health = 0;
			this->ProximityDamage = 0;
			this->PassDetonateDamage = 0;
		}
	}
}

void PhobosTrajectory::PassWithDetonateAt()
{
	if (!this->PassDetonateTimer.Completed())
		return;

	const auto pBullet = this->Bullet;
	const auto pType = this->GetType();
	auto pWH = pType->PassDetonateWarhead.Get();

	if (!pWH)
		pWH = pBullet->WH;

	this->PassDetonateTimer.Start(pType->PassDetonateDelay);
	auto detonateCoords = pBullet->Location;

	// Whether to detonate at ground level?
	if (pType->PassDetonateLocal)
		detonateCoords.Z = MapClass::Instance.GetCellFloorHeight(detonateCoords);

	const auto pFirer = pBullet->Owner;
	const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
	const auto damage = this->GetTheTrueDamage(this->PassDetonateDamage, false);
	WarheadTypeExt::DetonateAt(pWH, detonateCoords, pBullet->Owner, damage, pOwner);
	this->CalculateNewDamage();
}

// Select suitable targets and choose the closer targets then attack each target only once.
void PhobosTrajectory::PrepareForDetonateAt()
{
	const auto pType = this->GetType();
	const auto pBullet = this->Bullet;
	const auto pFirer = pBullet->Owner;
	const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
	const auto radius = pType->ProximityRadius.Get();
	// Step 1: Find valid targets on the ground within range.
	std::vector<CellClass*> recCellClass = this->GetCellsInProximityRadius();
	const size_t cellSize = recCellClass.size() * 2;
	size_t vectSize = cellSize;
	size_t thisSize = 0;

	const auto velocityCrd = PhobosTrajectory::Vector2Coord(this->MovingVelocity);
	const auto velocitySq = velocityCrd.MagnitudeSquared();
	const auto pTarget = pBullet->Target;

	std::vector<TechnoClass*> validTechnos;
	validTechnos.reserve(vectSize);

	for (const auto& pRecCell : recCellClass)
	{
		for (auto pObject = pRecCell->GetContent(); pObject; pObject = pObject->NextObject)
		{
			const auto pTechno = abstract_cast<TechnoClass*>(pObject);

			if (!pTechno || PhobosTrajectory::CheckTechnoIsInvalid(pTechno))
				continue;

			const auto absType = pTechno->WhatAmI();

			if (absType == AbstractType::Building && static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame)
				continue;
			// Not directly harming friendly forces
			if (!pType->ProximityAllies && pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pTarget)
				continue;
			// Check distance
			const auto targetCrd = pTechno->GetCoords();
			const auto distanceCrd = targetCrd - pBullet->Location;

			if (distanceCrd * velocityCrd < 0 && distanceCrd.Magnitude() > radius) // In front of the bullet
				continue;

			const auto nextDistanceCrd = distanceCrd - velocityCrd;

			if (nextDistanceCrd * velocityCrd > 0 && nextDistanceCrd.Magnitude() > radius) // Behind the next frame bullet
				continue;

			const auto cross = distanceCrd.CrossProduct(nextDistanceCrd).MagnitudeSquared();
			const auto distance = (velocitySq > 1e-10) ? sqrt(cross / velocitySq) : distanceCrd.Magnitude();

			if (absType != AbstractType::Building && distance > radius) // In the cylinder
				continue;

			if (thisSize >= vectSize)
			{
				vectSize += cellSize;
				validTechnos.reserve(vectSize);
			}

			validTechnos.push_back(pTechno);
			thisSize += 1;
		}
	}
	// Step 2: Find valid targets in the air within range if necessary.
	if (pType->ProximityFlight)
	{
		const auto airTracker = &AircraftTrackerClass::Instance;
		airTracker->FillCurrentVector(MapClass::Instance.GetCellAt(pBullet->Location + velocityCrd * 0.5),
			Game::F2I(sqrt(radius * radius + (velocitySq / 4)) / Unsorted::LeptonsPerCell));

		for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
		{
			if (PhobosTrajectory::CheckTechnoIsInvalid(pTechno))
				continue;
			// Not directly harming friendly forces
			if (!pType->ProximityAllies && pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pTarget)
				continue;
			// Check distance
			const auto targetCrd = pTechno->GetCoords();
			const auto distanceCrd = targetCrd - pBullet->Location;

			if (distanceCrd * velocityCrd < 0 && distanceCrd.Magnitude() > radius) // In front of the bullet
				continue;

			const auto nextDistanceCrd = distanceCrd - velocityCrd;

			if (nextDistanceCrd * velocityCrd > 0 && nextDistanceCrd.Magnitude() > radius) // Behind the next frame bullet
				continue;

			const auto cross = distanceCrd.CrossProduct(nextDistanceCrd).MagnitudeSquared();
			const auto distance = (velocitySq > 1e-10) ? sqrt(cross / velocitySq) : distanceCrd.Magnitude();

			if (distance > radius) // In the cylinder
				continue;

			if (thisSize >= vectSize)
			{
				vectSize += cellSize;
				validTechnos.reserve(vectSize);
			}

			validTechnos.push_back(pTechno);
			thisSize += 1;
		}
	}
	// Step 3: Record each target without repetition.
	std::vector<int> casualtyChecked;
	casualtyChecked.reserve(Math::max(validTechnos.size(), this->TheCasualty.size()));
	// No impact on firer
	if (pFirer)
		this->TheCasualty[pFirer->UniqueID] = 5;
	// Update Record
	for (const auto& [ID, remainTime] : this->TheCasualty)
	{
		if (remainTime > 0)
			this->TheCasualty[ID] = remainTime - 1;
		else
			casualtyChecked.push_back(ID);
	}

	for (const auto& ID : casualtyChecked)
		this->TheCasualty.erase(ID);

	std::vector<TechnoClass*> validTargets;
	// checking for duplicate
	for (const auto& pTechno : validTechnos)
	{
		if (!this->TheCasualty.contains(pTechno->UniqueID))
			validTargets.push_back(pTechno);
		// Record 5 frames
		this->TheCasualty[pTechno->UniqueID] = 5;
	}
	// Step 4: Detonate warheads in sequence based on distance.
	const auto targetsSize = validTargets.size();

	if (this->ProximityImpact > 0 && static_cast<int>(targetsSize) > this->ProximityImpact)
	{
		std::sort(&validTargets[0], &validTargets[targetsSize],[pBullet](TechnoClass* pTechnoA, TechnoClass* pTechnoB)
		{
			const auto distanceA = pTechnoA->GetCoords().DistanceFromSquared(pBullet->SourceCoords);
			const auto distanceB = pTechnoB->GetCoords().DistanceFromSquared(pBullet->SourceCoords);
			// Distance priority
			if (distanceA < distanceB)
				return true;

			if (distanceA > distanceB)
				return false;

			return pTechnoA->UniqueID < pTechnoB->UniqueID;
		});
	}

	for (const auto& pTechno : validTargets)
	{
		// Not effective for the technos following it.
		if (pTechno == this->ExtraCheck)
			break;
		// Last chance
		if (this->ProximityImpact == 1)
		{
			this->ExtraCheck = pTechno;
			break;
		}
		// Skip technos that are within range but will not obstruct and cannot be passed through
		const auto absType = pTechno->WhatAmI();

		if (!pType->ThroughVehicles && (absType == AbstractType::Unit || absType == AbstractType::Aircraft))
			continue;

		if (absType == AbstractType::Building && (static_cast<BuildingClass*>(pTechno)->IsStrange() ? !pType->ThroughVehicles : !pType->ThroughBuilding))
			continue;

		this->ProximityDetonateAt(pOwner, pTechno);
		// Record the number of times
		if (this->ProximityImpact > 0)
			--this->ProximityImpact;
	}
}

void PhobosTrajectory::ProximityDetonateAt(HouseClass* pOwner, TechnoClass* pTarget)
{
	const auto pBullet = this->Bullet;
	const auto pType = this->GetType();
	auto damage = this->GetTheTrueDamage(this->ProximityDamage, false);
	auto pWH = pType->ProximityWarhead.Get();

	if (!pWH)
		pWH = pBullet->WH;
	// Choose the method of causing damage
	if (pType->ProximityDirect)
		pTarget->ReceiveDamage(&damage, 0, pWH, pBullet->Owner, false, false, pOwner);
	else if (pType->ProximityMedial)
		WarheadTypeExt::DetonateAt(pWH, pBullet->Location, pBullet->Owner, damage, pOwner);
	else
		WarheadTypeExt::DetonateAt(pWH, pTarget, pBullet->Owner, damage, pOwner);

	this->CalculateNewDamage();
}

int PhobosTrajectory::GetTheTrueDamage(int damage, bool self)
{
	if (damage == 0)
		return 0;

	const auto pType = this->GetType();
	// Calculate damage distance attenuation
	if (pType->DamageEdgeAttenuation != 1.0)
	{
		const auto damageMultiplier = this->GetExtraDamageMultiplier();
		const auto calculatedDamage = self ? damage * damageMultiplier : damage * this->FirepowerMult * damageMultiplier;
		const auto signal = Math::sgn(calculatedDamage);
		damage = static_cast<int>(calculatedDamage);
		// Retain minimal damage
		if (!damage && pType->DamageEdgeAttenuation > 0.0)
			damage = signal;
	}

	return damage;
}

double PhobosTrajectory::GetExtraDamageMultiplier()
{
	const auto pBullet = this->Bullet;
	double damageMult = 1.0;
	const auto distance = pBullet->Location.DistanceFrom(pBullet->SourceCoords);

	if (this->AttenuationRange < static_cast<int>(distance))
		return this->GetType()->DamageEdgeAttenuation;
	// Remove the first cell distance for calculation
	if (distance > 256.0)
		damageMult += (this->GetType()->DamageEdgeAttenuation - 1.0) * ((distance - 256.0) / (static_cast<double>(this->AttenuationRange - 256)));

	return damageMult;
}
