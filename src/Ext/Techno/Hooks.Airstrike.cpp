#include "Body.h"
#include <AirstrikeClass.h>
#include <AircraftClass.h>

#include <Ext/WarheadType/Body.h>

// Allow airstrike flare draw to foot
DEFINE_JUMP(LJMP, 0x6D481D, 0x6D482D)

DEFINE_HOOK(0x6F348F, TechnoClass_WhatWeaponShouldIUse_AirstrikeTargets, 0x7)
{
	enum { ContinueCheck = 0x6F349F, Primary = 0x6F37AD, Secondary = 0x6F3807 };

	GET(TechnoClass*, pTargetTechno, EBP);

	if (!pTargetTechno)
		return Primary;

	GET(WarheadTypeClass*, pSecondaryWH, ECX);
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pSecondaryWH);

	if (!EnumFunctions::IsTechnoEligible(pTargetTechno, pWHExt->AirstrikeTargets))
		return Primary;

	if (pTargetTechno->WhatAmI() == AbstractType::Building)
		return ContinueCheck;

	const auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetTechno->GetTechnoType());
	return pTargetTypeExt->AllowAirstrike.Get(true) ? Secondary : Primary;
}

DEFINE_HOOK(0x6F34B7, TechnoClass_WhatWeaponShouldIUse_AllowAirstrike, 0x6)
{
	enum { SkipGameCode = 0x6F34BD };

	GET(BuildingTypeClass*, pThis, ECX);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);
	R->EAX(pTypeExt->AllowAirstrike.Get(pThis->CanC4));

	return SkipGameCode;
}

DEFINE_HOOK_AGAIN(0x41DA80, AirstrikeClass_ResetTarget_SetTarget, 0x6)
DEFINE_HOOK(0x41DA52, AirstrikeClass_ResetTarget_SetTarget, 0x6)// Allow airstrike target to foot
{
	if (R->Origin() == 0x41DA52)
	{
		R->EDI(R->ESI());
		return 0x41DA7C;
	}
	else
	{
		R->ESI(R->EBX());
		return 0x41DA9C;
	}
}

DEFINE_HOOK(0x41D97B, AirstrikeClass_Fire_SetAirstrike, 0x7)
{
	enum { ContinueIn = 0x41D98B, Skip = 0x41DA0B };

	GET(TechnoClass*, pTarget, ESI);

	if (pTarget->WhatAmI() == AbstractType::Building)
		return ContinueIn;

	GET(AirstrikeClass*, pThis, EDI);
	pTarget->Airstrike = pThis;

	return Skip;
}

DEFINE_JUMP(LJMP, 0x41DBD4, 0x41DBE0)

DEFINE_HOOK(0x65E97F, HouseClass_CreateAirstrike_SetTaretForUnit, 0x6)
{
	enum { SkipGameCode = 0x65E992 };

	GET_STACK(AirstrikeClass*, pThis, STACK_OFFSET(0x38, 0x1C));
	const auto pOwner = pThis->Owner;

	if (!pOwner || !pOwner->Target)
		return 0;

	if (const auto pTarget = abstract_cast<TechnoClass*>(pOwner->Target))
	{
		GET(AircraftClass*, pFirer, ESI);
		pFirer->SetTarget(pTarget);
		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x51EAE0, TechnoClass_WhatAction_AllowAirstrike, 0x7)
{
	enum { CanAirstrike = 0x51EB06, Cannot = 0x51EB15 };

	GET(ObjectClass*, pObject, ESI);

	if (const auto pTechno = abstract_cast<TechnoClass*>(pObject))
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

		if (const auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		{
			const auto pBuildingType = pBuilding->Type;
			return pTypeExt->AllowAirstrike.Get(pBuildingType->CanC4) && !pBuildingType->InvisibleInGame ? CanAirstrike : Cannot;
		}
		else
		{
			return pTypeExt->AllowAirstrike.Get(true) ? CanAirstrike : Cannot;
		}
	}

	return Cannot;
}

DEFINE_HOOK(0x70E92F, TechnoClass_UpdateAirstrikeTint, 0x5)
{
	enum { ContinueIn = 0x70E96E, Skip = 0x70EC9F };

	GET(TechnoClass*, pThis, ESI);

	return pThis->Airstrike && pThis->Airstrike->Target == pThis ? ContinueIn : Skip;
}

DEFINE_HOOK(0x70632E, TechnoClass_DrawShape_GetTintIntensity, 0x6)
{
	enum { SkipGameCode = 0x706389 };

	GET(TechnoClass*, pThis, ESI);
	GET(int, intensity, EAX);

	if (pThis->IsIronCurtained())
		intensity = pThis->GetInvulnerabilityTintIntensity(intensity);

	if (pThis->Airstrike && pThis->Airstrike->Target == pThis)
		intensity = pThis->GetAirstrikeTintIntensity(intensity);

	R->EBP(intensity);
	return SkipGameCode;
}

DEFINE_HOOK(0x706786, TechnoClass_DrawVoxel_GetTintIntensity, 0x5)
{
	enum { SkipGameCode = 0x7067E4 };

	GET(TechnoClass*, pThis, EBP);
	GET(int, intensity, EAX);

	if (pThis->IsIronCurtained())
		intensity = pThis->GetInvulnerabilityTintIntensity(intensity);

	if (pThis->Airstrike && pThis->Airstrike->Target == pThis)
		intensity = pThis->GetAirstrikeTintIntensity(intensity);

	R->EDI(intensity);
	return SkipGameCode;
}
