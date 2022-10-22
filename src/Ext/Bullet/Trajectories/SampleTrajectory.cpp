#include "SampleTrajectory.h"

#include <Ext/BulletType/Body.h>

// Save and Load
bool SampleTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	Stm.Process(this->ExtraHeight, false);
	return true;
}

bool SampleTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	Stm.Process(this->ExtraHeight);
	return true;
}

// INI reading stuff
void SampleTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->ExtraHeight = pINI->ReadDouble(pSection, "Trajectory.Sample.ExtraHeight", 1145.14);
}

bool SampleTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);
	Stm.Process(this->IsFalling, false);
	return true;
}

bool SampleTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);
	Stm.Process(this->IsFalling);
	return true;
}

// Do some math here to set the initial speed of your proj
void SampleTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto extraZ = this->GetTrajectoryType<SampleTrajectoryType>(pBullet)->ExtraHeight;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z + extraZ - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

// Some early checks here, returns whether or not to detonate the bullet
bool SampleTrajectory::OnAI(BulletClass* pBullet)
{
	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < 100)
	{
		pBullet->Detonate(pBullet->Location);
		pBullet->UnInit();
		pBullet->LastMapCoords = CellClass::Coord2Cell(pBullet->Location);
	}
}

// Where you update the speed and position
// pSpeed: The speed of this proj in the next frame
// pPosition: Current position of the proj, and in the next frame it will be *pSpeed + *pPosition
void SampleTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	if (!this->IsFalling)
	{
		pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
		double dx = pBullet->TargetCoords.X - pBullet->Location.X;
		double dy = pBullet->TargetCoords.Y - pBullet->Location.Y;
		if (dx * dx + dy * dy < pBullet->Velocity.X * pBullet->Velocity.X + pBullet->Velocity.Y * pBullet->Velocity.Y)
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

// Where additional checks based on bullet reaching its target coordinate can be done.
// Vanilla code will do additional checks regarding buildings on target coordinate and Vertical projectiles and will detonate the projectile if they pass.
// Return value determines what is done regards to the game checks: they can be skipped, executed as normal or treated as if the condition is already satisfied.
TrajectoryCheckReturnType SampleTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

// Where additional checks based on a TechnoClass instance in same cell as the bullet can be done.
// Vanilla code will do additional trajectory alterations here if there is an enemy techno in the cell.
// Return value determines what is done regards to the game checks: they can be skipped, executed as normal or treated as if the condition is already satisfied.
// pTechno: TechnoClass instance in same cell as the bullet.
TrajectoryCheckReturnType SampleTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}
