#include "Commands.h"

#include "Dummy.h"
#include "ObjectInfo.h"

DEFINE_HOOK(533066, CommandClassCallback_Register, 6)
{
	// Load it after Ares'
	// Debug::Log("[Phobos] CommandClassCallback_Register Called!\n");

	//MakeCommand<DummyCommandClass>();
	MakeCommand<ObjectInfoCommandClass>();

	return 0;
}