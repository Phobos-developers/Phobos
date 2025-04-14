#include <Helpers/Macro.h>

#include <LaserDrawClass.h>
#include <GeneralStructures.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

namespace LaserDrawTemp
{
	ColorStruct maxColor;
}

DEFINE_HOOK(0x550D1F, LaserDrawClass_DrawInHouseColor_Context_Set, 0x6)
{
	LaserDrawTemp::maxColor = ColorStruct(*R->lea_Stack<ColorStruct*>(0x14));
	return 0;
}

//Enables proper laser thickness and falloff of it
DEFINE_HOOK(0x550F47, LaserDrawClass_DrawInHouseColor_BetterDrawing, 0x0)
{
	// Restore overridden code that's needed - Kerbiter
	GET_STACK(bool, noQuickDraw, 0x13);
	R->ESI(noQuickDraw ? 8u : 64u);

	GET(LaserDrawClass*, pThis, EBX);
	GET_STACK(int, currentThickness, 0x5C);

	double mult = 1.0;

	if (pThis->Thickness > 1)
	{
		double falloffStep = 1.0 / pThis->Thickness;
		double falloffMult = GeneralUtils::FastPow(1.0 - falloffStep, currentThickness);
		mult = (1.0 - falloffStep * currentThickness) * falloffMult;
	}

	unsigned int r = (unsigned int)(mult * LaserDrawTemp::maxColor.R);
	unsigned int g = (unsigned int)(mult * LaserDrawTemp::maxColor.G);
	unsigned int b = (unsigned int)(mult * LaserDrawTemp::maxColor.B);

	R->EAX(r);
	R->ECX(g);
	R->EDX(b);

	return 0x550F9D;
}
