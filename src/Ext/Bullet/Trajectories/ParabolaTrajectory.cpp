#include "ParabolaTrajectory.h"

#include <OverlayTypeClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>

namespace detail
{
	template <>
	inline bool read<ParabolaFireMode>(ParabolaFireMode& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			static std::pair<const char*, ParabolaFireMode> FlagNames[] =
			{
				{"Speed", ParabolaFireMode::Speed},
				{"Height", ParabolaFireMode::Height},
				{"Angle", ParabolaFireMode::Angle},
				{"SpeedAndHeight", ParabolaFireMode::SpeedAndHeight},
				{"HeightAndAngle", ParabolaFireMode::HeightAndAngle},
				{"SpeedAndAngle", ParabolaFireMode::SpeedAndAngle},
			};
			for (auto [name, flag] : FlagNames)
			{
				if (_strcmpi(parser.value(), name) == 0)
				{
					value = flag;
					return true;
				}
			}
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a new parabola fire mode");
		}

		return false;
	}
}

std::unique_ptr<PhobosTrajectory> ParabolaTrajectoryType::CreateInstance(BulletClass* pBullet) const
{
	return std::make_unique<ParabolaTrajectory>(this, pBullet);
}

template<typename T>
void ParabolaTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->OpenFireMode)
		.Process(this->ThrowHeight)
		.Process(this->LaunchAngle)
		.Process(this->DetonationAngle)
		.Process(this->BounceTimes)
		.Process(this->BounceOnWater)
		.Process(this->BounceDetonate)
		.Process(this->BounceAttenuation)
		.Process(this->BounceCoefficient)
		;
}

bool ParabolaTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->ActualTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool ParabolaTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->ActualTrajectoryType::Save(Stm);
	const_cast<ParabolaTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void ParabolaTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->PhobosTrajectoryType::Read(pINI, pSection);
	INI_EX exINI(pINI);
	// Actual
	this->RotateCoord.Read(exINI, pSection, "Trajectory.RotateCoord");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.OffsetCoord");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.AxisOfRotation");
	this->LeadTimeMaximum.Read(exINI, pSection, "Trajectory.LeadTimeMaximum");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.LeadTimeCalculate");
	this->EarlyDetonation.Read(exINI, pSection, "Trajectory.EarlyDetonation");
	this->DetonationHeight.Read(exINI, pSection, "Trajectory.DetonationHeight");
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.TargetSnapDistance");
	// Parabola
	this->OpenFireMode.Read(exINI, pSection, "Trajectory.Parabola.OpenFireMode");
	this->ThrowHeight.Read(exINI, pSection, "Trajectory.Parabola.ThrowHeight");
	this->LaunchAngle.Read(exINI, pSection, "Trajectory.Parabola.LaunchAngle");
	this->DetonationAngle.Read(exINI, pSection, "Trajectory.Parabola.DetonationAngle");
	this->BounceTimes.Read(exINI, pSection, "Trajectory.Parabola.BounceTimes");
	this->BounceOnWater.Read(exINI, pSection, "Trajectory.Parabola.BounceOnWater");
	this->BounceDetonate.Read(exINI, pSection, "Trajectory.Parabola.BounceDetonate");
	this->BounceAttenuation.Read(exINI, pSection, "Trajectory.Parabola.BounceAttenuation");
	this->BounceCoefficient.Read(exINI, pSection, "Trajectory.Parabola.BounceCoefficient");
}

template<typename T>
void ParabolaTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->ThrowHeight)
		.Process(this->BounceTimes)
		.Process(this->ShouldBounce)
		.Process(this->LastVelocity)
		;
}

bool ParabolaTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->ActualTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool ParabolaTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->ActualTrajectory::Save(Stm);
	const_cast<ParabolaTrajectory*>(this)->Serialize(Stm);
	return true;
}

void ParabolaTrajectory::OnUnlimbo()
{
	this->ActualTrajectory::OnUnlimbo();
	// Parabola
	this->RemainingDistance = INT_MAX;
	const auto pBullet = this->Bullet;
	// Special case: Set the target to the ground
	if (this->Type->DetonationDistance.Get() <= -1e-10)
	{
		const auto pTarget = pBullet->Target;

		if (pTarget->AbstractFlags & AbstractFlags::Foot)
		{
			if (const auto pCell = MapClass::Instance.TryGetCellAt(pTarget->GetCoords()))
			{
				pBullet->Target = pCell;
				pBullet->TargetCoords = pCell->GetCoords();
			}
		}
	}
	// Waiting for launch trigger
	if (!BulletExt::ExtMap.Find(pBullet)->DispersedTrajectory)
		this->OpenFire();
}

bool ParabolaTrajectory::OnVelocityCheck()
{
	const auto pBullet = this->Bullet;
	// Affected by gravity
	this->MovingVelocity.Z -= BulletTypeExt::GetAdjustedGravity(pBullet->Type);
	this->MovingSpeed = this->MovingVelocity.Magnitude();
	// Adopting independent logic
	double ratio = 1.0;
	int velocityCheck = 0;

	const auto pType = this->Type;
	const bool checkThrough = (!pType->ThroughBuilding || !pType->ThroughVehicles);
	const auto velocity = PhobosTrajectory::Get2DVelocity(this->MovingVelocity);
	// Low speed with checkSubject was already done well
	if (velocity < Unsorted::LeptonsPerCell)
	{
		// Blocked by obstacles?
		if (checkThrough)
		{
			const auto pFirer = pBullet->Owner;
			const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
			// Check for additional obstacles on the ground
			if (this->CheckThroughAndSubjectInCell(MapClass::Instance.GetCellAt(pBullet->Location), pOwner))
			{
				if (32.0 < velocity)
					ratio = (32.0 / velocity);

				velocityCheck = 2;
			}
		}
		// Check whether about to fall into the ground
		if (this->BounceTimes > 0 || std::abs(this->MovingVelocity.Z) > Unsorted::CellHeight)
		{
			const auto theTargetCoords = pBullet->Location + PhobosTrajectory::Vector2Coord(this->MovingVelocity);
			const auto cellHeight = MapClass::Instance.GetCellFloorHeight(theTargetCoords);
			// Check whether the height of the ground is about to exceed the height of the projectile
			if (cellHeight >= theTargetCoords.Z)
			{
				// How much reduction is needed to calculate the velocity vector
				const auto newRatio = std::abs((pBullet->Location.Z - cellHeight) / this->MovingVelocity.Z);
				// Only when the proportion is smaller, it needs to be recorded
				if (ratio > newRatio)
					ratio = newRatio;

				velocityCheck = 1;
			}
		}
	}
	else
	{
		// When in high speed, it's necessary to check each cell on the path that the next frame will pass through
		double locationDistance = 0.0;
		const auto pBulletType = pBullet->Type;
		// Anyway, at least check the ground
		const auto& theSourceCoords = pBullet->Location;
		const auto theTargetCoords = theSourceCoords + PhobosTrajectory::Vector2Coord(this->MovingVelocity);
		const auto pFirer = pBullet->Owner;
		const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		// No need to use these variables anymore
		{
			const auto pSourceCell = MapClass::Instance.GetCellAt(theSourceCoords);
			const auto sourceCell = pSourceCell->MapCoords;
			const auto pTargetCell = MapClass::Instance.GetCellAt(theTargetCoords);
			const auto targetCell = pTargetCell->MapCoords;
			auto pLastCell = MapClass::Instance.GetCellAt(pBullet->LastMapCoords);
			const auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBulletType);
			const bool subjectToWCS = pBulletType->SubjectToWalls || pBulletType->SubjectToCliffs || pBulletTypeExt->SubjectToSolid;
			const bool checkLevel = !pBulletTypeExt->SubjectToLand.isset() && !pBulletTypeExt->SubjectToWater.isset();
			const auto cellDist = sourceCell - targetCell;
			const auto cellPace = CellStruct { static_cast<short>(std::abs(cellDist.X)), static_cast<short>(std::abs(cellDist.Y)) };
			// Take big steps as much as possible to reduce check times, just ensure that each cell is inspected
			auto largePace = static_cast<size_t>(Math::max(cellPace.X, cellPace.Y));
			const auto stepCoord = !largePace ? CoordStruct::Empty : (theTargetCoords - theSourceCoords) * (1.0 / largePace);
			auto curCoord = theSourceCoords;
			auto pCurCell = MapClass::Instance.GetCellAt(sourceCell);
			// Check one by one towards the direction of the next frame's position
			for (size_t i = 0; i < largePace; ++i)
			{
				if ((checkThrough && this->CheckThroughAndSubjectInCell(pCurCell, pOwner)) // Blocked by obstacles?
					|| (subjectToWCS && TrajectoryHelper::GetObstacle(pSourceCell, pTargetCell, pLastCell, curCoord, pBulletType, pOwner)) // Impact on the wall/cliff/solid?
					|| (checkLevel ? (pBulletType->Level && pCurCell->IsOnFloor()) // Level or above land/water?
						: ((pCurCell->LandType == LandType::Water || pCurCell->LandType == LandType::Beach)
							? (pBulletTypeExt->SubjectToWater.Get(false) && pBulletTypeExt->SubjectToWater_Detonate)
							: (pBulletTypeExt->SubjectToLand.Get(false) && pBulletTypeExt->SubjectToLand_Detonate))))
				{
					locationDistance = PhobosTrajectory::Get2DDistance(curCoord, theSourceCoords);
					velocityCheck = 2;
					break;
				}
				else if (curCoord.Z < MapClass::Instance.GetCellFloorHeight(curCoord)) // Below ground level?
				{
					locationDistance = PhobosTrajectory::Get2DDistance(curCoord, theSourceCoords);
					velocityCheck = 1;
					break;
				}
				// There are no obstacles, continue to check the next cell
				curCoord += stepCoord;
				pLastCell = pCurCell;
				pCurCell = MapClass::Instance.GetCellAt(curCoord);
			}
		}
		// Check whether ignore firestorm wall before searching
		if (!pBulletType->IgnoresFirestorm)
		{
			const auto fireStormCoords = MapClass::Instance.FindFirstFirestorm(theSourceCoords, theTargetCoords, pOwner);
			// Not empty when firestorm wall exists
			if (fireStormCoords != CoordStruct::Empty)
			{
				const auto distance = PhobosTrajectory::Get2DDistance(fireStormCoords, theSourceCoords);
				// Only record when the ratio is smaller
				if (!velocityCheck || distance < locationDistance)
				{
					locationDistance = distance;
					velocityCheck = 2;
				}
			}
		}
		// Let the distance slightly exceed
		ratio = (locationDistance + 32.0) / velocity;
	}
	// No need for change
	if (!velocityCheck)
		return false;
	// Detonates itself in the next frame
	if (velocityCheck == 2)
	{
		this->MultiplyBulletVelocity(ratio, true);
		return false;
	}
	// Bounce in the next frame
	this->LastVelocity = this->MovingVelocity;
	this->MultiplyBulletVelocity(ratio, false);
	return false;
}

TrajectoryCheckReturnType ParabolaTrajectory::OnDetonateUpdate(const CoordStruct& position)
{
	if (this->WaitOneFrame)
		return TrajectoryCheckReturnType::SkipGameCheck;
	else if (this->PhobosTrajectory::OnDetonateUpdate(position) == TrajectoryCheckReturnType::Detonate)
		return TrajectoryCheckReturnType::Detonate;

	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(position) < pType->DetonationDistance.Get())
		return TrajectoryCheckReturnType::Detonate;
	// Height
	if (pType->DetonationHeight >= 0 && (pType->EarlyDetonation
		? ((position.Z - pBullet->SourceCoords.Z) > pType->DetonationHeight)
		: (this->MovingVelocity.Z < 1e-10 && (position.Z - pBullet->SourceCoords.Z) < pType->DetonationHeight)))
	{
		return TrajectoryCheckReturnType::Detonate;
	}
	// Angle
	if (std::abs(pType->DetonationAngle) < 1e-10)
	{
		if (this->MovingVelocity.Z < 1e-10)
			return TrajectoryCheckReturnType::Detonate;
	}
	else if (std::abs(pType->DetonationAngle) < 90.0)
	{
		const auto velocity = PhobosTrajectory::Get2DVelocity(this->MovingVelocity);

		if (velocity > 1e-10)
		{
			if ((this->MovingVelocity.Z / velocity) < Math::tan(pType->DetonationAngle * Math::Pi / 180.0))
				return TrajectoryCheckReturnType::Detonate;
		}
		else if (pType->DetonationAngle > 1e-10 || this->MovingVelocity.Z < 1e-10)
		{
			return TrajectoryCheckReturnType::Detonate;
		}
	}

	const auto pCell = MapClass::Instance.TryGetCellAt(position);
	// Bounce
	if (!pCell || (this->ShouldBounce && this->CalculateBulletVelocityAfterBounce(pCell, position)))
		return TrajectoryCheckReturnType::Detonate;

	return TrajectoryCheckReturnType::SkipGameCheck;
}

void ParabolaTrajectory::OnPreDetonate()
{
	const auto pBullet = this->Bullet;
	// If the speed is too fast, it may smash through the floor
	const auto cellHeight = MapClass::Instance.GetCellFloorHeight(pBullet->Location);

	if (pBullet->Location.Z < cellHeight)
		pBullet->SetLocation(CoordStruct{ pBullet->Location.X, pBullet->Location.Y, cellHeight });

	this->ActualTrajectory::OnPreDetonate();
}

void ParabolaTrajectory::OpenFire()
{
	// Wait, or launch immediately?
	if (!this->Type->LeadTimeCalculate.Get(false) || !abstract_cast<FootClass*>(this->Bullet->Target))
		this->FireTrajectory();
	else
		this->WaitOneFrame = 2;

	this->PhobosTrajectory::OpenFire();
}

void ParabolaTrajectory::FireTrajectory()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	auto& target = pBullet->TargetCoords;
	auto& source = pBullet->SourceCoords;
	const auto pTarget = pBullet->Target;

	if (pTarget)
		target = pTarget->GetCoords();
	// Calculate the orientation of the coordinate system
	const auto rotateRadian = this->Get2DOpRadian(((target == source && pBullet->Owner) ? pBullet->Owner->GetCoords() : source), target);
	// Add the fixed offset value
	if (pType->OffsetCoord != CoordStruct::Empty)
		target += this->GetOnlyStableOffsetCoords(rotateRadian);
	// Add random offset value
	if (pBullet->Type->Inaccurate)
		target = this->GetInaccurateTargetCoords(target, source.DistanceFrom(target));
	// Non positive gravity is not accepted
	const auto gravity = BulletTypeExt::GetAdjustedGravity(pBullet->Type);

	if (gravity <= 1e-10)
	{
		this->ShouldDetonate = true;
		return;
	}
	// Calculate the firing velocity vector of the bullet
	if (pType->LeadTimeCalculate.Get(false) && pTarget && pTarget->GetCoords() != this->LastTargetCoord)
		this->CalculateBulletVelocityLeadTime(source, gravity);
	else
		this->CalculateBulletVelocityRightNow(source, gravity);

	this->MovingSpeed = this->MovingVelocity.Magnitude();
	// Rotate the selected angle
	if (std::abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
		this->DisperseBurstSubstitution(rotateRadian);
}

void ParabolaTrajectory::MultiplyBulletVelocity(const double ratio, const bool shouldDetonate)
{
	if (ratio < 1.0)
	{
		this->MovingVelocity *= ratio;
		this->MovingSpeed = this->MovingSpeed * ratio;
	}
	// Is it detonating or bouncing?
	if (shouldDetonate || this->BounceTimes <= 0)
		this->ShouldDetonate = true;
	else
		this->ShouldBounce = true;
}

void ParabolaTrajectory::CalculateBulletVelocityLeadTime(const CoordStruct& source, const double gravity)
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	const auto target = pBullet->Target->GetCoords();
	const auto offset = pBullet->TargetCoords - target;

	switch (pType->OpenFireMode)
	{
	case ParabolaFireMode::Height: // Fixed max height and aim at the target
	{
		// Step 1: Using Newton Iteration Method to determine the time of encounter between the projectile and the target
		const auto meetTime = this->GetLeadTime(this->SearchFixedHeightMeetTime(source, target, offset, gravity));
		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (target - this->LastTargetCoord) * meetTime;
		const auto destinationCoords = pBullet->TargetCoords - source;
		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;
		// Step 4: Determine the maximum height that the projectile should reach
		const auto sourceHeight = source.Z;
		const auto targetHeight = pBullet->TargetCoords.Z;
		const auto maxHeight = destinationCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;
		// Step 5: Calculate the vertical component of the projectile velocity
		this->MovingVelocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight)) + gravity / 2;
		// Step 6: Calculate the total time it takes for the projectile to meet the target using the heights of the ascending and descending phases
		const auto time = sqrt(2 * (maxHeight - sourceHeight) / gravity) + sqrt(2 * (maxHeight - targetHeight) / gravity);
		// Step 7: Calculate the horizontal component of the projectile velocity
		this->MovingVelocity.X = destinationCoords.X / time;
		this->MovingVelocity.Y = destinationCoords.Y / time;
		return;
	}
	case ParabolaFireMode::Angle: // Fixed fire angle and aim at the target
	{
		// Step 1: Read the appropriate fire angle
		auto radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;
		// Step 2: Using Newton Iteration Method to determine the time of encounter between the projectile and the target
		const auto meetTime = this->GetLeadTime(this->SearchFixedAngleMeetTime(source, target, offset, radian, gravity));
		// Step 3: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (target - this->LastTargetCoord) * meetTime;
		const auto destinationCoords = pBullet->TargetCoords - source;
		// Step 4: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;
		// Step 5: Recalculate the speed when time is limited
		if (pType->LeadTimeMaximum > 0)
		{
			this->CalculateBulletVelocityRightNow(source, gravity);
			return;
		}
		// Step 6: Calculate each horizontal component of the projectile velocity
		this->MovingVelocity.X = destinationCoords.X / meetTime;
		this->MovingVelocity.Y = destinationCoords.Y / meetTime;
		// Step 7: Calculate whole horizontal component of the projectile velocity
		const auto horizontalDistance = PhobosTrajectory::Get2DDistance(destinationCoords);
		const auto horizontalVelocity = horizontalDistance / meetTime;
		// Step 8: Calculate the vertical component of the projectile velocity
		this->MovingVelocity.Z = horizontalVelocity * Math::tan(radian) + gravity / 2;
		return;
	}
	case ParabolaFireMode::SpeedAndHeight: // Fixed horizontal speed and fixed max height
	{
		// Step 1: Calculate the time when the projectile meets the target directly using horizontal velocity
		const auto meetTime = this->GetLeadTime(this->SolveFixedSpeedMeetTime(source, target, offset, pType->Speed));
		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (target - this->LastTargetCoord) * meetTime;
		const auto destinationCoords = pBullet->TargetCoords - source;
		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;
		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const auto horizontalDistance = PhobosTrajectory::Get2DDistance(destinationCoords);
		const auto mult = horizontalDistance > 1e-10 ? pType->Speed / horizontalDistance : 1.0;
		// Step 5: Calculate the horizontal component of the projectile velocity
		this->MovingVelocity.X = destinationCoords.X * mult;
		this->MovingVelocity.Y = destinationCoords.Y * mult;
		// Step 6: Determine the maximum height that the projectile should reach
		const auto sourceHeight = source.Z;
		const auto targetHeight = sourceHeight + destinationCoords.Z;
		const auto maxHeight = destinationCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;
		// Step 7: Calculate the vertical component of the projectile velocity
		this->MovingVelocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight)) + gravity / 2;
		return;
	}
	case ParabolaFireMode::HeightAndAngle: // Fixed max height and fixed fire angle
	{
		// Step 1: Using Newton Iteration Method to determine the time of encounter between the projectile and the target
		const auto meetTime = this->GetLeadTime(this->SearchFixedHeightMeetTime(source, target, offset, gravity));
		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (target - this->LastTargetCoord) * meetTime;
		const auto destinationCoords = pBullet->TargetCoords - source;
		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;
		// Step 4: Determine the maximum height that the projectile should reach
		const auto sourceHeight = source.Z;
		const auto targetHeight = sourceHeight + destinationCoords.Z;
		const auto maxHeight = destinationCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;
		// Step 5: Calculate the vertical component of the projectile velocity
		this->MovingVelocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight)) + gravity / 2;
		// Step 6: Read the appropriate fire angle
		auto radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= 1e-10) ? (Math::HalfPi / 3) : radian;
		// Step 7: Calculate the ratio of horizontal velocity to horizontal distance
		const auto horizontalDistance = PhobosTrajectory::Get2DDistance(destinationCoords);
		const auto mult = (this->MovingVelocity.Z / Math::tan(radian)) / horizontalDistance;
		// Step 8: Calculate the horizontal component of the projectile velocity
		this->MovingVelocity.X = destinationCoords.X * mult;
		this->MovingVelocity.Y = destinationCoords.Y * mult;
		return;
	}
	case ParabolaFireMode::SpeedAndAngle: // Fixed horizontal speed and fixed fire angle
	{
		// Step 1: Calculate the time when the projectile meets the target directly using horizontal velocity
		const auto meetTime = this->GetLeadTime(this->SolveFixedSpeedMeetTime(source, target, offset, pType->Speed));
		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (target - this->LastTargetCoord) * meetTime;
		const auto destinationCoords = pBullet->TargetCoords - source;
		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;
		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const auto horizontalDistance = PhobosTrajectory::Get2DDistance(destinationCoords);
		const auto mult = horizontalDistance > 1e-10 ? pType->Speed / horizontalDistance : 1.0;
		// Step 5: Calculate each horizontal component of the projectile velocity
		this->MovingVelocity.X = destinationCoords.X * mult;
		this->MovingVelocity.Y = destinationCoords.Y * mult;
		// Step 6: Calculate whole horizontal component of the projectile velocity
		const auto horizontalVelocity = horizontalDistance * mult;
		// Step 7: Read the appropriate fire angle
		auto radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;
		// Step 8: Calculate the vertical component of the projectile velocity
		this->MovingVelocity.Z = horizontalVelocity * Math::tan(radian) + gravity / 2;
		return;
	}
	default: // Fixed horizontal speed and aim at the target
	{
		// Step 1: Calculate the time when the projectile meets the target directly using horizontal velocity
		const auto meetTime = this->GetLeadTime(this->SolveFixedSpeedMeetTime(source, target, offset, pType->Speed));
		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (target - this->LastTargetCoord) * meetTime;
		const auto destinationCoords = pBullet->TargetCoords - source;
		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Magnitude() <= 1e-10)
			break;
		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const auto horizontalDistance = PhobosTrajectory::Get2DDistance(destinationCoords);
		const auto mult = horizontalDistance > 1e-10 ? pType->Speed / horizontalDistance : 1.0;
		// Step 5: Calculate the projectile velocity
		this->MovingVelocity.X = destinationCoords.X * mult;
		this->MovingVelocity.Y = destinationCoords.Y * mult;
		this->MovingVelocity.Z = destinationCoords.Z * mult + (gravity * horizontalDistance) / (2 * pType->Speed) + gravity / 2;
		return;
	}
	}
	// Reset target position
	pBullet->TargetCoords = target + offset;
	// Substitute into the no lead time algorithm
	this->CalculateBulletVelocityRightNow(source, gravity);
}

void ParabolaTrajectory::CalculateBulletVelocityRightNow(const CoordStruct& source, const double gravity)
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	// Calculate horizontal distance
	const auto distanceCoords = pBullet->TargetCoords - source;
	const auto distance = distanceCoords.Magnitude();
	const auto horizontalDistance = PhobosTrajectory::Get2DDistance(distanceCoords);

	if (distance <= 1e-10)
	{
		this->ShouldDetonate = true;
		return;
	}

	switch (pType->OpenFireMode)
	{
	case ParabolaFireMode::Height: // Fixed max height and aim at the target
	{
		// Step 1: Determine the maximum height that the projectile should reach
		const auto sourceHeight = source.Z;
		const auto targetHeight = pBullet->TargetCoords.Z;
		const auto maxHeight = distanceCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;
		// Step 2: Calculate the vertical component of the projectile velocity
		this->MovingVelocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight));
		// Step 3: Calculate the total time it takes for the projectile to meet the target using the heights of the ascending and descending phases
		const auto time = sqrt(2 * (maxHeight - sourceHeight) / gravity) + sqrt(2 * (maxHeight - targetHeight) / gravity);
		// Step 4: Calculate the horizontal component of the projectile velocity
		this->MovingVelocity.X = distanceCoords.X / time;
		this->MovingVelocity.Y = distanceCoords.Y / time;
		break;
	}
	case ParabolaFireMode::Angle: // Fixed fire angle and aim at the target
	{
		// Step 1: Read the appropriate fire angle
		const auto radian = pType->LaunchAngle * Math::Pi / 180.0;
		// Step 2: Using Newton Iteration Method to determine the projectile velocity
		const auto velocity = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? 100.0 : this->SearchVelocity(horizontalDistance, distanceCoords.Z, radian, gravity);
		// Step 3: Calculate the vertical component of the projectile velocity
		this->MovingVelocity.Z = velocity * Math::sin(radian);
		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const auto mult = velocity * Math::cos(radian) / horizontalDistance;
		// Step 5: Calculate the horizontal component of the projectile velocity
		this->MovingVelocity.X = distanceCoords.X * mult;
		this->MovingVelocity.Y = distanceCoords.Y * mult;
		break;
	}
	case ParabolaFireMode::SpeedAndHeight: // Fixed horizontal speed and fixed max height
	{
		// Step 1: Determine the maximum height that the projectile should reach
		const auto sourceHeight = source.Z;
		const auto targetHeight = pBullet->TargetCoords.Z;
		const auto maxHeight = distanceCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;
		// Step 2: Calculate the vertical component of the projectile velocity
		this->MovingVelocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight));
		// Step 3: Calculate the ratio of horizontal velocity to horizontal distance
		const auto mult = horizontalDistance > 1e-10 ? pType->Speed / horizontalDistance : 1.0;
		// Step 4: Calculate the horizontal component of the projectile velocity
		this->MovingVelocity.X = distanceCoords.X * mult;
		this->MovingVelocity.Y = distanceCoords.Y * mult;
		break;
	}
	case ParabolaFireMode::HeightAndAngle: // Fixed max height and fixed fire angle
	{
		// Step 1: Determine the maximum height that the projectile should reach
		const auto sourceHeight = source.Z;
		const auto targetHeight = pBullet->TargetCoords.Z;
		const auto maxHeight = distanceCoords.Z > 0 ? this->ThrowHeight + targetHeight : this->ThrowHeight + sourceHeight;
		// Step 2: Calculate the vertical component of the projectile velocity
		this->MovingVelocity.Z = sqrt(2 * gravity * (maxHeight - sourceHeight));
		// Step 3: Read the appropriate fire angle
		auto radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= 1e-10) ? (Math::HalfPi / 3) : radian;
		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const auto mult = (this->MovingVelocity.Z / Math::tan(radian)) / horizontalDistance;
		// Step 5: Calculate the horizontal component of the projectile velocity
		this->MovingVelocity.X = distanceCoords.X * mult;
		this->MovingVelocity.Y = distanceCoords.Y * mult;
		break;
	}
	case ParabolaFireMode::SpeedAndAngle: // Fixed horizontal speed and fixed fire angle
	{
		// Step 1: Calculate the ratio of horizontal velocity to horizontal distance
		const auto mult = horizontalDistance > 1e-10 ? pType->Speed / horizontalDistance : 1.0;
		// Step 2: Calculate the horizontal component of the projectile velocity
		this->MovingVelocity.X = distanceCoords.X * mult;
		this->MovingVelocity.Y = distanceCoords.Y * mult;
		// Step 3: Read the appropriate fire angle
		auto radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;
		// Step 4: Calculate the vertical component of the projectile velocity
		this->MovingVelocity.Z = pType->Speed * Math::tan(radian);
		break;
	}
	default: // Fixed horizontal speed and aim at the target
	{
		// Step 1: Calculate the ratio of horizontal velocity to horizontal distance
		const auto mult = horizontalDistance > 1e-10 ? pType->Speed / horizontalDistance : 1.0;
		// Step 2: Calculate the projectile velocity
		this->MovingVelocity.X = distanceCoords.X * mult;
		this->MovingVelocity.Y = distanceCoords.Y * mult;
		this->MovingVelocity.Z = distanceCoords.Z * mult + (gravity * horizontalDistance) / (2 * pType->Speed);
		break;
	}
	}
	// Offset the gravity effect of the first time update
	this->MovingVelocity.Z += gravity / 2;
}

double ParabolaTrajectory::SearchVelocity(const double horizontalDistance, int distanceCoordsZ, const double radian, const double gravity)
{
	// Estimate initial velocity
	const auto mult = Math::sin(2 * radian);
	auto velocity = std::abs(mult) > 1e-10 ? sqrt(horizontalDistance * gravity / mult) : 0.0;
	velocity += distanceCoordsZ / gravity;
	velocity = velocity > 8.0 ? velocity : 8.0;
	const auto error = velocity / 16;
	// Step size
	const auto delta = 1e-5;
	// Newton Iteration Method
	for (int i = 0; i < 10; ++i)
	{
		// Substitute into the estimate speed
		const auto differential = this->CheckVelocityEquation(horizontalDistance, distanceCoordsZ, velocity, radian, gravity);
		const auto dDifferential = (this->CheckVelocityEquation(horizontalDistance, distanceCoordsZ, (velocity + delta), radian, gravity) - differential) / delta;
		// Check unacceptable divisor
		if (std::abs(dDifferential) < 1e-10)
			return velocity;
		// Calculate the speed of the next iteration
		const auto difference = differential / dDifferential;
		const auto velocityNew = velocity - difference;
		// Check tolerable error
		if (std::abs(difference) < error)
			return velocityNew;
		// Update the speed
		velocity = velocityNew;
	}
	// Unsolvable
	return 10.0;
}

double ParabolaTrajectory::CheckVelocityEquation(const double horizontalDistance, int distanceCoordsZ, const double velocity, const double radian, const double gravity)
{
	// Calculate each component of the projectile velocity
	const auto horizontalVelocity = velocity * Math::cos(radian);
	const auto verticalVelocity = velocity * Math::sin(radian);
	// Calculate the time of the rising phase
	const auto upTime = verticalVelocity / gravity;
	// Calculate the maximum height that the projectile can reach
	const auto maxHeight = 0.5 * verticalVelocity * upTime;
	// Calculate the time of the descent phase
	const auto downTime = sqrt(2 * (maxHeight - distanceCoordsZ) / gravity);
	// Calculate the total time required for horizontal movement
	const auto wholeTime = horizontalDistance / horizontalVelocity;
	// Calculate the difference between the total vertical motion time and the total horizontal motion time
	return wholeTime - (upTime + downTime);
}

double ParabolaTrajectory::SolveFixedSpeedMeetTime(const CoordStruct& source, const CoordStruct& target, const CoordStruct& offset, const double horizontalSpeed)
{
	// Project all conditions onto a horizontal plane
	const Point2D targetSpeedCrd { target.X - this->LastTargetCoord.X, target.Y - this->LastTargetCoord.Y };
	const Point2D destinationCrd { target.X + offset.X - source.X, target.Y + offset.Y - source.Y };
	// Establishing a quadratic equation using time as a variable:
	// (destinationCrd + targetSpeedCrd * time).Magnitude() = horizontalSpeed * time
	// Solve this quadratic equation
	const auto targetSpeedSq = targetSpeedCrd.MagnitudeSquared();
	const auto destinationSq = destinationCrd.MagnitudeSquared();
	const auto speedSq = horizontalSpeed * horizontalSpeed;
	const auto divisor = targetSpeedSq - speedSq;
	const auto factor = targetSpeedCrd * destinationCrd;
	const auto cosTheta = factor / (sqrt(targetSpeedSq * destinationSq) + 1e-10);
	// The target speed is too fast
	if (speedSq < (1.0 + 0.2 * Math::max(0.0, -cosTheta)) * targetSpeedSq)
		return -1.0;
	// Normal solving
	const auto delta = factor * factor - divisor * destinationSq;
	// Check if there is no solution
	if (delta < 1e-10)
		return (delta >= -1e-10) ? (-factor / divisor) + (factor > 0 ? 1.0 : 0) : -1.0;
	// Quadratic formula
	const auto sqrtDelta = sqrt(delta);
	const auto timeP = (-factor + sqrtDelta) / divisor;
	const auto timeM = (-factor - sqrtDelta) / divisor;
	// When the target is moving away, provide an additional frame of correction
	if (timeM > 1e-10)
		return ((timeP > 1e-10) ? Math::min(timeM, timeP) : timeM) + (factor > 0 ? 1.0 : 0);
	else if (timeP > 1e-10)
		return timeP + (factor > 0 ? 1.0 : 0);
	// Unsolvable
	return -1.0;
}

double ParabolaTrajectory::SearchFixedHeightMeetTime(const CoordStruct& source, const CoordStruct& target, const CoordStruct& offset, const double gravity)
{
	// Similar to method SearchVelocity, no further elaboration will be provided
	const auto delta = 1e-5;
	auto meetTime = (this->ThrowHeight << 2) / gravity;

	for (int i = 0; i < 10; ++i)
	{
		const auto differential = this->CheckFixedHeightEquation(source, target, offset, meetTime, gravity);
		const auto dDifferential = (this->CheckFixedHeightEquation(source, target, offset, (meetTime + delta), gravity) - differential) / delta;

		if (std::abs(dDifferential) < 1e-10)
			return meetTime;

		const auto difference = differential / dDifferential;
		const auto meetTimeNew = meetTime - difference;

		if (std::abs(difference) < 1.0)
			return meetTimeNew;

		meetTime = meetTimeNew;
	}

	return -1.0;
}

double ParabolaTrajectory::CheckFixedHeightEquation(const CoordStruct& source, const CoordStruct& target, const CoordStruct& offset, const double meetTime, const double gravity)
{
	// Calculate how high the target will reach during this period of time
	const auto meetHeight = static_cast<int>((target.Z - this->LastTargetCoord.Z) * meetTime) + target.Z + offset.Z;
	// Calculate how high the projectile can fly during this period of time
	const auto maxHeight = meetHeight > source.Z ? this->ThrowHeight + meetHeight : this->ThrowHeight + source.Z;
	// Calculate the difference between these two times
	return sqrt((maxHeight - source.Z) * 2 / gravity) + sqrt((maxHeight - meetHeight) * 2 / gravity) - meetTime;
}

double ParabolaTrajectory::SearchFixedAngleMeetTime(const CoordStruct& source, const CoordStruct& target, const CoordStruct& offset, const double radian, const double gravity)
{
	// Similar to method SearchVelocity, no further elaboration will be provided
	const auto delta = 1e-5;
	auto meetTime = 512 * Math::sin(radian) / gravity;

	for (int i = 0; i < 10; ++i)
	{
		const auto differential = this->CheckFixedAngleEquation(source, target, offset, meetTime, radian, gravity);
		const auto dDifferential = (this->CheckFixedAngleEquation(source, target, offset, (meetTime + delta), radian, gravity) - differential) / delta;

		if (std::abs(dDifferential) < 1e-10)
			return meetTime;

		const auto difference = differential / dDifferential;
		const auto meetTimeNew = meetTime - difference;

		if (std::abs(difference) < 1.0)
			return meetTimeNew;

		meetTime = meetTimeNew;
	}

	return -1.0;
}

double ParabolaTrajectory::CheckFixedAngleEquation(const CoordStruct& source, const CoordStruct& target, const CoordStruct& offset, const double meetTime, const double radian, const double gravity)
{
	// Using the estimated time to obtain the predicted location of the target
	const auto distanceCoords = (target - this->LastTargetCoord) * meetTime + target + offset - source;
	// Calculate the horizontal distance between the target and the calculation
	const auto horizontalDistance = PhobosTrajectory::Get2DDistance(distanceCoords);
	// Calculate the horizontal velocity
	const auto horizontalVelocity = horizontalDistance / meetTime;
	// Calculate the vertical velocity
	const auto verticalVelocity = horizontalVelocity * Math::tan(radian);
	// Calculate the time of the rising phase
	const auto upTime = verticalVelocity / gravity;
	// Calculate the maximum height that the projectile can reach
	const auto maxHeight = 0.5 * verticalVelocity * upTime;
	// Calculate the time of the descent phase
	const auto downTime = sqrt(2 * (maxHeight - distanceCoords.Z) / gravity);
	// Calculate the difference between the actual flight time of the projectile obtained and the initially estimated time
	return upTime + downTime - meetTime;
}

bool ParabolaTrajectory::CalculateBulletVelocityAfterBounce(const CellClass* const pCell, const CoordStruct& position)
{
	const auto pType = this->Type;
	// Can bounce on water surface?
	if (pCell->LandType == LandType::Water && !pType->BounceOnWater)
		return true;
	// Obtain information on which surface to bounce on
	const auto groundNormalVector = this->GetGroundNormalVector(pCell, position);
	// Bounce only occurs when the velocity is in different directions or the surface is not cliff
	if (this->LastVelocity * groundNormalVector > 0 && std::abs(groundNormalVector.Z) < 1e-10)
	{
		// Restore original velocity
		this->MovingVelocity = this->LastVelocity;
		this->MovingSpeed = this->MovingVelocity.Magnitude();
		return false;
	}
	// Record bouncing once
	--this->BounceTimes;
	this->ShouldBounce = false;
	// Calculate the velocity vector after bouncing
	this->MovingVelocity = (this->LastVelocity - groundNormalVector * (this->LastVelocity * groundNormalVector) * 2) * pType->BounceCoefficient;
	this->MovingSpeed = this->MovingVelocity.Magnitude();
	const auto pBullet = this->Bullet;
	// Detonate an additional warhead when bouncing?
	if (pType->BounceDetonate)
	{
		const auto pFirer = pBullet->Owner;
		const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		WarheadTypeExt::DetonateAt(pBullet->WH, position, pFirer, pBullet->Health, pOwner);
	}
	// Calculate the attenuation damage after bouncing
	PhobosTrajectory::SetNewDamage(pBullet->Health, pType->BounceAttenuation);
	return false;
}

BulletVelocity ParabolaTrajectory::GetGroundNormalVector(const CellClass* const pCell, const CoordStruct& position)
{
	if (const auto index = pCell->SlopeIndex)
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
	const auto horizontalVelocity = PhobosTrajectory::Get2DVelocity(this->LastVelocity);
	const auto velocity = PhobosTrajectory::Vector2Coord(horizontalVelocity > 362.1 ? this->LastVelocity * (362.1 / horizontalVelocity) : this->LastVelocity);
	const auto cellHeight = pCell->Level * Unsorted::LevelHeight;
	const auto bulletHeight = position.Z;
	const auto lastCellHeight = MapClass::Instance.GetCellFloorHeight(position - velocity);
	// Check if it has hit a cliff (384 -> (4 * Unsorted::LevelHeight - 32(error range)))
	if (bulletHeight < cellHeight && (cellHeight - lastCellHeight) > 384)
	{
		auto cell = pCell->MapCoords;
		const auto reverseSgnX = static_cast<short>(this->LastVelocity.X > 0.0 ? -1 : 1);
		const auto reverseSgnY = static_cast<short>(this->LastVelocity.Y > 0.0 ? -1 : 1);
		int index = 0;
		// Determine the shape of the cliff using 9 surrounding cells
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
	// Just ordinary ground
	return BulletVelocity{ 0.0, 0.0, 1.0 };
}
