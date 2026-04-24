#include "Phobos.COM.h"

#include <Helpers/Macro.h>

#include <Locomotion/TestLocomotionClass.h>
#include <Locomotion/ShiftLocomotionClass.h>


DEFINE_HOOK(0x6BD68D, WinMain_PhobosRegistrations, 0x6)
{
	Debug::Log("Starting COM registration...\n");

	RegisterFactoryForClass<ShiftLocomotionClass>();

	Debug::Log("COM registration done!\n");

	return 0;
}
