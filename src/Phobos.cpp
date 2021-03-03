#include <StaticInits.cpp>
#include <CCINIClass.h>
#include <Unsorted.h>

#include "Phobos.h"

#ifndef IS_RELEASE_VER
	bool HideWarning = false;
#endif

char Phobos::readBuffer[Phobos::readLength];
wchar_t Phobos::wideBuffer[Phobos::readLength];
const char Phobos::readDelims[4] = ",";

const char* Phobos::AppIconPath = nullptr;

#ifdef STR_GIT_COMMIT
const wchar_t* Phobos::VersionDescription = L"Phobos nightly build (" STR_GIT_COMMIT L" @ " STR_GIT_BRANCH L"). DO NOT SHIP IN MODS!";
#elif !defined(IS_RELEASE_VER)
const wchar_t* Phobos::VersionDescription = L"Phobos development build #" str(BUILD_NUMBER) L". Please test the build before shipping.";
#else
//const wchar_t* Phobos::VersionDescription = L"Phobos release build v" FILE_VERSION_STR L".";
#endif

bool Phobos::UI::DisableEmptySpawnPositions = false;
bool Phobos::UI::ExtendedToolTips = false;
int Phobos::UI::MaxToolTipWidth = 0;
const wchar_t* Phobos::UI::CostLabel = L"";
const wchar_t* Phobos::UI::PowerLabel = L"";
const wchar_t* Phobos::UI::TimeLabel = L"";

bool Phobos::Config::ToolTipDescriptions = true;
bool Phobos::Config::PrioritySelectionFiltering = true;

void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; i++)
	{
		const char* pArg = ppArgs[i];

		if (_stricmp(pArg, "-Icon") == 0)
		{
			Phobos::AppIconPath = ppArgs[++i];
		}
		#ifndef IS_RELEASE_VER 
		if (_stricmp(pArg, "-b=" str(BUILD_NUMBER)) == 0)
		{
			HideWarning = true;
		}
		#endif
	}
}

CCINIClass* Phobos::OpenConfig(const char* file) {
	CCINIClass* pINI = GameCreate<CCINIClass>();

	if (pINI) {
		CCFileClass* cfg = GameCreate<CCFileClass>(file);

		if (cfg) {
			if (cfg->Exists()) {
				pINI->ReadCCFile(cfg);
			}
			GameDelete(cfg);
		}
	}

	return pINI;
}

void Phobos::CloseConfig(CCINIClass*& pINI) {
	if (pINI) {
		GameDelete(pINI);
		pINI = nullptr;
	}
}

// =============================
// hooks

/*
//DllMain
bool __stdcall DllMain(HANDLE hInstance, DWORD dwReason, LPVOID v)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}
*/

DEFINE_HOOK(52F639, _YR_CmdLineParse, 5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Phobos::CmdLineParse(ppArgs, nNumArgs);
	return 0;
}

DEFINE_HOOK(5FACDF, OptionsClass_LoadSettings_LoadPhobosSettings, 5)
{
	Phobos::Config::ToolTipDescriptions = Unsorted::RA2MDINI->ReadBool("Phobos", "ToolTipDescriptions", true);
	Phobos::Config::PrioritySelectionFiltering = Unsorted::RA2MDINI->ReadBool("Phobos", "PrioritySelectionFiltering", true);

	CCINIClass* pINI = Phobos::OpenConfig("uimd.ini");

	// LoadingScreen
	{
		Phobos::UI::DisableEmptySpawnPositions =
			pINI->ReadBool("LoadingScreen", "DisableEmptySpawnPositions", false);
	}

	// ToolTips
	{
		Phobos::UI::ExtendedToolTips =
			pINI->ReadBool(TOOLTIPS_SECTION, "ExtendedToolTips", false);

		Phobos::UI::MaxToolTipWidth =
			pINI->ReadInteger(TOOLTIPS_SECTION, "MaxWidth", 0);

		pINI->ReadString(TOOLTIPS_SECTION, "CostLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::CostLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"$");

		pINI->ReadString(TOOLTIPS_SECTION, "PowerLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::PowerLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"⚡");

		pINI->ReadString(TOOLTIPS_SECTION, "TimeLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::TimeLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"⌚");
	}

	Phobos::CloseConfig(pINI);

	return 0;
}

#ifndef IS_RELEASE_VER
DEFINE_HOOK(4F4583, GScreenClass_DrawText, 6)
{
#ifndef STR_GIT_COMMIT
	if (!HideWarning)
#endif // !STR_GIT_COMMIT
	{
		auto wanted = Drawing::GetTextDimensions(Phobos::VersionDescription);

		RectangleStruct rect = {
			DSurface::Composite->GetWidth() - wanted.Width - 10,
			0,
			wanted.Width + 10,
			wanted.Height + 10
		};

		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
		DSurface::Composite->DrawTextA(Phobos::VersionDescription, rect.X + 5, 5, COLOR_RED);
	}
	return 0;
}
#endif
