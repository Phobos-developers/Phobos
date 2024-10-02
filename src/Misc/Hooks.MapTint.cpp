#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <ScenarioClass.h>
#include <Utilities/TemplateDef.h>

namespace MapTintFix
{
	LightConvertClass* TiberiumLightDrawer;
}

DEFINE_HOOK(0x53C441, ScenarioClass_UpdateLighting, 5)
{
	if (!Phobos::Config::FixUnitLightingTint)
		return 0;

	auto tint = ScenarioClass::Instance->NormalLighting.Tint;
	ScenarioClass::RecalcLighting(tint.Red * 10, tint.Green * 10, tint.Blue * 10, true);
	return 0;
}

DEFINE_HOOK(0x683E7F, Start_Scenario_SetInitialTint, 7)
{
	if (!Phobos::Config::FixUnitLightingTint)
		return 0;

	auto tint = ScenarioClass::Instance->NormalLighting.Tint;
	ScenarioClass::RecalcLighting(tint.Red * 10, tint.Green * 10, tint.Blue * 10, true);
	return 0;
}

DEFINE_HOOK(0x52BE3B, InitGame_CreateTiberiumDrawer, 0x5)
{
	MapTintFix::TiberiumLightDrawer = GameCreate<LightConvertClass>(
		&FileSystem::TEMPERAT_PAL, &FileSystem::TEMPERAT_PAL,
		DSurface::Primary, 1000, 1000, 1000, false, nullptr, 53);

	return 0;
}

DEFINE_HOOK(0x53AD00, ScenarioClass_RecalcLighting_TintTiberiumDrawer, 5)
{
	if (!Phobos::Config::FixTiberiumLightingTint)
		return 0;

	GET(int, red, ECX);
	GET(int, green, EDX);
	GET_STACK(int, blue, STACK_OFFSET(0x0, 0x4));
	GET_STACK(bool, tint, STACK_OFFSET(0x0,0x8));

	MapTintFix::TiberiumLightDrawer->UpdateColors(red, green, blue, tint);
	return 0;
}

DEFINE_HOOK(0x47F94B, CellClass_DrawOverlay_ReplaceTiberiumDrawer_1, 6)
{
	R->EDX(MapTintFix::TiberiumLightDrawer);
	return 0x47F951;
}

DEFINE_HOOK(0x47FA5C, CellClass_DrawOverlay_ReplaceTiberiumDrawer_2, 6)
{
	R->EDX(MapTintFix::TiberiumLightDrawer);
	return 0x47FA62;
}

DEFINE_HOOK(0x47FA1F, CellClass_DrawOverlay_ReplaceWeirdDrawer, 6)
{
	R->EDX(MapTintFix::TiberiumLightDrawer);
	return 0x47FA25;
}

DEFINE_HOOK(0x5FE5F9, OverlayTypeClass_DrawIt_ReplaceTiberiumDrawer, 6)
{
	R->EDX(MapTintFix::TiberiumLightDrawer);
	return 0x5FE5FF;
}

DEFINE_HOOK(0x6BE468, Prog_End_DeleteTiberiumDrawer, 6)
{
	GameDelete(MapTintFix::TiberiumLightDrawer);
	return 0;
}
