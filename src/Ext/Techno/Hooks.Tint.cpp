#include <AircraftClass.h>
#include <InfantryClass.h>

#include "Body.h"

#include <Ext/Anim/Body.h>

DEFINE_HOOK(0x43D386, BuildingClass_Draw_TintColor, 0x6)
{
	enum { SkipGameCode = 0x43D4EB };

	GET(BuildingClass*, pThis, ESI);

	int color = TechnoExt::GetTintColor(pThis, pThis->IsIronCurtained(), pThis->Airstrike, false);
	color |= TechnoExt::GetCustomTintColor(pThis);
	R->EDI(color);

	return SkipGameCode;
}

DEFINE_HOOK(0x43DC1C, BuildingClass_Draw2_TintColor, 0x6)
{
	enum { SkipGameCode = 0x43DD8E };

	GET(BuildingClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x12C, -0x110));

	color = TechnoExt::GetTintColor(pThis, pThis->IsIronCurtained(), pThis->Airstrike, false);
	color |= TechnoExt::GetCustomTintColor(pThis);

	return SkipGameCode;
}

DEFINE_HOOK(0x73BF95, UnitClass_DrawAsVoxel_Tint, 0x7)
{
	enum { SkipGameCode = 0x73C141 };

	GET(UnitClass*, pThis, EBP);
	GET(int, flashIntensity, ESI);
	REF_STACK(int, intensity, STACK_OFFSET(0x1D0, 0x10));

	intensity = flashIntensity;

	bool isInvulnerable = pThis->IsIronCurtained();

	if (isInvulnerable)
		intensity = pThis->GetInvulnerabilityTintIntensity(intensity);

	int color = TechnoExt::GetTintColor(pThis, isInvulnerable, false, pThis->Berzerk);
	TechnoExt::ApplyCustomTintValues(pThis, color, intensity);

	R->ESI(color);
	return SkipGameCode;
}

DEFINE_HOOK(0x518FC8, InfantryClass_Draw_TintColor, 0x6)
{
	enum { SkipGameCode = 0x519082 };

	GET(InfantryClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x54, -0x40));

	color = 0;
	color |= TechnoExt::GetTintColor(pThis, pThis->IsIronCurtained(), false, pThis->Berzerk);
	color |= TechnoExt::GetCustomTintColor(pThis);

	return SkipGameCode;
}

DEFINE_HOOK(0x51946D, InfantryClass_Draw_TintIntensity, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, intensity, ESI);

	intensity = pThis->GetInvulnerabilityTintIntensity(intensity);
	intensity += TechnoExt::GetCustomTintIntensity(pThis);
	R->ESI(intensity);

	return 0;
}

DEFINE_HOOK(0x423420, AnimClass_Draw_TintColor, 0x6)
{
	enum { SkipGameCode = 0x4235D3 };

	GET(AnimClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EAX);
	REF_STACK(int, color, STACK_OFFSET(0x110, -0xF4));
	REF_STACK(int, intensity, STACK_OFFSET(0x110, -0xD8));

	if (!pBuilding)
		pBuilding = AnimExt::ExtMap.Find(pThis)->ParentBuilding;

	if (pBuilding)
	{
		int discard = 0;
		color |= TechnoExt::GetTintColor(pBuilding, pBuilding->IsIronCurtained(), pBuilding->Airstrike, false);
		TechnoExt::ApplyCustomTintValues(pBuilding, color, pThis->Type->UseNormalLight ? discard : intensity);
	}

	R->EBP(color);
	return SkipGameCode;
}

DEFINE_HOOK(0x706389, TechnoClass_DrawObject_TintColor, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, intensity, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x54, 0x2C));

	auto const rtti = pThis->WhatAmI();
	bool isAircraft = rtti == AbstractType::Aircraft;

	// SHP vehicles and aircraft
	if (rtti == AbstractType::Unit || isAircraft)
	{
		color |= TechnoExt::GetTintColor(pThis, true, false, !isAircraft);
		TechnoExt::ApplyCustomTintValues(pThis, color, intensity);
	}
	else if (pThis->WhatAmI() != AbstractType::Infantry)
	{
		intensity += TechnoExt::GetCustomTintIntensity(pThis);
	}

	R->EBP(intensity);

	return 0;
}

DEFINE_HOOK(0x706786, TechnoClass_DrawVoxel_TintColor, 0x5)
{
	enum { SkipTint = 0x7067E4 };

	GET(TechnoClass*, pThis, EBP);

	auto const rtti = pThis->WhatAmI();

	// Vehicles already have had tint intensity as well as custom tints applied, no need to do it twice.
	if (rtti == AbstractType::Unit)
		return SkipTint;

	GET(int, intensity, EAX);
	REF_STACK(int, color, STACK_OFFSET(0x50, 0x24));

	if (rtti == AbstractType::Aircraft)
		color = TechnoExt::GetTintColor(pThis, true, false, false);

	// Non-aircraft voxels do not need custom tint color applied again, discard that component for them.
	int discardColor = 0;
	TechnoExt::ApplyCustomTintValues(pThis, rtti == AbstractType::Aircraft ? color : discardColor, intensity);
	R->EAX(intensity);

	return 0;
}

DEFINE_HOOK(0x43FA19, BuildingClass_Mark_TintIntensity, 0x7)
{
	GET(BuildingClass*, pThis, EDI);
	GET(int, intensity, ESI);

	intensity += TechnoExt::GetCustomTintIntensity(pThis);
	R->ESI(intensity);

	return 0;
}

namespace ICTintTemp
{
	bool IsForceShield = false;
}

DEFINE_HOOK(0x70E380, TechnoClass_InvulnerabilityIntensity_SetContext, 0x6)
{
	GET(TechnoClass*, pThis, ECX);

	ICTintTemp::IsForceShield = pThis->ForceShielded;

	return 0;
}

DEFINE_HOOK(0x70E475, TechnoClass_InvulnerabilityIntensity_Adjust, 0x5)
{
	enum { SkipGameCode = 0x70E488 };

	GET(int, intensity, EAX);

	if (intensity > 2000)
		intensity = 2000;

	auto const& rules = RulesExt::Global();
	int max = static_cast<int>((ICTintTemp::IsForceShield ? rules->ForceShield_ExtraTintIntensity : rules->IronCurtain_ExtraTintIntensity) * 1000);

	R->EAX(intensity + max);
	return SkipGameCode;
}
