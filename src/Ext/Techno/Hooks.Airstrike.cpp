#include "Body.h"
#include <AirstrikeClass.h>
#include <AircraftClass.h>

#include <Ext/WarheadType/Body.h>

// Allow airstrike flare draw to foot
DEFINE_JUMP(LJMP, 0x6D481D, 0x6D482D)

DEFINE_HOOK(0x6F348F, TechnoClass_WhatWeaponShouldIUse_Airstrike, 0x7)
{
	enum { Primary = 0x6F37AD, Secondary = 0x6F3807 };

	GET(TechnoClass*, pTargetTechno, EBP);
	GET(WarheadTypeClass*, pSecondaryWH, ECX);

	if (!pTargetTechno)
		return Primary;

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pSecondaryWH);

	if (!EnumFunctions::IsTechnoEligible(pTargetTechno, pWHExt->AirstrikeTargets))
		return Primary;

	const auto pTargetType = pTargetTechno->GetTechnoType();
	const auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

	if (pTargetTechno->AbstractFlags & AbstractFlags::Foot)
		return pTargetTypeExt->AllowAirstrike.Get(true) ? Secondary : Primary;

	return pTargetTypeExt->AllowAirstrike.Get(static_cast<BuildingTypeClass*>(pTargetType)->CanC4) && (!pTargetType->ResourceDestination || !pTargetType->ResourceGatherer) ? Secondary : Primary;
}

DEFINE_HOOK(0x41D97B, AirstrikeClass_Fire_SetAirstrike, 0x7)
{
	enum { ContinueIn = 0x41D9A0, Skip = 0x41DA0B };

	GET(AirstrikeClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);

	TechnoExt::ExtMap.Find(pTarget)->AirstrikeTargetingMe = pThis;
	pTarget->StartAirstrikeTimer(100000);

	return pTarget->WhatAmI() == AbstractType::Building ? ContinueIn : Skip;
}

DEFINE_HOOK(0x41DA52, AirstrikeClass_ResetTarget_OriginalTarget, 0x6)
{
	enum { SkipGameCode = 0x41DA7C };

	R->EDI(R->ESI());

	return SkipGameCode;
}

DEFINE_HOOK(0x41DA80, AirstrikeClass_ResetTarget_NewTarget, 0x6)
{
	enum { SkipGameCode = 0x41DA9C };

	R->ESI(R->EBX());

	return SkipGameCode;
}

DEFINE_HOOK(0x41DAA4, AirstrikeClass_ResetTarget_ResetForOldTarget, 0xA)
{
	enum { SkipGameCode = 0x41DAAE };

	GET(TechnoClass*, pTargetTechno, EDI);

	TechnoExt::ExtMap.Find(pTargetTechno)->AirstrikeTargetingMe = nullptr;

	return SkipGameCode;
}

DEFINE_HOOK(0x41DAD4, AirstrikeClass_ResetTarget_ResetForNewTarget, 0x6)
{
	enum { SkipGameCode = 0x41DADA };

	GET(AirstrikeClass*, pThis, EBP);
	GET(TechnoClass*, pTargetTechno, ESI);

	TechnoExt::ExtMap.Find(pTargetTechno)->AirstrikeTargetingMe = pThis;

	return SkipGameCode;
}

DEFINE_HOOK(0x41DBD4, AirstrikeClass_Stop_ResetForTarget, 0x7)
{
	enum { SkipGameCode = 0x41DC3A };

	GET(AirstrikeClass*, pThis, EBP);
	GET(ObjectClass*, pTarget, ESI);

	if (const auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
	{
		const auto& array = Make_Global<DynamicVectorClass<AirstrikeClass*>>(0x889FB8);
		AirstrikeClass* pLastTargetingMe = nullptr;

		for (int idx = array.Count - 1; idx >= 0; --idx)
		{
			const auto pAirstrike = array.Items[idx];

			if (pAirstrike != pThis && pAirstrike->Target == pTarget)
			{
				pLastTargetingMe = pAirstrike;
				break;
			}
		}

		// Sometimes the target will DTOR first before it announce invalid pointer, so sanity check is necessary!
		// At this point, the target's vtable has already been reset to AbstractClass_vtbl.
		// If a virtual function that AbstractClass does not have is called without checking this, it will cause the vtable to exceed its bounds.
		if (const auto pTargetExt = TechnoExt::ExtMap.TryFind(pTargetTechno))
		{
			pTargetExt->AirstrikeTargetingMe = pLastTargetingMe;

			if (!pLastTargetingMe && Game::IsActive)
				pTarget->Mark(MarkType::Change);
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x41D604, AirstrikeClass_PointerGotInvalid_ResetForTarget, 0x6)
{
	enum { SkipGameCode = 0x41D634 };

	GET(ObjectClass*, pTarget, EAX);

	if (const auto pTargetTechnoExt = TechnoExt::ExtMap.TryFind(abstract_cast<TechnoClass*, true>(pTarget)))
		pTargetTechnoExt->AirstrikeTargetingMe = nullptr;

	return SkipGameCode;
}

DEFINE_HOOK(0x65E97F, HouseClass_CreateAirstrike_SetTargetForUnit, 0x6)
{
	enum { SkipGameCode = 0x65E992 };

	GET(AircraftClass*, pFirer, ESI);
	GET_STACK(AirstrikeClass*, pThis, STACK_OFFSET(0x38, 0x1C));

	const auto pOwner = pThis->Owner;

	if (!pOwner)
		return 0;

	if (const auto pTarget = abstract_cast<TechnoClass*>(pOwner->Target))
	{
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
		const auto pTypeExt = TechnoExt::ExtMap.Find(pTechno)->TypeExtData;

		if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pTechno))
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

#pragma region GetEffectTintIntensity

DEFINE_HOOK(0x70E92F, TechnoClass_UpdateAirstrikeTint, 0x5)
{
	enum { ContinueIn = 0x70E96E, Skip = 0x70EC9F };

	GET(TechnoClass*, pThis, ESI);

	return TechnoExt::ExtMap.Find(pThis)->AirstrikeTargetingMe ? ContinueIn : Skip;
}

// Jun 9, 2025 - Starkku: Moved to BuildingClass_AI hook in Buildings/Hooks.cpp for optimization's sake.
// Said hook is later but shouldn't matter in this case, the purpose is to force redraw on every frame.
/*
DEFINE_HOOK(0x43FDD6, BuildingClass_AI_Airstrike, 0x6)
{
	enum { SkipGameCode = 0x43FDF1 };

	GET(BuildingClass*, pThis, ESI);

	if (TechnoExt::ExtMap.Find(pThis)->AirstrikeTargetingMe)
		pThis->Mark(MarkType::Change);

	return SkipGameCode;
}*/

DEFINE_HOOK(0x43F9E0, BuildingClass_Mark_Airstrike, 0x6)
{
	enum { ContinueTintIntensity = 0x43FA0F, NonAirstrike = 0x43FA19 };

	GET(BuildingClass*, pThis, EDI);

	return TechnoExt::ExtMap.Find(pThis)->AirstrikeTargetingMe ? ContinueTintIntensity : NonAirstrike;
}

DEFINE_HOOK(0x448DF1, BuildingClass_SetOwningHouse_Airstrike, 0x6)
{
	enum { ContinueTintIntensity = 0x448E0D, NonAirstrike = 0x448E17 };

	GET(BuildingClass*, pThis, ESI);

	return TechnoExt::ExtMap.Find(pThis)->AirstrikeTargetingMe ? ContinueTintIntensity : NonAirstrike;
}

DEFINE_HOOK(0x451ABC, BuildingClass_PlayAnim_Airstrike, 0x6)
{
	enum { ContinueTintIntensity = 0x451AEB, NonAirstrike = 0x451AF5 };

	GET(BuildingClass*, pThis, ESI);

	return TechnoExt::ExtMap.Find(pThis)->AirstrikeTargetingMe ? ContinueTintIntensity : NonAirstrike;
}

DEFINE_HOOK(0x452041, BuildingClass_452000_Airstrike, 0x6)
{
	enum { ContinueTintIntensity = 0x452070, NonAirstrike = 0x45207A };

	GET(BuildingClass*, pThis, ESI);

	return TechnoExt::ExtMap.Find(pThis)->AirstrikeTargetingMe ? ContinueTintIntensity : NonAirstrike;
}

DEFINE_HOOK(0x456E5A, BuildingClass_Flash_Airstrike, 0x6)
{
	enum { ContinueTintIntensity = 0x456E89, NonAirstrike = 0x456E93 };

	GET(BuildingClass*, pThis, ESI);

	return TechnoExt::ExtMap.Find(pThis)->AirstrikeTargetingMe ? ContinueTintIntensity : NonAirstrike;
}

class BuildingClassFake final : public BuildingClass
{
	int _GetAirstrikeInvulnerabilityIntensity(int intensity) const;
};

int BuildingClassFake::_GetAirstrikeInvulnerabilityIntensity(int currentIntensity) const
{
	auto const pBuilding = (BuildingClass*)this;
	int newIntensity = pBuilding->GetFlashingIntensity(currentIntensity);

	if (pBuilding->IsIronCurtained() || TechnoExt::ExtMap.Find(pBuilding)->AirstrikeTargetingMe)
		newIntensity = pBuilding->GetEffectTintIntensity(newIntensity);

	return newIntensity;
}

DEFINE_FUNCTION_JUMP(CALL, 0x450A5D, BuildingClassFake::_GetAirstrikeInvulnerabilityIntensity); // BuildingClass_Animation_AI

#pragma endregion

#pragma region DrawAirstrikeFlare

namespace DrawAirstrikeFlareTemp
{
	TechnoClass* pTechno = nullptr;
}

DEFINE_HOOK(0x705860, TechnoClass_DrawAirstrikeFlare_SetContext, 0x8)
{
	GET(TechnoClass*, pThis, ECX);

	// This is not used in vanilla function so ECX gets overwritten later.
	DrawAirstrikeFlareTemp::pTechno = pThis;

	return 0;
}

DEFINE_HOOK(0x7058F6, TechnoClass_DrawAirstrikeFlare_LineColor, 0x5)
{
	enum { SkipGameCode = 0x705976 };

	GET(int, zSrc, EBP);
	GET(int, zDest, EBX);
	REF_STACK(ColorStruct, color, STACK_OFFSET(0x70, -0x60));

	// Fix depth buffer value.
	int zValue = Math::min(zSrc, zDest) + RulesExt::Global()->AirstrikeLineZAdjust;
	R->EBP(zValue);
	R->EBX(zValue);

	// Allow custom colors.
	auto const pThis = DrawAirstrikeFlareTemp::pTechno;
	auto const baseColor = TechnoExt::ExtMap.Find(pThis)->TypeExtData->AirstrikeLineColor.Get(RulesExt::Global()->AirstrikeLineColor);
	double percentage = Randomizer::Global.RandomRanged(745, 1000) / 1000.0;
	color = { (BYTE)(baseColor.R * percentage), (BYTE)(baseColor.G * percentage), (BYTE)(baseColor.B * percentage) };
	R->ESI(Drawing::RGB_To_Int(baseColor));

	return SkipGameCode;
}

// Always draw the dot and skip setting color, it is already done in previous hook.
DEFINE_HOOK(0x70597A, TechnoClass_DrawAirstrikeFlare_DotColor, 0x6)
{
	enum { SkipGameCode = 0x7059C7 };

	GET_STACK(int, xCoord, STACK_OFFSET(0x70, -0x38));

	// Restore overridden instructions.
	R->ECX(xCoord);

	return SkipGameCode;
}

#pragma endregion
