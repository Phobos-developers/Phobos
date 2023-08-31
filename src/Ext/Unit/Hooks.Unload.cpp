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
	GET(TechnoClass*, pThis, ESI);
	auto const pThisType = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	const bool canDeploy = pThis->CanDeploySlashUnload();
	R->AL(canDeploy);
	if (!canDeploy)
		return;

	const bool skipMinimum = pThisType->Ammo_DeployUnlockMinimumAmount < 0;
	const bool skipMaximum = pThisType->Ammo_DeployUnlockMaximumAmount < 0;

	if (skipMinimum && skipMaximum)
		return;

	const bool moreThanMinimum = pThis->Ammo >= pThisType->Ammo_DeployUnlockMinimumAmount;
	const bool lessThanMaximum = pThis->Ammo <= pThisType->Ammo_DeployUnlockMaximumAmount;

	if ((skipMinimum || moreThanMinimum) && (skipMaximum || lessThanMaximum))
		return;

	R->AL(false);
}

void UnitDeployConvertHelpers::ChangeAmmo(REGISTERS* R)
{
	GET(UnitClass*, pThis, ECX);
	auto const pThisExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Deployed && !pThis->Deploying && pThisExt->Ammo_AddOnDeploy)
	{
		const int ammoCalc = std::max(pThis->Ammo + pThisExt->Ammo_AddOnDeploy, 0);
		pThis->Ammo = std::min(pThis->Type->Ammo, ammoCalc);
	}

	R->EAX(pThis->Type);
}

void UnitDeployConvertHelpers::ChangeAmmoOnUnloading(REGISTERS* R)
{
	GET(UnitClass*, pThis, ESI);
	auto const pThisExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Type->IsSimpleDeployer && pThisExt->Ammo_AddOnDeploy && (pThis->Type->UnloadingClass == nullptr))
	{
		const int ammoCalc = std::max(pThis->Ammo + pThisExt->Ammo_AddOnDeploy, 0);
		pThis->Ammo = std::min(pThis->Type->Ammo, ammoCalc);
	}

	R->AL(pThis->Deployed);
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
	GET(TechnoClass*, pThis, ESI);

	if (abstract_cast<UnitClass*>(pThis))
		UnitDeployConvertHelpers::RemoveDeploying(R);
	else
		R->AL(pThis->CanDeploySlashUnload());

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
