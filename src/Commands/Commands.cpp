#include <CCINIClass.h>
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
#include "ToggleSWSidebar.h"
#include "FireTacticalSW.h"

DEFINE_HOOK(0x533066, CommandClassCallback_Register, 0x6)
{
	// Load it after Ares'

	MakeCommand<NextIdleHarvesterCommandClass>();
	MakeCommand<QuickSaveCommandClass>();
	MakeCommand<ToggleDigitalDisplayCommandClass>();
	MakeCommand<ToggleDesignatorRangeCommandClass>();
	MakeCommand<ToggleSWSidebar>();

	MakeCommand<FireTacticalSWCommandClass<0>>();
	MakeCommand<FireTacticalSWCommandClass<1>>();
	MakeCommand<FireTacticalSWCommandClass<2>>();
	MakeCommand<FireTacticalSWCommandClass<3>>();
	MakeCommand<FireTacticalSWCommandClass<4>>();
	MakeCommand<FireTacticalSWCommandClass<5>>();
	MakeCommand<FireTacticalSWCommandClass<6>>();
	MakeCommand<FireTacticalSWCommandClass<7>>();
	MakeCommand<FireTacticalSWCommandClass<8>>();
	MakeCommand<FireTacticalSWCommandClass<9>>();

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
