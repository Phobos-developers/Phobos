#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <ScenarioClass.h>
#include <Utilities/TemplateDef.h>

DEFINE_HOOK(0x53C441, ScenarioClass_UpdateLighting, 5)
{
	auto tint = ScenarioClass::Instance->NormalLighting.Tint;
	ScenarioClass::RecalcLighting(tint.Red * 10, tint.Green * 10, tint.Blue * 10, true);
	return 0;
}

DEFINE_HOOK(0x48CDF0, MainGame_SetInitialTinr_KindaCheating, 6)
{
	auto tint = ScenarioClass::Instance->NormalLighting.Tint;
	ScenarioClass::RecalcLighting(tint.Red * 10, tint.Green * 10, tint.Blue * 10, true);
	return 0;
}

/*
// useless?
DEFINE_HOOK(0x53988E, IonStormClass_InitClear, 5)
{
	auto tint = ScenarioClass::Instance->NormalLighting.Tint;
	ScenarioClass::RecalcLighting(1000, 0, 0, true); // debug value, all red
	return 0;
}
*/
