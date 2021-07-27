#include <Helpers/Macro.h>

#include <LaserDrawClass.h>
#include <GeneralStructures.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

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
	GET_STACK(int, currentThickness, 0x5C)

	// Map value from range of [1, Thickness] to [0, pi/2]
	double x = 0;
	if (pThis->Thickness > 1)
		x = Math::HalfPi * (currentThickness - 1) / (pThis->Thickness - 1);

	// Cosine function for falloff
	double mult = Math::cos(x);
	unsigned int r = (unsigned int)(mult * LaserDrawTemp::maxColor.R);
	unsigned int g = (unsigned int)(mult * LaserDrawTemp::maxColor.G);
	unsigned int b = (unsigned int)(mult * LaserDrawTemp::maxColor.B);

	R->EAX(r);
	R->ECX(g);
	R->EDX(b);

	return 0x550F9D;
}
