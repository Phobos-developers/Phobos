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
	// LiveShell
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
		const auto distance = pBullet->Location.DistanceFrom(pBullet->TargetCoords);

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
		if (this->OriginalDistance < (Unsorted::LeptonsPerCell * 5)) // When the distance is very close, the trajectory tends to be parabolic
			this->OriginalDistance = static_cast<int>(this->OriginalDistance * 1.2) + (Unsorted::LeptonsPerCell * 2);
		else if (this->OriginalDistance > (Unsorted::LeptonsPerCell * 15)) // When the distance is far enough, it is the complete trajectory
			this->OriginalDistance = static_cast<int>(this->OriginalDistance * 0.4) + (Unsorted::LeptonsPerCell * 2);
		else // The distance is neither long nor short, it is an adaptive trajectory
			this->OriginalDistance = (Unsorted::LeptonsPerCell * 8);
		// Calculate the maximum height during the ascending phase
		this->OriginalDistance = this->OriginalDistance < 3200 ? this->OriginalDistance / 2 : this->OriginalDistance - 1600;
	}
	else // Under normal circumstances, the trajectory is similar to ROT projectile with an initial launch direction
	{
		if (pType->SuicideAboveRange < 0)
			this->RemainingDistance = static_cast<int>(this->OriginalDistance * (-pType->SuicideAboveRange));
		else if (pType->SuicideAboveRange > 0)
			this->RemainingDistance = static_cast<int>(Unsorted::LeptonsPerCell * pType->SuicideAboveRange);
		else
			this->RemainingDistance = INT_MAX;
		// Without setting an initial direction, it will be launched directly towards the target
		if (pType->PreAimCoord == CoordStruct::Empty)
		{
			const auto pBullet = this->Bullet;
			this->InStraight = true;
			this->MovingVelocity = PhobosTrajectory::Coord2Vector(pBullet->TargetCoords - pBullet->SourceCoords);
		}
		else
		{
			this->PreAimDistance = (pType->PreAimCoord.Get()).Magnitude();
			// When the distance is short, the initial moving distance will be reduced
			if (pType->ReduceCoord && this->OriginalDistance < (Unsorted::LeptonsPerCell * 10))
				this->PreAimDistance *= this->OriginalDistance / (Unsorted::LeptonsPerCell * 10);
			// The calculation for each frame will be completed before the speed update, make up for the first frame here
			this->PreAimDistance += pType->LaunchSpeed;
			this->InitializeBulletNotCurve();
		}

		this->MovingSpeed = pType->LaunchSpeed;
		// Calculate speed
		if (this->CalculateBulletVelocity(pType->LaunchSpeed))
			this->ShouldDetonate = true;
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
	pBullet->SetTarget(pTarget);
	pBullet->TargetCoords = pTarget->GetCoords();
	this->LastTargetCoord = pBullet->TargetCoords;
	// Reset cruise flag
	if (this->Type->CruiseEnable)
		this->CruiseEnable = true;
}

bool MissileTrajectory::CalculateBulletVelocity(const double speed)
{
	const auto velocityLength = this->MovingVelocity.Magnitude();

	if (velocityLength < 1e-10)
		return true;

	this->MovingVelocity *= speed / velocityLength;
	return false;
}

void MissileTrajectory::InitializeBulletNotCurve()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
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
		this->OffsetCoord = this->GetInaccurateTargetCoords(this->OffsetCoord, source.DistanceFrom(target));
	// Determine the firing velocity vector of the bullet
	if (!this->CalculateReducedVelocity(rotateRadian))
		this->MovingVelocity = PhobosTrajectory::HorizontalRotate(this->GetPreAimCoordsWithBurst(), rotateRadian);
	// Rotate the selected angle
	if (std::abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
		this->DisperseBurstSubstitution(rotateRadian);
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

bool MissileTrajectory::CalculateReducedVelocity(double rotateRadian)
{
	const auto pType = this->Type;
	// Check if it can reduce
	if (!pType->ReduceCoord || pType->TurningSpeed <= 1e-10)
		return false;
	// Check if its steering ability is sufficient
	const auto coordMult = (this->OriginalDistance * pType->TurningSpeed / (Unsorted::LeptonsPerCell * 90 / 2));
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
	const bool checkValid = (pTarget && pTarget->WhatAmI() == AbstractType::Bullet) || (pTargetTechno && !CheckTechnoIsInvalid(pTargetTechno));
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
		const auto horizonDistance = horizonVelocity.Magnitude();

		if (horizonDistance > 0)
		{
			// Slowly step up
			auto horizonMult = std::abs(this->MovingVelocity.Z / 64.0) / horizonDistance;
			this->MovingVelocity.X += horizonMult * horizonVelocity.X;
			this->MovingVelocity.Y += horizonMult * horizonVelocity.Y;
			const auto horizonLength = sqrt(this->MovingVelocity.X * this->MovingVelocity.X + this->MovingVelocity.Y * this->MovingVelocity.Y);

			// Limit horizontal maximum speed
			if (horizonLength > 64.0)
			{
				horizonMult = 64.0 / horizonLength;
				this->MovingVelocity.X *= horizonMult;
				this->MovingVelocity.Y *= horizonMult;
			}
		}
		// The launch phase is divided into ascending and descending stages
		if ((pBullet->Location.Z - pBullet->SourceCoords.Z) < this->OriginalDistance && this->Accelerate)
		{
			if (this->MovingVelocity.Z < 160.0) // Accelerated phase of ascent
				this->MovingVelocity.Z += 4.0;
		}
		else // End of ascent
		{
			this->Accelerate = false;
			// Predict the lowest position
			const auto futureHeight = pBullet->Location.Z + 8 * this->MovingVelocity.Z;

			// Start decelerating/accelerating downwards
			if (this->MovingVelocity.Z > -160.0)
				this->MovingVelocity.Z -= 4.0;

			// Enter gliding phase below predicted altitude
			if (futureHeight <= targetLocation.Z || futureHeight <= pBullet->SourceCoords.Z)
				this->InStraight = true;
		}

		this->MovingSpeed = this->MovingVelocity.Magnitude();
	}
	else // In the gliding stage
	{
		// Predict hit time
		const auto timeMult = targetLocation.DistanceFrom(pBullet->Location) / 192.0;
		targetLocation.Z += static_cast<int>(timeMult * 48);
		// Calculate the target lead time
		if (checkValid)
		{
			targetLocation.X += static_cast<int>(timeMult * (targetLocation.X - this->LastTargetCoord.X));
			targetLocation.Y += static_cast<int>(timeMult * (targetLocation.Y - this->LastTargetCoord.Y));
		}
		// Stable the fixed flight speed
		this->MovingSpeed = this->MovingVelocity.Magnitude();

		if (this->MovingSpeed < 192.0)
			this->MovingSpeed += 4.0;

		if (this->MovingSpeed > 192.0)
			this->MovingSpeed = 192.0;
		// Calculate the speed change during gliding phase using common steering algorithm
		if (this->ChangeBulletVelocity(targetLocation) || this->CalculateBulletVelocity(this->MovingSpeed))
			return true;
	}

	return false;
}

bool MissileTrajectory::NotCurveVelocityChange()
{
	const auto pType = this->Type;

	if (this->PreAimDistance > 0)
		this->PreAimDistance -= this->MovingSpeed;

	bool velocityUp = false;
	// Calculate speed
	if (this->Accelerate && std::abs(pType->Acceleration) > 1e-10)
	{
		this->MovingSpeed += pType->Acceleration;
		// Judging whether to accelerate or decelerate based on acceleration
		if (pType->Acceleration > 0)
		{
			if (this->MovingSpeed >= pType->Speed)
			{
				this->MovingSpeed = pType->Speed;
				this->Accelerate = false;
			}
		}
		else if (this->MovingSpeed <= pType->Speed)
		{
			this->MovingSpeed = pType->Speed;
			this->Accelerate = false;
		}

		velocityUp = true;
	}
	// Calculate steering
	if (!pType->LockDirection || !this->InStraight)
	{
		// Make the turn
		if (this->PreAimDistance <= 0 && this->StandardVelocityChange())
			return true;

		velocityUp = true;
	}
	// Calculate velocity vector
	return velocityUp && this->CalculateBulletVelocity(this->MovingSpeed);
}

bool MissileTrajectory::StandardVelocityChange()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	const auto pTarget = pBullet->Target;
	const auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget);
	const bool checkValid = (pTarget && pTarget->WhatAmI() == AbstractType::Bullet) || (pTargetTechno && !CheckTechnoIsInvalid(pTargetTechno));
	// To be used later, no reference
	auto targetLocation = pBullet->TargetCoords;
	// Follow and track the target like a missile
	if (checkValid)
		targetLocation = pTarget->GetCoords();

	pBullet->TargetCoords = targetLocation;
	// Add calculated fixed offset
	targetLocation += this->OffsetCoord;
	// If the speed is too low, it will cause the lead time calculation results to be too far away and unable to be used
	if (pType->LeadTimeCalculate.Get(true) && checkValid && (pType->UniqueCurve || pType->Speed > 64.0))
	{
		const auto leadSpeed = (pType->Speed + this->MovingSpeed) / 2;
		const auto timeMult = targetLocation.DistanceFrom(pBullet->Location) / leadSpeed;
		targetLocation += (pBullet->TargetCoords - this->LastTargetCoord) * timeMult;
	}
	// If in the cruise phase, the steering target will be set at the fixed height
	if (this->CruiseEnable)
	{
		const auto horizontal = PhobosTrajectory::Coord2Point(targetLocation - pBullet->Location);
		const auto horizontalDistance = horizontal.Magnitude();
		// The distance is still long, continue cruising
		if (horizontalDistance > pType->CruiseUnableRange.Get())
		{
			const auto ratio = this->MovingSpeed / horizontalDistance;
			targetLocation.X = pBullet->Location.X + static_cast<int>(horizontal.X * ratio);
			targetLocation.Y = pBullet->Location.Y + static_cast<int>(horizontal.Y * ratio);
			const auto altitude = pType->CruiseAltitude + (pType->CruiseAlongLevel ? MapClass::Instance.GetCellFloorHeight(pBullet->Location) : pBullet->SourceCoords.Z);
			// Smooth curve for low turning speed projectile
			targetLocation.Z = (altitude + pBullet->Location.Z) / 2;
		}
		else
		{
			this->CruiseEnable = false;
			this->LastDotProduct = 0;
		}
	}
	// Calculate the velocity direction change
	return this->ChangeBulletVelocity(targetLocation);
}

bool MissileTrajectory::ChangeBulletVelocity(const CoordStruct& targetLocation)
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	auto& bulletVelocity = this->MovingVelocity;
	bulletVelocity.Z -= BulletTypeExt::GetAdjustedGravity(pBullet->Type);
	const auto targetVelocity = PhobosTrajectory::Coord2Vector(targetLocation - pBullet->Location);
	// Calculate the new velocity vector based on turning speed
	const auto dotProduct = (targetVelocity * bulletVelocity);
	const auto cosTheta = dotProduct / sqrt(targetVelocity.MagnitudeSquared() * bulletVelocity.MagnitudeSquared());
	const auto radian = Math::acos(Math::clamp(cosTheta, -1.0, 1.0)); // Ensure that the result range of cos is correct
	const auto turningRadius = pType->TurningSpeed * (Math::TwoPi / 360); // TurningSpeed uses angles as units and requires conversion
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
		this->InStraight = true;
	}
	// Record the current value for subsequent checks
	this->LastDotProduct = dotProduct;
	this->LastTargetCoord = pBullet->TargetCoords;
	return false;
}
