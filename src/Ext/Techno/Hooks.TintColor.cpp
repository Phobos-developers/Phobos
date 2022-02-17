#include <InfantryClass.h>
#include <UnitClass.h>
#include <BuildingClass.h>

#include "Body.h"

DEFINE_HOOK(0x706389, TechnoClass_Techno_Draw_Object_TintColor, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, nIntensity, EBP);
	REF_STACK(int, nTintColor, STACK_OFFS(0x54, -0x2C));

	TechnoExt::ApplyExtraTint(pThis, nTintColor, nIntensity);

	R->EBP(nIntensity);

	return 0;
}

DEFINE_HOOK(0x73C083, UnitClass_DrawAsVXL_TintColor, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	GET(int, nTintColor, ESI);
	REF_STACK(int, nIntensity, STACK_OFFS(0x1C4, -0x1C));

	TechnoExt::ApplyExtraTint(pThis, nTintColor, nIntensity);

	R->ESI(nTintColor);

	return 0;
}

DEFINE_HOOK(0x42342C, AnimClass_Draw_TintColor, 0x8)
{
	GET(BuildingClass*, pBld, EAX);
	REF_STACK(int, nTintColor, STACK_OFFS(0x110, 0xF4));
	REF_STACK(int, nIntensity, STACK_OFFS(0x110, 0xD8));

	TechnoExt::ApplyExtraTint(pBld, nTintColor, nIntensity);

	return 0;
}

//DEFINE_HOOK(0x43D4EB, BuildingClass_DrawIt2_TintColor, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//	GET(int, nTintColor, EDI);
//
//	TechnoExt::ApplyExtraTint(pThis, nTintColor);
//
//	R->EDI(nTintColor);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x43DD8E, BuildingClass_DrawIt_TintColor, 0xA)
//{
//	GET(BuildingClass*, pThis, EBP);
//	REF_STACK(int, nTintColor, STACK_OFFS(0x12C, 0x110));
//
//	TechnoExt::ApplyExtraTint(pThis, nTintColor);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x519082, InfantryClass_DrawIt_TintColor, 0x7)
//{
//	GET(InfantryClass*, pThis, EBP);
//	REF_STACK(int, nTintColor, STACK_OFFS(0x54, 0x40));
//
//	TechnoExt::ApplyExtraTint(pThis, nTintColor);
//
//	return 0;
//}