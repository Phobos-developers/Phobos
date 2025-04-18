#include "Phobos.COM.h"

#include <Helpers/Macro.h>

#include <Locomotion/TestLocomotionClass.h>


#ifdef CUSTOM_LOCO_EXAMPLE_ENABLED // Register the loco
DEFINE_HOOK(0x6BD68D, WinMain_PhobosRegistrations, 0x6)
{
	Debug::Log("Starting COM registration...\n");

	// Add new classes to be COM-registered below
	RegisterFactoryForClass<TestLocomotionClass>();

	Debug::Log("COM registration done!\n");

	return 0;
}
#endif
