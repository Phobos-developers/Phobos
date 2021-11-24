                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 #include "StraightTrajactory.h"

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
	return Stm.Load(this->StraightSpeed);
}

bool StraightTrajactory::Save(PhobosStreamWriter& Stm) const
{
	Stm.Save(this->StraightSpeed);

	return true;
}

void StraightTrajactory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	this->StraightSpeed.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	this->StraightSpeed.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	this->StraightSpeed.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	this->StraightSpeed *= pBullet->WeaponType->Speed / this->StraightSpeed.Magnitude();
	pBullet->Velocity = this->StraightSpeed;
}

void StraightTrajactory::OnAI(BulletClass* pBullet)
{
	pBullet->Velocity = this->StraightSpeed;
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < 100)
	{
		pBullet->Detonate(pBullet->Location);
		pBullet->UnInit();
		pBullet->LastMapCoords = CellClass::Coord2Cell(pBullet->Location);
	}
	
}
