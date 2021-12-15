#include "SampleTrajactory.h"

#include <Ext/BulletType/Body.h>

// Save and Load
bool SampleTrajactoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	bool ret = this->PhobosTrajactoryType::Load(Stm, RegisterForChange);

	if (ret)
	{
		ret &= Stm.Load(this->ExtraHeight);
	}

	return ret;
}

bool SampleTrajactoryType::Save(PhobosStreamWriter& Stm) const
{
	bool ret = this->PhobosTrajactoryType::Save(Stm);

	Stm.Save(this->ExtraHeight);

	return ret;
}

// INI reading stuff
void SampleTrajactoryType::Read(CCINIClass* const pINI, const char* pSection, const char* pMainKey)
{
	this->PhobosTrajactoryType::Read(pINI, pSection, pMainKey);

	this->ExtraHeight = pINI->ReadDouble(pSection, "Trajactory.Sample.ExtraHeight", 0.0);
}

bool SampleTrajactory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	Stm.Process(this->IsFalling, RegisterForChange);
	return true;
}

bool SampleTrajactory::Save(PhobosStreamWriter& Stm) const
{
	Stm.Process(this->IsFalling);
	return true;
}

// Do some math here to set the initial speed of your proj
void SampleTrajactory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto extraZ = BulletTypeExt::ExtMap.Find(pBullet->Type)->Trajactory_Sample.ExtraHeight;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z + extraZ - pBullet->SourceCoords.Z);
	pBullet->Velocity *= pBullet->WeaponType->Speed / pBullet->Velocity.Magnitude();
}

// Some early checks here
void SampleTrajactory::OnAI(BulletClass* pBullet)
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
void SampleTrajactory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
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
