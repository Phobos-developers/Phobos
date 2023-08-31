#include <UnitClass.h>
#include <TechnoClass.h>

#include <Ext/TechnoType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

namespace UnitDeployConvertHelpers
{
	void RemoveDeploying(REGISTERS* R);
	void ChangeAmmo(REGISTERS* R);
	void ChangeAmmoOnUnloading(REGISTERS* R);
}

void UnitDeployConvertHelpers::RemoveDeploying(REGISTERS* R)
{
	GET(UnitClass*, pThis, ESI);
	auto const pThisType = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	R->AL(pThis->CanDeploySlashUnload());
	if (!pThis->CanDeploySlashUnload())
		return;

	if (((pThisType->Ammo_DeployUnlockMinimumAmount >= 0)
			|| (pThis->Ammo < pThisType->Ammo_DeployUnlockMinimumAmount))
		&& ((pThisType->Ammo_DeployUnlockMaximumAmount >= 0)
			|| (pThis->Ammo > pThisType->Ammo_DeployUnlockMaximumAmount)))
	{
		R->AL(false);
	}
}

void UnitDeployConvertHelpers::ChangeAmmo(REGISTERS* R)
{
	GET(UnitClass*, pThis, ECX);
	auto const pThisExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pThis->Deployed && !pThis->Deploying && pThisExt->Ammo_AddOnDeploy)
	{
		const int ammoCalc = std::max(pThis->Ammo + pThisExt->Ammo_AddOnDeploy, 0);
		pThis->Ammo = std::min(pThis->Type->Ammo, ammoCalc);
	}

	R->EAX(pThis->GetTechnoType());
}

void UnitDeployConvertHelpers::ChangeAmmoOnUnloading(REGISTERS* R)
{
	GET(TechnoClass*, pThis, ESI);
	auto const pThisExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto pUnit = abstract_cast<UnitClass*>(pThis);

	if (pUnit->Type->IsSimpleDeployer && pThisExt->Ammo_AddOnDeploy && (pUnit->Type->UnloadingClass == nullptr))
	{
		const int ammoCalc = std::max(pUnit->Ammo + pThisExt->Ammo_AddOnDeploy, 0);
		pUnit->Ammo = std::min(pUnit->Type->Ammo, ammoCalc);
	}

	R->AL(pUnit->Deployed);
}

DEFINE_HOOK(0x73FFE6, UnitClass_WhatAction_RemoveDeploying, 0xA)
{
	enum { Continue = 0x73FFF0 };
	UnitDeployConvertHelpers::RemoveDeploying(R);
	return Continue;
}

DEFINE_HOOK(0x730C70, DeployClass_Execute_RemoveDeploying, 0xA)
{
	enum { Continue = 0x730C7A };
	UnitDeployConvertHelpers::RemoveDeploying(R);
	return Continue;
}

DEFINE_HOOK(0x739C74, UnitClass_ToggleDeployState_ChangeAmmo, 0x6) // deploying
{
	enum { Continue = 0x739C7A };
	UnitDeployConvertHelpers::ChangeAmmo(R);
	return Continue;
}

DEFINE_HOOK(0x739E5A, UnitClass_ToggleSimpleDeploy_ChangeAmmo, 0x6) // undeploying
{
	enum { Continue = 0x739E60 };
	UnitDeployConvertHelpers::ChangeAmmo(R);
	return Continue;
}

DEFINE_HOOK(0x73DE78, UnitClass_Unload_ChangeAmmo, 0x6) // converters
{
	enum { Continue = 0x73DE7E };
	UnitDeployConvertHelpers::ChangeAmmoOnUnloading(R);
	return Continue;
}
