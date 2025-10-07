#include <Helpers/Macro.h>

#include <LaserDrawClass.h>
#include <GeneralStructures.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Misc/FogOfWar.h>
#include <ScenarioClass.h>
#include <HouseClass.h>
#include <MapClass.h>
#include <CellClass.h>

#define LASER_FOW_DEBUG 0

namespace LaserDrawTemp
{
	ColorStruct maxColor;
	bool FogHidden = false; // Flag to track if current laser should be hidden due to fog
}

// Simple fog gate for vanilla lasers - basic working version
DEFINE_HOOK(0x550260, LaserDrawClass_Draw_FogGate, 0x6)
{
    enum { SkipDrawing = 0x5509D2 };

    GET(LaserDrawClass*, pThis, ECX);
    if(!pThis) return 0;

    // Treat (0,0) source as fogged to avoid a single startup frame
    if((pThis->Source.X | pThis->Source.Y) == 0) return SkipDrawing;

    // Simple fog check using source coordinates
    if(ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar) {
        if(HouseClass::CurrentPlayer && !HouseClass::CurrentPlayer->SpySatActive) {
            const CoordStruct from = pThis->Source;
            const CellStruct cell = CellClass::Coord2Cell(from);
            if(auto* c = MapClass::Instance.GetCellAt(cell)) {
                if(c->IsFogged()) {
                    return SkipDrawing;
                }
            }
        }
    }

    return 0;
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
	GET_STACK(const bool, noQuickDraw, 0x13);
	R->ESI(noQuickDraw ? 8u : 64u);

	GET(LaserDrawClass*, pThis, EBX);
	GET_STACK(const int, currentThickness, 0x5C);

	double mult = 1.0;

	if (pThis->Thickness > 1)
	{
		const double falloffStep = 1.0 / pThis->Thickness;
		const double falloffMult = GeneralUtils::FastPow(1.0 - falloffStep, currentThickness);
		mult = (1.0 - falloffStep * currentThickness) * falloffMult;
	}

	const unsigned int r = (unsigned int)(mult * LaserDrawTemp::maxColor.R);
	const unsigned int g = (unsigned int)(mult * LaserDrawTemp::maxColor.G);
	const unsigned int b = (unsigned int)(mult * LaserDrawTemp::maxColor.B);

	R->EAX(r);
	R->ECX(g);
	R->EDX(b);

	return 0x550F9D;
}

