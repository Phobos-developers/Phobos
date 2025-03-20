#pragma once

#include "PhobosTrajectory.h"

class VirtualTrajectoryType : public PhobosTrajectoryType
{
public:
	VirtualTrajectoryType() : PhobosTrajectoryType()
		, VirtualSourceCoord { { 0, 0, 0 } }
		, VirtualTargetCoord { { 0, 0, 0 } }
		, AllowFirerTurning { true }
	{ }

	Valueable<PartialVector3D<int>> VirtualSourceCoord; // Initial location of the projectile
	Valueable<PartialVector3D<int>> VirtualTargetCoord; // move to location of the projectile
	Valueable<bool> AllowFirerTurning; // Allow firer not facing projectiles

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
//	virtual void Read(CCINIClass* const pINI, const char* pSection) override; // Read separately

private:
	template <typename T>
	void Serialize(T& Stm);
};

class VirtualTrajectory : public PhobosTrajectory
{
public:
	VirtualTrajectory() { }
	VirtualTrajectory(VirtualTrajectoryType const* trajType, BulletClass* pBullet)
		: PhobosTrajectory(trajType, pBullet)
		, SurfaceFirerID { 0 }
	{ }

	DWORD SurfaceFirerID; // UniqueID of the "launcher"

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual void OnUnlimbo() override;
	virtual bool OnEarlyUpdate() override;

	bool InvalidFireCondition(TechnoClass* pTechno);

private:
	template <typename T>
	void Serialize(T& Stm);
};
