#include <Utilities/Macro.h>
#include <GameOptionsClass.h>

DEFINE_HOOK(0x6D4B50, Print_Timer_On_Tactical, 0x6)
{
	REF_STACK(int, value, STACK_OFFS(0, 0x8));
	switch (GameOptionsClass::Instance->GameSpeed)
	{
	case 0: // no cap
	case 1:	// 60 FPS
		value = value / 4;
		break;
	case 2:	// 30 FPS
		value = value / 2;
		break;
	case 3:	// 20 FPS
		value = (value * 3) / 4;
		break;
	case 4:	// 15 FPS
		break;
	case 5:	// 12 FPS
		value = (value * 5) / 4;
		break;
	case 6:	// 10 FPS
		value = (value * 3) / 2;
		break;
	default:
		break;
	}
}
