#include <Phobos.h>
#include <Utilities/Macro.h>
#include <SessionClass.h>
#include <GameOptionsClass.h>
#include <Unsorted.h>

namespace GameSpeedTemp
{
	static int counter = 0;
}

DEFINE_HOOK(0x69BAE7, SessionClass_Resume_CampaignGameSpeed, 0xA)
{
	GameOptionsClass::Instance->GameSpeed = Phobos::Config::CampaignDefaultGameSpeed;
	return 0x69BAF1;
}

constexpr reference<CDTimerClass, 0x887348> FrameTimer;

DEFINE_HOOK(0x55E160, SyncDelay_Start, 0x6)
{
	//constexpr reference<CDTimerClass, 0x887328> NFTTimer;
	if (!Phobos::Misc::CustomGS || SessionClass::IsMultiplayer())
		return 0;
	if ((Phobos::Misc::CustomGS_ChangeInterval[FrameTimer->TimeLeft] > 0)
		&& (GameSpeedTemp::counter % Phobos::Misc::CustomGS_ChangeInterval[FrameTimer->TimeLeft] == 0))
	{
		FrameTimer->TimeLeft = Phobos::Misc::CustomGS_ChangeDelay[FrameTimer->TimeLeft];
		GameSpeedTemp::counter = 1;
	}
	else
	{
		FrameTimer->TimeLeft = Phobos::Misc::CustomGS_DefaultDelay[FrameTimer->TimeLeft];
		GameSpeedTemp::counter++;
	}

	return 0;
}

DEFINE_HOOK(0x55E33B, SyncDelay_End, 0x6)
{
	if (Phobos::Misc::CustomGS && SessionClass::IsSingleplayer())
		FrameTimer->TimeLeft = GameOptionsClass::Instance->GameSpeed;
	return 0;
}

// note: currently vanilla code, doesn't do anything, changing PrecalcDesiredFrameRate doesn't effect anything either
/*
void SetNetworkFrameRate()
{
	static constexpr reference<int, 0xA8B550u> const PrecalcDesiredFrameRate {};
	switch (GameOptionsClass::Instance->GameSpeed)
	{
	case 0:
		Game::Network.MaxAhead = 40;
		PrecalcDesiredFrameRate = 60;
		Game::Network.FrameSendRate = 10;
		break;
	case 1:
		Game::Network.MaxAhead = 40;
		PrecalcDesiredFrameRate = 45;
		Game::Network.FrameSendRate = 10;
		break;
	case 2:
		Game::Network.MaxAhead = 30;
		PrecalcDesiredFrameRate = 30;
		break;
	case 3:
		Game::Network.MaxAhead = 20;
		PrecalcDesiredFrameRate = 20;
		break;
	case 4:
		Game::Network.MaxAhead = 20;
		PrecalcDesiredFrameRate = 15;
		break;
	case 5:
		Game::Network.MaxAhead = 20;
		PrecalcDesiredFrameRate = 12;
		break;
	default:
		Game::Network.MaxAhead = 10;
		PrecalcDesiredFrameRate = 10;
		break;
	}
}

DEFINE_HOOK(0x5C49A7, MPCooperative_5C46E0_FPS, 0x9)
{
	SetNetworkFrameRate();
	return 0x5C4A40;
}

DEFINE_HOOK(0x794F7E, Start_Game_Now_FPS, 0x8)
{
	SetNetworkFrameRate();
	return 0x79501F;
}
*/
