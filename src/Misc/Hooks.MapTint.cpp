#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <ScenarioClass.h>
#include <Utilities/TemplateDef.h>

DEFINE_HOOK(0x53C441, ScenarioClass_UpdateLighting, 5)
{
	if (!Phobos::Config::FixLightingTint)
		return 0;

	auto tint = ScenarioClass::Instance->NormalLighting.Tint;
	ScenarioClass::RecalcLighting(tint.Red * 10, tint.Green * 10, tint.Blue * 10, true);
	return 0;
}

DEFINE_HOOK(0x683E7F, Start_Scenario_SetInitialTint, 7)
{
	if (!Phobos::Config::FixLightingTint)
		return 0;

	auto tint = ScenarioClass::Instance->NormalLighting.Tint;
	ScenarioClass::RecalcLighting(tint.Red * 10, tint.Green * 10, tint.Blue * 10, true);
	return 0;
}
