#include <UnitClass.h>
#include <TechnoClass.h>

#include <Ext/TechnoType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x73FFE6, UnitClass_WhatAction_RemoveDeploying, 0xA)
{
	enum { Continue = 0x73FFF0 };

	GET(UnitClass*, pThis, ESI);
	auto const pThisType = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	R->AL(pThis->CanDeploySlashUnload());
	if(pThis->CanDeploySlashUnload())
	{
		if(pThisType->OnAmmoDepletion_DeployUnlockAmount != 0 && pThis->Ammo < pThisType->OnAmmoDepletion_DeployUnlockAmount)
			R->AL(false);
	}

	return Continue;
}

DEFINE_HOOK(0x730C70, DeployClass_Execute_RemoveDeploying, 0xA)
{
	enum { Continue = 0x730C7A };

	GET(TechnoClass*, pThis, ESI);
	auto const pThisType = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	R->AL(pThis->CanDeploySlashUnload());
	if(pThis->CanDeploySlashUnload())
	{
		if(pThisType->OnAmmoDepletion_DeployUnlockAmount != 0 && pThis->Ammo < pThisType->OnAmmoDepletion_DeployUnlockAmount)
			R->AL(false);
	}

	return Continue;
}
