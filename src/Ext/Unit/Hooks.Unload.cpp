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
		if(pThisType->Ammo_DeployUnlockAmount != 0 && pThis->Ammo < pThisType->Ammo_DeployUnlockAmount)
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
		if(pThisType->Ammo_DeployUnlockAmount != 0 && pThis->Ammo < pThisType->Ammo_DeployUnlockAmount)
			R->AL(false);
	}

	return Continue;
}

DEFINE_HOOK(0x73D63B, UnitClass_Unload_ChangeAmmo, 0x6)
{
	enum { Continue = 0x73D647, SkipBunker = 0x73D6E6 };

	GET(TechnoClass*, pThis, ECX);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if ((pTypeExt->Ammo_AddOnDeploy != 0) && (pThis->WhatAmI() == AbstractType::Unit))
		pThis->Ammo += pTypeExt->Ammo_AddOnDeploy;

	auto const pUnit = abstract_cast<UnitClass*>(pThis);
	if(!pUnit->BunkerLinkedItem)
		return SkipBunker;
	return Continue;
}
