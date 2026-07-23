#include <GameOptionsClass.h>
#include <FPSCounter.h>

#include <Ext/Rules/Body.h>

namespace TimerValueTemp
{
	static int oldValue;
};

DEFINE_HOOK(0x6D4B50, PrintTimerOnTactical_Start, 0x6)
{
	if (!Phobos::Config::RealTimeTimers)
		return 0;

	REF_STACK(int, value, STACK_OFFSET(0, 0x4));
	TimerValueTemp::oldValue = value;

	const bool isMP = SessionClass::IsMultiplayer();

	// In SP/Skirmish, GameSpeed 0 is unlimited (no frame delay) so use adaptive FPS path.
	// In MP, GameSpeed 0 is 60 FPS (valid fixed speed), so it must not enter this path.
	if (Phobos::Config::RealTimeTimers_Adaptive
		|| (!isMP && GameOptionsClass::Instance.GameSpeed == 0)
		|| (Phobos::Misc::CustomGS && !isMP))
	{
		value = (int)((double)value / (std::max((double)FPSCounter::CurrentFrameRate, 1.0) / 15.0));
		return 0;
	}

	const int gs = GameOptionsClass::Instance.GameSpeed;

	if (isMP)
	{
		// MP GameSpeed indices (0-6): 60, 45, 30, 20, 15, 12, 10 FPS
		// MP has an extra 45 FPS option (index 1) that SP/Skirmish does not.
		switch (gs)
		{
		case 0: // 60 FPS
			value = value / 4;
			break;
		case 1: // 45 FPS
			value = value / 3;
			break;
		case 2: // 30 FPS
			value = value / 2;
			break;
		case 3: // 20 FPS
			value = (value * 3) / 4;
			break;
		case 4: // 15 FPS
			break;
		case 5: // 12 FPS
			value = (value * 5) / 4;
			break;
		case 6: // 10 FPS
			value = (value * 3) / 2;
			break;
		default:
			break;
		}
	}
	else
	{
		// SP/Skirmish GameSpeed indices (1-6): 60, 30, 20, 15, 12, 10 FPS
		// Index 0 (unlimited) is already handled above via the adaptive path.
		switch (gs)
		{
		case 1: // 60 FPS
			value = value / 4;
			break;
		case 2: // 30 FPS
			value = value / 2;
			break;
		case 3: // 20 FPS
			value = (value * 3) / 4;
			break;
		case 4: // 15 FPS
			break;
		case 5: // 12 FPS
			value = (value * 5) / 4;
			break;
		case 6: // 10 FPS
			value = (value * 3) / 2;
			break;
		default:
			break;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6D4C68, PrintTimerOnTactical_End, 0x8)
{
	if (!Phobos::Config::RealTimeTimers)
		return 0;

	REF_STACK(int, value, STACK_OFFSET(0x654, 0x4));
	value = TimerValueTemp::oldValue;
	return 0;
}

DEFINE_HOOK(0x6D4CD9, PrintTimerOnTactical_BlinkColor, 0x6)
{
	enum { SkipGameCode = 0x6D4CE2 };

	R->EDI(ColorScheme::Array.GetItem(RulesExt::Global()->TimerBlinkColorScheme));

	return SkipGameCode;
}

#pragma region ShowGameTime

static const wchar_t* GetUIName()
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_GAMETIME", L"Time");
}

DEFINE_HOOK(0x4F4573, GScreenClass_Draw_GameTime, 0x5)
{
	if (!Phobos::Config::ShowGameTime || HouseClass::CurrentPlayer->IsObserver()) // already has a timer
		return 0;

	wchar_t buffer[0x20] {};
	const int total_seconds = Unsorted::CurrentFrame / 15;

	const int hours = total_seconds / 3600;
	const int minutes = (total_seconds / 60) % 60;
	const int seconds = total_seconds % 60;

	if (hours > 0)
	{
		swprintf(buffer, std::size(buffer), L"%ls %d:%02d:%02d", GetUIName(), hours, minutes, seconds);
	}
	else
	{
		swprintf(buffer, std::size(buffer), L"%ls %02d:%02d", GetUIName(), minutes, seconds);
	}

	auto wanted = Drawing::GetTextDimensions(buffer, { 0,0 }, 0, 2, 0);

	RectangleStruct rect = {
		DSurface::Composite->GetWidth() - wanted.Width - 30,
		0,
		wanted.Width + 10,
		wanted.Height + 10
	};

	Point2D location { rect.X + 5 ,5 };
	ColorStruct color { 0x0, 0x0 ,0x0 };
	DSurface::Composite->FillRectTrans(&rect, &color, Phobos::Config::ShowGameTime_BoardOpacity);
	//DSurface::Composite->DrawRect(&rect, COLOR_WHITE);
	DSurface::Composite->DrawText(buffer, &location, COLOR_WHITE);

	//Phobos' extended tooltips interferred
	R->ECX(*(int*)0x887640);
	return 0x4F4589;
}

#pragma endregion
