#include "ParabolaTrajectory.h"
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>

std::unique_ptr<PhobosTrajectory> ParabolaTrajectoryType::CreateInstance() const
{
	return std::make_unique<ParabolaTrajectory>(this);
}

template<typename T>
void ParabolaTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
		.Process(this->OpenFireMode)
		.Process(this->ThrowHeight)
		.Process(this->LaunchAngle)
		.Process(this->LeadTimeCalculate)
		.Process(this->LeadTimeSimplify)
		.Process(this->LeadTimeMultiplier)
		.Process(this->DetonationAngle)
		.Process(this->DetonationHeight)
		.Process(this->BounceTimes)
		.Process(this->BounceOnWater)
		.Process(this->BounceDetonate)
		.Process(this->BounceAttenuation)
		.Process(this->BounceCoefficient)
		.Process(this->OffsetCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
		;
}

bool ParabolaTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool ParabolaTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<ParabolaTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void ParabolaTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Parabola.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Parabola.TargetSnapDistance");

	pINI->ReadString(pSection, "Trajectory.Parabola.OpenFireMode", "", Phobos::readBuffer);
	if (INIClass::IsBlank(Phobos::readBuffer))
		this->OpenFireMode = ParabolaFireMode::Speed;
	else if (_stricmp(Phobos::readBuffer, "height") == 0)
		this->OpenFireMode = ParabolaFireMode::Height;
	else if (_stricmp(Phobos::readBuffer, "angle") == 0)
		this->OpenFireMode = ParabolaFireMode::Angle;
	else if (_stricmp(Phobos::readBuffer, "speedandheight") == 0)
		this->OpenFireMode = ParabolaFireMode::SpeedAndHeight;
	else if (_stricmp(Phobos::readBuffer, "heightandangle") == 0)
		this->OpenFireMode = ParabolaFireMode::HeightAndAngle;
	else if (_stricmp(Phobos::readBuffer, "speedandangle") == 0)
		this->OpenFireMode = ParabolaFireMode::SpeedAndAngle;
	else
		this->OpenFireMode = ParabolaFireMode::Speed;

	this->ThrowHeight.Read(exINI, pSection, "Trajectory.Parabola.ThrowHeight");
	this->LaunchAngle.Read(exINI, pSection, "Trajectory.Parabola.LaunchAngle");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.Parabola.LeadTimeCalculate");
	this->LeadTimeSimplify.Read(exINI, pSection, "Trajectory.Parabola.LeadTimeSimplify");
	this->LeadTimeMultiplier.Read(exINI, pSection, "Trajectory.Parabola.LeadTimeMultiplier");
	this->DetonationAngle.Read(exINI, pSection, "Trajectory.Parabola.DetonationAngle");
	this->DetonationHeight.Read(exINI, pSection, "Trajectory.Parabola.DetonationHeight");
	this->BounceTimes.Read(exINI, pSection, "Trajectory.Parabola.BounceTimes");
	this->BounceOnWater.Read(exINI, pSection, "Trajectory.Parabola.BounceOnWater");
	this->BounceDetonate.Read(exINI, pSection, "Trajectory.Parabola.BounceDetonate");
	this->BounceAttenuation.Read(exINI, pSection, "Trajectory.Parabola.BounceAttenuation");
	this->BounceCoefficient.Read(exINI, pSection, "Trajectory.Parabola.BounceCoefficient");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.Parabola.OffsetCoord");
	this->RotateCoord.Read(exINI, pSection, "Trajectory.Parabola.RotateCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Parabola.MirrorCoord");
	this->UseDisperseBurst.Read(exINI, pSection, "Trajectory.Parabola.UseDisperseBurst");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.Parabola.AxisOfRotation");
}

template<typename T>
void ParabolaTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->ThrowHeight)
		.Process(this->BounceTimes)
		.Process(this->OffsetCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->ShouldDetonate)
		.Process(this->ShouldBounce)
		.Process(this->NeedExtraCheck)
		.Process(this->LastTargetCoord)
		.Process(this->CurrentBurst)
		.Process(this->CountOfBurst)
		.Process(this->WaitOneFrame)
		.Process(this->LastVelocity)
		;
}

bool ParabolaTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Serialize(Stm);
	return true;
}

bool ParabolaTrajectory::Save(PhobosStreamWriter& Stm) const
{
	const_cast<ParabolaTrajectory*>(this)->Serialize(Stm);
	return true;
}

void ParabolaTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	const ParabolaTrajectoryType* const pType = this->Type;
	this->LastTargetCoord = pBullet->TargetCoords;
	pBullet->Velocity = BulletVelocity::Empty;
	FootClass* const pTarget = abstract_cast<FootClass*>(pBullet->Target);
	bool resetTarget = false;

	if (static_cast<Leptons>(pType->DetonationDistance) <= -1e-10 && pTarget)
	{
		if (CellClass* const pCell = MapClass::Instance->TryGetCellAt(pTarget->GetCoords()))
		{
			pBullet->Target = pCell;
			pBullet->TargetCoords = pCell->GetCoords();
			resetTarget = true;
		}
	}

	if (WeaponTypeClass* const pWeapon = pBullet->WeaponType)
		this->CountOfBurst = pWeapon->Burst;

	if (TechnoClass* const pOwner = pBullet->Owner)
	{
		this->CurrentBurst = pOwner->CurrentBurstIndex;

		if (pType->MirrorCoord && pOwner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	if (!pType->LeadTimeCalculate || !pTarget || resetTarget)
		this->PrepareForOpenFire(pBullet);
	else
		this->WaitOneFrame.Start(1);
}

bool ParabolaTrajectory::OnAI(BulletClass* pBullet)
{
	if (this->WaitOneFrame.IsTicking() && this->BulletPrepareCheck(pBullet))
		return false;

	if (this->BulletDetonatePreCheck(pBullet))
		return true;

	CellClass* const pCell = MapClass::Instance->TryGetCellAt(pBullet->Location);
	const bool bounce = this->ShouldBounce;

	if (!pCell || (bounce && this->CalculateBulletVelocityAfterBounce(pBullet, pCell)))
		return true;

	return this->BulletDetonateLastCheck(pBullet, pCell, BulletTypeExt::GetAdjustedGravity(pBullet->Type), bounce);
}

void ParabolaTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	const Leptons targetSnapDistance = this->Type->TargetSnapDistance;

	if (targetSnapDistance > 0)
	{
		const ObjectClass* const pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
		const CoordStruct coords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

		if (coords.DistanceFrom(pBullet->Location) <= targetSnapDistance)
		{
			auto const pExt = BulletExt::ExtMap.Find(pBullet);
			pExt->SnappedToTarget = true;
			pBullet->SetLocation(coords);
			return;
		}
	}

	const int cellHeight = MapClass::Instance->GetCellFloorHeight(pBullet->Location);

	if (pBullet->Location.Z < cellHeight)
		pBullet->SetLocation(CoordStruct{ pBullet->Location.X, pBullet->Location.Y, cellHeight });
}

void ParabolaTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // Seems like this is useless
}

TrajectoryCheckReturnType ParabolaTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

TrajectoryCheckReturnType ParabolaTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

void ParabolaTrajectory::PrepareForOpenFire(BulletClass* pBullet)
{
	const ParabolaTrajectoryType* const pType = this->Type;
	const AbstractClass* const pTarget = pBullet->Target;
	bool leadTimeCalculate = pType->LeadTimeCalculate && pTarget;
	CoordStruct theTargetCoords = leadTimeCalculate ? pTarget->GetCoords() : pBullet->TargetCoords;
	CoordStruct theSourceCoords = leadTimeCalculate ? pBullet->Location : pBullet->SourceCoords;
	leadTimeCalculate &= theTargetCoords != this->LastTargetCoord;
	double rotateAngle = 0.0;

	if (!pType->LeadTimeCalculate && theTargetCoords == theSourceCoords && pBullet->Owner) //For disperse.
	{
		const CoordStruct theOwnerCoords = pBullet->Owner->GetCoords();
		rotateAngle = Math::atan2(theTargetCoords.Y - theOwnerCoords.Y , theTargetCoords.X - theOwnerCoords.X);
	}
	else
	{
		rotateAngle = Math::atan2(theTargetCoords.Y - theSourceCoords.Y , theTargetCoords.X - theSourceCoords.X);
	}

	if (this->OffsetCoord != CoordStruct::Empty)
	{
		theTargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(rotateAngle) + this->OffsetCoord.Y * Math::sin(rotateAngle));
		theTargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(rotateAngle) - this->OffsetCoord.Y * Math::cos(rotateAngle));
		theTargetCoords.Z += this->OffsetCoord.Z;
	}

	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		const double offsetMult = 0.0004 * theSourceCoords.DistanceFrom(theTargetCoords);
		const int offsetMin = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Min.Get(Leptons(0)));
		const int offsetMax = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter)));
		const int offsetDistance = ScenarioClass::Instance->Random.RandomRanged(offsetMin, offsetMax);
		theTargetCoords = MapClass::GetRandomCoordsNear(theTargetCoords, offsetDistance, false);
	}

	pBullet->TargetCoords = theTargetCoords;
	const double gravity = BulletTypeExt::GetAdjustedGravity(pBullet->Type);

	if (gravity <= 1e-10)
	{
		pBullet->Velocity = BulletVelocity::Empty;
		this->ShouldDetonate = true;
		return;
	}

	if (leadTimeCalculate)
		this->CalculateBulletVelocityLeadTime(pBullet, &theSourceCoords, gravity);
	else
		this->CalculateBulletVelocityRightNow(pBullet, &theSourceCoords, gravity);

	if (!this->UseDisperseBurst && abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
	{
		const CoordStruct axis = pType->AxisOfRotation;

		BulletVelocity rotationAxis
		{
			axis.X * Math::cos(rotateAngle) + axis.Y * Math::sin(rotateAngle),
			axis.X * Math::sin(rotateAngle) - axis.Y * Math::cos(rotateAngle),
			static_cast<double>(axis.Z)
		};

		const double rotationAxisLengthSquared = rotationAxis.MagnitudeSquared();

		if (abs(rotationAxisLengthSquared) > 1e-10)
		{
			double extraRotate = 0.0;
			rotationAxis *= 1 / sqrt(rotationAxisLengthSquared);

			if (pType->MirrorCoord)
			{
				if (pBullet->Owner && pBullet->Owner->CurrentBurstIndex % 2 == 1)
					rotationAxis *= -1;

				extraRotate = Math::Pi * (pType->RotateCoord * ((this->CurrentBurst / 2) / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}
			else
			{
				extraRotate = Math::Pi * (pType->RotateCoord * (this->CurrentBurst / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}

			const double cosRotate = Math::cos(extraRotate);
			pBullet->Velocity = (pBullet->Velocity * cosRotate) + (rotationAxis * ((1 - cosRotate) * (pBullet->Velocity * rotationAxis))) + (rotationAxis.CrossProduct(pBullet->Velocity) * Math::sin(extraRotate));
		}
	}
}

bool ParabolaTrajectory::BulletPrepareCheck(BulletClass* pBullet)
{
	if (this->WaitOneFrame.HasTimeLeft())
		return true;

	this->PrepareForOpenFire(pBullet);
	this->WaitOneFrame.Stop();

	return false;
}

void ParabolaTrajectory::CalculateBulletVelocityLeadTime(BulletClass* pBullet, CoordStruct* pSourceCoords, double gravity)
{
	const ParabolaTrajectoryType* const pType = this->Type;

	if (pType->LeadTimeSimplify) // Only simple guess, not exact solution
	{
		int leadTime = 0;

		// Step 1: Guess the time of encounter between the projectile and the target based on known conditions
		// Directly assume that the distance between the position where the projectile hits the target and the starting point is 4 grids
		switch (pType->OpenFireMode)
		{
		case ParabolaFireMode::Height:
		case ParabolaFireMode::HeightAndAngle:
		{
			// Assuming equal height
			leadTime = static_cast<int>(sqrt((this->ThrowHeight << 1) / gravity) * 1.25);
			break;
		}
		case ParabolaFireMode::Angle:
		{
			double radian = pType->LaunchAngle * Math::Pi / 180.0;
			radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;
			const double factor = Math::cos(radian);

			// Check if the angle is appropriate
			if (abs(factor) < 1e-10)
				break;

			const double mult = Math::sin(2 * radian);

			// Check if the angle is appropriate again
			if (abs(mult) < 1e-10)
				break;

			const double velocity = sqrt((Unsorted::LeptonsPerCell << 2) * gravity / mult);

			// Assuming equal height
			leadTime = static_cast<int>((Unsorted::LeptonsPerCell << 2) / (velocity * factor));
			break;
		}
		default:
		{
			// Assuming equal height
			leadTime = static_cast<int>((Unsorted::LeptonsPerCell << 2) / pType->Trajectory_Speed);
			break;
		}
		}

		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (pBullet->Target->GetCoords() - this->LastTargetCoord) * (pType->LeadTimeMultiplier * leadTime);

		// Step 3: Calculate the parabolic starting point vector
		this->CalculateBulletVelocityRightNow(pBullet, pSourceCoords, gravity);
		return;
	}

	CoordStruct targetCoords = pBullet->Target->GetCoords();
	CoordStruct offsetCoords = pBullet->TargetCoords - targetCoords;

	// A coefficient that should not exist here normally, but even so, there are still errors
	const double speedFixMult = pType->LeadTimeMultiplier * 0.75;

	switch (pType->OpenFireMode)
	{
	case ParabolaFireMode::Height: // Fixed max height and aim at the target
	{
		// Step 1: Using Newton Iteration Method to determine the time of encounter between the projectile and the target
		const double meetTime = this->SearchFixedHeightMeetTime(pSourceCoords, &targetCoords, &offsetCoords, gravity);

		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;

		// Step 4: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = destinationCoords.X / meetTime;
		pBullet->Velocity.Y = destinationCoords.Y / meetTime;

		// Step 5: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = sourceHeight + destinationCoords.Z;
		const int maxHeight = destinationCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;

		// Step 6: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight)) + gravity / 2;

		// Step 7: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck(pBullet);
		return;
	}
	case ParabolaFireMode::Angle: // Fixed fire angle and aim at the target
	{
		// Step 1: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;

		// Step 2: Using Newton Iteration Method to determine the time of encounter between the projectile and the target
		const double meetTime = this->SearchFixedAngleMeetTime(pSourceCoords, &targetCoords, &offsetCoords, radian, gravity);

		// Step 3: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 4: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;

		// Step 5: Calculate each horizontal component of the projectile velocity
		pBullet->Velocity.X = destinationCoords.X / meetTime;
		pBullet->Velocity.Y = destinationCoords.Y / meetTime;

		// Step 6: Calculate whole horizontal component of the projectile velocity
		const double horizontalDistance = Point2D{ destinationCoords.X, destinationCoords.Y }.Magnitude();
		const double horizontalVelocity = horizontalDistance / meetTime;

		// Step 7: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = horizontalVelocity * Math::tan(radian) + gravity / 2;

		// Step 8: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck(pBullet);
		return;
	}
	case ParabolaFireMode::SpeedAndHeight: // Fixed horizontal speed and fixed max height
	{
		// Step 1: Calculate the time when the projectile meets the target directly using horizontal velocity
		const double meetTime = this->SolveFixedSpeedMeetTime(pSourceCoords, &targetCoords, &offsetCoords, pType->Trajectory_Speed);

		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;

		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const double horizontalDistance = Point2D{ destinationCoords.X, destinationCoords.Y }.Magnitude();
		const double mult = horizontalDistance > 1e-10 ? pType->Trajectory_Speed / horizontalDistance : 1.0;

		// Step 5: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = destinationCoords.X * mult;
		pBullet->Velocity.Y = destinationCoords.Y * mult;

		// Step 6: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = sourceHeight + destinationCoords.Z;
		const int maxHeight = destinationCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;

		// Step 7: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight)) + gravity / 2;

		// Step 8: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck(pBullet);
		return;
	}
	case ParabolaFireMode::HeightAndAngle: // Fixed max height and fixed fire angle
	{
		// Step 1: Using Newton Iteration Method to determine the time of encounter between the projectile and the target
		const double meetTime = this->SearchFixedHeightMeetTime(pSourceCoords, &targetCoords, &offsetCoords, gravity);

		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;

		// Step 4: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = sourceHeight + destinationCoords.Z;
		const int maxHeight = destinationCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;

		// Step 5: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight)) + gravity / 2;

		// Step 6: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= 1e-10) ? (Math::HalfPi / 3) : radian;

		// Step 7: Calculate the ratio of horizontal velocity to horizontal distance
		const double horizontalDistance = Point2D{ destinationCoords.X, destinationCoords.Y }.Magnitude();
		const double mult = (pBullet->Velocity.Z / Math::tan(radian)) / horizontalDistance;

		// Step 8: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = destinationCoords.X * mult;
		pBullet->Velocity.Y = destinationCoords.Y * mult;

		// Step 9: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck(pBullet);
		return;
	}
	case ParabolaFireMode::SpeedAndAngle: // Fixed horizontal speed and fixed fire angle
	{
		// Step 1: Calculate the time when the projectile meets the target directly using horizontal velocity
		const double meetTime = this->SolveFixedSpeedMeetTime(pSourceCoords, &targetCoords, &offsetCoords, pType->Trajectory_Speed);

		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;

		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const double horizontalDistance = Point2D{ destinationCoords.X, destinationCoords.Y }.Magnitude();
		const double mult = horizontalDistance > 1e-10 ? pType->Trajectory_Speed / horizontalDistance : 1.0;

		// Step 5: Calculate each horizontal component of the projectile velocity
		pBullet->Velocity.X = destinationCoords.X * mult;
		pBullet->Velocity.Y = destinationCoords.Y * mult;

		// Step 6: Calculate whole horizontal component of the projectile velocity
		const double horizontalVelocity = horizontalDistance * mult;

		// Step 7: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;

		// Step 8: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = horizontalVelocity * Math::tan(radian) + gravity / 2;

		// Step 9: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck(pBullet);
		return;
	}
	default: // Fixed horizontal speed and aim at the target
	{
		// Step 1: Calculate the time when the projectile meets the target directly using horizontal velocity
		const double meetTime = this->SolveFixedSpeedMeetTime(pSourceCoords, &targetCoords, &offsetCoords, pType->Trajectory_Speed);

		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;

		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const double horizontalDistance = Point2D{ destinationCoords.X, destinationCoords.Y }.Magnitude();
		const double mult = horizontalDistance > 1e-10 ? pType->Trajectory_Speed / horizontalDistance : 1.0;

		// Step 5: Calculate the projectile velocity
		pBullet->Velocity.X = destinationCoords.X * mult;
		pBullet->Velocity.Y = destinationCoords.Y * mult;
		pBullet->Velocity.Z = destinationCoords.Z * mult + (gravity * horizontalDistance) / (2 * pType->Trajectory_Speed) + gravity / 2;

		// Step 6: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck(pBullet);
		return;
	}
	}

	// Reset target position
	pBullet->TargetCoords = targetCoords + offsetCoords;

	// Substitute into the no lead time algorithm
	this->CalculateBulletVelocityRightNow(pBullet, pSourceCoords, gravity);
}

void ParabolaTrajectory::CalculateBulletVelocityRightNow(BulletClass* pBullet, CoordStruct* pSourceCoords, double gravity)
{
	const ParabolaTrajectoryType* const pType = this->Type;
	// Calculate horizontal distance
	const CoordStruct distanceCoords = pBullet->TargetCoords - *pSourceCoords;
	const double distance = distanceCoords.Magnitude();
	const double horizontalDistance = Point2D{ distanceCoords.X, distanceCoords.Y }.Magnitude();

	if (distance <= 1e-10)
	{
		pBullet->Velocity = BulletVelocity::Empty;
		this->ShouldDetonate = true;
		return;
	}

	switch (pType->OpenFireMode)
	{
	case ParabolaFireMode::Height: // Fixed max height and aim at the target
	{
		// Step 1: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = pBullet->TargetCoords.Z;
		const int maxHeight = distanceCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;

		// Step 2: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight));

		// Step 3: Calculate the total time it takes for the projectile to meet the target using the heights of the ascending and descending phases
		const double meetTime = sqrt(2 * (maxHeight - sourceHeight) / gravity) + sqrt(2 * (maxHeight - targetHeight) / gravity);

		// Step 4: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = distanceCoords.X / meetTime;
		pBullet->Velocity.Y = distanceCoords.Y / meetTime;
		break;
	}
	case ParabolaFireMode::Angle: // Fixed fire angle and aim at the target
	{
		// Step 1: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;

		// Step 2: Using Newton Iteration Method to determine the projectile velocity
		double velocity = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? 100.0 : this->SearchVelocity(horizontalDistance, distanceCoords.Z, radian, gravity);

		// Step 3: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = velocity * Math::sin(radian);

		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const double mult = velocity * Math::cos(radian) / horizontalDistance;

		// Step 5: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = distanceCoords.X * mult;
		pBullet->Velocity.Y = distanceCoords.Y * mult;
		break;
	}
	case ParabolaFireMode::SpeedAndHeight: // Fixed horizontal speed and fixed max height
	{
		// Step 1: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = pBullet->TargetCoords.Z;
		const int maxHeight = distanceCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;

		// Step 2: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight));

		// Step 3: Calculate the ratio of horizontal velocity to horizontal distance
		const double mult = horizontalDistance > 1e-10 ? pType->Trajectory_Speed / horizontalDistance : 1.0;

		// Step 4: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = distanceCoords.X * mult;
		pBullet->Velocity.Y = distanceCoords.Y * mult;
		break;
	}
	case ParabolaFireMode::HeightAndAngle: // Fixed max height and fixed fire angle
	{
		// Step 1: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = pBullet->TargetCoords.Z;
		const int maxHeight = distanceCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;

		// Step 2: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight));

		// Step 3: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= 1e-10) ? (Math::HalfPi / 3) : radian;

		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const double mult = (pBullet->Velocity.Z / Math::tan(radian)) / horizontalDistance;

		// Step 5: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = distanceCoords.X * mult;
		pBullet->Velocity.Y = distanceCoords.Y * mult;
		break;
	}
	case ParabolaFireMode::SpeedAndAngle: // Fixed horizontal speed and fixed fire angle
	{
		// Step 1: Calculate the ratio of horizontal velocity to horizontal distance
		const double mult = horizontalDistance > 1e-10 ? pType->Trajectory_Speed / horizontalDistance : 1.0;

		// Step 2: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = distanceCoords.X * mult;
		pBullet->Velocity.Y = distanceCoords.Y * mult;

		// Step 3: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;

		// Step 4: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = pType->Trajectory_Speed * Math::tan(radian);
		break;
	}
	default: // Fixed horizontal speed and aim at the target
	{
		// Step 1: Calculate the ratio of horizontal velocity to horizontal distance
		const double mult = horizontalDistance > 1e-10 ? pType->Trajectory_Speed / horizontalDistance : 1.0;

		// Step 2: Calculate the projectile velocity
		pBullet->Velocity.X = distanceCoords.X * mult;
		pBullet->Velocity.Y = distanceCoords.Y * mult;
		pBullet->Velocity.Z = distanceCoords.Z * mult + (gravity * horizontalDistance) / (2 * pType->Trajectory_Speed);
		break;
	}
	}

	// Record whether it requires additional checks during the flight
	this->CheckIfNeedExtraCheck(pBullet);

	// Offset the gravity effect of the first time update
	pBullet->Velocity.Z += gravity / 2;
}

void ParabolaTrajectory::CheckIfNeedExtraCheck(BulletClass* pBullet)
{
	const ParabolaTrajectoryType* const pType = this->Type;

	switch (pType->OpenFireMode)
	{
	case ParabolaFireMode::Height: // Fixed max height and aim at the target
	case ParabolaFireMode::Angle: // Fixed fire angle and aim at the target
	case ParabolaFireMode::HeightAndAngle: // Fixed max height and fixed fire angle
	{
		this->NeedExtraCheck = Vector2D<double>{ pBullet->Velocity.X, pBullet->Velocity.Y }.MagnitudeSquared() > 65536.0;
		break;
	}
	default: // Fixed horizontal speed and blabla
	{
		this->NeedExtraCheck = pType->Trajectory_Speed > 256.0;
		break;
	}
	}
}

double ParabolaTrajectory::SearchVelocity(double horizontalDistance, int distanceCoordsZ, double radian, double gravity)
{
	// Estimate initial velocity
	const double mult = Math::sin(2 * radian);
	double velocity = abs(mult) > 1e-10 ? sqrt(horizontalDistance * gravity / mult) : 0.0;
	velocity += distanceCoordsZ / gravity;
	velocity = velocity > 8.0 ? velocity : 8.0;
	const double error = velocity / 16;

	// Step size
	const double delta = 1e-5;

	// Newton Iteration Method
	for (int i = 0; i < 10; ++i)
	{
		// Substitute into the estimate speed
		const double differential = this->CheckVelocityEquation(horizontalDistance, distanceCoordsZ, velocity, radian, gravity);
		const double dDifferential = (this->CheckVelocityEquation(horizontalDistance, distanceCoordsZ, (velocity + delta), radian, gravity) - differential) / delta;

		// Check unacceptable divisor
		if (abs(dDifferential) < 1e-10)
			return velocity;

		// Calculate the speed of the next iteration
		const double difference = differential / dDifferential;
		const double velocityNew = velocity - difference;

		// Check tolerable error
		if (abs(difference) < error)
			return velocityNew;

		// Update the speed
		velocity = velocityNew;
	}

	// Unsolvable
	return 10.0;
}

double ParabolaTrajectory::CheckVelocityEquation(double horizontalDistance, int distanceCoordsZ, double velocity, double radian, double gravity)
{
	// Calculate each component of the projectile velocity
	const double horizontalVelocity = velocity * Math::cos(radian);
	const double verticalVelocity = velocity * Math::sin(radian);

	// Calculate the time of the rising phase
	const double upTime = verticalVelocity / gravity;

	// Calculate the maximum height that the projectile can reach
	const double maxHeight = 0.5 * verticalVelocity * upTime;

	// Calculate the time of the descent phase
	const double downTime = sqrt(2 * (maxHeight - distanceCoordsZ) / gravity);

	// Calculate the total time required for horizontal movement
	const double wholeTime = horizontalDistance / horizontalVelocity;

	// Calculate the difference between the total vertical motion time and the total horizontal motion time
	return wholeTime - (upTime + downTime);
}

double ParabolaTrajectory::SolveFixedSpeedMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double horizontalSpeed)
{
	// Project all conditions onto a horizontal plane
	const Point2D targetSpeedCrd { pTargetCrd->X - this->LastTargetCoord.X, pTargetCrd->Y - this->LastTargetCoord.Y };
	const Point2D destinationCrd { pTargetCrd->X + pOffsetCrd->X - pSourceCrd->X, pTargetCrd->Y + pOffsetCrd->Y - pSourceCrd->Y };

	// Establishing a quadratic equation using time as a variable:
	// (destinationCrd + targetSpeedCrd * time).Magnitude() = horizontalSpeed * time

	// Solve this quadratic equation
	const double divisor = (targetSpeedCrd.MagnitudeSquared() - horizontalSpeed * horizontalSpeed) * 2;
	const double factor = 2 * (targetSpeedCrd * destinationCrd);
	const double delta = factor * factor - 2 * divisor * destinationCrd.MagnitudeSquared();

	if (delta >= 1e-10)
	{
		const double timeP = (-factor + sqrt(delta)) / divisor;
		const double timeM = (-factor - sqrt(delta)) / divisor;

		if (timeM > 1e-10)
			return timeM;

		return timeP;
	}

	return -1.0;
}

double ParabolaTrajectory::SearchFixedHeightMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double gravity)
{
	// Similar to method SearchVelocity, no further elaboration will be provided
	const double delta = 1e-5;
	double meetTime = (this->ThrowHeight << 2) / gravity;

	for (int i = 0; i < 10; ++i)
	{
		const double differential = this->CheckFixedHeightEquation(pSourceCrd, pTargetCrd, pOffsetCrd, meetTime, gravity);
		const double dDifferential = (this->CheckFixedHeightEquation(pSourceCrd, pTargetCrd, pOffsetCrd, (meetTime + delta), gravity) - differential) / delta;

		if (abs(dDifferential) < 1e-10)
			return meetTime;

		const double difference = differential / dDifferential;
		const double meetTimeNew = meetTime - difference;

		if (abs(difference) < 1.0)
			return meetTimeNew;

		meetTime = meetTimeNew;
	}

	return -1.0;
}

double ParabolaTrajectory::CheckFixedHeightEquation(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double meetTime, double gravity)
{
	// Calculate how high the target will reach during this period of time
	const int meetHeight = static_cast<int>((pTargetCrd->Z - this->LastTargetCoord.Z) * meetTime) + pTargetCrd->Z + pOffsetCrd->Z;

	// Calculate how high the projectile can fly during this period of time
	const int maxHeight = meetHeight > pSourceCrd->Z ? this->ThrowHeight + meetHeight : this->ThrowHeight + pSourceCrd->Z;

	// Calculate the difference between these two times
	return sqrt((maxHeight - pSourceCrd->Z) * 2 / gravity) + sqrt((maxHeight - meetHeight) * 2 / gravity) - meetTime;
}

double ParabolaTrajectory::SearchFixedAngleMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double radian, double gravity)
{
	// Similar to method SearchVelocity, no further elaboration will be provided
	const double delta = 1e-5;
	double meetTime = 512 * Math::sin(radian) / gravity;

	for (int i = 0; i < 10; ++i)
	{
		const double differential = this->CheckFixedAngleEquation(pSourceCrd, pTargetCrd, pOffsetCrd, meetTime, radian, gravity);
		const double dDifferential = (this->CheckFixedAngleEquation(pSourceCrd, pTargetCrd, pOffsetCrd, (meetTime + delta), radian, gravity) - differential) / delta;

		if (abs(dDifferential) < 1e-10)
			return meetTime;

		const double difference = differential / dDifferential;
		const double meetTimeNew = meetTime - difference;

		if (abs(difference) < 1.0)
			return meetTimeNew;

		meetTime = meetTimeNew;
	}

	return -1.0;
}

double ParabolaTrajectory::CheckFixedAngleEquation(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double meetTime, double radian, double gravity)
{
	// Using the estimated time to obtain the predicted location of the target
	const CoordStruct distanceCoords = (*pTargetCrd - this->LastTargetCoord) * meetTime + *pTargetCrd + *pOffsetCrd - *pSourceCrd;

	// Calculate the horizontal distance between the target and the calculation
	const double horizontalDistance = Point2D{ distanceCoords.X, distanceCoords.Y }.Magnitude();

	// Calculate the horizontal velocity
	const double horizontalVelocity = horizontalDistance / meetTime;

	// Calculate the vertical velocity
	const double verticalVelocity = horizontalVelocity * Math::tan(radian);

	// Calculate the time of the rising phase
	const double upTime = verticalVelocity / gravity;

	// Calculate the maximum height that the projectile can reach
	const double maxHeight = 0.5 * verticalVelocity * upTime;

	// Calculate the time of the descent phase
	const double downTime = sqrt(2 * (maxHeight - distanceCoords.Z) / gravity);

	// Calculate the difference between the actual flight time of the projectile obtained and the initially estimated time
	return upTime + downTime - meetTime;
}

bool ParabolaTrajectory::CalculateBulletVelocityAfterBounce(BulletClass* pBullet, CellClass* pCell)
{
	const ParabolaTrajectoryType* const pType = this->Type;

	if (pCell->LandType == LandType::Water && !pType->BounceOnWater)
		return true;

	--this->BounceTimes;
	this->ShouldBounce = false;

	const BulletVelocity groundNormalVector = this->GetGroundNormalVector(pBullet, pCell);
	pBullet->Velocity = (this->LastVelocity - groundNormalVector * (this->LastVelocity * groundNormalVector) * 2) * pType->BounceCoefficient;

	if (pType->BounceDetonate)
	{
		TechnoClass* const pFirer = pBullet->Owner;
		HouseClass* const pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		WarheadTypeExt::DetonateAt(pBullet->WH, pBullet->Location, pFirer, pBullet->Health, pOwner);
	}

	if (const int damage = pBullet->Health)
	{
		if (const int newDamage = static_cast<int>(damage * pType->BounceAttenuation))
			pBullet->Health = newDamage;
		else
			pBullet->Health = damage > 0 ? 1 : -1;
	}

	return false;
}

BulletVelocity ParabolaTrajectory::GetGroundNormalVector(BulletClass* pBullet, CellClass* pCell)
{
	if (const unsigned char index = pCell->SlopeIndex)
	{
		Vector2D<double> factor { 0.0, 0.0 };

		// 0.3763770469559380854890894443664 -> Unsorted::LevelHeight / sqrt(Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)
		// 0.9264665771223091335116047861327 -> Unsorted::LeptonsPerCell / sqrt(Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)
		// 0.3522530794922131411764879370407 -> Unsorted::LevelHeight / sqrt(2 * Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)
		// 0.8670845033654477321267395373309 -> Unsorted::LeptonsPerCell / sqrt(2 * Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)
		// 0.5333964609104418418483761938761 -> Unsorted::CellHeight / sqrt(2 * Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)
		// 0.6564879518897745745826168540013 -> Unsorted::LeptonsPerCell / sqrt(2 * Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)
		if (index <= 4)
			factor = Vector2D<double>{ 0.3763770469559380854890894443664, 0.9264665771223091335116047861327 };
		else if (index <= 12)
			factor = Vector2D<double>{ 0.3522530794922131411764879370407, 0.8670845033654477321267395373309 };
		else
			factor = Vector2D<double>{ 0.5333964609104418418483761938761, 0.6564879518897745745826168540013 };

		switch (index)
		{
		case 1:
			return BulletVelocity{ -factor.X, 0.0, factor.Y };
		case 2:
			return BulletVelocity{ 0.0, -factor.X, factor.Y };
		case 3:
			return BulletVelocity{ factor.X, 0.0, factor.Y };
		case 4:
			return BulletVelocity{ 0.0, factor.X, factor.Y };
		case 5:
		case 9:
		case 13:
			return BulletVelocity{ -factor.X, -factor.X, factor.Y };
		case 6:
		case 10:
		case 14:
			return BulletVelocity{ factor.X, -factor.X, factor.Y };
		case 7:
		case 11:
		case 15:
			return BulletVelocity{ factor.X, factor.X, factor.Y };
		case 8:
		case 12:
		case 16:
			return BulletVelocity{ -factor.X, factor.X, factor.Y };
		default:
			return BulletVelocity{ 0.0, 0.0, 1.0 };
		}
	}

	// 362.1 -> Unsorted::LeptonsPerCell * sqrt(2)
	const double horizontalVelocity = Vector2D<double>{ pBullet->Velocity.X, pBullet->Velocity.Y }.Magnitude();
	const BulletVelocity velocity = horizontalVelocity > 362.1 ? pBullet->Velocity * (362.1 / horizontalVelocity) : pBullet->Velocity;
	const CoordStruct velocityCoords { static_cast<int>(velocity.X), static_cast<int>(velocity.Y), static_cast<int>(velocity.Z) };

	const int cellHeight = pCell->Level * Unsorted::LevelHeight;
	const int bulletHeight = pBullet->Location.Z;
	const int lastCellHeight = MapClass::Instance->GetCellFloorHeight(pBullet->Location - velocityCoords);

	if (bulletHeight < cellHeight && (cellHeight - lastCellHeight) > 384)
	{
		CellStruct cell = pCell->MapCoords;
		const short reverseSgnX = pBullet->Velocity.X > 0.0 ? -1 : 1;
		const short reverseSgnY = pBullet->Velocity.Y > 0.0 ? -1 : 1;
		int index = 0;

		if (this->CheckBulletHitCliff(cell.X + reverseSgnX, cell.Y, bulletHeight, lastCellHeight))
		{
			if (!this->CheckBulletHitCliff(cell.X, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
			{
				if (!this->CheckBulletHitCliff(cell.X - reverseSgnX, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
					return BulletVelocity{ 0.0, static_cast<double>(reverseSgnY), 0.0 };

				index = 2;
			}
		}
		else
		{
			if (this->CheckBulletHitCliff(cell.X + reverseSgnX, cell.Y - reverseSgnY, bulletHeight, lastCellHeight))
			{
				if (this->CheckBulletHitCliff(cell.X, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
					index = 1;
				else if (!this->CheckBulletHitCliff(cell.X - reverseSgnX, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
					index = 2;
			}
			else
			{
				if (this->CheckBulletHitCliff(cell.X, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
					return BulletVelocity{ static_cast<double>(reverseSgnX), 0.0, 0.0 };
				else if (this->CheckBulletHitCliff(cell.X - reverseSgnX, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
					index = 1;
			}
		}

		// 0.4472135954999579392818347337463 -> 1 / sqrt(5)
		// 0.8944271909999158785636694674925 -> 2 / sqrt(5)
		if (index == 1)
			return BulletVelocity{ 0.8944271909999158785636694674925 * reverseSgnX, 0.4472135954999579392818347337463 * reverseSgnY, 0.0 };
		else if (index == 2)
			return BulletVelocity{ 0.4472135954999579392818347337463 * reverseSgnX, 0.8944271909999158785636694674925 * reverseSgnY, 0.0 };

		// 0.7071067811865475244008443621049 -> 1 / sqrt(2)
		return BulletVelocity{ 0.7071067811865475244008443621049 * reverseSgnX, 0.7071067811865475244008443621049 * reverseSgnY, 0.0 };
	}

	return BulletVelocity{ 0.0, 0.0, 1.0 };
}

bool ParabolaTrajectory::CheckBulletHitCliff(short X, short Y, int bulletHeight, int lastCellHeight)
{
	if (CellClass* const pCell = MapClass::Instance->TryGetCellAt(CellStruct{ X, Y }))
	{
		const int cellHeight = pCell->Level * Unsorted::LevelHeight;

		if (bulletHeight < cellHeight && (cellHeight - lastCellHeight) > 384)
			return true;
	}

	return false;
}

bool ParabolaTrajectory::BulletDetonatePreCheck(BulletClass* pBullet)
{
	if (this->ShouldDetonate)
		return true;

	const ParabolaTrajectoryType* const pType = this->Type;

	if (pType->DetonationHeight >= 0 && pBullet->Velocity.Z < 1e-10 && (pBullet->Location.Z - pBullet->SourceCoords.Z) < pType->DetonationHeight)
		return true;

	if (abs(pType->DetonationAngle) < 1e-10)
	{
		if (pBullet->Velocity.Z < 1e-10)
			return true;
	}
	else if (abs(pType->DetonationAngle) < 90.0)
	{
		const double horizontalVelocity = Vector2D<double>{ pBullet->Velocity.X, pBullet->Velocity.Y }.Magnitude();

		if (horizontalVelocity > 1e-10)
		{
			if ((pBullet->Velocity.Z / horizontalVelocity) < Math::tan(pType->DetonationAngle * Math::Pi / 180.0))
				return true;
		}
		else if (pType->DetonationAngle > 1e-10 || pBullet->Velocity.Z < 1e-10)
		{
			return true;
		}
	}

	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < static_cast<Leptons>(pType->DetonationDistance))
		return true;

	return false;
}

bool ParabolaTrajectory::BulletDetonateLastCheck(BulletClass* pBullet, CellClass* pCell, double gravity, bool bounce)
{
	pBullet->Velocity.Z -= gravity;

	const CoordStruct velocityCoords { static_cast<int>(pBullet->Velocity.X), static_cast<int>(pBullet->Velocity.Y), static_cast<int>(pBullet->Velocity.Z) };
	const CoordStruct futureCoords = pBullet->Location + velocityCoords;

	if (this->NeedExtraCheck)
	{
		const CellStruct cellDist = CellClass::Coord2Cell(pBullet->Location) - CellClass::Coord2Cell(futureCoords);
		const CellStruct cellPace = CellStruct { static_cast<short>(std::abs(cellDist.X)), static_cast<short>(std::abs(cellDist.Y)) };
		const size_t largePace = static_cast<size_t>(std::max(cellPace.X, cellPace.Y));
		const CoordStruct stepCoord = largePace ? velocityCoords * (1.0 / largePace) : CoordStruct::Empty;
		CoordStruct curCoord = pBullet->Location + stepCoord;

		for (size_t i = 1; i <= largePace; ++i)
		{
			const int cellHeight = MapClass::Instance->GetCellFloorHeight(curCoord);

			if (curCoord.Z < cellHeight)
			{
				if (bounce)
					return true;

				this->LastVelocity = pBullet->Velocity;
				this->BulletDetonateEffectuate(pBullet, (static_cast<double>(i - 0.5) / largePace));
				break;
			}

			if (pBullet->Type->SubjectToWalls && pCell->OverlayTypeIndex != -1 && OverlayTypeClass::Array->GetItem(pCell->OverlayTypeIndex)->Wall)
			{
				pBullet->Velocity *= static_cast<double>(i) / largePace;
				this->ShouldDetonate = true;
				return false;
			}

			curCoord += stepCoord;
			pCell = MapClass::Instance->GetCellAt(curCoord);
		}
	}
	else
	{
		const int cellHeight = MapClass::Instance->GetCellFloorHeight(futureCoords);

		if (cellHeight < futureCoords.Z)
			return false;

		if (bounce)
			return true;

		this->LastVelocity = pBullet->Velocity;
		this->BulletDetonateEffectuate(pBullet, abs((pBullet->Location.Z - cellHeight) / pBullet->Velocity.Z));
	}

	return false;
}

void ParabolaTrajectory::BulletDetonateEffectuate(BulletClass* pBullet, double velocityMult)
{
	if (velocityMult < 1.0)
		pBullet->Velocity *= velocityMult;

	if (this->BounceTimes > 0)
		this->ShouldBounce = true;
	else
		this->ShouldDetonate = true;
}
