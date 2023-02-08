#include <Utilities/Macro.h>
#include <GameOptionsClass.h>
#include <FPSCounter.h>
#include <SessionClass.h>

int oldValue;
DEFINE_HOOK(0x6D4B50, Print_Timer_On_Tactical_Start, 0x6)
{
	REF_STACK(int, value, STACK_OFFSET(0, 0x4));
	oldValue = value;

	if (false && !SessionClass::IsMultiplayer()) // TODO change when custom game speed gets merged
	{
		value = std::max(value / (int)(std::max((double)FPSCounter::CurrentFrameRate, 1.0) / 15.0), 1);
		return 0;
	}

	switch (GameOptionsClass::Instance->GameSpeed)
	{
	case 0: // no cap
		value = (int)((double)value / (std::max((double)FPSCounter::CurrentFrameRate, 1.0) / 15.0));
		break;
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

	return 0;
}

DEFINE_HOOK(0x6D4C68, Print_Timer_On_Tactical_End, 0x8)
{
	REF_STACK(int, value, STACK_OFFSET(0x654, 0x4));
	value = oldValue;
	return 0;
}
