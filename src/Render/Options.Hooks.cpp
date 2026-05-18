#include "Options.h"

#include <Helpers/Macro.h>

#include <CCINIClass.h>

DEFINE_HOOK(0x6BC141, RenderConfig_LoadRA2MD, 0x7) {
	auto& config = RenderOptions::Config();
	config.PreserveAspectRatio = CCINIClass::INI_RA2MD.ReadBool("DXRender", "PreserveAspectRatio", config.PreserveAspectRatio);
	config.WindowedBorder = CCINIClass::INI_RA2MD.ReadBool("DXRender", "WindowedBorder", config.WindowedBorder);
	config.StartFullscreen = CCINIClass::INI_RA2MD.ReadBool("DXRender", "StartFullscreen", config.StartFullscreen);
	config.PauseGameWhenLoseFocus = CCINIClass::INI_RA2MD.ReadBool("DXRender", "PauseGameWhenLoseFocus", config.PauseGameWhenLoseFocus);
	return 0;
}
