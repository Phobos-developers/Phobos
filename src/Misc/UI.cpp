#include "../Phobos.h"
#include <StringTable.h>

const wchar_t* LoadStringOrDefault(char* key, const wchar_t* defaultValue)
{
	if (strlen(key) != 0)
		return StringTable::LoadStringA(key);
	else
		return defaultValue;
}

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

DEFINE_HOOK(5FACDF, UIMD_LoadFromINI, 5)
{
	CCINIClass* pINI = Phobos::OpenConfig("uimd.ini");

	#pragma region LoadingScreen

	Phobos::UI::DisableEmptySpawnPositions =
		pINI->ReadBool("LoadingScreen", "DisableEmptySpawnPositions", false);

	#pragma endregion

	#pragma region ToolTips
	
	Phobos::UI::ExtendedToolTips =
		pINI->ReadBool(TOOLTIPS_SECTION, "ExtendedToolTips", false);

	pINI->ReadString(TOOLTIPS_SECTION, "CostLabel", "", Phobos::readBuffer);
	Phobos::UI::CostLabel = LoadStringOrDefault(Phobos::readBuffer, L"$");

	pINI->ReadString(TOOLTIPS_SECTION, "PowerLabel", "", Phobos::readBuffer);
	Phobos::UI::PowerLabel = LoadStringOrDefault(Phobos::readBuffer, L"⚡");

	pINI->ReadString(TOOLTIPS_SECTION, "TimeLabel", "", Phobos::readBuffer);
	Phobos::UI::TimeLabel = LoadStringOrDefault(Phobos::readBuffer, L"⌚");

	#pragma endregion

	Phobos::CloseConfig(pINI);
	return 0;
}

