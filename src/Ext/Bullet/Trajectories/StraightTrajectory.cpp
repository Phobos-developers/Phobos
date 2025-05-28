#include "StraightTrajectory.h"

#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>

std::unique_ptr<PhobosTrajectory> StraightTrajectoryType::CreateInstance(BulletClass* pBullet) const
{
	return std::make_unique<StraightTrajectory>(this, pBullet);
}

template<typename T>
void StraightTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->PassThrough)
		.Process(this->ConfineAtHeight)
		;
}

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->ActualTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool StraightTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->ActualTrajectoryType::Save(Stm);
	const_cast<StraightTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void StraightTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->PhobosTrajectoryType::Read(pINI, pSection);
	INI_EX exINI(pINI);
	// Actual
	this->RotateCoord.Read(exINI, pSection, "Trajectory.RotateCoord");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.OffsetCoord");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.AxisOfRotation");
	this->LeadTimeMaximum.Read(exINI, pSection, "Trajectory.LeadTimeMaximum");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.LeadTimeCalculate");
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.TargetSnapDistance");
	// Straight
	this->PassThrough.Read(exINI, pSection, "Trajectory.Straight.PassThrough");
	this->ConfineAtHeight.Read(exINI, pSection, "Trajectory.Straight.ConfineAtHeight");
}

template<typename T>
void StraightTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->DetonationDistance)
		;
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->ActualTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool StraightTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->ActualTrajectory::Save(Stm);
	const_cast<StraightTrajectory*>(this)->Serialize(Stm);
	return true;
}

void StraightTrajectory::OnUnlimbo()
{
	this->ActualTrajectory::OnUnlimbo();
	// Straight
	const auto pBullet = this->Bullet;
	// Calculate range bonus
	if (this->Type->ApplyRangeModifiers)
	{
		if (const auto pFirer = pBullet->Owner)
		{
			if (const auto pWeapon = pBullet->WeaponType)
			{
				// Determine the range of the bullet
				if (this->DetonationDistance >= 0)
					this->DetonationDistance = Leptons(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer, this->DetonationDistance));
				else
					this->DetonationDistance = Leptons(-WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer, -this->DetonationDistance));
			}
		}
	}
	// Waiting for launch trigger
	if (!BulletExt::ExtMap.Find(pBullet)->DispersedTrajectory)
		this->OpenFire();
}

bool StraightTrajectory::OnVelocityCheck()
{
	const auto pType = this->Type;
	// Hover
	if (pType->Speed < 256.0 && pType->ConfineAtHeight > 0 && this->PassAndConfineAtHeight())
		return true;

	return this->PhobosTrajectory::OnVelocityCheck();
}

TrajectoryCheckReturnType StraightTrajectory::OnDetonateUpdate(const CoordStruct& position)
{
	if (this->WaitOneFrame)
		return TrajectoryCheckReturnType::SkipGameCheck;
	else if (this->PhobosTrajectory::OnDetonateUpdate(position) == TrajectoryCheckReturnType::Detonate)
		return TrajectoryCheckReturnType::Detonate;

	const auto pType = this->Type;
	const auto distance = pType->Speed < 256.0 && pType->ConfineAtHeight > 0 ? PhobosTrajectory::Get2DVelocity(this->MovingVelocity) : this->MovingSpeed;
	this->RemainingDistance -= static_cast<int>(distance);
	// Check the remaining travel distance of the bullet
	if (this->RemainingDistance < 0)
		return TrajectoryCheckReturnType::Detonate;

	const auto pBullet = this->Bullet;
	// Close enough
	if (!pType->PassThrough && pBullet->TargetCoords.DistanceFrom(position) < pType->DetonationDistance.Get())
		return TrajectoryCheckReturnType::Detonate;

	return TrajectoryCheckReturnType::SkipGameCheck;
}

void StraightTrajectory::OnPreDetonate()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	// Whether to detonate at ground level?
	if (pType->PassDetonateLocal)
		pBullet->SetLocation(CoordStruct { pBullet->Location.X, pBullet->Location.Y, MapClass::Instance.GetCellFloorHeight(pBullet->Location) });

	if (!pType->PassThrough)
		this->ActualTrajectory::OnPreDetonate();
	else
		this->PhobosTrajectory::OnPreDetonate();
}

void StraightTrajectory::OpenFire()
{
	// Wait, or launch immediately?
	if (!this->Type->LeadTimeCalculate.Get(false) || !abstract_cast<FootClass*>(this->Bullet->Target))
		this->FireTrajectory();
	else
		this->WaitOneFrame = 2;

	this->PhobosTrajectory::OpenFire();
}

void StraightTrajectory::FireTrajectory()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	const auto& source = pBullet->SourceCoords;
	auto& target = pBullet->TargetCoords;
	target += this->CalculateBulletLeadTime();
	// Calculate the orientation of the coordinate system
	const auto rotateRadian = this->Get2DOpRadian(((target == source && pBullet->Owner) ? pBullet->Owner->GetCoords() : source), target);
	// Add the fixed offset value
	if (pType->OffsetCoord != CoordStruct::Empty)
		target += this->GetOnlyStableOffsetCoords(rotateRadian);
	// Add random offset value
	if (pBullet->Type->Inaccurate)
		target = this->GetInaccurateTargetCoords(target, source.DistanceFrom(target));
	// Determine the distance that the bullet can travel
	if (!pType->PassThrough)
		this->RemainingDistance += static_cast<int>(source.DistanceFrom(target));
	else if (this->DetonationDistance > 0)
		this->RemainingDistance += static_cast<int>(this->DetonationDistance);
	else if (this->DetonationDistance < 0)
		this->RemainingDistance += static_cast<int>(source.DistanceFrom(target) - this->DetonationDistance);
	else
		this->RemainingDistance = INT_MAX;
	// Determine the firing velocity vector of the bullet
	pBullet->TargetCoords = target;
	this->MovingVelocity.X = static_cast<double>(target.X - source.X);
	this->MovingVelocity.Y = static_cast<double>(target.Y - source.Y);
	this->MovingVelocity.Z = (pType->Speed < 256.0 && pType->ConfineAtHeight > 0 && pType->PassDetonateLocal) ? 0 : static_cast<double>(this->GetVelocityZ());
	// Substitute the speed to calculate velocity
	if (this->CalculateBulletVelocity(pType->Speed))
		this->ShouldDetonate = true;
	// Rotate the selected angle
	if (std::abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
		this->DisperseBurstSubstitution(rotateRadian);
}

CoordStruct StraightTrajectory::CalculateBulletLeadTime()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;

	if (pType->LeadTimeCalculate.Get(false))
	{
		if (const auto pTarget = pBullet->Target)
		{
			const auto target = pTarget->GetCoords();
			const auto source = pBullet->Location;
			// Solving trigonometric functions
			if (target != this->LastTargetCoord)
			{
				const auto extraOffsetCoord = target - this->LastTargetCoord;
				const auto targetSourceCoord = source - target;
				const auto lastSourceCoord = source - this->LastTargetCoord;

				const auto theDistanceSquared = targetSourceCoord.MagnitudeSquared();
				const auto targetSpeedSquared = extraOffsetCoord.MagnitudeSquared();

				const auto crossFactor = lastSourceCoord.CrossProduct(targetSourceCoord).MagnitudeSquared();
				const auto verticalDistanceSquared = crossFactor / targetSpeedSquared;

				const auto horizonDistanceSquared = theDistanceSquared - verticalDistanceSquared;
				const auto horizonDistance = sqrt(horizonDistanceSquared);
				// Calculate using vertical distance
				if (horizonDistance < 1e-10)
					return extraOffsetCoord * this->GetLeadTime(std::round(sqrt(verticalDistanceSquared) / pType->Speed));

				const auto targetSpeed = sqrt(targetSpeedSquared);
				const auto straightSpeedSquared = pType->Speed * pType->Speed;
				const auto baseFactor = straightSpeedSquared - targetSpeedSquared;
				// When the target is moving away, provide an additional frame of correction
				const int extraTime = theDistanceSquared >= lastSourceCoord.MagnitudeSquared() ? 2 : 1;
				// Linear equation solving
				if (std::abs(baseFactor) < 1e-10)
					return extraOffsetCoord * this->GetLeadTime(static_cast<int>(theDistanceSquared / (2 * horizonDistance * targetSpeed)) + extraTime);

				const auto squareFactor = baseFactor * verticalDistanceSquared + straightSpeedSquared * horizonDistanceSquared;
				// Is there a solution?
				if (squareFactor > 1e-10)
				{
					const auto minusFactor = -(horizonDistance * targetSpeed);
					const auto factor = sqrt(squareFactor);
					const auto travelTimeM = static_cast<int>((minusFactor - factor) / baseFactor);
					const auto travelTimeP = static_cast<int>((minusFactor + factor) / baseFactor);

					if (travelTimeM > 0)
						return extraOffsetCoord * this->GetLeadTime((travelTimeP > 0 ? Math::min(travelTimeM, travelTimeP) : travelTimeM) + extraTime);
					else if (travelTimeP > 0)
						return extraOffsetCoord * this->GetLeadTime(travelTimeP + extraTime);
				}
			}
		}
	}

	return CoordStruct::Empty;
}

int StraightTrajectory::GetVelocityZ()
{
	const auto pBullet = this->Bullet;
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

		if (pCell->ContainsBridge() && pTechno->OnBridge)
			sourceCellZ += CellClass::BridgeHeight;
	}

	if (const auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target))
	{
		const auto pCell = pTarget->GetCell();
		targetCellZ = pCell->Level * Unsorted::LevelHeight;

		if (pCell->ContainsBridge() && pTarget->OnBridge)
			targetCellZ += CellClass::BridgeHeight;
	}
	// If both are at the same height, use the DetonationDistance to calculate which position behind the target needs to be aimed (32 -> error range)
	if (sourceCellZ == targetCellZ || std::abs(bulletVelocityZ) <= 32)
	{
		// Infinite distance, horizontal emission
		if (!this->DetonationDistance)
			return 0;

		const auto distanceOfTwo = PhobosTrajectory::Get2DDistance(pBullet->SourceCoords, pBullet->TargetCoords);
		const auto theDistance = (this->DetonationDistance < 0) ? (distanceOfTwo - this->DetonationDistance) : this->DetonationDistance;
		// Calculate the ratio for subsequent speed calculation
		if (std::abs(theDistance) < 1e-10)
			return 0;

		bulletVelocityZ = static_cast<int>(bulletVelocityZ * (distanceOfTwo / theDistance));
	}

	return bulletVelocityZ;
}

bool StraightTrajectory::PassAndConfineAtHeight()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	// To prevent twitching and floating up and down, it is necessary to maintain a fixed distance when predicting the position
	const auto horizontalVelocity = PhobosTrajectory::Get2DVelocity(this->MovingVelocity);
	const auto ratio = pType->Speed / horizontalVelocity;
	auto velocityCoords = PhobosTrajectory::Vector2Coord(this->MovingVelocity);
	velocityCoords.X = static_cast<int>(velocityCoords.X * ratio);
	velocityCoords.Y = static_cast<int>(velocityCoords.Y * ratio);
	const auto futureCoords = pBullet->Location + velocityCoords;
	auto checkDifference = MapClass::Instance.GetCellFloorHeight(futureCoords) - futureCoords.Z;
	// Bridges require special treatment
	if (MapClass::Instance.GetCellAt(futureCoords)->ContainsBridge())
	{
		const auto differenceOnBridge = checkDifference + CellClass::BridgeHeight;

		if (std::abs(differenceOnBridge) < std::abs(checkDifference))
			checkDifference = differenceOnBridge;
	}
	// The height does not exceed the cliff, or the cliff can be ignored? (384 -> (4 * Unsorted::LevelHeight - 32(error range)))
	if (std::abs(checkDifference) >= 384 && pBullet->Type->SubjectToCliffs)
		return true;

	this->MovingVelocity.Z += static_cast<double>(checkDifference + pType->ConfineAtHeight);

	if (pType->PassDetonateLocal)
	{
		// In this case, the vertical speed will not be limited, and the horizontal speed will not be affected
		this->MovingSpeed = this->MovingVelocity.Magnitude();
	}
	else
	{
		// The maximum climbing ratio is limited to 8:1
		const auto maxZ = horizontalVelocity * 8;
		this->MovingVelocity.Z = Math::clamp(this->MovingVelocity.Z, -maxZ, maxZ);

		if (this->CalculateBulletVelocity(pType->Speed))
			return true;
	}

	return false;
}
