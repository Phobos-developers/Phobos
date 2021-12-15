#include "StraightTrajactory.h"
#include <Ext/BulletType/Body.h>

bool StraightTrajactoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	bool ret = this->PhobosTrajactoryType::Load(Stm, RegisterForChange);
	
	if (ret)
	{

	}

	return ret;
}

bool StraightTrajactoryType::Save(PhobosStreamWriter& Stm) const
{
	bool ret = this->PhobosTrajactoryType::Save(Stm);
	
	

	return ret;
}


void StraightTrajactoryType::Read(CCINIClass* const pINI, const char* pSection, const char* pMainKey)
{
	this->PhobosTrajactoryType::Read(pINI, pSection, pMainKey);

	
}

bool StraightTrajactory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return true;
}

bool StraightTrajactory::Save(PhobosStreamWriter& Stm) const
{

	return true;
}

void StraightTrajactory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= pBullet->WeaponType->Speed / pBullet->Velocity.Magnitude();
}

void StraightTrajactory::OnAI(BulletClass* pBullet)
{
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < 100)
	{
		pBullet->Detonate(pBullet->Location);
		pBullet->UnInit();
		pBullet->LastMapCoords = CellClass::Coord2Cell(pBullet->Location);
	}
	
}

void StraightTrajactory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}
