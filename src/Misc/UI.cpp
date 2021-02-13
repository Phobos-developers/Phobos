#include "../Phobos.h"

DEFINE_HOOK(777C41, UI_ApplyAppIcon, 9)
{
	if (Phobos::AppIconPath != nullptr) {
		Debug::Log("Applying AppIcon from \"%s\"\n", Phobos::AppIconPath);

		R->EAX(LoadImage(NULL, Phobos::AppIconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
		return 0x777C4A;
	}

	return 0;
}

DEFINE_HOOK(640B8D, LoadingScreen_DisableEmptySpawnPositions, 6)
{
	GET(bool, esi, ESI);
	if (Phobos::UI::DisableEmptySpawnPositions || !esi) {
		return 0x640CE2;
	}
	return 0x640B93;
}

//DEFINE_HOOK(640E78, LoadingScreen_DisableColorPoints, 6)
//{
//	return 0x641071;
//}
