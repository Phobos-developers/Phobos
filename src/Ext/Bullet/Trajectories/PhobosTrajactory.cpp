#include "PhobosTrajactory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

#include <BulletClass.h>
#include <Helpers/Macro.h>

bool PhobosTrajactoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Load(this->IsTrajactoryEnabled);
}

bool PhobosTrajactoryType::Save(PhobosStreamWriter& Stm) const
{
	Stm.Save(this->IsTrajactoryEnabled);
	return true;
}

void PhobosTrajactoryType::Read(CCINIClass* const pINI, const char* pSection, const char* pMainKey)
{
	this->IsTrajactoryEnabled = pINI->ReadBool(pSection, pMainKey, false);
}

DEFINE_HOOK(0x4666F7, BulletClass_AI_Trajectories, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	auto const pData = BulletTypeExt::ExtMap.Find(pThis->Type);
	auto const pExt = BulletExt::ExtMap.Find(pThis);

	if (pData->Trajactory_Straight.IsTrajactoryEnabled)
		pExt->Trajactory_Straight.OnAI(pThis);

	return 0;
}

DEFINE_HOOK(0x468B72, BulletClass_Unlimbo_Trajectories, 0x5)
{
	GET(BulletClass*, pThis, EBX);
	GET_STACK(CoordStruct*, pCoord, STACK_OFFS(0x54, -0x4)); // GET_BASE(, , 0x8)
	GET_STACK(BulletVelocity*, pVelocity, STACK_OFFS(0x54, -0x8)); // GET_BASE(, , 0xC)

	auto const pData = BulletTypeExt::ExtMap.Find(pThis->Type);
	auto const pExt = BulletExt::ExtMap.Find(pThis);

	if (pData->Trajactory_Straight.IsTrajactoryEnabled)
		pExt->Trajactory_Straight.OnUnlimbo(pThis, pCoord, pVelocity);

	return 0;
}