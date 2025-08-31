#include "EngraveTrajectory.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>

std::unique_ptr<PhobosTrajectory> EngraveTrajectoryType::CreateInstance(BulletClass* pBullet) const
{
	return std::make_unique<EngraveTrajectory>(this, pBullet);
}

template<typename T>
void EngraveTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->AttachToTarget)
		.Process(this->UpdateDirection)
		;
}

bool EngraveTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->VirtualTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool EngraveTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->VirtualTrajectoryType::Save(Stm);
	const_cast<EngraveTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void EngraveTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->PhobosTrajectoryType::Read(pINI, pSection);
	INI_EX exINI(pINI);

	// Limitation
	this->Speed = Math::min(128.0, this->Speed);

	// Virtual
	this->VirtualSourceCoord.Read(exINI, pSection, "Trajectory.Engrave.SourceCoord");
	this->VirtualTargetCoord.Read(exINI, pSection, "Trajectory.Engrave.TargetCoord");
	this->AllowFirerTurning.Read(exINI, pSection, "Trajectory.AllowFirerTurning");

	// Engrave
	this->AttachToTarget.Read(exINI, pSection, "Trajectory.Engrave.AttachToTarget");
	this->UpdateDirection.Read(exINI, pSection, "Trajectory.Engrave.UpdateDirection");
}

template<typename T>
void EngraveTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->RotateRadian)
		;
}

bool EngraveTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->VirtualTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool EngraveTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->VirtualTrajectory::Save(Stm);
	const_cast<EngraveTrajectory*>(this)->Serialize(Stm);
	return true;
}

bool EngraveTrajectory::OnVelocityCheck()
{
	const auto pType = this->Type;

	if (pType->AttachToTarget || pType->UpdateDirection)
		this->ChangeVelocity();

	if (!BulletExt::ExtMap.Find(this->Bullet)->TargetIsInAir && this->PlaceOnCorrectHeight())
		return true;

	return this->PhobosTrajectory::OnVelocityCheck();
}

void EngraveTrajectory::OpenFire()
{
	const auto pBullet = this->Bullet;
	const auto pBulletExt = BulletExt::ExtMap.Find(pBullet);
	const auto pType = this->Type;
	const auto pFirer = pBullet->Owner;
	auto virtualSource = BulletExt::Coord2Point(pType->VirtualSourceCoord.Get());
	auto virtualTarget = BulletExt::Coord2Point(pType->VirtualTargetCoord.Get());

	// Mirror trajectory
	if (!pBulletExt->NotMainWeapon && pType->MirrorCoord && this->CurrentBurst < 0)
	{
		virtualSource.Y = -virtualSource.Y;
		virtualTarget.Y = -virtualTarget.Y;
	}

	// To be used later, no reference
	auto source = pBullet->SourceCoords;
	auto target = pBullet->TargetCoords;
	this->RotateRadian = BulletExt::Get2DOpRadian((pFirer ? pFirer->GetCoords() : source), target);

	// Special case: Starting from the launch position
	if (virtualSource.X != 0 || virtualSource.Y != 0)
		source = target + BulletExt::Point2Coord(BulletExt::PointRotate(virtualSource, this->RotateRadian));

	// If the target is in the air, there is no need to attach it to the ground
	if (!pBulletExt->TargetIsInAir)
		source.Z = this->GetFloorCoordHeight(source);

	// set initial status
	pBullet->SetLocation(source);
	target += BulletExt::Point2Coord(BulletExt::PointRotate(virtualTarget, this->RotateRadian));

	this->MovingVelocity.X = target.X - source.X;
	this->MovingVelocity.Y = target.Y - source.Y;
	this->MovingVelocity.Z = 0;

	if (this->CalculateBulletVelocity(pType->Speed))
		pBulletExt->Status |= TrajectoryStatus::Detonate;

	this->PhobosTrajectory::OpenFire();
}

bool EngraveTrajectory::CalculateBulletVelocity(const double speed)
{
	// Only call once
	// Substitute the speed to calculate velocity
	double velocityLength = this->MovingVelocity.Magnitude();

	if (velocityLength < BulletExt::Epsilon)
		return true;

	const auto pBullet = this->Bullet;
	const auto pBulletExt = BulletExt::ExtMap.Find(pBullet);
	const auto pBulletTypeExt = pBulletExt->TypeExtData;
	const auto pType = this->Type;
	const auto pFirer = pBullet->Owner;

	// Calculate additional range
	if (pBulletTypeExt->ApplyRangeModifiers && pFirer)
	{
		if (const auto pWeapon = pBullet->WeaponType)
			velocityLength = static_cast<double>(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer, static_cast<int>(velocityLength)));
	}

	// Automatically calculate duration
	if (pBulletTypeExt->LifeDuration <= 0)
		pBulletExt->LifeDurationTimer.Start(static_cast<int>(velocityLength / pType->Speed) + 1);
	else
		pBulletExt->LifeDurationTimer.Start(pBulletTypeExt->LifeDuration);

	this->MovingVelocity *= speed / velocityLength;
	this->MovingSpeed = speed;

	return false;
}

int EngraveTrajectory::GetFloorCoordHeight(const CoordStruct& coord)
{
	const int onFloor = MapClass::Instance.GetCellFloorHeight(coord);
	const int onBridge = MapClass::Instance.GetCellAt(coord)->ContainsBridge() ? onFloor + CellClass::BridgeHeight : onFloor;
	const auto pBullet = this->Bullet;

	// Take the higher position
	return (pBullet->SourceCoords.Z >= onBridge || pBullet->TargetCoords.Z >= onBridge) ? onBridge : onFloor;
}

void EngraveTrajectory::ChangeVelocity()
{
	const auto pBullet = this->Bullet;
	const auto pBulletExt = BulletExt::ExtMap.Find(pBullet);
	const auto pType = this->Type;

	// The center is located on the target
	if (pType->AttachToTarget)
	{
		// No need to synchronize the target again
		if (!pBulletExt->TypeExtData->Synchronize)
		{
			if (const auto pTarget = pBullet->Target)
				pBullet->TargetCoords = pTarget->GetCoords();
		}
	}

	// The angle will be updated according to the orientation
	if (pType->UpdateDirection)
	{
		const auto pFirer = pBullet->Owner;
		this->RotateRadian = BulletExt::Get2DOpRadian((pFirer ? pFirer->GetCoords() : pBullet->SourceCoords), pBullet->TargetCoords);
	}

	// Recalculate speed and position
	auto virtualSource = BulletExt::Coord2Point(pType->VirtualSourceCoord.Get());
	auto virtualTarget = BulletExt::Coord2Point(pType->VirtualTargetCoord.Get());

	if (!pBulletExt->NotMainWeapon && pType->MirrorCoord && this->CurrentBurst < 0)
	{
		virtualSource.Y = -virtualSource.Y;
		virtualTarget.Y = -virtualTarget.Y;
	}

	const double path = (pBulletExt->LifeDurationTimer.CurrentTime - pBulletExt->LifeDurationTimer.StartTime + 1) * pType->Speed;
	auto source = BulletExt::Coord2Point(pBullet->SourceCoords);
	auto target = BulletExt::Coord2Point(pBullet->TargetCoords);

	// Special case: Starting from the launch position
	if (virtualSource.X != 0 || virtualSource.Y != 0)
		source = target + BulletExt::PointRotate(virtualSource, this->RotateRadian);

	target += BulletExt::PointRotate(virtualTarget, this->RotateRadian);
	const auto delta = target - source;
	const double distance = delta.Magnitude();

	if (distance < BulletExt::Epsilon)
	{
		pBulletExt->Status |= TrajectoryStatus::Detonate;
		return;
	}

	const auto newLocation = BulletExt::Point2Coord((source + delta * (path / distance)), pBullet->TargetCoords.Z);
	this->MovingVelocity = BulletExt::Coord2Vector(newLocation - pBullet->Location);
	this->MovingSpeed = this->MovingVelocity.Magnitude();
}

bool EngraveTrajectory::PlaceOnCorrectHeight()
{
	const auto pBullet = this->Bullet;
	auto bulletCoords = pBullet->Location;
	const auto futureCoords = bulletCoords + BulletExt::Vector2Coord(this->MovingVelocity);

	// Calculate where will be located in the next frame
	const auto checkDifference = this->GetFloorCoordHeight(futureCoords) - futureCoords.Z;

	// When crossing the cliff, directly move the position of the bullet, otherwise change the vertical velocity (384 -> (4 * Unsorted::LevelHeight - 32(error range)))
	if (std::abs(checkDifference) >= 384)
	{
		if (pBullet->Type->SubjectToCliffs)
			return true;

		// Move from low altitude to high altitude
		if (checkDifference > 0)
		{
			bulletCoords.Z += checkDifference;
			pBullet->SetLocation(bulletCoords);
		}
		else
		{
			const int nowDifference = bulletCoords.Z - this->GetFloorCoordHeight(bulletCoords);

			// Less than 384 and greater than the maximum difference that can be achieved between two non cliffs
			if (nowDifference >= 256)
			{
				bulletCoords.Z -= nowDifference;
				pBullet->SetLocation(bulletCoords);
			}
		}
	}
	else
	{
		this->MovingVelocity.Z += checkDifference;
		this->MovingSpeed = this->MovingVelocity.Magnitude();
	}

	return false;
}
