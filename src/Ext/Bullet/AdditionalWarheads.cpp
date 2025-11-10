#include "Body.h"

#include <AircraftTrackerClass.h>

#include <Ext/WarheadType/Body.h>

// A rectangular shape with a custom width from the current frame to the next frame in length.
std::vector<CellClass*> BulletExt::ExtData::GetCellsInProximityRadius()
{
	const auto pBullet = this->OwnerObject();
	const auto pTraj = this->Trajectory.get();

	// Seems like the y-axis is reversed, but it's okay.
	const auto walkCoord = pTraj ? Vector2D<double>{ pTraj->MovingVelocity.X, pTraj->MovingVelocity.Y } : Vector2D<double>{ pBullet->Velocity.X, pBullet->Velocity.Y };
	const double walkDistance = walkCoord.Magnitude();
	const auto radius = this->TypeExtData->ProximityRadius.Get();
	const auto thisCell = CellClass::Coord2Cell(pBullet->Location);

	// Special case of zero speed
	if (walkDistance <= BulletExt::Epsilon)
	{
		const double range = radius / static_cast<double>(Unsorted::LeptonsPerCell);
		std::vector<CellClass*> cirCellClass;
		const auto roundRange = static_cast<size_t>(range + 0.99);
		cirCellClass.reserve(roundRange * roundRange);

		for (CellSpreadEnumerator checkCell(roundRange); checkCell; ++checkCell)
		{
			if (const auto pCirCell = MapClass::Instance.TryGetCellAt(*checkCell + thisCell))
				cirCellClass.push_back(pCirCell);
		}

		return cirCellClass;
	}

	const double sideMult = radius / walkDistance;

	const CoordStruct cor1Coord { static_cast<int>(walkCoord.Y * sideMult), static_cast<int>((-walkCoord.X) * sideMult), 0 };
	const CoordStruct cor4Coord { static_cast<int>((-walkCoord.Y) * sideMult), static_cast<int>(walkCoord.X * sideMult), 0 };

	auto cor1Cell = CellClass::Coord2Cell(pBullet->Location + cor1Coord);
	auto cor4Cell = CellClass::Coord2Cell(pBullet->Location + cor4Coord);

	const auto off1Cell = cor1Cell - thisCell;
	const auto off4Cell = cor4Cell - thisCell;

	const double predictRatio = (walkDistance + radius) / walkDistance;
	const CoordStruct predictCoord { static_cast<int>(walkCoord.X * predictRatio), static_cast<int>(walkCoord.Y * predictRatio), 0 };
	const auto nextCell = CellClass::Coord2Cell(pBullet->Location + predictCoord);

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
	cornerIndex = (cornerIndex + 1) % 4;
	cor2Cell = corner[cornerIndex];
	cornerIndex = (cornerIndex + 1) % 4;
	cor3Cell = corner[cornerIndex];
	cornerIndex = (cornerIndex + 1) % 4;
	cor4Cell = corner[cornerIndex];

	// Obtain cells through vertices
	std::vector<CellStruct> recCells = BulletExt::GetCellsInRectangle(cor1Cell, cor4Cell, cor2Cell, cor3Cell);
	std::vector<CellClass*> recCellClass;
	recCellClass.reserve(recCells.size());

	for (const auto& pCells : recCells)
	{
		if (const auto pRecCell = MapClass::Instance.TryGetCellAt(pCells))
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
std::vector<CellStruct> BulletExt::GetCellsInRectangle(const CellStruct bottomStaCell, const CellStruct leftMidCell, const CellStruct rightMidCell, const CellStruct topEndCell)
{
	std::vector<CellStruct> recCells;
	const int cellNums = (std::abs(topEndCell.Y - bottomStaCell.Y) + 1) * (std::abs(rightMidCell.X - leftMidCell.X) + 1);
	recCells.reserve(cellNums);
	recCells.push_back(bottomStaCell);

	if (bottomStaCell == leftMidCell || bottomStaCell == rightMidCell) // A straight line
	{
		auto middleCurCell = bottomStaCell;

		const auto middleTheDist = topEndCell - bottomStaCell;
		const CellStruct middleTheUnit { static_cast<short>(Math::sgn(middleTheDist.X)), static_cast<short>(Math::sgn(middleTheDist.Y)) };
		const CellStruct middleThePace { static_cast<short>(middleTheDist.X * middleTheUnit.X), static_cast<short>(middleTheDist.Y * middleTheUnit.Y) };
		short mTheCurN = static_cast<short>((middleThePace.Y - middleThePace.X) / 2);

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
		short left1stCurN = static_cast<short>((left1stPace.Y - left1stPace.X) / 2);

		const auto left2ndDist = topEndCell - leftMidCell;
		const CellStruct left2ndUnit { static_cast<short>(Math::sgn(left2ndDist.X)), static_cast<short>(Math::sgn(left2ndDist.Y)) };
		const CellStruct left2ndPace { static_cast<short>(left2ndDist.X * left2ndUnit.X), static_cast<short>(left2ndDist.Y * left2ndUnit.Y) };
		short left2ndCurN = static_cast<short>((left2ndPace.Y - left2ndPace.X) / 2);

		const auto right1stDist = rightMidCell - bottomStaCell;
		const CellStruct right1stUnit { static_cast<short>(Math::sgn(right1stDist.X)), static_cast<short>(Math::sgn(right1stDist.Y)) };
		const CellStruct right1stPace { static_cast<short>(right1stDist.X * right1stUnit.X), static_cast<short>(right1stDist.Y * right1stUnit.Y) };
		short right1stCurN = static_cast<short>((right1stPace.Y - right1stPace.X) / 2);

		const auto right2ndDist = topEndCell - rightMidCell;
		const CellStruct right2ndUnit { static_cast<short>(Math::sgn(right2ndDist.X)), static_cast<short>(Math::sgn(right2ndDist.Y)) };
		const CellStruct right2ndPace { static_cast<short>(right2ndDist.X * right2ndUnit.X), static_cast<short>(right2ndDist.Y * right2ndUnit.Y) };
		short right2ndCurN = static_cast<short>((right2ndPace.Y - right2ndPace.X) / 2);

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

bool BulletExt::ExtData::CheckThroughAndSubjectInCell(CellClass* pCell, HouseClass* pOwner)
{
	const auto pTarget = this->OwnerObject()->Target;
	const auto pType = this->TypeExtData;

	for (auto pObject = pCell->GetContent(); pObject; pObject = pObject->NextObject)
	{
		const auto pTechno = abstract_cast<TechnoClass*, true>(pObject);

		// Non technos and not target friendly forces will be excluded
		if (!pTechno || (pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pTarget))
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

void BulletExt::ExtData::CalculateNewDamage()
{
	const auto pBullet = this->OwnerObject();
	const double ratio = this->TypeExtData->DamageCountAttenuation.Get();

	// Calculate the attenuation damage under three different scenarios
	if (ratio != 1.0)
	{
		// If the ratio is not 0, the lowest damage will be retained
		if (ratio)
		{
			BulletExt::SetNewDamage(pBullet->Health, ratio);
			BulletExt::SetNewDamage(this->ProximityDamage, ratio);
			BulletExt::SetNewDamage(this->PassDetonateDamage, ratio);
		}
		else
		{
			pBullet->Health = 0;
			this->ProximityDamage = 0;
			this->PassDetonateDamage = 0;
		}
	}
}

void BulletExt::ExtData::PassWithDetonateAt()
{
	if (!this->PassDetonateTimer.Completed())
		return;

	const auto pBullet = this->OwnerObject();
	const auto pType = this->TypeExtData;
	auto pWH = pType->PassDetonateWarhead.Get();

	if (!pWH)
		pWH = pBullet->WH;

	this->PassDetonateTimer.Start(pType->PassDetonateDelay);
	auto detonateCoords = pBullet->Location;

	// Whether to detonate at ground level?
	if (pType->PassDetonateLocal)
		detonateCoords.Z = MapClass::Instance.GetCellFloorHeight(detonateCoords);

	const auto pFirer = pBullet->Owner;
	const auto pOwner = pFirer ? pFirer->Owner : this->FirerHouse;
	const int damage = this->GetTrueDamage(this->PassDetonateDamage, false);
	WarheadTypeExt::DetonateAt(pWH, detonateCoords, pBullet->Owner, damage, pOwner);
	this->CalculateNewDamage();
}

template<bool sphere, bool checkBuilding>
static inline bool TargetInRange(TechnoClass* pTechno, BulletClass* pBullet, const CoordStruct& velocityCrd, const double& velocity, const Leptons radius)
{
	// For building use
	int distanceOffset = 0;
	bool isBuilding = false;

	if constexpr (checkBuilding)
	{
		isBuilding = pTechno->WhatAmI() == AbstractType::Building;

		if (isBuilding && static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame)
			return false;
	}

	// Check distance within the range of half capsule shape
	auto distanceCrd = pTechno->GetCoords() - pBullet->Location;

	auto getDotProduct = [](const CoordStruct& a, const CoordStruct& b) -> double
	{
		if constexpr (sphere)
			return a * b;
		else
			return static_cast<double>(a.X * b.X + a.Y * b.Y);
	};

	// Should be in front of the bullet's current position
	if (getDotProduct(distanceCrd, velocityCrd) < 0)
		return false;

	if constexpr (checkBuilding)
	{
		if (isBuilding)
		{
			// Building type have an extra bonus to distance (0x5F6403)
			const auto pBldType = static_cast<BuildingClass*>(pTechno)->Type;
			distanceOffset = 64 * (pBldType->GetFoundationHeight(false) + pBldType->GetFoundationWidth());
		}
	}

	auto getMagnitude = [&getDotProduct](const CoordStruct& a) -> double
	{
		if constexpr (!sphere)
			return std::hypot(a.X, a.Y);
		else
			return std::hypot(a.X, a.Y, a.Z);
	};
	auto getRadius = [radius, distanceOffset]() -> int
	{
		if constexpr (checkBuilding)
			return radius + distanceOffset;
		else
			return static_cast<int>(radius);
	};

	const auto nextDistanceCrd = distanceCrd - velocityCrd;

	// Should be behind the bullet's next frame position, otherwise, at least within the spherical range of future position
	if (getDotProduct(nextDistanceCrd, velocityCrd) > 0 && static_cast<int>(getMagnitude(nextDistanceCrd)) > getRadius())
		return false;

	// Calculate the distance between the point and the line
	auto getDistance = [&velocity, &distanceCrd, &nextDistanceCrd]()
	{
		if constexpr (sphere)
			return (velocity > BulletExt::Epsilon) ? (distanceCrd.CrossProduct(nextDistanceCrd).Magnitude() / velocity) : distanceCrd.Magnitude();
		else
			return (velocity > BulletExt::Epsilon) ? (std::abs(distanceCrd.X * nextDistanceCrd.Y - distanceCrd.Y * nextDistanceCrd.X) / velocity) : std::hypot(distanceCrd.X, distanceCrd.Y);
	};

	// Should be in the center cylinder
	return static_cast<int>(getDistance()) <= getRadius();
}

template<bool allies, bool sphere>
std::vector<TechnoClass*> BulletExt::ExtData::GetTargetsInProximityRadius(HouseClass* pOwner)
{
	const auto pType = this->TypeExtData;
	const auto pBullet = this->OwnerObject();
	const auto pTarget = pBullet->Target;
	const auto radius = pType->ProximityRadius.Get();
	auto pWH = pType->ProximityWarhead.Get();

	if (!pWH)
		pWH = pBullet->WH;

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	// Step 1: Find valid targets on the ground within range.
	std::vector<CellClass*> recCellClass = this->GetCellsInProximityRadius();
	std::vector<TechnoClass*> validTechnos;
	validTechnos.reserve(recCellClass.size() * 2);

	const auto pTraj = this->Trajectory.get();

	auto getVelocityCrd = [pTraj, pBullet]()
	{
		auto velocityCrd = BulletExt::Vector2Coord(pTraj ? pTraj->MovingVelocity : pBullet->Velocity);

		if constexpr (!sphere)
			velocityCrd.Z = 0;

		return velocityCrd;
	};
	const auto velocityCrd = getVelocityCrd();

	auto getVelocity = [pTraj, pBullet, &velocityCrd]()
	{
		if constexpr (sphere)
			return pTraj ? pTraj->MovingSpeed : pBullet->Velocity.Magnitude();
		else
			return BulletExt::Get2DDistance(velocityCrd);
	};
	const double velocity = getVelocity();

	auto checkTechno = [pOwner, pTarget, pWHExt](TechnoClass* pTechno) -> bool
	{
		if (BulletExt::CheckTechnoIsInvalid(pTechno))
			return false;

		if constexpr (!allies)
		{
			if (pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pTarget)
				return false;
		}

		return !pTechno->IsBeingWarpedOut() && pWHExt->IsHealthInThreshold(pTechno);
	};

	auto checkCellContent = [pBullet, radius, &velocityCrd, &velocity, &checkTechno, &validTechnos](ObjectClass* pFirstObject)
	{
		for (auto pObject = pFirstObject; pObject; pObject = pObject->NextObject)
		{
			if (const auto pTechno = abstract_cast<TechnoClass*, true>(pObject))
			{
				if (checkTechno(pTechno) && TargetInRange<sphere, true>(pTechno, pBullet, velocityCrd, velocity, radius))
					validTechnos.push_back(pTechno);
			}
		}
	};

	for (const auto& pRecCell : recCellClass)
	{
		checkCellContent(pRecCell->FirstObject);

		if (pRecCell->ContainsBridge())
			checkCellContent(pRecCell->AltObject);
	}

	// Step 2: Find valid targets in the air within range if necessary.
	if (pType->ProximityFlight)
	{
		const auto airTracker = &AircraftTrackerClass::Instance;
		airTracker->FillCurrentVector(MapClass::Instance.GetCellAt(pBullet->Location + velocityCrd * 0.5), Game::F2I((velocity / 2 + radius) / Unsorted::LeptonsPerCell));

		for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
		{
			if (checkTechno(pTechno) && TargetInRange<sphere, false>(pTechno, pBullet, velocityCrd, velocity, radius))
				validTechnos.push_back(pTechno);
		}
	}

	return validTechnos;
}

// Select suitable targets and choose the closer targets then attack each target only once.
void BulletExt::ExtData::PrepareForDetonateAt()
{
	const auto pType = this->TypeExtData;
	const auto pBullet = this->OwnerObject();
	const auto pFirer = pBullet->Owner;
	const auto pOwner = pFirer ? pFirer->Owner : this->FirerHouse;

	auto getTargets = [this, pType, pOwner]() -> std::vector<TechnoClass*>
	{
		if (pType->ProximityAllies)
			return pType->ProximitySphere ? this->GetTargetsInProximityRadius<true, true>(pOwner) : this->GetTargetsInProximityRadius<true, false>(pOwner);

		return pType->ProximitySphere ? this->GetTargetsInProximityRadius<false, true>(pOwner) : this->GetTargetsInProximityRadius<false, false>(pOwner);
	};

	std::vector<TechnoClass*> validTechnos = getTargets();

	// Step 3: Record each target without repetition.
	std::vector<int> casualtyChecked;
	casualtyChecked.reserve(Math::max(validTechnos.size(), this->Casualty.size()));

	// No impact on firer
	if (pFirer)
		this->Casualty[pFirer->UniqueID] = 5;

	// Update Record
	for (const auto& [ID, remainTime] : this->Casualty)
	{
		if (remainTime > 0)
			this->Casualty[ID] = remainTime - 1;
		else
			casualtyChecked.push_back(ID);
	}

	for (const auto& ID : casualtyChecked)
		this->Casualty.erase(ID);

	std::vector<TechnoClass*> validTargets;
	validTargets.reserve(validTechnos.size());

	// checking for duplicate
	for (const auto& pTechno : validTechnos)
	{
		if (!this->Casualty.contains(pTechno->UniqueID))
			validTargets.push_back(pTechno);

		// Record 5 frames
		this->Casualty[pTechno->UniqueID] = 5;
	}

	// Step 4: Detonate warheads in sequence based on distance.
	const auto targetsSize = validTargets.size();

	if (this->ProximityImpact > 0 && static_cast<int>(targetsSize) > this->ProximityImpact)
	{
		std::sort(&validTargets[0], &validTargets[targetsSize],[pBullet](TechnoClass* pTechnoA, TechnoClass* pTechnoB)
			{
				const double distanceA = pTechnoA->GetCoords().DistanceFromSquared(pBullet->SourceCoords);
				const double distanceB = pTechnoB->GetCoords().DistanceFromSquared(pBullet->SourceCoords);

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

void BulletExt::ExtData::ProximityDetonateAt(HouseClass* pOwner, TechnoClass* pTarget)
{
	const auto pBullet = this->OwnerObject();
	const auto pType = this->TypeExtData;
	int damage = this->GetTrueDamage(this->ProximityDamage, false);
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

int BulletExt::ExtData::GetTrueDamage(int damage, bool self)
{
	if (damage == 0)
		return 0;

	const auto pType = this->TypeExtData;

	// Calculate damage distance attenuation
	if (pType->DamageEdgeAttenuation != 1.0)
	{
		const double damageMultiplier = this->GetExtraDamageMultiplier();
		const double calculatedDamage = self ? damage * damageMultiplier : damage * this->FirepowerMult * damageMultiplier;
		const int signal = Math::sgn(calculatedDamage);
		damage = static_cast<int>(calculatedDamage);

		// Retain minimal damage
		if (!damage && pType->DamageEdgeAttenuation > 0.0)
			damage = signal;
	}
	else if (!self)
	{
		const double calculatedDamage = damage * this->FirepowerMult;
		const int signal = Math::sgn(calculatedDamage);
		damage = static_cast<int>(calculatedDamage);

		// Retain minimal damage
		if (!damage)
			damage = signal;
	}

	return damage;
}

double BulletExt::ExtData::GetExtraDamageMultiplier()
{
	const auto pBullet = this->OwnerObject();
	const double distance = pBullet->Location.DistanceFrom(pBullet->SourceCoords);

	// Directly use edge value if the distance is too far
	if (this->AttenuationRange <= static_cast<int>(distance))
		return this->TypeExtData->DamageEdgeAttenuation;

	// Remove the first cell distance for calculation
	const double calculateDistance = distance - static_cast<double>(Unsorted::LeptonsPerCell);

	// Directly use original value if the distance is too close
	if (calculateDistance <= 0.0)
		return 1.0;

	// this->AttenuationRange > distance > Unsorted::LeptonsPerCell -> deltaRange > 0
	const double deltaMult = this->TypeExtData->DamageEdgeAttenuation - 1.0;
	const int deltaRange = this->AttenuationRange - Unsorted::LeptonsPerCell;
	return 1.0 + deltaMult * (calculateDistance / deltaRange);
}
