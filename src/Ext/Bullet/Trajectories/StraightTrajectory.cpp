#include "StraightTrajectory.h"
#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>

std::unique_ptr<PhobosTrajectory> StraightTrajectoryType::CreateInstance() const
{
	return std::make_unique<StraightTrajectory>(this);
}

template<typename T>
void StraightTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->DetonationDistance)
		.Process(this->ApplyRangeModifiers)
		.Process(this->TargetSnapDistance)
		.Process(this->PassThrough)
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

	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Straight.DetonationDistance");
	this->ApplyRangeModifiers.Read(exINI, pSection, "Trajectory.Straight.ApplyRangeModifiers");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Straight.TargetSnapDistance");
	this->PassThrough.Read(exINI, pSection, "Trajectory.Straight.PassThrough");
}

template<typename T>
void StraightTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
		.Process(this->FirerZPosition)
		.Process(this->TargetZPosition)
		.Process(this->Type)
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
	if (this->Type->ApplyRangeModifiers)
		this->DetonationDistance = Leptons(WeaponTypeExt::GetRangeWithModifiers(pBullet->WeaponType, pBullet->Owner, this->DetonationDistance));

	this->FirerZPosition = this->GetFirerZPosition(pBullet);
	this->TargetZPosition = this->GetTargetZPosition(pBullet);

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = this->GetVelocityZ(pBullet);
	pBullet->Velocity *= this->Type->Trajectory_Speed / pBullet->Velocity.Magnitude();
}

bool StraightTrajectory::OnAI(BulletClass* pBullet)
{
	if (this->Type->PassThrough)
	{
		pBullet->Data.Distance = INT_MAX;
		int maxTravelDistance = this->DetonationDistance > 0 ? this->DetonationDistance : INT_MAX;

		if (pBullet->SourceCoords.DistanceFrom(pBullet->Location) >= maxTravelDistance)
			return true;
	}
	else if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance) // Close enough
	{
		return true;
	}

	return false;
}

void StraightTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	if (this->Type->PassThrough)
		return;

	auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= this->TargetSnapDistance)
	{
		auto const pExt = BulletExt::ExtMap.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

void StraightTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType StraightTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	if (this->Type->PassThrough)
	{
		if (this->FirerZPosition > this->TargetZPosition && pBullet->Location.Z <= pBullet->TargetCoords.Z)
			return TrajectoryCheckReturnType::Detonate; // Detonate projectile.
	}
	else if (ElevationDetonationCheck(pBullet))
	{
		return TrajectoryCheckReturnType::Detonate; // Detonate projectile.
	}

	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

TrajectoryCheckReturnType StraightTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

int StraightTrajectory::GetVelocityZ(BulletClass* pBullet)
{
	int velocity = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;

	if (!this->Type->PassThrough)
		return velocity;

	if (this->FirerZPosition == this->TargetZPosition)
		return 0;

	return velocity;
}

int StraightTrajectory::GetFirerZPosition(BulletClass* pBullet)
{
	if (!pBullet)
		return 0;

	auto coords = pBullet->SourceCoords;

	if (pBullet->Owner)
	{
		auto const pCell = pBullet->Owner->GetCell();

		if (pCell)
			coords = pCell->GetCoordsWithBridge();
	}

	return coords.Z;
}

int StraightTrajectory::GetTargetZPosition(BulletClass* pBullet)
{
	if (!pBullet)
		return 0;

	auto coords = pBullet->TargetCoords;

	if (pBullet->Target)
	{
		auto const pCell = MapClass::Instance()->TryGetCellAt(pBullet->Target->GetCoords());

		if (pCell)
			coords = pCell->GetCoordsWithBridge();
	}

	return coords.Z;
}

// Should bullet detonate based on elevation conditions.
bool StraightTrajectory::ElevationDetonationCheck(BulletClass* pBullet)
{
	if (!pBullet)
		return false;

	auto const location = pBullet->Location;
	auto const target = pBullet->TargetCoords;

	// Special case - detonate if it is on same cell as target and lower or at same level as it and beneath the cell floor.
	if (pBullet->GetCell() == MapClass::Instance->TryGetCellAt(pBullet->TargetCoords)
		&& pBullet->Location.Z <= pBullet->TargetCoords.Z
		&& pBullet->Location.Z < MapClass::Instance->GetCellFloorHeight(pBullet->TargetCoords))
	{
		return true;
	}

	bool sourceObjectAboveTarget = this->FirerZPosition > this->TargetZPosition;
	bool sourceCoordAboveTarget = pBullet->SourceCoords.Z > pBullet->TargetCoords.Z;

	// If it is not coming from above then no.
	if (!sourceObjectAboveTarget || !sourceCoordAboveTarget)
		return false;

	// If it is not currently above or at target then no.
	if (pBullet->Location.Z >= pBullet->TargetCoords.Z)
		return false;

	return true;
}
