#pragma once

#include "PhobosTrajactory.h"

/*
* This is a sample class telling you how to make a new type of trajactory
* Author: secsome
*/

// Used in BulletTypeExt
class SampleTrajactoryType final : public PhobosTrajactoryType
{
public:
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection, const char* pMainKey) override;

	// Your type properties
	double ExtraHeight;
};

// Used in BulletExt
class SampleTrajactory final : public PhobosTrajactory
{
public:
	// Construct it here
	SampleTrajactory() : PhobosTrajactory()
		, IsFalling { false }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual void OnAI(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;

	// Your properties
	bool IsFalling;
};