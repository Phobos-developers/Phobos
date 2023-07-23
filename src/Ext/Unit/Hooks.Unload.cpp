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

DEFINE_HOOK(0x739C74, UnitClass_ToggleDeployState_ChangeAmmo, 0x6) //deploying
{
	enum { Continue = 0x739C7A };

	GET(UnitClass*, pThis, ECX);
	auto const pThisExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pThis->Deployed && !pThis->Deploying && pThisExt->Ammo_AddOnDeploy)
	{
		int ammoCalc = (pThis->Ammo + pThisExt->Ammo_AddOnDeploy) < 0 ? 0 : (pThis->Ammo + pThisExt->Ammo_AddOnDeploy);
		pThis->Ammo = ammoCalc > pThis->Type->Ammo ? pThis->Type->Ammo : ammoCalc;
	}

	R->EAX(pThis->GetTechnoType());
	return Continue;
}

DEFINE_HOOK(0x739E5A, UnitClass_ToggleSimpleDeploy_ChangeAmmo, 0x6) //undeploying
{
	enum { Continue = 0x739E60 };

	GET(UnitClass*, pThis, ECX);
	auto const pThisExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pThis->Deployed && !pThis->Undeploying && pThisExt->Ammo_AddOnDeploy)
	{
		int ammoCalc = (pThis->Ammo + pThisExt->Ammo_AddOnDeploy) < 0 ? 0 : (pThis->Ammo + pThisExt->Ammo_AddOnDeploy);
		pThis->Ammo = ammoCalc > pThis->Type->Ammo ? pThis->Type->Ammo : ammoCalc;
	}

	R->AL(pThis->IsDisguised());
	return Continue;
}

DEFINE_HOOK(0x73DE78, UnitClass_Unload_ChangeAmmo, 0x6) //converters
{
	enum { Continue = 0x73DE7E };

	GET(TechnoClass*, pThis, ESI);
	auto const pThisExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto pUnit = abstract_cast<UnitClass*>(pThis);

	if (pUnit->Type->IsSimpleDeployer && pThisExt->Ammo_AddOnDeploy && (pUnit->Type->UnloadingClass == nullptr))
	{
		int ammoCalc = (pUnit->Ammo + pThisExt->Ammo_AddOnDeploy) < 0 ? 0 : (pUnit->Ammo + pThisExt->Ammo_AddOnDeploy);
		pUnit->Ammo = ammoCalc > pUnit->Type->Ammo ? pUnit->Type->Ammo : ammoCalc;
	}

	R->AL(pUnit->Deployed);
	return Continue;
}
