#include "Commands.h"

#include "Dummy.h"
#include "ObjectInfo.h"
#include "NextIdleHarvester.h"

DEFINE_HOOK(533066, CommandClassCallback_Register, 6)
{
	// Load it after Ares'
	// Debug::Log("[Phobos] CommandClassCallback_Register Called!\n");

	//MakeCommand<DummyCommandClass>();
	MakeCommand<ObjectInfoCommandClass>();
	MakeCommand<NextIdleHarvesterCommandClass>();

	return 0;
}