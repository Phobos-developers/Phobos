#include "Commands.h"

#include "ObjectInfo.h"
#include "NextIdleHarvester.h"
#include "QuickSave.h"
#include "DamageDisplay.h"
#include "FrameByFrame.h"
#include "FrameStep.h"

DEFINE_HOOK(0x533066, CommandClassCallback_Register, 0x6)
{
	// Load it after Ares'

	MakeCommand<ObjectInfoCommandClass>();
	MakeCommand<NextIdleHarvesterCommandClass>();
	MakeCommand<QuickSaveCommandClass>();
	MakeCommand<DamageDisplayCommandClass>();

	MakeCommand<FrameByFrameCommandClass>();
	MakeCommand<FrameStepCommandClass<1>>(); // Single step in
	MakeCommand<FrameStepCommandClass<5>>(); // Speed 1
	MakeCommand<FrameStepCommandClass<10>>(); // Speed 2
	MakeCommand<FrameStepCommandClass<15>>(); // Speed 3
	MakeCommand<FrameStepCommandClass<30>>(); // Speed 4
	MakeCommand<FrameStepCommandClass<60>>(); // Speed 5

	return 0;
}
