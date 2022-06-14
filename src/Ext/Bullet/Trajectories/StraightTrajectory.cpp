#include "StraightTrajectory.h"
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);

	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		;

	return true;
}

bool StraightTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);

	Stm
		.Process(this->SnapOnTarget)
		.Process(this->SnapThreshold)
		;

	return true;
}


void StraightTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!pINI->GetSection(pSection))
		return;

	this->PhobosTrajectoryType::Read(pINI, pSection);

	INI_EX exINI(pINI);

	this->SnapOnTarget.Read(exINI, pSection, "Trajectory.Straight.SnapOnTarget");
	this->SnapThreshold.Read(exINI, pSection, "Trajectory.Straight.SnapThreshold");
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		;

	return true;
}

bool StraightTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->SnapOnTarget)
		.Process(this->SnapThreshold)
		;

	return true;
}

void StraightTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto type = this->GetTrajectoryType<StraightTrajectoryType>(pBullet);
	this->SnapOnTarget = type->SnapOnTarget;
	this->SnapThreshold = type->SnapThreshold;
	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

bool StraightTrajectory::OnAI(BulletClass* pBullet)
{
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	return false;
}

void StraightTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	if (this->SnapOnTarget)
	{
		auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
		auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

		if (pCoords.DistanceFrom(pBullet->Location) <= this->SnapThreshold)
		{
			auto const pExt = BulletExt::ExtMap.Find(pBullet);
			pExt->SnappedToTarget = true;
			pBullet->SetLocation(pCoords);
		}
	}
}

void StraightTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType StraightTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	int bulletX = pBullet->Location.X / Unsorted::LeptonsPerCell;
	int bulletY = pBullet->Location.Y / Unsorted::LeptonsPerCell;
	int targetX = pBullet->TargetCoords.X / Unsorted::LeptonsPerCell;
	int targetY = pBullet->TargetCoords.Y / Unsorted::LeptonsPerCell;

	if (bulletX == targetX && bulletY == targetY && pBullet->GetHeight() < 2 * Unsorted::LevelHeight)
		return TrajectoryCheckReturnType::Detonate; // Detonate projectile.

	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

TrajectoryCheckReturnType StraightTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

