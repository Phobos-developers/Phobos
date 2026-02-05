#include <Phobos.h>
#include <Utilities/Macro.h>
#include <algorithm>
#include <SessionClass.h>
#include <GameOptionsClass.h>

DEFINE_HOOK(0x69BAE7, SessionClass_Resume_CampaignGameSpeed, 0xA)
{
	GameOptionsClass::Instance.GameSpeed = Phobos::Config::CampaignDefaultGameSpeed;
	return 0x69BAF1;
}

// For custom game speeds:
// Add to rulesmd.ini under [General]:
//   EnableCustomFPS=yes              ; Enable/disable custom FPS (default: yes)
//   CustomGameSpeedFPS.0=120         ; Per-speed FPS. 0 = vanilla. Vanilla: 0=60, 1=45, 2=30, 3=20, 4=15, 5=12, 6=10
//
//   -Each speed slot with a non-zero CustomGameSpeedFPS.N runs at that target FPS
//   -Slots with 0 (or unset) use vanilla FPS for that position
//   -Practical max is ~1000 FPS (timeGetTime() resolution)

// Queue_AI_Multiplayer
// Patch v26 to INT_MAX disables the 60fps cap on multiplayer.
DEFINE_PATCH(0x647C28, 0xBE, 0xFF, 0xFF, 0xFF, 0x7F); // mov esi, INT_MAX

// Queue_AI_Multiplayer
// Override the GameSpeed-to-fps calculation in multiplayer.
DEFINE_HOOK(0x647C4D, Queue_AI_Multiplayer_CustomFPSCalculation, 0x1F)
{
	int gameSpeed = GameOptionsClass::Instance.GameSpeed;
	int calculatedFPS;

	if (Phobos::Misc::EnableCustomFPS && Phobos::Misc::CustomGameSpeedFPS[gameSpeed] > 0)
	{
		calculatedFPS = Phobos::Misc::CustomGameSpeedFPS[gameSpeed];
	}
	else if (gameSpeed == 0)
	{
		// Vanilla: GameSpeed 0 = 60 FPS
		calculatedFPS = 60;
	}
	else if (gameSpeed == 1)
	{
		// Vanilla: GameSpeed 1 = 45 FPS
		calculatedFPS = 45;
	}
	else
	{
		// Vanilla: GameSpeed 2+ = 60 / GameSpeed
		calculatedFPS = 60 / gameSpeed;
	}

	R->EAX(calculatedFPS);

	return 0x647C6C;
}

struct NFTTimerStruct
{
	DWORD StartTime;
	DWORD CurrentTime;
	int   TimeLeft;
};
DEFINE_REFERENCE(NFTTimerStruct, NFTTimer, 0x887328);

struct FrameTimerStruct
{
	DWORD StartTime;
	DWORD CurrentTime;
	int   DelayTime;
};
DEFINE_REFERENCE(FrameTimerStruct, GameFrameTimer, 0x887348);

// Hook MainLoop skirmish/campaign FPS calculation
// The normal route FrameTimer.DelayTime rounds to 0 for >60 FPS (tick-based timeGetTime >> 4),
// NFTTimer (ms-based timeGetTime) provides the frame timing that SyncDelay's NFTTimer loop uses.
// We need to set up NFTTimer like multiplayer mode does for >60 FPS support.
DEFINE_HOOK(0x55D7B6, MainLoop_SkirmishFPSFix, 0xC)
{
	const DWORD timerValue = R->ECX();
	const int gameSpeed = R->ESI();

	const bool shouldUseCustomFPS = Phobos::Misc::EnableCustomFPS
		&& Phobos::Misc::CustomGameSpeedFPS[gameSpeed] > 0
		&& (SessionClass::IsSkirmish() || SessionClass::IsCampaign());

	if (!shouldUseCustomFPS)
	{
		// Use vanilla behavior
		GameFrameTimer.CurrentTime = timerValue;
		GameFrameTimer.DelayTime = gameSpeed;
		return 0x55D7C2;
	}

	const int customFPS = Phobos::Misc::CustomGameSpeedFPS[gameSpeed];

	// calc frame timings
	const int targetFrameDelayTicks = 60 / customFPS;
	const int targetFrameTimeMs = std::max(1, 1000 / customFPS); // 1ms floor = ~1000 FPS ceiling

	const DWORD currentTime = timeGetTime();

	// just in case
	GameFrameTimer.CurrentTime = timerValue;
	GameFrameTimer.DelayTime = targetFrameDelayTicks;

	// SyncDelay compares elapsed time since CurrentTime against TimeLeft.
	NFTTimer.StartTime = currentTime;
	NFTTimer.CurrentTime = currentTime;
	NFTTimer.TimeLeft = targetFrameTimeMs;

	// "Requested FPS"
	SessionClass::Instance.DesiredFrameRate = customFPS;

	return 0x55D7C2; // Past the two MOVs we replaced
}

// SyncDelay
// Redirect skirmish/campaign to the NFTTimer path
DEFINE_HOOK(0x55E1B6, SyncDelay_RedirectSkirmishToNFTTimer, 0x6)
{
	if (SessionClass::IsSkirmish() || SessionClass::IsCampaign())
	{
		if (Phobos::Misc::EnableCustomFPS && Phobos::Misc::CustomGameSpeedFPS[GameOptionsClass::Instance.GameSpeed] > 0)
			return 0x55E1BC; // Custom FPS: use NFTTimer path like multiplayer

		return 0x55E2B4; // Vanilla FrameTimer path
	}

	return 0x55E1BC;
}
