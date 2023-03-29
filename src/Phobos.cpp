#include <Phobos.h>

#include <Helpers/Macro.h>
#include <GameStrings.h>
#include <CCINIClass.h>
#include <Unsorted.h>
#include <Drawing.h>

#include "Utilities/Parser.h"
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include "Misc/BlittersFix.h"

#ifndef IS_RELEASE_VER
bool HideWarning = false;
#endif

HANDLE Phobos::hInstance = 0;

char Phobos::readBuffer[Phobos::readLength];
wchar_t Phobos::wideBuffer[Phobos::readLength];
const char Phobos::readDelims[4] = ",";

const char* Phobos::AppIconPath = nullptr;

bool Phobos::Debug_DisplayDamageNumbers = false;

#ifdef STR_GIT_COMMIT
const wchar_t* Phobos::VersionDescription = L"Phobos nightly build (" STR_GIT_COMMIT L" @ " STR_GIT_BRANCH L"). DO NOT SHIP IN MODS!";
#elif !defined(IS_RELEASE_VER)
const wchar_t* Phobos::VersionDescription = L"Phobos development build #" _STR(BUILD_NUMBER) L". Please test the build before shipping.";
#else
//const wchar_t* Phobos::VersionDescription = L"Phobos release build v" FILE_VERSION_STR L".";
#endif

bool Phobos::UI::DisableEmptySpawnPositions = false;
bool Phobos::UI::ExtendedToolTips = false;
int Phobos::UI::MaxToolTipWidth = 0;
bool Phobos::UI::ShowHarvesterCounter = false;
double Phobos::UI::HarvesterCounter_ConditionYellow = 0.99;
double Phobos::UI::HarvesterCounter_ConditionRed = 0.5;
bool Phobos::UI::ShowProducingProgress = false;
const wchar_t* Phobos::UI::CostLabel = L"";
const wchar_t* Phobos::UI::PowerLabel = L"";
const wchar_t* Phobos::UI::PowerBlackoutLabel = L"";
const wchar_t* Phobos::UI::TimeLabel = L"";
const wchar_t* Phobos::UI::HarvesterLabel = L"";
bool Phobos::UI::ShowPowerDelta = false;
double Phobos::UI::PowerDelta_ConditionYellow = 0.75;
double Phobos::UI::PowerDelta_ConditionRed = 1.0;
bool Phobos::UI::CenterPauseMenuBackground = false;

bool Phobos::Config::ToolTipDescriptions = true;
bool Phobos::Config::ToolTipBlur = false;
bool Phobos::Config::PrioritySelectionFiltering = true;
bool Phobos::Config::DevelopmentCommands = true;
bool Phobos::Config::ArtImageSwap = false;
bool Phobos::Config::ShowPlacementPreview = false;

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
		if (_stricmp(pArg, "-b=" _STR(BUILD_NUMBER)) == 0)
		{
			HideWarning = true;
		}
#endif
	}

	Debug::Log("Initialized Phobos " PRODUCT_VERSION "\n");
}

CCINIClass* Phobos::OpenConfig(const char* file)
{
	CCINIClass* pINI = GameCreate<CCINIClass>();

	if (pINI)
	{
		CCFileClass* cfg = GameCreate<CCFileClass>(file);

		if (cfg)
		{
			if (cfg->Exists())
			{
				pINI->ReadCCFile(cfg);
			}
			GameDelete(cfg);
		}
	}

	return pINI;
}

void Phobos::CloseConfig(CCINIClass*& pINI)
{
	if (pINI)
	{
		GameDelete(pINI);
		pINI = nullptr;
	}
}

void Phobos::ExeRun()
{
	Patch::ApplyStatic();

#ifdef DEBUG

	if (Phobos::DetachFromDebugger())
	{
		MessageBoxW(NULL,
		L"You can now attach a debugger.\n\n"

		L"Press OK to continue YR execution.",
		L"Debugger Notice", MB_OK);
	}
	else
	{
		MessageBoxW(NULL,
		L"You can now attach a debugger.\n\n"

		L"To attach a debugger find the YR process in Process Hacker "
		L"/ Visual Studio processes window and detach debuggers from it, "
		L"then you can attach your own debugger. After this you should "
		L"terminate Syringe.exe because it won't automatically exit when YR is closed.\n\n"

		L"Press OK to continue YR execution.",
		L"Debugger Notice", MB_OK);
	}

	if (!Console::Create())
	{
		MessageBoxW(NULL,
		L"Failed to allocate the debug console!",
		L"Debug Console Notice", MB_OK);
	}

#endif
}

void Phobos::ExeTerminate()
{
	Console::Release();
}

// =============================
// hooks

bool __stdcall DllMain(HANDLE hInstance, DWORD dwReason, LPVOID v)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		Phobos::hInstance = hInstance;
	}
	return true;
}

DEFINE_HOOK(0x7CD810, ExeRun, 0x9)
{
	Phobos::ExeRun();

	return 0;
}

void NAKED _ExeTerminate()
{
	// Call WinMain
	SET_REG32(EAX, 0x6BB9A0);
	CALL(EAX);
	PUSH_REG(EAX);

	Phobos::ExeTerminate();

	// Jump back
	POP_REG(EAX);
	SET_REG32(EBX, 0x7CD8EF);
	__asm {jmp ebx};
}
DEFINE_JUMP(LJMP, 0x7CD8EA, GET_OFFSET(_ExeTerminate));

DEFINE_HOOK(0x52F639, _YR_CmdLineParse, 0x5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Phobos::CmdLineParse(ppArgs, nNumArgs);
	return 0;
}

DEFINE_HOOK(0x5FACDF, OptionsClass_LoadSettings_LoadPhobosSettings, 0x5)
{
	Phobos::Config::ToolTipDescriptions = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ToolTipDescriptions", true);
	Phobos::Config::ToolTipBlur = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ToolTipBlur", false);
	Phobos::Config::PrioritySelectionFiltering = CCINIClass::INI_RA2MD->ReadBool("Phobos", "PrioritySelectionFiltering", true);
	Phobos::Config::ShowPlacementPreview = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowPlacementPreview", true);

	CCINIClass* pINI_UIMD = Phobos::OpenConfig(GameStrings::UIMD_INI);

	// LoadingScreen
	{
		Phobos::UI::DisableEmptySpawnPositions =
			pINI_UIMD->ReadBool("LoadingScreen", "DisableEmptySpawnPositions", false);
	}

	// ToolTips
	{
		Phobos::UI::ExtendedToolTips =
			pINI_UIMD->ReadBool(TOOLTIPS_SECTION, "ExtendedToolTips", false);

		Phobos::UI::MaxToolTipWidth =
			pINI_UIMD->ReadInteger(TOOLTIPS_SECTION, "MaxWidth", 0);

		pINI_UIMD->ReadString(TOOLTIPS_SECTION, "CostLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::CostLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"$");

		pINI_UIMD->ReadString(TOOLTIPS_SECTION, "PowerLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::PowerLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26a1"); // ⚡

		pINI_UIMD->ReadString(TOOLTIPS_SECTION, "PowerBlackoutLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::PowerBlackoutLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26a1\u274c"); // ⚡❌

		pINI_UIMD->ReadString(TOOLTIPS_SECTION, "TimeLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::TimeLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u231a"); // ⌚
	}

	// Sidebar
	{
		Phobos::UI::ShowHarvesterCounter =
			pINI_UIMD->ReadBool(SIDEBAR_SECTION, "HarvesterCounter.Show", false);

		pINI_UIMD->ReadString(SIDEBAR_SECTION, "HarvesterCounter.Label", NONE_STR, Phobos::readBuffer);
		Phobos::UI::HarvesterLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26cf"); // ⛏

		Phobos::UI::HarvesterCounter_ConditionYellow =
			pINI_UIMD->ReadDouble(SIDEBAR_SECTION, "HarvesterCounter.ConditionYellow", Phobos::UI::HarvesterCounter_ConditionYellow);

		Phobos::UI::HarvesterCounter_ConditionRed =
			pINI_UIMD->ReadDouble(SIDEBAR_SECTION, "HarvesterCounter.ConditionRed", Phobos::UI::HarvesterCounter_ConditionRed);

		Phobos::UI::ShowProducingProgress =
			pINI_UIMD->ReadBool(SIDEBAR_SECTION, "ProducingProgress.Show", false);

		Phobos::UI::ShowPowerDelta =
			pINI_UIMD->ReadBool(SIDEBAR_SECTION, "PowerDelta.Show", false);

		Phobos::UI::PowerDelta_ConditionYellow =
			pINI_UIMD->ReadDouble(SIDEBAR_SECTION, "PowerDelta.ConditionYellow", Phobos::UI::PowerDelta_ConditionYellow);

		Phobos::UI::PowerDelta_ConditionRed =
			pINI_UIMD->ReadDouble(SIDEBAR_SECTION, "PowerDelta.ConditionRed", Phobos::UI::PowerDelta_ConditionRed);

		Phobos::UI::CenterPauseMenuBackground =
			pINI_UIMD->ReadBool(SIDEBAR_SECTION, "CenterPauseMenuBackground", Phobos::UI::CenterPauseMenuBackground);
	}

	Phobos::CloseConfig(pINI_UIMD);

	CCINIClass* pINI_RULESMD = Phobos::OpenConfig(GameStrings::RULESMD_INI);

	Phobos::Config::ArtImageSwap = pINI_RULESMD->ReadBool(GameStrings::General, "ArtImageSwap", false);

	if (pINI_RULESMD->ReadBool(GameStrings::General, "FixTransparencyBlitters", true))
		BlittersFix::Apply();

	Phobos::CloseConfig(pINI_RULESMD);

	return 0;
}

DEFINE_HOOK(0x66E9DF, RulesClass_Process_Phobos, 0x8)
{
	GET(CCINIClass*, rulesINI, EDI);

#ifndef DEBUG
	Phobos::Config::DevelopmentCommands = rulesINI->ReadBool("GlobalControls", "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);
#endif

	return 0;
}

#ifndef IS_RELEASE_VER
DEFINE_HOOK(0x4F4583, GScreenClass_DrawText, 0x6)
{
#ifndef STR_GIT_COMMIT
	if (!HideWarning)
#endif // !STR_GIT_COMMIT
	{
		auto wanted = Drawing::GetTextDimensions(Phobos::VersionDescription, { 0,0 }, 0, 2, 0);

		RectangleStruct rect = {
			DSurface::Composite->GetWidth() - wanted.Width - 10,
			0,
			wanted.Width + 10,
			wanted.Height + 10
		};

		Point2D location { rect.X + 5,5 };

		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
		DSurface::Composite->DrawText(Phobos::VersionDescription, &location, COLOR_RED);
	}
	return 0;
}
#endif
