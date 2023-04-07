#include "PhobosTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>

#include <BulletClass.h>
#include <Helpers/Macro.h>

#include "BombardTrajectory.h"
#include "StraightTrajectory.h"

bool PhobosTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	Stm.Process(this->Flag, false);
	return true;
}

bool PhobosTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	Stm.Process(this->Flag);
	return true;
}

void PhobosTrajectoryType::CreateType(PhobosTrajectoryType*& pType, CCINIClass* const pINI, const char* pSection, const char* pKey)
{
	PhobosTrajectoryType* pNewType = nullptr;
	bool bUpdateType = true;

	pINI->ReadString(pSection, pKey, "", Phobos::readBuffer);
	if (INIClass::IsBlank(Phobos::readBuffer))
		pNewType = nullptr;
	else if (_stricmp(Phobos::readBuffer, "Straight") == 0)
		pNewType = GameCreate<StraightTrajectoryType>();
	else if (_stricmp(Phobos::readBuffer, "Bombard") == 0)
		pNewType = GameCreate<BombardTrajectoryType>();
	else
		bUpdateType = false;

	if (pNewType)
		pNewType->Read(pINI, pSection);

	if (bUpdateType)
	{
		GameDelete(pType); // GameDelete already has if(pType) check here.
		pType = pNewType;
	}
}

PhobosTrajectoryType* PhobosTrajectoryType::LoadFromStream(PhobosStreamReader& Stm)
{
	PhobosTrajectoryType* pType = nullptr;
	TrajectoryFlag flag = TrajectoryFlag::Invalid;
	Stm.Process(pType, false);

	if (pType)
	{
		Stm.Process(flag, false);

		switch (flag)
		{
		case TrajectoryFlag::Straight:
			pType = GameCreate<StraightTrajectoryType>();
			break;
		case TrajectoryFlag::Bombard:
			pType = GameCreate<BombardTrajectoryType>();
			break;
		default:
			return nullptr;
		}

		pType->Flag = flag;
		pType->Load(Stm, false);
	}

	return pType;
}

void PhobosTrajectoryType::WriteToStream(PhobosStreamWriter& Stm, PhobosTrajectoryType* pType)
{
	Stm.Process(pType);
	if (pType)
	{
		Stm.Process(pType->Flag);
		pType->Save(Stm);
	}
}

PhobosTrajectoryType* PhobosTrajectoryType::ProcessFromStream(PhobosStreamReader& Stm, PhobosTrajectoryType* pType)
{
	UNREFERENCED_PARAMETER(pType);
	return LoadFromStream(Stm);
}

PhobosTrajectoryType* PhobosTrajectoryType::ProcessFromStream(PhobosStreamWriter& Stm, PhobosTrajectoryType* pType)
{
	WriteToStream(Stm, pType);
	return pType;
}

bool PhobosTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	Stm.Process(this->Flag, false);
	return true;
}

bool PhobosTrajectory::Save(PhobosStreamWriter& Stm) const
{
	Stm.Process(this->Flag);
	return true;
}

double PhobosTrajectory::GetTrajectorySpeed(BulletClass* pBullet) const
{
	if (auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type))
		return pBulletTypeExt->Trajectory_Speed;
	else
		return 100.0;
}

PhobosTrajectory* PhobosTrajectory::CreateInstance(PhobosTrajectoryType* pType, BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	PhobosTrajectory* pRet = nullptr;

	switch (pType->Flag)
	{
	case TrajectoryFlag::Straight:
		pRet = GameCreate<StraightTrajectory>(pType);
		break;

	case TrajectoryFlag::Bombard:
		pRet = GameCreate<BombardTrajectory>(pType);
		break;
	}

	if (pRet)
		pRet->OnUnlimbo(pBullet, pCoord, pVelocity);

	return pRet;
}

PhobosTrajectory* PhobosTrajectory::LoadFromStream(PhobosStreamReader& Stm)
{
	PhobosTrajectory* pTraj = nullptr;
	TrajectoryFlag flag = TrajectoryFlag::Invalid;
	Stm.Process(pTraj, false);

	if (pTraj)
	{
		Stm.Process(flag, false);

		switch (flag)
		{
		case TrajectoryFlag::Straight:
			pTraj = GameCreate<StraightTrajectory>();
			break;
		case TrajectoryFlag::Bombard:
			pTraj = GameCreate<BombardTrajectory>();
			break;
		default:
			return nullptr;
		}

		pTraj->Flag = flag;
		pTraj->Load(Stm, false);
	}

	return pTraj;
}

void PhobosTrajectory::WriteToStream(PhobosStreamWriter& Stm, PhobosTrajectory* pTraj)
{
	Stm.Process(pTraj);
	if (pTraj)
	{
		Stm.Process(pTraj->Flag);
		pTraj->Save(Stm);
	}
}

PhobosTrajectory* PhobosTrajectory::ProcessFromStream(PhobosStreamReader& Stm, PhobosTrajectory* pTraj)
{
	UNREFERENCED_PARAMETER(pTraj);
	return LoadFromStream(Stm);
}

PhobosTrajectory* PhobosTrajectory::ProcessFromStream(PhobosStreamWriter& Stm, PhobosTrajectory* pTraj)
{
	WriteToStream(Stm, pTraj);
	return pTraj;
}


DEFINE_HOOK(0x4666F7, BulletClass_AI_Trajectories, 0x6)
{
	enum { Detonate = 0x467E53 };

	GET(BulletClass*, pThis, EBP);

	auto const pExt = BulletExt::ExtMap.Find(pThis);
	bool detonate = false;

	if (auto pTraj = pExt->Trajectory)
		detonate = pTraj->OnAI(pThis);

	if (detonate && !pThis->SpawnNextAnim)
		return Detonate;

	return 0;
}

DEFINE_HOOK(0x46745C, BulletClass_AI_Position_Trajectories, 0x7)
{
	GET(BulletClass*, pThis, EBP);
	LEA_STACK(BulletVelocity*, pSpeed, STACK_OFFSET(0x1AC, -0x11C));
	LEA_STACK(BulletVelocity*, pPosition, STACK_OFFSET(0x1AC, -0x144));

	auto const pExt = BulletExt::ExtMap.Find(pThis);

	if (auto pTraj = pExt->Trajectory)
		pTraj->OnAIVelocity(pThis, pSpeed, pPosition);

	return 0;
}

DEFINE_HOOK(0x4677D3, BulletClass_AI_TargetCoordCheck_Trajectories, 0x5)
{
	enum { SkipCheck = 0x4678F8, ContinueAfterCheck = 0x467879 };

	GET(BulletClass*, pThis, EBP);

	auto const pExt = BulletExt::ExtMap.Find(pThis);

	if (auto pTraj = pExt->Trajectory)
	{
		auto flag = pTraj->OnAITargetCoordCheck(pThis);

		if (flag == TrajectoryCheckReturnType::SkipGameCheck)
			return SkipCheck;
		if (flag == TrajectoryCheckReturnType::SatisfyGameCheck)
			return ContinueAfterCheck;
	}

	return 0;
}

DEFINE_HOOK(0x467927, BulletClass_AI_TechnoCheck_Trajectories, 0x5)
{
	enum { SkipCheck = 0x467A26, ContinueAfterCheck = 0x467514 };

	GET(BulletClass*, pThis, EBP);
	GET(TechnoClass*, pTechno, ESI);

	auto const pExt = BulletExt::ExtMap.Find(pThis);

	if (auto pTraj = pExt->Trajectory)
	{
		auto flag = pTraj->OnAITechnoCheck(pThis, pTechno);

		if (flag == TrajectoryCheckReturnType::SkipGameCheck)
			return SkipCheck;
		if (flag == TrajectoryCheckReturnType::SatisfyGameCheck)
			return ContinueAfterCheck;
	}

	return 0;
}

DEFINE_HOOK(0x468B72, BulletClass_Unlimbo_Trajectories, 0x5)
{
	GET(BulletClass*, pThis, EBX);
	GET_STACK(CoordStruct*, pCoord, STACK_OFFSET(0x54, 0x4));
	GET_STACK(BulletVelocity*, pVelocity, STACK_OFFSET(0x54, 0x8));

	auto const pExt = BulletExt::ExtMap.Find(pThis);
	auto const pTypeExt = pExt->TypeExtData;

	if (pTypeExt && pTypeExt->TrajectoryType)
		pExt->Trajectory = PhobosTrajectory::CreateInstance(pTypeExt->TrajectoryType, pThis, pCoord, pVelocity);

	return 0;
}
