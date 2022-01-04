#include <Phobos.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x55D774, MainLoop_CampaignGameSpeed, 0xA)
{
	Options.GameSpeed = Phobos::Config::CampaignDefaultGameSpeed;
	return 0x55D77E;
}

DEFINE_HOOK(0x69BAE7, SessionClass_Resume_CampaignGameSpeed, 0xA)
{
	Options.GameSpeed = Phobos::Config::CampaignDefaultGameSpeed;
	return 0x69BAF1;
}
