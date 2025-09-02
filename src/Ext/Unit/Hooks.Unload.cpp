#include <AnimClass.h>
#include <UnitClass.h>
#include <TechnoClass.h>
#include <TunnelLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Techno/Body.h>
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
	const auto pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	const bool canDeploy = pThis->CanDeploySlashUnload();
	R->AL(canDeploy);

	if (!canDeploy || pThis->BunkerLinkedItem)
		return;

	const int skipMinimum = pTypeExt->Ammo_DeployUnlockMinimumAmount;
	const int skipMaximum = pTypeExt->Ammo_DeployUnlockMaximumAmount;

	if (skipMinimum < 0 && skipMaximum < 0)
		return;

	const int ammo = pThis->Ammo;
	const bool moreThanMinimum = ammo >= skipMinimum;
	const bool lessThanMaximum = ammo <= skipMaximum;

	if ((skipMinimum || moreThanMinimum) && (skipMaximum || lessThanMaximum))
		return;

	R->AL(false);
}

void UnitDeployConvertHelpers::ChangeAmmo(REGISTERS* R)
{
	GET(UnitClass*, pThis, ECX);
	auto const pType = pThis->Type;

	if (pThis->Deployed && !pThis->BunkerLinkedItem && !pThis->Deploying)
	{
		if (const bool addOnDeploy = TechnoTypeExt::ExtMap.Find(pType)->Ammo_AddOnDeploy)
		{
			const int ammoCalc = std::max(pThis->Ammo + addOnDeploy, 0);
			pThis->Ammo = std::min(pType->Ammo, ammoCalc);
		}
	}

	R->EAX(pType);
}

void UnitDeployConvertHelpers::ChangeAmmoOnUnloading(REGISTERS* R)
{
	GET(UnitClass*, pThis, ESI);
	auto const pType = pThis->Type;

	if (pType->IsSimpleDeployer && !pThis->BunkerLinkedItem && pType->UnloadingClass == nullptr)
	{
		if (const bool addOnDeploy = TechnoTypeExt::ExtMap.Find(pType)->Ammo_AddOnDeploy)
		{
			const int ammoCalc = std::max(pThis->Ammo + addOnDeploy, 0);
			pThis->Ammo = std::min(pType->Ammo, ammoCalc);
		}
	}

	R->AL(pThis->Deployed);
}

DEFINE_HOOK(0x7396D2, UnitClass_TryToDeploy_Transfer, 0x5)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	if (pUnit->Type->DeployToFire && pUnit->Target)
		pStructure->LastTarget = pUnit->Target;

	const auto pStructureExt = BuildingExt::ExtMap.Find(pStructure);
	pStructureExt->DeployedTechno = true;

	return 0;
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

// Prevent subterranean units from deploying while underground.
DEFINE_HOOK(0x73D6E6, UnitClass_Unload_Subterranean, 0x6)
{
	enum { ReturnFromFunction = 0x73DFB0 };

	GET(UnitClass*, pThis, ESI);

	if (pThis->Type->Locomotor == LocomotionClass::CLSIDs::Tunnel)
	{
		auto const pLoco = static_cast<TunnelLocomotionClass*>(pThis->Locomotor.GetInterfacePtr());

		if (pLoco->State != TunnelLocomotionClass::State::Idle)
			return ReturnFromFunction;
	}

	return 0;
}

