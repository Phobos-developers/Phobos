#include "ArtilleryTrajectory.h"
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>


bool ArtilleryTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	Stm.Process(this->MaxHeight, false);
	return true;
}

bool ArtilleryTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	Stm.Process(this->MaxHeight);
	return true;
}


void ArtilleryTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->MaxHeight = pINI->ReadDouble(pSection, "Trajectory.Artillery.MaxHeight", 1500);
}

bool ArtilleryTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->MaxHeight) // Creo que esto no hace falta aquí porque no se actualiza....
		;

	return true;
}

bool ArtilleryTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->MaxHeight) // Creo que esto no hace falta aquí porque no se actualiza....
		;

	return true;
}

void ArtilleryTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	//this->Height = this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->Height;

	this->InitialTargetLocation = pBullet->TargetCoords; // YYY0
	this->InitialSourceLocation = pBullet->SourceCoords; // Y0
	CoordStruct initialSourceLocation = this->InitialSourceLocation;
	initialSourceLocation.Z = 0;
	//pBullet->GetTargetCoords()     //<--- Current Target
	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

void ArtilleryTrajectory::OnAI(BulletClass* pBullet)
{
	double maxHeight = this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->MaxHeight;

	CoordStruct currentTargetCoords = pBullet->TargetCoords; // ¿Se usa esto?
	CoordStruct bulletCoords = pBullet->Location;
	bulletCoords.Z = 0;

	CoordStruct initialTargetLocation = this->InitialTargetLocation;
	initialTargetLocation.Z = 0;
	CoordStruct initialSourceLocation = this->InitialSourceLocation;
	initialSourceLocation.Z = 0;

	double fullInitialDistance = initialSourceLocation.DistanceFrom(initialTargetLocation);
	double halfInitialDistance = fullInitialDistance / 2;
	double currentBulletDistance = initialSourceLocation.DistanceFrom(bulletCoords);

	int sinDecimalTrajectoryAngle = 90;
	double sinRadTrajectoryAngle = Math::sin(Math::deg2rad(sinDecimalTrajectoryAngle));

	double angle = (currentBulletDistance * sinDecimalTrajectoryAngle) / halfInitialDistance;
	double sinAngle = Math::sin(Math::deg2rad(angle));

	double currHeight = (sinAngle * maxHeight) / sinRadTrajectoryAngle;

	int fallAcceleration = 0;

	// Needed for increasing the bullet aim when the target is in lower locations compared to the source
	if (angle >= 90 && this->InitialSourceLocation.Z > pBullet->TargetCoords.Z)
		fallAcceleration = ((int)angle - 90) * 21;

	if (currHeight != 0)
		pBullet->Location.Z = this->InitialSourceLocation.Z + (int)currHeight - fallAcceleration;

	// Close enough
	double closeEnough = pBullet->TargetCoords.DistanceFrom(pBullet->Location);
	//closeEnough < 150 || 
	if (closeEnough < 100 || fallAcceleration > 0 && pBullet->Location.Z < pBullet->Target->GetCoords().Z)// || (closeEnough < 550.0 && currHeight < 0)) // This value maybe adjusted?
	{
		auto pBulletExt = BulletExt::ExtMap.Find(pBullet);

		if (pBulletExt && pBulletExt->LaserTrails.size())
			pBulletExt->LaserTrails.clear();

		if (pBullet->Target->WhatAmI() == AbstractType::Unit ||
			pBullet->Target->WhatAmI() == AbstractType::Building ||
			pBullet->Target->WhatAmI() == AbstractType::Infantry ||
			pBullet->Target->WhatAmI() == AbstractType::Aircraft)
		{
			pBullet->Location = pBullet->TargetCoords;
		}

		pBullet->Explode(true);
		pBullet->UnInit();
		pBullet->LastMapCoords = CellClass::Coord2Cell(pBullet->Location);
	}
}

void ArtilleryTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType ArtilleryTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType ArtilleryTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}
