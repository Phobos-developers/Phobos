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
//		.Process(this->Laser) // Should not save
		.Process(this->LaserTimer)
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

void EngraveTrajectory::OnUnlimbo()
{
	this->VirtualTrajectory::OnUnlimbo();

	// Engrave
	const auto pBullet = this->Bullet;
	const auto pWeapon = pBullet->WeaponType;

	if (pWeapon && pWeapon->IsLaser)
		this->LaserTimer.Start(pWeapon->LaserDuration);

	// Waiting for launch trigger
	if (!BulletExt::ExtMap.Find(pBullet)->DispersedTrajectory)
		this->OpenFire();
}

bool EngraveTrajectory::OnEarlyUpdate()
{
	if (this->VirtualTrajectory::OnEarlyUpdate())
		return true;

	// Draw laser
	if (this->Laser)
		this->UpdateEngraveLaser();
	else if (this->LaserTimer.HasTimeLeft())
		this->DrawEngraveLaser();

	return false;
}

bool EngraveTrajectory::OnVelocityCheck()
{
	const auto pType = this->Type;

	if (pType->AttachToTarget || pType->UpdateDirection)
		this->ChangeVelocity();

	if (!this->TargetIsInAir && this->PlaceOnCorrectHeight())
		return true;

	return this->PhobosTrajectory::OnVelocityCheck();
}

void EngraveTrajectory::OnPreDetonate()
{
	this->PhobosTrajectory::OnPreDetonate();

	if (const auto pLaser = this->Laser)
	{
		// Auto free
		pLaser->Duration = 0;
		this->Laser = nullptr;
	}
}

void EngraveTrajectory::OpenFire()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	const auto pFirer = pBullet->Owner;
	auto virtualSource = PhobosTrajectory::Coord2Point(pType->VirtualSourceCoord.Get());
	auto virtualTarget = PhobosTrajectory::Coord2Point(pType->VirtualTargetCoord.Get());

	// Mirror trajectory
	if (!this->NotMainWeapon && pType->MirrorCoord && this->CurrentBurst < 0)
	{
		virtualSource.Y = -virtualSource.Y;
		virtualTarget.Y = -virtualTarget.Y;
	}

	// To be used later, no reference
	auto source = pBullet->SourceCoords;
	auto target = pBullet->TargetCoords;
	this->RotateRadian = this->Get2DOpRadian((pFirer ? pFirer->GetCoords() : source), target);

	// Special case: Starting from the launch position
	if (virtualSource.X != 0 || virtualSource.Y != 0)
		source = target + PhobosTrajectory::Point2Coord(PhobosTrajectory::PointRotate(virtualSource, this->RotateRadian));

	// If the target is in the air, there is no need to attach it to the ground
	if (!this->TargetIsInAir)
		source.Z = this->GetFloorCoordHeight(source);

	// set initial status
	pBullet->SetLocation(source);
	target += PhobosTrajectory::Point2Coord(PhobosTrajectory::PointRotate(virtualTarget, this->RotateRadian));

	this->MovingVelocity.X = target.X - source.X;
	this->MovingVelocity.Y = target.Y - source.Y;
	this->MovingVelocity.Z = 0;

	if (this->CalculateBulletVelocity(pType->Speed))
		this->Status |= TrajectoryStatus::Detonate;

	this->PhobosTrajectory::OpenFire();
}

bool EngraveTrajectory::CalculateBulletVelocity(const double speed)
{
	// Only call once
	// Substitute the speed to calculate velocity
	double velocityLength = this->MovingVelocity.Magnitude();

	if (velocityLength < PhobosTrajectory::Epsilon)
		return true;

	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	const auto pFirer = pBullet->Owner;

	// Calculate additional range
	if (pType->ApplyRangeModifiers && pFirer)
	{
		if (const auto pWeapon = pBullet->WeaponType)
			velocityLength = static_cast<double>(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer, static_cast<int>(velocityLength)));
	}

	// Automatically calculate duration
	if (pType->Duration <= 0)
		this->DurationTimer.Start(static_cast<int>(velocityLength / pType->Speed) + 1);
	else
		this->DurationTimer.Start(pType->Duration);

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
	const auto pType = this->Type;

	// The center is located on the target
	if (pType->AttachToTarget)
	{
		// No need to synchronize the target again
		if (!pType->Synchronize)
		{
			if (const auto pTarget = pBullet->Target)
				pBullet->TargetCoords = pTarget->GetCoords();
		}
	}

	// The angle will be updated according to the orientation
	if (pType->UpdateDirection)
	{
		const auto pFirer = pBullet->Owner;
		this->RotateRadian = this->Get2DOpRadian((pFirer ? pFirer->GetCoords() : pBullet->SourceCoords), pBullet->TargetCoords);
	}

	// Recalculate speed and position
	auto virtualSource = PhobosTrajectory::Coord2Point(pType->VirtualSourceCoord.Get());
	auto virtualTarget = PhobosTrajectory::Coord2Point(pType->VirtualTargetCoord.Get());

	if (!this->NotMainWeapon && pType->MirrorCoord && this->CurrentBurst < 0)
	{
		virtualSource.Y = -virtualSource.Y;
		virtualTarget.Y = -virtualTarget.Y;
	}

	const double path = (this->DurationTimer.CurrentTime - this->DurationTimer.StartTime + 1) * pType->Speed;
	auto source = PhobosTrajectory::Coord2Point(pBullet->SourceCoords);
	auto target = PhobosTrajectory::Coord2Point(pBullet->TargetCoords);

	// Special case: Starting from the launch position
	if (virtualSource.X != 0 || virtualSource.Y != 0)
		source = target + PhobosTrajectory::PointRotate(virtualSource, this->RotateRadian);

	target += PhobosTrajectory::PointRotate(virtualTarget, this->RotateRadian);
	const auto delta = target - source;
	const double distance = delta.Magnitude();

	if (distance < PhobosTrajectory::Epsilon)
	{
		this->Status |= TrajectoryStatus::Detonate;
		return;
	}

	const auto newLocation = PhobosTrajectory::Point2Coord((source + delta * (path / distance)), pBullet->TargetCoords.Z);
	this->MovingVelocity = PhobosTrajectory::Coord2Vector(newLocation - pBullet->Location);
	this->MovingSpeed = this->MovingVelocity.Magnitude();
}

bool EngraveTrajectory::PlaceOnCorrectHeight()
{
	const auto pBullet = this->Bullet;
	auto bulletCoords = pBullet->Location;
	const auto futureCoords = bulletCoords + PhobosTrajectory::Vector2Coord(this->MovingVelocity);

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

void EngraveTrajectory::DrawEngraveLaser()
{
	const auto pBullet = this->Bullet;
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon || !pWeapon->IsLaser)
		return;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto pFirer = pBullet->Owner;
	const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
	auto fireCoord = pBullet->SourceCoords;

	// Find the outermost transporter
	pFirer = this->GetSurfaceFirer(pFirer);

	// Considering that the CurrentBurstIndex may be different, it is not possible to call existing functions
	if (!this->NotMainWeapon && pFirer && !pFirer->InLimbo)
		fireCoord = TechnoExt::GetFLHAbsoluteCoords(pFirer, this->FLHCoord, pFirer->HasTurret());

	// Draw laser from head to tail
	if (pWeapon->IsHouseColor || pWeaponExt->Laser_IsSingleColor)
	{
		const auto pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, ((pWeapon->IsHouseColor && pOwner) ? pOwner->LaserColor : pWeapon->LaserInnerColor), ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, INT_MAX);
		this->Laser = pLaser;
		pLaser->IsHouseColor = true;
		pLaser->Thickness = pWeaponExt->LaserThickness;
		pLaser->IsSupported = pLaser->Thickness > 3;
		pLaser->Fades = false;
		pLaser->Progress.Value = 0;
	}
	else
	{
		const auto pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, pWeapon->LaserInnerColor, pWeapon->LaserOuterColor, pWeapon->LaserOuterSpread, INT_MAX);
		this->Laser = pLaser;
		pLaser->IsHouseColor = false;
		pLaser->Thickness = 3;
		pLaser->IsSupported = false;
		pLaser->Fades = false;
		pLaser->Progress.Value = 0;
	}
}

void EngraveTrajectory::UpdateEngraveLaser()
{
	const auto pLaser = this->Laser;

	// Check whether the timer expired
	if (!this->LaserTimer.HasTimeLeft())
	{
		// Auto free
		pLaser->Duration = 0;
		this->Laser = nullptr;
		return;
	}

	const auto pBullet = this->Bullet;

	// Find the outermost transporter
	const auto pFirer = this->GetSurfaceFirer(pBullet->Owner);

	// Considering that the CurrentBurstIndex may be different, it is not possible to call existing functions
	if (!this->NotMainWeapon && pFirer && !pFirer->InLimbo)
		pLaser->Source = TechnoExt::GetFLHAbsoluteCoords(pFirer, this->FLHCoord, pFirer->HasTurret());

	pLaser->Target = pBullet->Location;
}
