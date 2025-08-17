#include "MissileTrajectory.h"

#include <Ext/Bullet/Body.h>

std::unique_ptr<PhobosTrajectory> MissileTrajectoryType::CreateInstance(BulletClass* pBullet) const
{
	return std::make_unique<MissileTrajectory>(this, pBullet);
}

template<typename T>
void MissileTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->UniqueCurve)
		.Process(this->FacingCoord)
		.Process(this->ReduceCoord)
		.Process(this->PreAimCoord)
		.Process(this->LaunchSpeed)
		.Process(this->Acceleration)
		.Process(this->TurningSpeed)
		.Process(this->LockDirection)
		.Process(this->CruiseEnable)
		.Process(this->CruiseUnableRange)
		.Process(this->CruiseAltitude)
		.Process(this->CruiseAlongLevel)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideShortOfROT)
		;
}

bool MissileTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->ActualTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool MissileTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->ActualTrajectoryType::Save(Stm);
	const_cast<MissileTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void MissileTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->PhobosTrajectoryType::Read(pINI, pSection);
	INI_EX exINI(pINI);

	// Actual
	this->RotateCoord.Read(exINI, pSection, "Trajectory.RotateCoord");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.OffsetCoord");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.AxisOfRotation");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.LeadTimeCalculate");
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.TargetSnapDistance");

	// Missile
	this->UniqueCurve.Read(exINI, pSection, "Trajectory.Missile.UniqueCurve");

	// Independent reading section
	if (this->UniqueCurve)
	{
		// 154 -> 0.6 * Unsorted::LeptonsPerCell (Used to ensure correct hit at the fixed speed)
		this->TargetSnapDistance = Leptons(154);

		// 154 -> 0.5 * Unsorted::LeptonsPerCell (Used to ensure correct hit at the fixed speed)
		this->DetonationDistance = Leptons(128);

		// Fixed and appropriate final speed
		this->Speed = MissileTrajectory::UniqueCurveSpeed;

		// Fixed and appropriate acceleration
		this->Acceleration = MissileTrajectory::UniqueCurveAcceleration;

		// Fixed and appropriate steering speed
		this->TurningSpeed = 10.0;
		return;
	}

	// Otherwise, read all
	this->FacingCoord.Read(exINI, pSection, "Trajectory.Missile.FacingCoord");
	this->ReduceCoord.Read(exINI, pSection, "Trajectory.Missile.ReduceCoord");
	this->PreAimCoord.Read(exINI, pSection, "Trajectory.Missile.PreAimCoord");
	this->LaunchSpeed.Read(exINI, pSection, "Trajectory.Missile.LaunchSpeed");
	this->LaunchSpeed = Math::max(0.001, this->LaunchSpeed);
	this->Acceleration.Read(exINI, pSection, "Trajectory.Missile.Acceleration");
	this->TurningSpeed.Read(exINI, pSection, "Trajectory.Missile.TurningSpeed");
	this->TurningSpeed = Math::max(0.0, this->TurningSpeed);
	this->LockDirection.Read(exINI, pSection, "Trajectory.Missile.LockDirection");
	this->CruiseEnable.Read(exINI, pSection, "Trajectory.Missile.CruiseEnable");
	this->CruiseUnableRange.Read(exINI, pSection, "Trajectory.Missile.CruiseUnableRange");
	this->CruiseUnableRange = Leptons(Math::max(128, this->CruiseUnableRange.Get()));
	this->CruiseAltitude.Read(exINI, pSection, "Trajectory.Missile.CruiseAltitude");
	this->CruiseAlongLevel.Read(exINI, pSection, "Trajectory.Missile.CruiseAlongLevel");
	this->SuicideAboveRange.Read(exINI, pSection, "Trajectory.Missile.SuicideAboveRange");
	this->SuicideShortOfROT.Read(exINI, pSection, "Trajectory.Missile.SuicideShortOfROT");
}

template<typename T>
void MissileTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->CruiseEnable)
		.Process(this->InStraight)
		.Process(this->Accelerate)
		.Process(this->OriginalDistance)
		.Process(this->OffsetCoord)
		.Process(this->PreAimDistance)
		.Process(this->LastDotProduct)
		;
}

bool MissileTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->ActualTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool MissileTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->ActualTrajectory::Save(Stm);
	const_cast<MissileTrajectory*>(this)->Serialize(Stm);
	return true;
}

void MissileTrajectory::OnUnlimbo()
{
	this->ActualTrajectory::OnUnlimbo();

	// Missile
	const auto pBullet = this->Bullet;

	// Record the initial distance
	this->OriginalDistance = static_cast<int>(pBullet->TargetCoords.DistanceFrom(pBullet->SourceCoords));

	// Waiting for launch trigger
	if (!BulletExt::ExtMap.Find(pBullet)->DispersedTrajectory)
		this->OpenFire();
}

bool MissileTrajectory::OnEarlyUpdate()
{
	// No need to wait for the calculation of lead time
	if (this->PhobosTrajectory::OnEarlyUpdate())
		return true;

	// Restore ProjectileRange
	if (!this->Type->UniqueCurve)
		this->CheckProjectileRange();

	// Waiting for new location calculated
	return false;
}

bool MissileTrajectory::OnVelocityCheck()
{
	// Calculate new speed
	if (this->Type->UniqueCurve ? this->CurveVelocityChange() : this->NotCurveVelocityChange())
		return true;

	// Check if the bullet needs to slow down the speed since it will pass through the target
	if (this->LastDotProduct > 0)
	{
		const auto pBullet = this->Bullet;
		const double distance = pBullet->Location.DistanceFrom(pBullet->TargetCoords + this->OffsetCoord);

		if (this->MovingSpeed > distance)
			this->MultiplyBulletVelocity(distance / this->MovingSpeed, true);
	}

	return this->PhobosTrajectory::OnVelocityCheck();
}

TrajectoryCheckReturnType MissileTrajectory::OnDetonateUpdate(const CoordStruct& position)
{
	if (this->PhobosTrajectory::OnDetonateUpdate(position) == TrajectoryCheckReturnType::Detonate)
		return TrajectoryCheckReturnType::Detonate;

	this->RemainingDistance -= static_cast<int>(this->MovingSpeed);

	// Check the remaining travel distance of the bullet
	if (this->RemainingDistance < 0)
		return TrajectoryCheckReturnType::Detonate;

	// Close enough
	if ((this->Bullet->TargetCoords + this->OffsetCoord).DistanceFrom(position) < this->Type->DetonationDistance.Get())
		return TrajectoryCheckReturnType::Detonate;

	return TrajectoryCheckReturnType::SkipGameCheck;
}

void MissileTrajectory::OpenFire()
{
	const auto pType = this->Type;

	// Set the initial launch state of the projectile
	if (pType->UniqueCurve) // Simulate ballistic missile trajectory
	{
		// Basic speed
		this->MovingVelocity.X = 0;
		this->MovingVelocity.Y = 0;
		this->MovingVelocity.Z = 4.0;
		this->MovingSpeed = 4.0;

		// OriginalDistance is converted to record the maximum height
		if (this->OriginalDistance < (Unsorted::LeptonsPerCell * 8)) // When the distance is very close, the trajectory tends to be parabolic
			this->OriginalDistance = static_cast<int>(this->OriginalDistance * 0.75) + (Unsorted::LeptonsPerCell * 2);
		else if (this->OriginalDistance > (Unsorted::LeptonsPerCell * 15)) // When the distance is far enough, it is the complete trajectory
			this->OriginalDistance = static_cast<int>(this->OriginalDistance * 0.4) + (Unsorted::LeptonsPerCell * 2);
		else // The distance is neither long nor short, it is an adaptive trajectory
			this->OriginalDistance = (Unsorted::LeptonsPerCell * 8);

		// Calculate the maximum height during the ascending phase
		constexpr int thresholdDistance = 3200;
		this->OriginalDistance = this->OriginalDistance < thresholdDistance ? this->OriginalDistance / 2 : this->OriginalDistance - (thresholdDistance / 2);
		this->RemainingDistance = INT_MAX;
	}
	else // Under normal circumstances, the trajectory is similar to ROT projectile with an initial launch direction
	{
		this->InitializeBulletNotCurve();
		this->MovingSpeed = pType->LaunchSpeed;

		// Calculate speed
		if (this->CalculateBulletVelocity(pType->LaunchSpeed))
			this->Status |= TrajectoryStatus::Detonate;
	}

	this->PhobosTrajectory::OpenFire();
}

CoordStruct MissileTrajectory::GetRetargetCenter() const
{
	const auto pBullet = this->Bullet;

	// When in the tracking phase, it only retarget within the range in front of it
	if (!this->InStraight)
		return pBullet->TargetCoords;

	// Calculate the coordinates of the radius distance ahead
	const auto futureVelocity = this->MovingVelocity * ((this->Type->RetargetRadius * Unsorted::LeptonsPerCell) / this->MovingSpeed);
	return CoordStruct { pBullet->Location.X + static_cast<int>(futureVelocity.X), pBullet->Location.Y + static_cast<int>(futureVelocity.Y), pBullet->Location.Z };
}

void MissileTrajectory::SetBulletNewTarget(AbstractClass* const pTarget)
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	pBullet->SetTarget(pTarget);

	// Skip set target coords if is locked
	if (!pType->LockDirection || !this->InStraight)
	{
		pBullet->TargetCoords = pTarget->GetCoords();
		this->LastTargetCoord = pBullet->TargetCoords;
	}

	// Reset cruise flag
	if (pType->CruiseEnable)
		this->CruiseEnable = true;
}

bool MissileTrajectory::CalculateBulletVelocity(const double speed)
{
	const double velocityLength = this->MovingVelocity.Magnitude();

	if (velocityLength < PhobosTrajectory::Epsilon)
		return true;

	this->MovingVelocity *= speed / velocityLength;
	return false;
}

void MissileTrajectory::InitializeBulletNotCurve()
{
	const auto pType = this->Type;

	if (pType->SuicideAboveRange < 0)
		this->RemainingDistance = static_cast<int>(this->OriginalDistance * (-pType->SuicideAboveRange));
	else if (pType->SuicideAboveRange > 0)
		this->RemainingDistance = static_cast<int>(Unsorted::LeptonsPerCell * pType->SuicideAboveRange);
	else
		this->RemainingDistance = INT_MAX;

	const auto pBullet = this->Bullet;
	const auto pFirer = pBullet->Owner;
	const auto& source = pFirer ? pFirer->GetCoords() : pBullet->SourceCoords;
	const auto& target = pBullet->TargetCoords;
	double rotateRadian = 0.0;

	// Calculate the orientation of the coordinate system
	if (!pType->FacingCoord && (target.Y != source.Y || target.X != source.X) || !pFirer)
		rotateRadian = PhobosTrajectory::Get2DOpRadian(source, target);
	else if (pFirer->HasTurret())
		rotateRadian = -(pFirer->TurretFacing().GetRadian<32>());
	else
		rotateRadian = -(pFirer->PrimaryFacing.Current().GetRadian<32>());

	// Add the fixed offset value
	if (pType->OffsetCoord != CoordStruct::Empty)
		this->OffsetCoord = this->GetOnlyStableOffsetCoords(rotateRadian);

	// Add random offset value
	if (pBullet->Type->Inaccurate)
		this->OffsetCoord = this->GetInaccurateTargetCoords((target + this->OffsetCoord), source.DistanceFrom(target)) - target;

	// Without setting an initial direction, it will be launched directly towards the target
	if (pType->PreAimCoord == CoordStruct::Empty)
	{
		this->InStraight = true;
		this->MovingVelocity = PhobosTrajectory::Coord2Vector(target + this->OffsetCoord - pBullet->SourceCoords);
	}
	else
	{
		this->PreAimDistance = (pType->PreAimCoord.Get()).Magnitude();
		constexpr int coordReducingBaseCells = 10;

		// When the distance is short, the initial moving distance will be reduced
		if (pType->ReduceCoord && this->OriginalDistance < (Unsorted::LeptonsPerCell * coordReducingBaseCells))
			this->PreAimDistance *= this->OriginalDistance / (Unsorted::LeptonsPerCell * coordReducingBaseCells);

		// Determine the firing velocity vector of the bullet
		if (!this->CalculateReducedVelocity(rotateRadian))
			this->MovingVelocity = PhobosTrajectory::HorizontalRotate(this->GetPreAimCoordsWithBurst(), rotateRadian);

		// Rotate the selected angle
		if (std::abs(pType->RotateCoord) > PhobosTrajectory::Epsilon && this->CountOfBurst > 1)
			this->DisperseBurstSubstitution(rotateRadian);
	}
}

CoordStruct MissileTrajectory::GetPreAimCoordsWithBurst()
{
	const auto pType = this->Type;
	auto preAimCoord = pType->PreAimCoord.Get();

	// Check if mirroring is required
	if (pType->MirrorCoord && this->CurrentBurst < 0)
		preAimCoord.Y = -preAimCoord.Y;

	// No rotate now, return original value
	return preAimCoord;
}

bool MissileTrajectory::CalculateReducedVelocity(const double rotateRadian)
{
	const auto pType = this->Type;

	// Check if it can reduce
	if (!pType->ReduceCoord || pType->TurningSpeed <= PhobosTrajectory::Epsilon)
		return false;

	// Check if its steering ability is sufficient
	const double coordMult = (this->OriginalDistance * pType->TurningSpeed / (Unsorted::LeptonsPerCell * 90 / 2));

	// Cancel when the coordinate correction is greater than the original coordinate
	if (coordMult >= 1.0)
		return false;

	// Calculate the rotated coordinates
	const auto theAimCoord = PhobosTrajectory::HorizontalRotate(this->GetPreAimCoordsWithBurst(), rotateRadian);
	const auto pBullet = this->Bullet;
	const auto distance = PhobosTrajectory::Coord2Vector(pBullet->TargetCoords - pBullet->SourceCoords);

	// Reduce the initial rotation angle
	this->MovingVelocity = (distance - theAimCoord) * (1 - coordMult) + theAimCoord;
	return true;
}

bool MissileTrajectory::CurveVelocityChange()
{
	const auto pBullet = this->Bullet;
	const auto pTarget = pBullet->Target;
	const auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget);
	const bool checkValid = (pTargetTechno && !PhobosTrajectory::CheckTechnoIsInvalid(pTargetTechno)) || (pTarget && pTarget->WhatAmI() == AbstractType::Bullet);
	auto targetLocation = pBullet->TargetCoords;

	// Follow and track the target like a missile
	if (checkValid)
		targetLocation = pTarget->GetCoords();

	pBullet->TargetCoords = targetLocation;

	// Add calculated fixed offset
	targetLocation += this->OffsetCoord;

	// Update projectile velocity based on stage
	if (!this->InStraight) // In the launch phase
	{
		const auto horizonVelocity = PhobosTrajectory::Coord2Point(targetLocation - pBullet->Location);
		const double horizonDistance = horizonVelocity.Magnitude();

		if (horizonDistance > 0)
		{
			// Slowly step up
			constexpr double uniqueCurveVelocityScale = 64.0;
			double horizonMult = std::abs(this->MovingVelocity.Z / uniqueCurveVelocityScale) / horizonDistance;
			this->MovingVelocity.X += horizonMult * horizonVelocity.X;
			this->MovingVelocity.Y += horizonMult * horizonVelocity.Y;
			const double horizonLength = sqrt(this->MovingVelocity.X * this->MovingVelocity.X + this->MovingVelocity.Y * this->MovingVelocity.Y);
			constexpr double uniqueCurveMaxHorizontalSpeed = 64.0;

			// Limit horizontal maximum speed
			if (horizonLength > uniqueCurveMaxHorizontalSpeed)
			{
				horizonMult = uniqueCurveMaxHorizontalSpeed / horizonLength;
				this->MovingVelocity.X *= horizonMult;
				this->MovingVelocity.Y *= horizonMult;
			}
		}

		constexpr double uniqueCurveMaxVerticalSpeed = 160.0;

		// The launch phase is divided into ascending and descending stages
		if (this->Accelerate && (pBullet->Location.Z - pBullet->SourceCoords.Z) < this->OriginalDistance)
		{
			if (this->MovingVelocity.Z < uniqueCurveMaxVerticalSpeed) // Accelerated phase of ascent
				this->MovingVelocity.Z += MissileTrajectory::UniqueCurveAcceleration;
		}
		else // End of ascent
		{
			this->Accelerate = false;

			// Predict the lowest position
			constexpr int predictFrame = 8;
			const double futureHeight = pBullet->Location.Z + predictFrame * this->MovingVelocity.Z;

			// Start decelerating/accelerating downwards
			if (this->MovingVelocity.Z > -uniqueCurveMaxVerticalSpeed)
				this->MovingVelocity.Z -= MissileTrajectory::UniqueCurveAcceleration;

			// Enter gliding phase below predicted altitude
			if (futureHeight <= targetLocation.Z || futureHeight <= pBullet->SourceCoords.Z)
				this->InStraight = true;
		}

		this->MovingSpeed = this->MovingVelocity.Magnitude();
	}
	else // In the gliding stage
	{
		// Predict hit time
		const double timeMult = targetLocation.DistanceFrom(pBullet->Location) / MissileTrajectory::UniqueCurveSpeed;
		constexpr int uniqueCurveTimeHeightBaseOffset = 48;
		targetLocation.Z += static_cast<int>(timeMult * uniqueCurveTimeHeightBaseOffset);

		// Calculate the target lead time
		if (checkValid)
		{
			targetLocation.X += static_cast<int>(timeMult * (targetLocation.X - this->LastTargetCoord.X));
			targetLocation.Y += static_cast<int>(timeMult * (targetLocation.Y - this->LastTargetCoord.Y));
		}

		// Calculate the speed change during gliding phase using common steering algorithm
		if (this->ChangeBulletVelocity(targetLocation))
			return true;
	}

	return false;
}

bool MissileTrajectory::NotCurveVelocityChange()
{
	// Calculate steering
	if (this->StandardVelocityChange())
		return true;

	if (this->PreAimDistance > 0)
		this->PreAimDistance -= this->MovingSpeed;

	// Calculate velocity vector
	return false;
}

bool MissileTrajectory::StandardVelocityChange()
{
	const auto pBullet = this->Bullet;
	auto targetLocation = CoordStruct::Empty;

	if (this->PreAimDistance > 0)
	{
		targetLocation = pBullet->Location + PhobosTrajectory::Vector2Coord(this->MovingVelocity);
	}
	else
	{
		const auto pType = this->Type;
		const auto pTarget = pBullet->Target;
		const bool checkValid = (!pType->LockDirection || !this->InStraight) && pTarget;

		// Follow and track the target like a missile
		if (checkValid)
			pBullet->TargetCoords = pTarget->GetCoords();

		// Add calculated fixed offset
		targetLocation = pBullet->TargetCoords + this->OffsetCoord;

		constexpr double minLeadTimeAllowableSpeed = 64.0;

		// If the speed is too low, it will cause the lead time calculation results to be too far away and unable to be used
		if (pType->LeadTimeCalculate.Get(true) && checkValid && (pType->UniqueCurve || pType->Speed > minLeadTimeAllowableSpeed))
		{
			const auto pTargetFoot = abstract_cast<FootClass*, true>(pTarget);

			// Only movable targets need to be calculated
			if ((pTargetFoot && !PhobosTrajectory::CheckTechnoIsInvalid(pTargetFoot)) || pTarget->WhatAmI() == AbstractType::Bullet)
			{
				const double leadSpeed = (pType->Speed + this->MovingSpeed) / 2;
				const double timeMult = targetLocation.DistanceFrom(pBullet->Location) / leadSpeed;
				targetLocation += (pBullet->TargetCoords - this->LastTargetCoord) * timeMult;
			}
		}

		// If in the cruise phase, the steering target will be set at the fixed height
		if (this->CruiseEnable)
		{
			const auto horizontal = PhobosTrajectory::Coord2Point(targetLocation - pBullet->Location);
			const double horizontalDistance = horizontal.Magnitude();

			// The distance is still long, continue cruising
			if (horizontalDistance > pType->CruiseUnableRange.Get())
			{
				const double ratio = this->MovingSpeed / horizontalDistance;
				targetLocation.X = pBullet->Location.X + static_cast<int>(horizontal.X * ratio);
				targetLocation.Y = pBullet->Location.Y + static_cast<int>(horizontal.Y * ratio);

				// Smooth curve for low turning speed projectile
				targetLocation.Z = (this->GetCruiseAltitude() + pBullet->Location.Z) / 2;
			}
			else
			{
				this->CruiseEnable = false;
				this->LastDotProduct = 0.0;
			}
		}
	}

	// Calculate new speed
	return this->ChangeBulletVelocity(targetLocation);
}

bool MissileTrajectory::ChangeBulletVelocity(const CoordStruct& targetLocation)
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;

	// Add gravity
	auto& bulletVelocity = this->MovingVelocity;
	bulletVelocity.Z -= BulletTypeExt::GetAdjustedGravity(this->Bullet->Type);
	const double currentSpeed = bulletVelocity.Magnitude();
	this->MovingSpeed = currentSpeed + pType->Acceleration;

	// Calculate new speed with acceleration
	if (pType->Acceleration > 0 || (pType->Acceleration == 0 && pType->LaunchSpeed <= pType->Speed))
	{
		if (this->MovingSpeed > pType->Speed)
			this->MovingSpeed = pType->Speed;
	}
	else
	{
		if (this->MovingSpeed < pType->Speed)
			this->MovingSpeed = pType->Speed;
	}

	bulletVelocity *= this->MovingSpeed / currentSpeed;
	const auto targetVelocity = PhobosTrajectory::Coord2Vector(targetLocation - pBullet->Location);

	// Calculate the new velocity vector based on turning speed
	const double dotProduct = (targetVelocity * bulletVelocity);
	const double cosTheta = dotProduct / (targetVelocity.Magnitude() * this->MovingSpeed);

	// Ensure that the result range of cos is correct
	const double radian = Math::acos(Math::clamp(cosTheta, -1.0, 1.0));

	// TurningSpeed uses angles as units and requires conversion
	const double turningRadius = pType->TurningSpeed * (Math::TwoPi / 360);

	// The angle that needs to be rotated is relatively large
	if (std::abs(radian) > turningRadius)
	{
		// Calculate the rotation axis
		auto rotationAxis = targetVelocity.CrossProduct(bulletVelocity);

		// Substitute to calculate new velocity
		PhobosTrajectory::RotateAboutTheAxis(bulletVelocity, rotationAxis, (radian < 0 ? turningRadius : -turningRadius));

		// Check if the steering ability is insufficient
		if (!pType->UniqueCurve && pType->SuicideShortOfROT && dotProduct <= 0 && (this->InStraight || this->LastDotProduct > 0))
			return true;
	}
	else
	{
		bulletVelocity = targetVelocity;

		if (this->PreAimDistance <= 0)
			this->InStraight = true;
	}

	// Record the current value for subsequent checks
	this->LastDotProduct = dotProduct;
	this->LastTargetCoord = pBullet->TargetCoords;
	return this->CalculateBulletVelocity(this->MovingSpeed);
}

int MissileTrajectory::GetCruiseAltitude()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;

	if (!pType->CruiseAlongLevel || pType->TurningSpeed <= PhobosTrajectory::Epsilon)
		return pType->CruiseAltitude + pBullet->SourceCoords.Z;

	constexpr int shift = 8; // >> shift -> / Unsorted::LeptonsPerCell
	constexpr auto point2Cell = [](const Point2D& point) -> CellStruct
	{
		return CellStruct { static_cast<short>(point.X >> shift), static_cast<short>(point.Y >> shift) };
	};
	auto getFloorHeight = [](const CellClass* const pCell, const Point2D& point) -> int
	{
		return pCell->GetFloorHeight(Point2D { point.X, point.Y }) + (pCell->ContainsBridge() ? CellClass::BridgeHeight : 0);
	};

	// Initialize
	auto curCoord = Point2D { pBullet->Location.X, pBullet->Location.Y };
	const auto tgtCoord = Point2D { pBullet->TargetCoords.X, pBullet->TargetCoords.Y };
	const CellClass* pCurCell = MapClass::Instance.GetCellAt(point2Cell(curCoord));
	int maxHeight = getFloorHeight(pCurCell, curCoord);

	// Prepare for prediction
	auto lastCoord = Point2D::Empty;
	const double checkLength = (pType->Speed / pType->TurningSpeed) * (180 / Math::Pi);
	const double angle = Math::atan2(this->MovingVelocity.Y, this->MovingVelocity.X);
	const auto checkCoord = Point2D { static_cast<int>(checkLength * Math::cos(angle)), static_cast<int>(checkLength * Math::sin(angle)) };
	const int largeStep = Math::max(std::abs(checkCoord.X), std::abs(checkCoord.Y));
	const int checkSteps = (largeStep > Unsorted::LeptonsPerCell) ? (largeStep / Unsorted::LeptonsPerCell + 1) : 1;
	const auto stepCoord = Point2D { (checkCoord.X / checkSteps), (checkCoord.Y / checkSteps) };

	auto checkStepHeight = [&]() -> bool
	{
		// Check forward
		lastCoord = curCoord;
		curCoord += stepCoord;
		pCurCell = MapClass::Instance.TryGetCellAt(point2Cell(curCoord));

		if (!pCurCell)
			return false;

		maxHeight = Math::max(maxHeight, getFloorHeight(pCurCell, curCoord));

		auto getSideHeight = [](const CellClass* const pCell) -> int
		{
			return (pCell->Level * Unsorted::LevelHeight) + (pCell->ContainsBridge() ? CellClass::BridgeHeight : 0);
		};
		auto getAntiAliasingCell = [&]() -> CellClass*
		{
			// Check if it is a diagonal relationship
			if ((curCoord.X >> shift) == (lastCoord.X >> shift) || (curCoord.Y >> shift) == (lastCoord.Y >> shift))
				return nullptr;

			constexpr int mask = 0xFF; // & mask -> % Unsorted::LeptonsPerCell
			bool lastX = false;

			// Calculate the bias of the previous cell
			if (std::abs(stepCoord.X) > std::abs(stepCoord.Y))
			{
				const int offsetX = curCoord.X & mask;
				const int deltaX = (stepCoord.X > 0) ? offsetX : (offsetX - Unsorted::LeptonsPerCell);
				const int projectedY = curCoord.Y - deltaX * checkCoord.Y / checkCoord.X;
				lastX = (projectedY ^ curCoord.Y) >> shift == 0;
			}
			else
			{
				const int offsetY = curCoord.Y & mask;
				const int deltaY = (stepCoord.Y > 0) ? offsetY : (offsetY - Unsorted::LeptonsPerCell);
				const int projectedX = curCoord.X - deltaY * checkCoord.X / checkCoord.Y;
				lastX = (projectedX ^ curCoord.X) >> shift != 0;
			}

			// Get cell
			return MapClass::Instance.TryGetCellAt(lastX
				? CellStruct { static_cast<short>(lastCoord.X >> shift), static_cast<short>(curCoord.Y >> shift) }
				: CellStruct { static_cast<short>(curCoord.X >> shift), static_cast<short>(lastCoord.Y >> shift) });
		};

		// "Anti-Aliasing"
		if (const auto pCheckCell = getAntiAliasingCell())
			maxHeight = Math::max(maxHeight, getSideHeight(pCheckCell));

		return true;
	};

	// Predict height
	for (int i = 0; i < checkSteps && checkStepHeight(); ++i);

	return pType->CruiseAltitude + maxHeight;
}
