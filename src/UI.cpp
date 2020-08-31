#include "Phobos.h"

DEFINE_HOOK(777C41, UI_ApplyAppIcon, 9)
{
	if (Phobos::AppIconPath != nullptr) {
		Debug::Log("Apply AppIcon from \"%s\"\n", Phobos::AppIconPath);

		R->EAX(LoadImage(NULL, Phobos::AppIconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
		return 0x777C4A;
	}

	return 0;
}
