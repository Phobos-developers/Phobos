#pragma once

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Savegame.h>

#include <BulletClass.h>

enum class TrajectoryFlag : int
{
	Invalid = -1,
	Sample = 0,
	Straight = 1,
};

class PhobosTrajectoryType
{
public:
	PhobosTrajectoryType(noinit_t) { }
	PhobosTrajectoryType(TrajectoryFlag flag) : Flag { flag } { }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

	virtual void Read(CCINIClass* const pINI, const char* pSection) = 0;

	static PhobosTrajectoryType* CreateType(CCINIClass* const pINI, const char* pSection, const char* pKey);

	static PhobosTrajectoryType* LoadFromStream(PhobosStreamReader& Stm);
	static void WriteToStream(PhobosStreamWriter& Stm, PhobosTrajectoryType* pType);
	static PhobosTrajectoryType* ProcessFromStream(PhobosStreamReader& Stm, PhobosTrajectoryType* pType);
	static PhobosTrajectoryType* ProcessFromStream(PhobosStreamWriter& Stm, PhobosTrajectoryType* pType);

	TrajectoryFlag Flag;
};

class PhobosTrajectory
{
public:
	PhobosTrajectory(noinit_t) { }
	PhobosTrajectory(TrajectoryFlag flag) : Flag { flag } { }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) = 0;
	virtual void OnAI(BulletClass* pBullet) = 0;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) = 0;

	template<typename T = PhobosTrajectoryType>
	T* GetTrajectoryType(BulletClass* pBullet) const
	{
		return static_cast<T*>(BulletTypeExt::ExtMap.Find(pBullet->Type)->TrajectoryType);
	}

	static PhobosTrajectory* CreateInstance(PhobosTrajectoryType* pType, BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity);

	static PhobosTrajectory* LoadFromStream(PhobosStreamReader& Stm);
	static void WriteToStream(PhobosStreamWriter& Stm, PhobosTrajectory* pTraj);
	static PhobosTrajectory* ProcessFromStream(PhobosStreamReader& Stm, PhobosTrajectory* pTraj);
	static PhobosTrajectory* ProcessFromStream(PhobosStreamWriter& Stm, PhobosTrajectory* pTraj);

	TrajectoryFlag Flag { TrajectoryFlag::Invalid };
};