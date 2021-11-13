#include "PhobosTrajactory.h"

#include <BulletClass.h>
#include <Helpers/Macro.h>

DEFINE_HOOK(0x4666F7, BulletClass_AI_Trajectories, 0x6)
{
	GET(BulletClass*, pThis, EBP);



	return 0;
}

DEFINE_HOOK(0x468B72, BulletClass_Unlimbo_Trajectories, 0x5)
{
	GET(BulletClass*, pThis, EBX);
	GET_STACK(CoordStruct*, pCoord, STACK_OFFS(0x54, -0x4)); // GET_BASE(, , 0x8)
	GET_STACK(BulletVelocity*, pVelocity, STACK_OFFS(0x54, -0x8)); // GET_BASE(, , 0xC)



	return 0;
}