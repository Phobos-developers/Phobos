#include "Commands.h"

#include "ObjectInfo.h"
#include "NextIdleHarvester.h"
#include "QuickSave.h"
#include "DamageDisplay.h"
#include "FrameByFrame.h"
#include "FrameStep.h"
#include "ToggleDigitalDisplay.h"
#include "ToggleDesignatorRange.h"
#include "SaveVariablesToFile.h"
#include "DistributionMode.h"

#include <CCINIClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x533066, CommandClassCallback_Register, 0x6)
{
	// Load it after Ares'

	MakeCommand<NextIdleHarvesterCommandClass>();
	MakeCommand<QuickSaveCommandClass>();
	MakeCommand<ToggleDigitalDisplayCommandClass>();
	MakeCommand<ToggleDesignatorRangeCommandClass>();

	if (Phobos::Config::AllowDistributionCommand)
	{
		MakeCommand<DistributionModeSpreadCommandClass>();
		MakeCommand<DistributionModeFilterCommandClass>();
		MakeCommand<DistributionModeHoldDownCommandClass>();
	}

	if (Phobos::Config::DevelopmentCommands)
	{
		MakeCommand<DamageDisplayCommandClass>();
		MakeCommand<SaveVariablesToFileCommandClass>();
		MakeCommand<ObjectInfoCommandClass>();
		MakeCommand<FrameByFrameCommandClass>();
		MakeCommand<FrameStepCommandClass<1>>(); // Single step in
		MakeCommand<FrameStepCommandClass<5>>(); // Speed 1
		MakeCommand<FrameStepCommandClass<10>>(); // Speed 2
		MakeCommand<FrameStepCommandClass<15>>(); // Speed 3
		MakeCommand<FrameStepCommandClass<30>>(); // Speed 4
		MakeCommand<FrameStepCommandClass<60>>(); // Speed 5
	}

	return 0;
}

static void MouseWheelDownCommand()
{
//	Debug::LogAndMessage("[Frame: %d] Mouse Wheel Down", Unsorted::CurrentFrame());

	if (DistributionModeHoldDownCommandClass::Enabled)
		DistributionModeHoldDownCommandClass::DistributionSpreadModeReduce();
}

static void MouseWheelUpCommand()
{
//	Debug::LogAndMessage("[Frame: %d] Mouse Wheel Up", Unsorted::CurrentFrame());

	if (DistributionModeHoldDownCommandClass::Enabled)
		DistributionModeHoldDownCommandClass::DistributionSpreadModeExpand();
}

DEFINE_HOOK(0x777998, Game_WndProc_ScrollMouseWheel, 0x6)
{
	GET(const WPARAM, WParam, ECX);

	if (WParam & 0x80000000u)
		MouseWheelDownCommand();
	else
		MouseWheelUpCommand();

	return 0;
}

static inline bool CheckSkipScrollSidebar()
{
	return DistributionModeHoldDownCommandClass::Enabled;
}

DEFINE_HOOK(0x533F50, Game_ScrollSidebar_Skip, 0x5)
{
	enum { SkipScrollSidebar = 0x533FC3 };
	return CheckSkipScrollSidebar() ? SkipScrollSidebar : 0;
}
