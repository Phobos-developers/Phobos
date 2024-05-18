#include <InfantryClass.h>

#include "Body.h"

#include <Ext/Anim/Body.h>

#pragma region ICColorBugFix

DEFINE_HOOK(0x43D442, BuildingClass_Draw_ICFSColor, 0x7)
{
	enum { SkipGameCode = 0x43D45B };

	GET(BuildingClass*, pThis, ESI);

	RulesClass* rules = RulesClass::Instance;

	R->ECX(rules);
	R->EAX(pThis->ForceShielded ? rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

DEFINE_HOOK(0x43DCE1, BuildingClass_Draw2_ICFSColor, 0x7)
{
	enum { SkipGameCode = 0x43DCFA };

	GET(BuildingClass*, pThis, EBP);

	RulesClass* rules = RulesClass::Instance;

	R->ECX(rules);
	R->EAX(pThis->ForceShielded ? rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

DEFINE_HOOK(0x73BFBF, UnitClass_DrawAsVoxel_ICFSColor, 0x6)
{
	enum { SkipGameCode = 0x73BFC5 };

	GET(UnitClass*, pThis, EBP);

	RulesClass* rules = RulesClass::Instance;

	R->EAX(pThis->ForceShielded ? rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

DEFINE_HOOK(0x423519, AnimClass_Draw_ICFSColor, 0x6)
{
	enum { SkipGameCode = 0x423525 };

	GET(BuildingClass*, pBuilding, ECX);

	RulesClass* rules = RulesClass::Instance;

	R->ECX(rules);
	R->EAX(pBuilding->ForceShielded ? rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

#pragma endregion

DEFINE_HOOK(0x706389, TechnoClass_DrawObject_TintColor, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, intensity, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x54, 0x2C));

	bool isVehicle = pThis->WhatAmI() == AbstractType::Unit;
	bool isAircraft = pThis->WhatAmI() == AbstractType::Aircraft;

	// SHP vehicles and aircraft
	if (isVehicle || isAircraft)
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

DEFINE_HOOK(0x7067E4, TechnoClass_DrawVoxel_TintColor, 0x8)
{
	GET(TechnoClass*, pThis, EBP);
	GET(int, intensity, EDI);
	REF_STACK(int, color, STACK_OFFSET(0x50, 0x24));

	color |= TechnoExt::GetTintColor(pThis, true, false, true);
	TechnoExt::ApplyCustomTintValues(pThis, color, intensity);
	R->EDI(intensity);

	return 0;
}

DEFINE_HOOK(0x43D4EB, BuildingClass_Draw_TintColor, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(int, color, EDI);

	color |= TechnoExt::GetCustomTintColor(pThis);
	R->EDI(color);

	return 0;
}

DEFINE_HOOK(0x43DD8E, BuildingClass_Draw2_TintColor, 0xA)
{
	GET(BuildingClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x12C, -0x110));

	color |= TechnoExt::GetCustomTintColor(pThis);

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

DEFINE_HOOK(0x519082, InfantryClass_Draw_TintColor, 0x7)
{
	GET(InfantryClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x54, -0x40));

	color |= TechnoExt::GetTintColor(pThis, true, false, false);
	color |= TechnoExt::GetCustomTintColor(pThis);

	return 0;
}

DEFINE_HOOK(0x51946D, InfantryClass_Draw_TintIntensity, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, intensity, ESI);

	intensity = pThis->GetEffectTintIntensity(intensity);
	intensity += TechnoExt::GetCustomTintIntensity(pThis);
	R->ESI(intensity);

	return 0;
}

DEFINE_HOOK(0x73C083, UnitClass_DrawAsVoxel_TintColor, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	GET(int, color, ESI);
	REF_STACK(int, intensity, STACK_OFFSET(0x1D0, 0x10));

	TechnoExt::ApplyCustomTintValues(pThis, color, intensity);

	R->ESI(color);

	return 0;
}

DEFINE_HOOK(0x0423420, AnimClass_Draw_ParentBuildingCheck, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EAX);

	if (!pBuilding)
		R->EAX(AnimExt::ExtMap.Find(pThis)->ParentBuilding);

	return 0;
}

DEFINE_HOOK(0x4235D3, AnimClass_Draw_TintColor, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(int, color, EBP);
	REF_STACK(int, intensity, STACK_OFFSET(0x110, -0xD8));

	auto const pBuilding = AnimExt::ExtMap.Find(pThis)->ParentBuilding;

	if (!pBuilding)
		return 0;

	int dummy = 0;
	TechnoExt::ApplyCustomTintValues(pBuilding, color, !pThis->Type->UseNormalLight ? intensity : dummy);
	R->EBP(color);

	return 0;
}
