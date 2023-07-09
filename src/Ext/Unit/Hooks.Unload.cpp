#include <UnitClass.h>
#include <GameStrings.h>

#include <Ext/TechnoType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x7394FF, UnitClass_TryToDeploy_CantDeployVoice, 0x10)
{
	GET(UnitClass* const, pThis, EBP);

	auto const pThisTechno = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if(pThis->Owner->IsControlledByCurrentPlayer())
	{
		if(pThisTechno->VoiceCantDeploy.isset())
			VocClass::PlayGlobal(pThisTechno->VoiceCantDeploy, 0x2000, 1.0);
		else
			VoxClass::Play(GameStrings::EVA_CannotDeployHere);
	}

	return 0x73950F;
}

DEFINE_HOOK(0x739AC6, UnitClass_ToggleDeployState_IgnoreDeploying, 0x6)
{
	enum {Continue = 0x739ACC, Ignore = 0x739CBF};

	GET(UnitClass* const, pThis, ECX);

	R->EAX(pThis->GetTechnoType());
	auto const pThisTechno = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if(pThisTechno->OnAmmoDepletion_DeployUnlockAmount != 0 && pThis->Ammo < pThisTechno->OnAmmoDepletion_DeployUnlockAmount)
	{
		if(pThisTechno->VoiceCantDeploy.isset() && pThis->Owner->IsControlledByCurrentPlayer())
			VocClass::PlayGlobal(pThisTechno->VoiceCantDeploy, 0x2000, 1.0);
		return Ignore;
	}

	return Continue;
}

//blockForDeployed
DEFINE_HOOK(0x739CD7, UnitClass_ToggleSimpleDeploy_IgnoreDeploying, 0x6)
{
	enum {Continue = 0x739CDD, Ignore = 0x739EB8};

	GET(UnitClass* const, pThis, ECX);

	R->AL(pThis->Deployed);
	auto const pThisTechno = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if(pThisTechno->OnAmmoDepletion_DeployUnlockAmount != 0 && pThis->Ammo < pThisTechno->OnAmmoDepletion_DeployUnlockAmount)
	{
		if(pThisTechno->VoiceCantDeploy.isset() && pThis->Owner->IsControlledByCurrentPlayer())
			VocClass::PlayGlobal(pThisTechno->VoiceCantDeploy, 0x2000, 1.0);
		return Ignore;
	}

	return Continue;
}
