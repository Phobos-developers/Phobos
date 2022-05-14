#include "Commands.h"

#include "Dummy.h"
#include "ObjectInfo.h"
#include "NextIdleHarvester.h"
#include "QuickSave.h"
#include "DamageDisplay.h"

DEFINE_HOOK(0x533066, CommandClassCallback_Register, 0x6)
{
	// Load it after Ares'
	// Debug::Log("[Phobos] CommandClassCallback_Register Called!\n");

	//MakeCommand<DummyCommandClass>();
	MakeCommand<ObjectInfoCommandClass>();
	MakeCommand<NextIdleHarvesterCommandClass>();
	MakeCommand<QuickSaveCommandClass>();
	MakeCommand<DamageDisplayCommandClass>();

	return 0;
}