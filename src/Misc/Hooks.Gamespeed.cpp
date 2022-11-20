#include <Phobos.h>
#include <Utilities/Macro.h>
#include <SessionClass.h>
#include <GameOptionsClass.h>

DEFINE_HOOK(0x55D774, MainLoop_CampaignGameSpeed_NotSpeedControl, 0xA)
{
	GameOptionsClass::Instance->GameSpeed = Phobos::Misc::CampaignDefaultGameSpeed;
	return 0x55D77E;
}

DEFINE_HOOK(0x55D78C, MainLoop_CampaignGameSpeed_NotSpeedControl2, 0x5)
{
	R->ECX(Phobos::Misc::CampaignDefaultGameSpeed);
	return 0x55D791;
}

// todo just write to memory there once, this is executed every frame
DEFINE_HOOK(0x55D79E, MainLoop_CampaignGameSpeed_SpeedControl, 0x6)
{
	if (SessionClass::IsCampaign())
	{
		R->ESI(Phobos::Misc::CampaignDefaultGameSpeed);
		return 0x55D7A4;
	}
	return 0;
}

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
