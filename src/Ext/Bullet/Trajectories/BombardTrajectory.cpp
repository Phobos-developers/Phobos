#include "BombardTrajectory.h"

#include <Ext/BulletType/Body.h>

bool BombardTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	Stm.Process(this->Height, false);
	return true;
}

bool BombardTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	Stm.Process(this->Height);
	return true;
}

void BombardTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->Height = pINI->ReadDouble(pSection, "Trajectory.Bombard.Height", 0.0);
}

bool BombardTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);
	
	Stm
		.Process(this->IsFalling)
		.Process(this->Height)
		;

	return true;
}

bool BombardTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);
	
	Stm
		.Process(this->IsFalling)
		.Process(this->Height)
		;
	
	return true;
}

void BombardTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	this->Height = this->GetTrajectoryType<BombardTrajectoryType>(pBullet)->Height + pBullet->TargetCoords.Z;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(this->Height - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

void BombardTrajectory::OnAI(BulletClass* pBullet)
{
	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < 100) // This value maybe adjusted?
	{
		pBullet->Explode(true);
		pBullet->UnInit();
		pBullet->LastMapCoords = CellClass::Coord2Cell(pBullet->Location);
	}
}

void BombardTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	if (!this->IsFalling)
	{
		pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
		if (pBullet->Location.Z + pBullet->Velocity.Z >= this->Height)
		{
			this->IsFalling = true;
			pSpeed->X = 0.0;
			pSpeed->Y = 0.0;
			pSpeed->Z = 0.0;
			pPosition->X = pBullet->TargetCoords.X;
			pPosition->Y = pBullet->TargetCoords.Y;
		}
	}

}

TrajectoryCheckReturnType BombardTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType BombardTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}