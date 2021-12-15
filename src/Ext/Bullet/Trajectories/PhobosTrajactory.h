#pragma once

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Savegame.h>

#include <BulletClass.h>

class PhobosTrajactoryType
{
public:
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) = 0;
	virtual bool Save(PhobosStreamWriter& Stm) const = 0;

	virtual void Read(CCINIClass* const pINI, const char* pSection, const char* pMainKey) = 0;

	bool IsTrajactoryEnabled;
};

class PhobosTrajactory
{
public:
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) = 0;
	virtual bool Save(PhobosStreamWriter& Stm) const = 0;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) = 0;
	virtual void OnAI(BulletClass* pBullet) = 0;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) = 0;
};