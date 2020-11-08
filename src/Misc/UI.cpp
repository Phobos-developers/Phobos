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
bool LS_DisableEmptySpawnPosition = false;
DEFINE_HOOK(640B8D, LoadingScreen_DisableEmptySpawnPosition, 6)
{
	GET(bool, esi, ESI);
	if (LS_DisableEmptySpawnPosition || !esi) {
		return 0x640CE2;
	}
	return 0x640B93;
}

//DEFINE_HOOK(640E78, LoadingScreen_DisableColorPoints, 6)
//{
//	return 0x641071;
//}

DEFINE_HOOK(5FACDF, UIMD_LoadFromINI, 5)
{
	CCINIClass* pINI = Phobos::OpenConfig("uimd.ini");

	LS_DisableEmptySpawnPosition =
		pINI->ReadBool("LoadingScreen", "DisableEmptySpawnPosition", false);

	Phobos::CloseConfig(pINI);
	return 0;
}
