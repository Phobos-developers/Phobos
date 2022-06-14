#include "FullLineTrajectory.h"
#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>

bool FullLineTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	return true;
}

bool FullLineTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	return true;
}


void FullLineTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
}

bool FullLineTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return true;
}

bool FullLineTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return true;
}

void FullLineTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	pBullet->TargetCoords.X = INT_MAX;
	pBullet->TargetCoords.Y = INT_MAX;
	pBullet->TargetCoords.Z = INT_MAX;
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

void FullLineTrajectory::OnAI(BulletClass* pBullet)
{
	double currentDistance = pBullet->SourceCoords.DistanceFrom(pBullet->Location);
	double ProjectileMaxRange = 0.0;
	pBullet->Data.Distance = INT_MAX;

	if (auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pBullet->WeaponType))
		ProjectileMaxRange = pWeaponTypeExt->ProjectileRange * 256;
	
	if (currentDistance > ProjectileMaxRange - 100)
	{
		pBullet->UnInit();
		pBullet->LastMapCoords = CellClass::Coord2Cell(pBullet->Location);
	}
}

void FullLineTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType FullLineTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType FullLineTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}
