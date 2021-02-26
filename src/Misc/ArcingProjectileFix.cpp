#include <Helpers/Macro.h>
#include <GeneralStructures.h>
#include <YRMath.h>

DEFINE_HOOK(6FE4F6, TechnoClass_FireAt_ArcingFix, 0)
{
	LEA_STACK(CoordStruct*, coordA, 0x44);
	LEA_STACK(CoordStruct*, coordB, 0x88);

	R->EAX((int)(*coordA - *coordB).Magnitude());
	
	return 0x6FE537;
}