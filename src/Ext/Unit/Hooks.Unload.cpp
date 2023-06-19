#include <UnitClass.h>

//#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x739AC6, UnitClass_ToggleDeployState_IgnoreDeploying, 0x6)
{
	enum {Continue = 0x739ACC, Ignore = 0x739CBF};

	GET(UnitClass* const, pThis, ECX);

	R->EAX(pThis->GetTechnoType());
	auto const pThisTechno = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if(pThisTechno->OnAmmoDepletion_DeployUnlockAmount != 0 && pThis->Ammo < pThisTechno->OnAmmoDepletion_DeployUnlockAmount)
	{
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
		return Ignore;
	}
	return Continue;
}
