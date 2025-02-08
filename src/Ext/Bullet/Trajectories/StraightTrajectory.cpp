#include "StraightTrajectory.h"

#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <AircraftTrackerClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Techno/Body.h>

std::unique_ptr<PhobosTrajectory> StraightTrajectoryType::CreateInstance() const
{
	return std::make_unique<StraightTrajectory>(this);
}

template<typename T>
void StraightTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
		.Process(this->ApplyRangeModifiers)
		.Process(this->PassThrough)
		.Process(this->PassDetonate)
		.Process(this->PassDetonateWarhead)
		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateDelay)
		.Process(this->PassDetonateInitialDelay)
		.Process(this->PassDetonateLocal)
		.Process(this->LeadTimeCalculate)
		.Process(this->OffsetCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
		.Process(this->ProximityImpact)
		.Process(this->ProximityWarhead)
		.Process(this->ProximityDamage)
		.Process(this->ProximityRadius)
		.Process(this->ProximityDirect)
		.Process(this->ProximityMedial)
		.Process(this->ProximityAllies)
		.Process(this->ProximityFlight)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->SubjectToGround)
		.Process(this->ConfineAtHeight)
		.Process(this->EdgeAttenuation)
		.Process(this->CountAttenuation)
		;
}

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool StraightTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<StraightTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void StraightTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->ApplyRangeModifiers.Read(exINI, pSection, "Trajectory.Straight.ApplyRangeModifiers");
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Straight.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Straight.TargetSnapDistance");
	this->PassThrough.Read(exINI, pSection, "Trajectory.Straight.PassThrough");
	this->PassDetonate.Read(exINI, pSection, "Trajectory.Straight.PassDetonate");
	this->PassDetonateWarhead.Read<true>(exINI, pSection, "Trajectory.Straight.PassDetonateWarhead");
	this->PassDetonateDamage.Read(exINI, pSection, "Trajectory.Straight.PassDetonateDamage");
	this->PassDetonateDelay.Read(exINI, pSection, "Trajectory.Straight.PassDetonateDelay");
	this->PassDetonateInitialDelay.Read(exINI, pSection, "Trajectory.Straight.PassDetonateInitialDelay");
	this->PassDetonateLocal.Read(exINI, pSection, "Trajectory.Straight.PassDetonateLocal");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.Straight.LeadTimeCalculate");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.Straight.OffsetCoord");
	this->RotateCoord.Read(exINI, pSection, "Trajectory.Straight.RotateCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Straight.MirrorCoord");
	this->UseDisperseBurst.Read(exINI, pSection, "Trajectory.Straight.UseDisperseBurst");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.Straight.AxisOfRotation");
	this->ProximityImpact.Read(exINI, pSection, "Trajectory.Straight.ProximityImpact");
	this->ProximityWarhead.Read<true>(exINI, pSection, "Trajectory.Straight.ProximityWarhead");
	this->ProximityDamage.Read(exINI, pSection, "Trajectory.Straight.ProximityDamage");
	this->ProximityRadius.Read(exINI, pSection, "Trajectory.Straight.ProximityRadius");
	this->ProximityDirect.Read(exINI, pSection, "Trajectory.Straight.ProximityDirect");
	this->ProximityMedial.Read(exINI, pSection, "Trajectory.Straight.ProximityMedial");
	this->ProximityAllies.Read(exINI, pSection, "Trajectory.Straight.ProximityAllies");
	this->ProximityFlight.Read(exINI, pSection, "Trajectory.Straight.ProximityFlight");
	this->ThroughVehicles.Read(exINI, pSection, "Trajectory.Straight.ThroughVehicles");
	this->ThroughBuilding.Read(exINI, pSection, "Trajectory.Straight.ThroughBuilding");
	this->SubjectToGround.Read(exINI, pSection, "Trajectory.Straight.SubjectToGround");
	this->ConfineAtHeight.Read(exINI, pSection, "Trajectory.Straight.ConfineAtHeight");
	this->EdgeAttenuation.Read(exINI, pSection, "Trajectory.Straight.EdgeAttenuation");
	this->EdgeAttenuation = Math::max(0.0, this->EdgeAttenuation);
	this->CountAttenuation.Read(exINI, pSection, "Trajectory.Straight.CountAttenuation");
	this->CountAttenuation = Math::max(0.0, this->CountAttenuation);
}

template<typename T>
void StraightTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->DetonationDistance)
		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateTimer)
		.Process(this->OffsetCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->ProximityImpact)
		.Process(this->ProximityDamage)
		.Process(this->RemainingDistance)
		.Process(this->ExtraCheck)
		.Process(this->TheCasualty)
		.Process(this->FirepowerMult)
		.Process(this->AttenuationRange)
		.Process(this->LastTargetCoord)
		.Process(this->CurrentBurst)
		.Process(this->CountOfBurst)
		.Process(this->WaitOneFrame)
		;
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Serialize(Stm);
	return true;
}

bool StraightTrajectory::Save(PhobosStreamWriter& Stm) const
{
	const_cast<StraightTrajectory*>(this)->Serialize(Stm);
	return true;
}

void StraightTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	const auto pType = this->Type;
	this->PassDetonateTimer.Start(pType->PassDetonateInitialDelay > 0 ? pType->PassDetonateInitialDelay : 0);
	this->LastTargetCoord = pBullet->TargetCoords;
	pBullet->Velocity = BulletVelocity::Empty;
	const auto pFirer = pBullet->Owner;

	// Determine the range of the bullet
	if (const auto pWeapon = pBullet->WeaponType)
	{
		this->AttenuationRange = pWeapon->Range;
		this->CountOfBurst = pWeapon->Burst;

		if (pType->ApplyRangeModifiers && pFirer)
		{
			if (this->DetonationDistance >= 0)
				this->DetonationDistance = Leptons(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer, this->DetonationDistance));
			else
				this->DetonationDistance = Leptons(-WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer, -this->DetonationDistance));

			this->AttenuationRange = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer);
		}
	}

	// Record some information of the attacker
	if (pFirer)
	{
		this->CurrentBurst = pFirer->CurrentBurstIndex;
		this->FirepowerMult = pFirer->FirepowerMultiplier;

		if (const auto pExt = TechnoExt::ExtMap.Find(pFirer))
			this->FirepowerMult *= pExt->AE.FirepowerMultiplier;

		if (pType->MirrorCoord && pFirer->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	// Wait, or launch immediately?
	if (!pType->LeadTimeCalculate || !abstract_cast<FootClass*>(pBullet->Target))
		this->PrepareForOpenFire(pBullet);
	else
		this->WaitOneFrame = 2;
}

bool StraightTrajectory::OnAI(BulletClass* pBullet)
{
	if (this->WaitOneFrame && this->BulletPrepareCheck(pBullet))
		return false;

	const auto pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
	const auto pType = this->Type;

	if (this->BulletDetonatePreCheck(pBullet))
		return true;

	this->BulletDetonateVelocityCheck(pBullet, pOwner);

	if (pType->PassDetonate)
		this->PassWithDetonateAt(pBullet, pOwner);

	if (this->ProximityImpact != 0 && pType->ProximityRadius.Get() > 0)
		this->PrepareForDetonateAt(pBullet, pOwner);

	if (pType->Trajectory_Speed < 256.0 && pType->ConfineAtHeight > 0 && this->PassAndConfineAtHeight(pBullet))
		return true;

	this->BulletDetonateLastCheck(pBullet, pOwner);

	return false;
}

void StraightTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	const auto pType = this->Type;

	// Calculate the current damage
	const auto pTechno = abstract_cast<TechnoClass*>(pBullet->Target);
	pBullet->Health = this->GetTheTrueDamage(pBullet->Health, pBullet, pTechno, true);

	// Whether to detonate at ground level?
	if (pType->PassDetonateLocal)
	{
		CoordStruct detonateCoords = pBullet->Location;
		detonateCoords.Z = MapClass::Instance->GetCellFloorHeight(detonateCoords);
		pBullet->SetLocation(detonateCoords);
	}

	// Can snap to target?
	const auto targetSnapDistance = pType->TargetSnapDistance.Get();

	if (pType->PassThrough || targetSnapDistance <= 0)
		return;

	const auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	const auto coords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	// Whether to snap to target?
	if (coords.DistanceFrom(pBullet->Location) <= targetSnapDistance)
	{
		const auto pExt = BulletExt::ExtMap.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(coords);
	}
}

void StraightTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType StraightTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // No longer needed.
}

TrajectoryCheckReturnType StraightTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

void StraightTrajectory::PrepareForOpenFire(BulletClass* pBullet)
{
	const auto pType = this->Type;
	double rotateAngle = 0.0;
	const auto pTarget = pBullet->Target;
	auto theTargetCoords = pBullet->TargetCoords;
	auto theSourceCoords = pBullet->SourceCoords;

	// TODO If I could calculate this before firing, perhaps it can solve the problem of one frame delay and not so correct turret orientation.
	if (pType->LeadTimeCalculate && pTarget)
	{
		theTargetCoords = pTarget->GetCoords();
		theSourceCoords = pBullet->Location;

		// Solving trigonometric functions
		if (theTargetCoords != this->LastTargetCoord)
		{
			const auto extraOffsetCoord = theTargetCoords - this->LastTargetCoord;
			const auto targetSourceCoord = theSourceCoords - theTargetCoords;
			const auto lastSourceCoord = theSourceCoords - this->LastTargetCoord;

			const auto theDistanceSquared = targetSourceCoord.MagnitudeSquared();
			const auto targetSpeedSquared = extraOffsetCoord.MagnitudeSquared();
			const auto targetSpeed = sqrt(targetSpeedSquared);

			const auto crossFactor = lastSourceCoord.CrossProduct(targetSourceCoord).MagnitudeSquared();
			const auto verticalDistanceSquared = crossFactor / targetSpeedSquared;

			const auto horizonDistanceSquared = theDistanceSquared - verticalDistanceSquared;
			const auto horizonDistance = sqrt(horizonDistanceSquared);

			const auto straightSpeedSquared = pType->Trajectory_Speed * pType->Trajectory_Speed;
			const auto baseFactor = straightSpeedSquared - targetSpeedSquared;
			const auto squareFactor = baseFactor * verticalDistanceSquared + straightSpeedSquared * horizonDistanceSquared;

			// Is there a solution?
			if (squareFactor > 1e-10)
			{
				const auto minusFactor = -(horizonDistance * targetSpeed);
				int travelTime = 0;

				if (std::abs(baseFactor) < 1e-10)
				{
					travelTime = std::abs(horizonDistance) > 1e-10 ? (static_cast<int>(theDistanceSquared / (2 * horizonDistance * targetSpeed)) + 1) : 0;
				}
				else
				{
					const auto travelTimeM = static_cast<int>((minusFactor - sqrt(squareFactor)) / baseFactor);
					const auto travelTimeP = static_cast<int>((minusFactor + sqrt(squareFactor)) / baseFactor);

					if (travelTimeM > 0 && travelTimeP > 0)
						travelTime = travelTimeM < travelTimeP ? travelTimeM : travelTimeP;
					else if (travelTimeM > 0)
						travelTime = travelTimeM;
					else if (travelTimeP > 0)
						travelTime = travelTimeP;

					if (targetSourceCoord.MagnitudeSquared() < lastSourceCoord.MagnitudeSquared())
						travelTime += 1;
					else
						travelTime += 2;
				}

				theTargetCoords += extraOffsetCoord * travelTime;
			}
		}
	}

	// Calculate the orientation of the coordinate system
	if (!pType->LeadTimeCalculate && theTargetCoords == theSourceCoords && pBullet->Owner) // For disperse.
	{
		const auto theOwnerCoords = pBullet->Owner->GetCoords();
		rotateAngle = Math::atan2(theTargetCoords.Y - theOwnerCoords.Y , theTargetCoords.X - theOwnerCoords.X);
	}
	else
	{
		rotateAngle = Math::atan2(theTargetCoords.Y - theSourceCoords.Y , theTargetCoords.X - theSourceCoords.X);
	}

	// Add the fixed offset value
	if (this->OffsetCoord != CoordStruct::Empty)
	{
		theTargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(rotateAngle) + this->OffsetCoord.Y * Math::sin(rotateAngle));
		theTargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(rotateAngle) - this->OffsetCoord.Y * Math::cos(rotateAngle));
		theTargetCoords.Z += this->OffsetCoord.Z;
	}

	// Add random offset value
	if (pBullet->Type->Inaccurate)
	{
		const auto pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		const auto offsetMult = 0.0004 * theSourceCoords.DistanceFrom(theTargetCoords);
		const auto offsetMin = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Min.Get(Leptons(0)));
		const auto offsetMax = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter)));
		const auto offsetDistance = ScenarioClass::Instance->Random.RandomRanged(offsetMin, offsetMax);
		theTargetCoords = MapClass::GetRandomCoordsNear(theTargetCoords, offsetDistance, false);
	}

	// Determine the distance that the bullet can travel
	if (pType->PassThrough)
	{
		if (this->DetonationDistance > 0)
			this->RemainingDistance += static_cast<int>(this->DetonationDistance + pType->Trajectory_Speed);
		else if (this->DetonationDistance < 0)
			this->RemainingDistance += static_cast<int>(theSourceCoords.DistanceFrom(theTargetCoords) - this->DetonationDistance + pType->Trajectory_Speed);
		else
			this->RemainingDistance = INT_MAX;
	}
	else
	{
		this->RemainingDistance += static_cast<int>(theSourceCoords.DistanceFrom(theTargetCoords) + pType->Trajectory_Speed);
	}

	// Determine the firing velocity vector of the bullet
	pBullet->TargetCoords = theTargetCoords;
	pBullet->Velocity.X = static_cast<double>(theTargetCoords.X - theSourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(theTargetCoords.Y - theSourceCoords.Y);

	if (pType->ConfineAtHeight > 0 && pType->PassDetonateLocal)
		pBullet->Velocity.Z = 0;
	else
		pBullet->Velocity.Z = static_cast<double>(this->GetVelocityZ(pBullet));

	// Rotate the selected angle
	if (!this->UseDisperseBurst && std::abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
	{
		const auto axis = pType->AxisOfRotation.Get();

		BulletVelocity rotationAxis
		{
			axis.X * Math::cos(rotateAngle) + axis.Y * Math::sin(rotateAngle),
			axis.X * Math::sin(rotateAngle) - axis.Y * Math::cos(rotateAngle),
			static_cast<double>(axis.Z)
		};

		const auto rotationAxisLengthSquared = rotationAxis.MagnitudeSquared();

		// Rotate around the axis of rotation
		if (std::abs(rotationAxisLengthSquared) > 1e-10)
		{
			double extraRotate = 0.0;
			rotationAxis *= 1 / sqrt(rotationAxisLengthSquared);

			if (pType->MirrorCoord)
			{
				if (this->CurrentBurst % 2 == 1)
					rotationAxis *= -1;

				extraRotate = Math::Pi * (pType->RotateCoord * ((this->CurrentBurst / 2) / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}
			else
			{
				extraRotate = Math::Pi * (pType->RotateCoord * (this->CurrentBurst / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}

			const auto cosRotate = Math::cos(extraRotate);
			pBullet->Velocity = (pBullet->Velocity * cosRotate) + (rotationAxis * ((1 - cosRotate) * (pBullet->Velocity * rotationAxis))) + (rotationAxis.CrossProduct(pBullet->Velocity) * Math::sin(extraRotate));
		}
	}

	// Substitute the speed to calculate velocity
	if (this->CalculateBulletVelocity(pBullet))
		this->RemainingDistance = 0;
}

int StraightTrajectory::GetVelocityZ(BulletClass* pBullet)
{
	const auto pType = this->Type;
	auto sourceCellZ = pBullet->SourceCoords.Z;
	auto targetCellZ = pBullet->TargetCoords.Z;
	auto bulletVelocityZ = static_cast<int>(targetCellZ - sourceCellZ);

	// Subtract directly if no need to pass through the target
	if (!pType->PassThrough)
		return bulletVelocityZ;

	if (const auto pTechno = pBullet->Owner)
	{
		const auto pCell = pTechno->GetCell();
		sourceCellZ = pCell->Level * Unsorted::LevelHeight;

		if (pCell->ContainsBridge())
			sourceCellZ += CellClass::BridgeHeight;
	}

	if (const auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target))
	{
		const auto pCell = pTarget->GetCell();
		targetCellZ = pCell->Level * Unsorted::LevelHeight;

		if (pCell->ContainsBridge())
			targetCellZ += CellClass::BridgeHeight;
	}

	// If both are at the same height, use the DetonationDistance to calculate which position behind the target needs to be aimed
	if (sourceCellZ == targetCellZ || std::abs(bulletVelocityZ) <= 32)
	{
		// Infinite distance, horizontal emission
		if (!this->DetonationDistance)
			return 0;

		const CoordStruct sourceCoords { pBullet->SourceCoords.X, pBullet->SourceCoords.Y, 0 };
		const CoordStruct targetCoords { pBullet->TargetCoords.X, pBullet->TargetCoords.Y, 0 };
		const auto distanceOfTwo = sourceCoords.DistanceFrom(targetCoords);
		const auto theDistance = (this->DetonationDistance < 0) ? (distanceOfTwo - this->DetonationDistance) : this->DetonationDistance;

		// Calculate the ratio for subsequent speed calculation
		if (std::abs(theDistance) > 1e-10)
			bulletVelocityZ = static_cast<int>(bulletVelocityZ * (distanceOfTwo / theDistance));
		else
			return 0;
	}

	return bulletVelocityZ;
}

bool StraightTrajectory::CalculateBulletVelocity(BulletClass* pBullet)
{
	const auto velocityLength = pBullet->Velocity.Magnitude();

	if (velocityLength > 1e-10)
		pBullet->Velocity *= this->Type->Trajectory_Speed / velocityLength;
	else
		return true;

	return false;
}

bool StraightTrajectory::BulletPrepareCheck(BulletClass* pBullet)
{
	// The time between bullets' Unlimbo() and Update() is completely uncertain.
	// Technos will update its location after firing, which may result in inaccurate
	// target position recorded by the LastTargetCoord in Unlimbo(). Therefore, it's
	// necessary to record the position during the first Update(). - CrimRecya
	if (this->WaitOneFrame == 2)
	{
		if (const auto pTarget = pBullet->Target)
		{
			this->LastTargetCoord = pTarget->GetCoords();
			this->WaitOneFrame = 1;
			return true;
		}
	}

	this->WaitOneFrame = 0;
	this->PrepareForOpenFire(pBullet);

	return false;
}

bool StraightTrajectory::BulletDetonatePreCheck(BulletClass* pBullet)
{
	// If this value is not empty, it means that the projectile should be directly detonated at this time. This cannot be taken out here for use.
	if (this->ExtraCheck)
		return true;

	// Check the remaining travel distance of the bullet
	this->RemainingDistance -= static_cast<int>(this->Type->Trajectory_Speed);

	if (this->RemainingDistance < 0)
		return true;

	const auto pType = this->Type;

	// Need to detonate it in advance?
	if (!pType->PassThrough && pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	// Below ground level?
	if (pType->SubjectToGround && MapClass::Instance->GetCellFloorHeight(pBullet->Location) >= (pBullet->Location.Z + 15))
		return true;

	// Out of map?
	if (const auto pCell = MapClass::Instance->TryGetCellAt(pBullet->Location))
		return false;
	else
		return true;
}

// If there is an obstacle on the route, the bullet should need to reduce its speed so it will not penetrate the obstacle.
void StraightTrajectory::BulletDetonateVelocityCheck(BulletClass* pBullet, HouseClass* pOwner)
{
	const auto pType = this->Type;
	bool velocityCheck = false;
	double locationDistance = this->RemainingDistance;

	// Check if the distance to the destination exceeds the speed limit
	if (locationDistance < pType->Trajectory_Speed)
		velocityCheck = true;

	const bool checkThrough = (!pType->ThroughBuilding || !pType->ThroughVehicles);
	const bool checkSubject = (pType->SubjectToGround || pBullet->Type->SubjectToWalls);

	if (pType->Trajectory_Speed < 256.0) // Low speed with checkSubject was already done well.
	{
		// Blocked by obstacles?
		if (checkThrough && this->CheckThroughAndSubjectInCell(pBullet, MapClass::Instance->GetCellAt(pBullet->Location), pOwner))
		{
			locationDistance = 0.0;
			velocityCheck = true;
		}
	}
	else if (checkThrough || checkSubject) // When in high speed, it's necessary to check each cell on the path that the next frame will pass through
	{
		const auto theSourceCoords = pBullet->Location;
		const CoordStruct theTargetCoords
		{
			pBullet->Location.X + static_cast<int>(pBullet->Velocity.X),
			pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y),
			pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z)
		};

		const auto sourceCell = CellClass::Coord2Cell(theSourceCoords);
		const auto targetCell = CellClass::Coord2Cell(theTargetCoords);
		const auto cellDist = sourceCell - targetCell;
		const auto cellPace = CellStruct { static_cast<short>(std::abs(cellDist.X)), static_cast<short>(std::abs(cellDist.Y)) };

		auto largePace = static_cast<size_t>(std::max(cellPace.X, cellPace.Y));
		const auto stepCoord = !largePace ? CoordStruct::Empty : (theTargetCoords - theSourceCoords) * (1.0 / largePace);
		auto curCoord = theSourceCoords;
		auto pCurCell = MapClass::Instance->GetCellAt(sourceCell);
		double cellDistance = locationDistance;

		for (size_t i = 0; i < largePace; ++i)
		{
			// Below ground level?
			if (pType->SubjectToGround && (curCoord.Z + 15) < MapClass::Instance->GetCellFloorHeight(curCoord))
			{
				velocityCheck = true;
				cellDistance = curCoord.DistanceFrom(theSourceCoords);
				break;
			}

			// Impact on the wall?
			if (pBullet->Type->SubjectToWalls && pCurCell->OverlayTypeIndex != -1 && OverlayTypeClass::Array->GetItem(pCurCell->OverlayTypeIndex)->Wall)
			{
				velocityCheck = true;
				cellDistance = curCoord.DistanceFrom(theSourceCoords);
				break;
			}

			// Blocked by obstacles?
			if (checkThrough && this->CheckThroughAndSubjectInCell(pBullet, pCurCell, pOwner))
			{
				velocityCheck = true;
				cellDistance = curCoord.DistanceFrom(theSourceCoords);
				break;
			}

			curCoord += stepCoord;
			pCurCell = MapClass::Instance->GetCellAt(curCoord);
		}

		locationDistance = cellDistance;
	}

	// Check if the bullet needs to slow down the speed
	if (velocityCheck)
	{
		this->RemainingDistance = 0;
		locationDistance += 32.0;

		if (locationDistance < pType->Trajectory_Speed)
			pBullet->Velocity *= (locationDistance / pType->Trajectory_Speed);
	}
}

// If the check result here is true, it only needs to be detonated in the next frame, without returning.
void StraightTrajectory::BulletDetonateLastCheck(BulletClass* pBullet, HouseClass* pOwner)
{
	const auto pType = this->Type;

	// Obstacles were detected in the current frame here
	if (const auto pDetonateAt = this->ExtraCheck)
	{
		// Slow down and reset the target
		const auto position = pDetonateAt->GetCoords();
		const auto distance = position.DistanceFrom(pBullet->Location);
		const auto velocity = pBullet->Velocity.Magnitude();

		pBullet->SetTarget(pDetonateAt);
		pBullet->TargetCoords = position;

		if (std::abs(velocity) > 1e-10 && distance < velocity)
			pBullet->Velocity *= distance / velocity;

		// Need to cause additional damage?
		if (this->ProximityImpact != 0)
		{
			const auto pWH = pType->ProximityWarhead;

			if (!pWH)
				return;

			auto damage = this->GetTheTrueDamage(this->ProximityDamage, pBullet, pType->ProximityMedial ? nullptr : pDetonateAt, false);

			if (pType->ProximityDirect)
				pDetonateAt->ReceiveDamage(&damage, 0, pWH, pBullet->Owner, false, false, pOwner);
			else if (pType->ProximityMedial)
				WarheadTypeExt::DetonateAt(pWH, pBullet->Location, pBullet->Owner, damage, pOwner);
			else
				WarheadTypeExt::DetonateAt(pWH, position, pBullet->Owner, damage, pOwner, pDetonateAt);

			this->CalculateNewDamage(pBullet);
		}
	}
}

bool StraightTrajectory::CheckThroughAndSubjectInCell(BulletClass* pBullet, CellClass* pCell, HouseClass* pOwner)
{
	const auto pType = this->Type;
	auto pObject = pCell->GetContent();

	while (pObject)
	{
		const auto pTechno = abstract_cast<TechnoClass*>(pObject);
		pObject = pObject->NextObject;

		// Non technos and not target friendly forces will be excluded
		if (!pTechno || (pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pBullet->Target))
			continue;

		const auto technoType = pTechno->WhatAmI();

		// Check building obstacles
		if (technoType == AbstractType::Building)
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
		if (!pType->ThroughVehicles && (technoType == AbstractType::Unit || technoType == AbstractType::Aircraft))
		{
			this->ExtraCheck = pTechno;
			return true;
		}
	}

	return false;
}

void StraightTrajectory::CalculateNewDamage(BulletClass* pBullet)
{
	const auto pType = this->Type;
	const auto ratio = pType->CountAttenuation.Get();

	// Calculate the attenuation damage under three different scenarios
	if (ratio != 1.0)
	{
		// If the ratio is not 0, the lowest damage will be retained
		if (ratio)
		{
			if (pBullet->Health)
			{
				if (const auto newDamage = static_cast<int>(pBullet->Health * ratio))
					pBullet->Health = newDamage;
				else
					pBullet->Health = Math::sgn(pBullet->Health);
			}

			if (this->ProximityDamage)
			{
				if (const auto newDamage = static_cast<int>(this->ProximityDamage * ratio))
					this->ProximityDamage = newDamage;
				else
					this->ProximityDamage = Math::sgn(this->ProximityDamage);
			}

			if (this->PassDetonateDamage)
			{
				if (const auto newDamage = static_cast<int>(this->PassDetonateDamage * ratio))
					this->PassDetonateDamage = newDamage;
				else
					this->PassDetonateDamage = Math::sgn(this->PassDetonateDamage);
			}
		}
		else
		{
			pBullet->Health = 0;
			this->ProximityDamage = 0;
			this->PassDetonateDamage = 0;
		}
	}
}

void StraightTrajectory::PassWithDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->PassDetonateTimer.Completed())
	{
		const auto pType = this->Type;
		const auto pWH = pType->PassDetonateWarhead;

		if (!pWH)
			return;

		this->PassDetonateTimer.Start(pType->PassDetonateDelay > 0 ? pType->PassDetonateDelay : 1);
		auto detonateCoords = pBullet->Location;

		// Whether to detonate at ground level?
		if (pType->PassDetonateLocal)
			detonateCoords.Z = MapClass::Instance->GetCellFloorHeight(detonateCoords);

		const auto damage = this->GetTheTrueDamage(this->PassDetonateDamage, pBullet, nullptr, false);
		WarheadTypeExt::DetonateAt(pWH, detonateCoords, pBullet->Owner, damage, pOwner);
		this->CalculateNewDamage(pBullet);
	}
}

// Select suitable targets and choose the closer targets then attack each target only once.
void StraightTrajectory::PrepareForDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
	const auto pType = this->Type;
	const auto pWH = pType->ProximityWarhead;

	if (!pWH)
		return;

	// Step 1: Find valid targets on the ground within range.
	std::vector<CellClass*> recCellClass = PhobosTrajectoryType::GetCellsInProximityRadius(pBullet, pType->ProximityRadius.Get());
	const size_t cellSize = recCellClass.size() * 2;
	size_t vectSize = cellSize;
	size_t thisSize = 0;

	const CoordStruct velocityCrd
	{
		static_cast<int>(pBullet->Velocity.X),
		static_cast<int>(pBullet->Velocity.Y),
		static_cast<int>(pBullet->Velocity.Z)
	};

	std::vector<TechnoClass*> validTechnos;
	validTechnos.reserve(vectSize);
	const auto pTarget = pBullet->Target;

	for (const auto& pRecCell : recCellClass)
	{
		auto pObject = pRecCell->GetContent();

		while (pObject)
		{
			const auto pTechno = abstract_cast<TechnoClass*>(pObject);
			pObject = pObject->NextObject;

			if (!pTechno || !pTechno->IsAlive || !pTechno->IsOnMap || pTechno->Health <= 0 || pTechno->InLimbo || pTechno->IsSinking)
				continue;

			const auto technoType = pTechno->WhatAmI();

			if (technoType == AbstractType::Building && static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame)
				continue;

			// Not directly harming friendly forces
			if (!pType->ProximityAllies && pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pTarget)
				continue;

			const auto distanceCrd = pTechno->GetCoords() - pBullet->SourceCoords;
			const auto locationCrd = (velocityCrd + (pBullet->Location - pBullet->SourceCoords));
			const auto terminalCrd = distanceCrd - locationCrd;
			auto distance = locationCrd.MagnitudeSquared(); // Not true distance yet.

			// Between front and back
			if (distanceCrd * velocityCrd < 0 || terminalCrd * velocityCrd > 0)
				continue;

			distance = (distance > 1e-10) ? sqrt(distanceCrd.CrossProduct(terminalCrd).MagnitudeSquared() / distance) : distanceCrd.Magnitude();

			// Between left and right (cylindrical)
			if (technoType != AbstractType::Building && distance > pType->ProximityRadius.Get())
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
		airTracker->FillCurrentVector(MapClass::Instance->GetCellAt(pBullet->Location + velocityCrd * 0.5), static_cast<int>((pType->ProximityRadius.Get() + pType->Trajectory_Speed / 2) / Unsorted::LeptonsPerCell));

		for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
		{
			if (!pTechno->IsAlive || !pTechno->IsOnMap || pTechno->Health <= 0 || pTechno->InLimbo || pTechno->IsSinking)
				continue;

			if (!pType->ProximityAllies && pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pTarget)
				continue;

			const auto distanceCrd = pTechno->GetCoords() - pBullet->Location;
			const auto terminalCrd = distanceCrd - velocityCrd;
			auto distance = velocityCrd.MagnitudeSquared(); // Not true distance yet.

			if (distanceCrd * velocityCrd < 0 || terminalCrd * velocityCrd > 0)
				continue;

			distance = (distance > 1e-10) ? sqrt(distanceCrd.CrossProduct(terminalCrd).MagnitudeSquared() / distance) : distanceCrd.Magnitude();

			if (distance > pType->ProximityRadius.Get())
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
	casualtyChecked.reserve(std::max(validTechnos.size(), this->TheCasualty.size()));

	if (const auto pFirer = pBullet->Owner)
		this->TheCasualty[pFirer->UniqueID] = 20;

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

		this->TheCasualty[pTechno->UniqueID] = 20;
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
		const auto technoType = pTechno->WhatAmI();

		if (!pType->ThroughVehicles && (technoType == AbstractType::Unit || technoType == AbstractType::Aircraft))
			continue;

		if (technoType == AbstractType::Building && (static_cast<BuildingClass*>(pTechno)->IsStrange() ? !pType->ThroughVehicles : !pType->ThroughBuilding))
			continue;

		// Cause damage
		auto damage = this->GetTheTrueDamage(this->ProximityDamage, pBullet, pType->ProximityMedial ? nullptr : pTechno, false);

		if (pType->ProximityDirect)
			pTechno->ReceiveDamage(&damage, 0, pWH, pBullet->Owner, false, false, pOwner);
		else if (pType->ProximityMedial)
			WarheadTypeExt::DetonateAt(pWH, pBullet->Location, pBullet->Owner, damage, pOwner);
		else
			WarheadTypeExt::DetonateAt(pWH, pTechno->GetCoords(), pBullet->Owner, damage, pOwner, pTechno);

		this->CalculateNewDamage(pBullet);

		if (this->ProximityImpact > 0)
			--this->ProximityImpact;
	}
}

int StraightTrajectory::GetTheTrueDamage(int damage, BulletClass* pBullet, TechnoClass* pTechno, bool self)
{
	if (damage == 0)
		return 0;

	const auto pType = this->Type;

	// Calculate damage distance attenuation
	if (pType->EdgeAttenuation != 1.0)
	{
		const auto damageMultiplier = this->GetExtraDamageMultiplier(pBullet, pTechno);
		const auto calculatedDamage = self ? damage * damageMultiplier : damage * this->FirepowerMult * damageMultiplier;
		const auto signal = Math::sgn(calculatedDamage);
		damage = static_cast<int>(calculatedDamage);

		// Retain minimal damage
		if (!damage && pType->EdgeAttenuation > 0.0)
			damage = signal;
	}

	return damage;
}

double StraightTrajectory::GetExtraDamageMultiplier(BulletClass* pBullet, TechnoClass* pTechno)
{
	double distance = 0.0;
	double damageMult = 1.0;

	// Here it may not be fair to the architecture
	if (pTechno)
		distance = pTechno->GetCoords().DistanceFrom(pBullet->SourceCoords);
	else
		distance = pBullet->Location.DistanceFrom(pBullet->SourceCoords);

	if (this->AttenuationRange < static_cast<int>(distance))
		return this->Type->EdgeAttenuation;

	// Remove the first cell distance for calculation
	if (distance > 256.0)
		damageMult += (this->Type->EdgeAttenuation - 1.0) * ((distance - 256.0) / (static_cast<double>(this->AttenuationRange - 256)));

	return damageMult;
}

bool StraightTrajectory::PassAndConfineAtHeight(BulletClass* pBullet)
{
	const CoordStruct futureCoords
	{
		pBullet->Location.X + static_cast<int>(pBullet->Velocity.X),
		pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y),
		pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z)
	};

	auto checkDifference = MapClass::Instance->GetCellFloorHeight(futureCoords) - futureCoords.Z;
	const auto cellCoords = MapClass::Instance->GetCellAt(futureCoords)->GetCoordsWithBridge();
	const auto differenceOnBridge = cellCoords.Z - futureCoords.Z;

	if (std::abs(differenceOnBridge) < std::abs(checkDifference))
		checkDifference = differenceOnBridge;

	// The height does not exceed the cliff, or the cliff can be ignored?
	if (std::abs(checkDifference) < 384 || !pBullet->Type->SubjectToCliffs)
	{
		const auto pType = this->Type;
		pBullet->Velocity.Z += static_cast<double>(checkDifference + pType->ConfineAtHeight);

		if (!pType->PassDetonateLocal && this->CalculateBulletVelocity(pBullet))
			return true;
	}
	else
	{
		return true;
	}

	return false;
}
