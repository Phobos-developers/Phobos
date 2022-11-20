#include <Phobos.h>
#include <Utilities/Macro.h>
#include <SessionClass.h>
#include <GameOptionsClass.h>

DEFINE_HOOK(0x69BAE7, SessionClass_Resume_CampaignGameSpeed, 0xA)
{
	GameOptionsClass::Instance->GameSpeed = Phobos::Misc::CampaignDefaultGameSpeed;
	return 0x69BAF1;
}

int counter = 0;
DEFINE_HOOK(0x55E160, SyncDelay_Start, 0x6)
{
	constexpr reference<CDTimerClass, 0x887348> FrameTimer;
	if (!Phobos::Misc::CustomGS)
		return 0;
	if ((Phobos::Misc::CustomGS_ChangeInterval[FrameTimer->TimeLeft] > 0)
		&& (counter % Phobos::Misc::CustomGS_ChangeInterval[FrameTimer->TimeLeft] == 0))
	{
		FrameTimer->TimeLeft = Phobos::Misc::CustomGS_ChangeDelay[FrameTimer->TimeLeft];
		counter = 1;
	}
	else
	{
		counter += 1;
		FrameTimer->TimeLeft = Phobos::Misc::CustomGS_DefaultDelay[FrameTimer->TimeLeft];
	}
	return 0;
}

DEFINE_HOOK(0x55E33B, SyncDelay_End, 0x6)
{
	constexpr reference<CDTimerClass, 0x887348> FrameTimer;
	FrameTimer->TimeLeft = GameOptionsClass::Instance->GameSpeed;
	return 0;
}
