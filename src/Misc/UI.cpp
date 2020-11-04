#include "../Phobos.h"

DEFINE_HOOK(777C41, UI_ApplyAppIcon, 9)
{
	if (Phobos::AppIconPath != nullptr) {
		Debug::Log("Apply AppIcon from \"%s\"\n", Phobos::AppIconPath);

		R->EAX(LoadImage(NULL, Phobos::AppIconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
		return 0x777C4A;
	}

	return 0;
}

DEFINE_HOOK(640B8D, UI_DisableBlackPoints, 6)
{
	return 0x640CE2;
}

//DEFINE_HOOK(640E78, UI_DisableColorPoints, 6)
//{
//	return 0x641071;
//}
