#include "StraightTrajectory.h"
#include <Ext/Bullet/Body.h>

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	return true;
}

bool StraightTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	return true;
}


void StraightTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Straight.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Straight.TargetSnapDistance");
	this->PassThrough.Read(exINI, pSection, "Trajectory.Straight.PassThrough");
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return true;
}

bool StraightTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return true;
}

void StraightTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto const pType = this->GetTrajectoryType<StraightTrajectoryType>(pBullet);
	this->DetonationDistance = pType->DetonationDistance;
	this->TargetSnapDistance = pType->TargetSnapDistance;
	this->PassThrough = pType->PassThrough;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = this->GetVelocityZ(pBullet);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

bool StraightTrajectory::OnAI(BulletClass* pBullet)
{
	if (this->PassThrough)
	{
		pBullet->Data.Distance = INT_MAX;
		int maxTravelDistance = this->DetonationDistance > 0 ? this->DetonationDistance : INT_MAX;

		if (pBullet->SourceCoords.DistanceFrom(pBullet->Location) >= maxTravelDistance)
			return true;
	}
	else if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance) // Close enough
		return true;

	return false;
}

void StraightTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	if (this->PassThrough)
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
	if (!this->PassThrough)
	{
		int bulletX = pBullet->Location.X / Unsorted::LeptonsPerCell;
		int bulletY = pBullet->Location.Y / Unsorted::LeptonsPerCell;
		int targetX = pBullet->TargetCoords.X / Unsorted::LeptonsPerCell;
		int targetY = pBullet->TargetCoords.Y / Unsorted::LeptonsPerCell;

		if (bulletX == targetX && bulletY == targetY && pBullet->GetHeight() < 2 * Unsorted::LevelHeight)
			return TrajectoryCheckReturnType::Detonate; // Detonate projectile.

		if (pBullet->Location.Z < pBullet->TargetCoords.Z)
			return TrajectoryCheckReturnType::Detonate; // Detonate projectile.
	}
	else
	{
		bool isAboveTarget = false;

		if ((pBullet->Owner && pBullet->Owner->Location.Z > pBullet->TargetCoords.Z) || (!pBullet->Owner && pBullet->SourceCoords.Z > pBullet->TargetCoords.Z))
			isAboveTarget = true;

		if (isAboveTarget && pBullet->Location.Z <= pBullet->TargetCoords.Z)
			return TrajectoryCheckReturnType::Detonate; // Detonate projectile.*/
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

	if (!this->PassThrough)
		return velocity;

	if (pBullet->Owner && pBullet->Owner->Location.Z == pBullet->TargetCoords.Z)
		return 0;

	return velocity;
}
