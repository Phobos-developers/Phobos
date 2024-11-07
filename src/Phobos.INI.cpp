#include "Phobos.h"

#include <CCINIClass.h>
#include <ScenarioClass.h>
#include <SessionClass.h>
#include <MessageListClass.h>
#include <HouseClass.h>

#include <Utilities/Parser.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include "Misc/BlittersFix.h"

bool Phobos::UI::DisableEmptySpawnPositions = false;
bool Phobos::UI::ExtendedToolTips = false;
int Phobos::UI::MaxToolTipWidth = 0;
bool Phobos::UI::HarvesterCounter_Show = false;
double Phobos::UI::HarvesterCounter_ConditionYellow = 0.99;
double Phobos::UI::HarvesterCounter_ConditionRed = 0.5;
bool Phobos::UI::ProducingProgress_Show = false;
const wchar_t* Phobos::UI::CostLabel = L"";
const wchar_t* Phobos::UI::PowerLabel = L"";
const wchar_t* Phobos::UI::PowerBlackoutLabel = L"";
const wchar_t* Phobos::UI::TimeLabel = L"";
const wchar_t* Phobos::UI::HarvesterLabel = L"";
const wchar_t* Phobos::UI::ShowBriefingResumeButtonLabel = L"";
const wchar_t* Phobos::UI::SWShotsFormat = L"";
char Phobos::UI::ShowBriefingResumeButtonStatusLabel[32];
bool Phobos::UI::PowerDelta_Show = false;
double Phobos::UI::PowerDelta_ConditionYellow = 0.75;
double Phobos::UI::PowerDelta_ConditionRed = 1.0;
bool Phobos::UI::CenterPauseMenuBackground = false;
bool Phobos::UI::WeedsCounter_Show = false;
bool Phobos::UI::AnchoredToolTips = false;

bool Phobos::Config::ToolTipDescriptions = true;
bool Phobos::Config::ToolTipBlur = false;
bool Phobos::Config::PrioritySelectionFiltering = true;
bool Phobos::Config::DevelopmentCommands = true;
bool Phobos::Config::ShowPlanningPath = false;
bool Phobos::Config::ArtImageSwap = false;
bool Phobos::Config::ShowPlacementPreview = false;
bool Phobos::Config::DigitalDisplay_Enable = false;
bool Phobos::Config::RealTimeTimers = false;
bool Phobos::Config::RealTimeTimers_Adaptive = false;
int Phobos::Config::CampaignDefaultGameSpeed = 2;
bool Phobos::Config::SkirmishUnlimitedColors = false;
bool Phobos::Config::ShowDesignatorRange = false;
bool Phobos::Config::SaveVariablesOnScenarioEnd = false;
bool Phobos::Config::SaveGameOnScenarioStart = true;
bool Phobos::Config::ShowBriefing = true;
bool Phobos::Config::ShowHarvesterCounter = false;
bool Phobos::Config::ShowPowerDelta = true;
bool Phobos::Config::ShowWeedsCounter = false;
bool Phobos::Config::HideLightFlashEffects = true;

bool Phobos::Misc::CustomGS = false;
int Phobos::Misc::CustomGS_ChangeInterval[7] = { -1, -1, -1, -1, -1, -1, -1 };
int Phobos::Misc::CustomGS_ChangeDelay[7] = { 0, 1, 2, 3, 4, 5, 6 };
int Phobos::Misc::CustomGS_DefaultDelay[7] = { 0, 1, 2, 3, 4, 5, 6 };

DEFINE_HOOK(0x5FACDF, OptionsClass_LoadSettings_LoadPhobosSettings, 0x5)
{
	Phobos::Config::ToolTipDescriptions = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ToolTipDescriptions", true);
	Phobos::Config::ToolTipBlur = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ToolTipBlur", false);
	Phobos::Config::PrioritySelectionFiltering = CCINIClass::INI_RA2MD->ReadBool("Phobos", "PrioritySelectionFiltering", true);
	Phobos::Config::ShowPlacementPreview = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowPlacementPreview", true);
	Phobos::Config::RealTimeTimers = CCINIClass::INI_RA2MD->ReadBool("Phobos", "RealTimeTimers", false);
	Phobos::Config::RealTimeTimers_Adaptive = CCINIClass::INI_RA2MD->ReadBool("Phobos", "RealTimeTimers.Adaptive", false);
	Phobos::Config::DigitalDisplay_Enable = CCINIClass::INI_RA2MD->ReadBool("Phobos", "DigitalDisplay.Enable", false);
	Phobos::Config::SaveGameOnScenarioStart = CCINIClass::INI_RA2MD->ReadBool("Phobos", "SaveGameOnScenarioStart", true);
	Phobos::Config::ShowBriefing = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowBriefing", true);
	Phobos::Config::ShowPowerDelta = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowPowerDelta", true);
	Phobos::Config::ShowHarvesterCounter = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowHarvesterCounter", true);
	Phobos::Config::ShowWeedsCounter = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowWeedsCounter", true);
	Phobos::Config::HideLightFlashEffects = CCINIClass::INI_RA2MD->ReadBool("Phobos", "HideLightFlashEffects", false);

	// Custom game speeds, 6 - i so that GS6 is index 0, just like in the engine
	Phobos::Config::CampaignDefaultGameSpeed = 6 - CCINIClass::INI_RA2MD->ReadInteger("Phobos", "CampaignDefaultGameSpeed", 4);
	if (Phobos::Config::CampaignDefaultGameSpeed > 6 || Phobos::Config::CampaignDefaultGameSpeed < 0)
	{
		Phobos::Config::CampaignDefaultGameSpeed = 2;
	}

	{
		const byte temp = (byte)Phobos::Config::CampaignDefaultGameSpeed;

		Patch::Apply_RAW(0x55D77A, { temp }); // We overwrite the instructions that force GameSpeed to 2 (GS4)
		Patch::Apply_RAW(0x55D78D, { temp }); // when speed control is off. Doesn't need a hook.
	}

	Phobos::Config::ShowDesignatorRange = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowDesignatorRange", false);

	CCINIClass ini_uimd {};
	ini_uimd.LoadFromFile(GameStrings::UIMD_INI);

	// LoadingScreen
	{
		Phobos::UI::DisableEmptySpawnPositions =
			ini_uimd.ReadBool("LoadingScreen", "DisableEmptySpawnPositions", false);
	}

	// ToolTips
	{
		Phobos::UI::ExtendedToolTips =
			ini_uimd.ReadBool(GameStrings::ToolTips, "ExtendedToolTips", false);

		Phobos::UI::AnchoredToolTips =
			ini_uimd.ReadBool(GameStrings::ToolTips, "AnchoredToolTips", false);

		Phobos::UI::MaxToolTipWidth =
			ini_uimd.ReadInteger(GameStrings::ToolTips, "MaxWidth", 0);

		ini_uimd.ReadString(GameStrings::ToolTips, "CostLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::CostLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"$");

		ini_uimd.ReadString(GameStrings::ToolTips, "PowerLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::PowerLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26a1"); // ⚡

		ini_uimd.ReadString(GameStrings::ToolTips, "PowerBlackoutLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::PowerBlackoutLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26a1\u274c"); // ⚡❌

		ini_uimd.ReadString(GameStrings::ToolTips, "TimeLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::TimeLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u231a"); // ⌚

		ini_uimd.ReadString(GameStrings::ToolTips, "SWShotsFormat", NONE_STR, Phobos::readBuffer);
		Phobos::UI::SWShotsFormat = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"Shots: %d"); // ⌚
	}

	// Sidebar
	{
		Phobos::UI::HarvesterCounter_Show =
			ini_uimd.ReadBool(SIDEBAR_SECTION, "HarvesterCounter.Show", false);

		ini_uimd.ReadString(SIDEBAR_SECTION, "HarvesterCounter.Label", NONE_STR, Phobos::readBuffer);
		Phobos::UI::HarvesterLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26cf"); // ⛏

		Phobos::UI::HarvesterCounter_ConditionYellow =
			ini_uimd.ReadDouble(SIDEBAR_SECTION, "HarvesterCounter.ConditionYellow", Phobos::UI::HarvesterCounter_ConditionYellow);

		Phobos::UI::HarvesterCounter_ConditionRed =
			ini_uimd.ReadDouble(SIDEBAR_SECTION, "HarvesterCounter.ConditionRed", Phobos::UI::HarvesterCounter_ConditionRed);

		Phobos::UI::WeedsCounter_Show =
			ini_uimd.ReadBool(SIDEBAR_SECTION, "WeedsCounter.Show", false);

		Phobos::UI::ProducingProgress_Show =
			ini_uimd.ReadBool(SIDEBAR_SECTION, "ProducingProgress.Show", false);

		Phobos::UI::PowerDelta_Show =
			ini_uimd.ReadBool(SIDEBAR_SECTION, "PowerDelta.Show", false);

		Phobos::UI::PowerDelta_ConditionYellow =
			ini_uimd.ReadDouble(SIDEBAR_SECTION, "PowerDelta.ConditionYellow", Phobos::UI::PowerDelta_ConditionYellow);

		Phobos::UI::PowerDelta_ConditionRed =
			ini_uimd.ReadDouble(SIDEBAR_SECTION, "PowerDelta.ConditionRed", Phobos::UI::PowerDelta_ConditionRed);

		Phobos::UI::CenterPauseMenuBackground =
			ini_uimd.ReadBool(SIDEBAR_SECTION, "CenterPauseMenuBackground", Phobos::UI::CenterPauseMenuBackground);
	}

	// UISettings
	{
		ini_uimd.ReadString(UISETTINGS_SECTION, "ShowBriefingResumeButtonLabel", "GUI:Resume", Phobos::readBuffer);
		Phobos::UI::ShowBriefingResumeButtonLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"");

		ini_uimd.ReadString(UISETTINGS_SECTION, "ShowBriefingResumeButtonStatusLabel", "STT:BriefingButtonReturn", Phobos::readBuffer);
		strcpy_s(Phobos::UI::ShowBriefingResumeButtonStatusLabel, Phobos::readBuffer);
	}

	return 0;
}

DEFINE_HOOK(0x52D21F, InitRules_ThingsThatShouldntBeSerailized, 0x6)
{
	CCINIClass* const pINI_RULESMD = CCINIClass::INI_Rules;

	RulesClass::Instance->Read_JumpjetControls(pINI_RULESMD);

	Phobos::Config::ArtImageSwap = pINI_RULESMD->ReadBool(GameStrings::General, "ArtImageSwap", false);


	Phobos::Misc::CustomGS = pINI_RULESMD->ReadBool(GameStrings::General, "CustomGS", false);

	char tempBuffer[26];
	for (size_t i = 0; i <= 6; ++i)
	{
		int temp;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "CustomGS%d.ChangeDelay", 6 - i);
		temp = pINI_RULESMD->ReadInteger(GameStrings::General, tempBuffer, -1);
		if (temp >= 0 && temp <= 6)
			Phobos::Misc::CustomGS_ChangeDelay[i] = 6 - temp;

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "CustomGS%d.DefaultDelay", 6 - i);
		temp = pINI_RULESMD->ReadInteger(GameStrings::General, tempBuffer, -1);
		if (temp >= 1)
			Phobos::Misc::CustomGS_DefaultDelay[i] = 6 - temp;

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "CustomGS%d.ChangeInterval", 6 - i);
		temp = pINI_RULESMD->ReadInteger(GameStrings::General, tempBuffer, -1);
		if (temp >= 1)
			Phobos::Misc::CustomGS_ChangeInterval[i] = temp;
	}

	if (pINI_RULESMD->ReadBool(GameStrings::General, "FixTransparencyBlitters", true))
		BlittersFix::Apply();

	Phobos::Config::SkirmishUnlimitedColors = pINI_RULESMD->ReadBool(GameStrings::General, "SkirmishUnlimitedColors", false);
	// Disable Ares hook at this address so that our logic can run.
	if (Phobos::Config::SkirmishUnlimitedColors)
		Patch::Apply_RAW(0x69A310, { 0x8B, 0x44, 0x24, 0x04, 0xD1, 0xE0, 0x40 });

	Phobos::Config::SaveVariablesOnScenarioEnd = pINI_RULESMD->ReadBool(GameStrings::General, "SaveVariablesOnScenarioEnd", false);
#ifndef DEBUG
	Phobos::Config::DevelopmentCommands = pINI_RULESMD->ReadBool("GlobalControls", "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);
#endif
	Phobos::Config::ShowPlanningPath = pINI_RULESMD->ReadBool("GlobalControls", "DebugPlanningPaths", Phobos::Config::ShowPlanningPath);

	return 0;
}


bool Phobos::ShouldQuickSave = false;
std::wstring Phobos::CustomGameSaveDescription {};

void Phobos::PassiveSaveGame()
{
	auto PrintMessage = [](const wchar_t* pMessage)
	{
		MessageListClass::Instance->PrintMessage(
			pMessage,
			RulesClass::Instance->MessageDelay,
			HouseClass::CurrentPlayer->ColorSchemeIndex,
			true
		);
	};

	PrintMessage(StringTable::LoadString(GameStrings::TXT_SAVING_GAME));
	char fName[0x80];

	SYSTEMTIME time;
	GetLocalTime(&time);

	_snprintf_s(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.sav",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

	if (ScenarioClass::SaveGame(fName, Phobos::CustomGameSaveDescription.c_str()))
		PrintMessage(StringTable::LoadString(GameStrings::TXT_GAME_WAS_SAVED));
	else
		PrintMessage(StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME));
}

DEFINE_HOOK(0x55DBCD, MainLoop_SaveGame, 0x6)
{
	// This happens right before LogicClass::Update()
	enum { SkipSave = 0x55DC99, InitialSave = 0x55DBE6 };

	bool& scenario_saved = *reinterpret_cast<bool*>(0xABCE08);
	if (SessionClass::IsSingleplayer() && !scenario_saved)
	{
		scenario_saved = true;
		if (Phobos::ShouldQuickSave)
		{
			Phobos::PassiveSaveGame();
			Phobos::ShouldQuickSave = false;
			Phobos::CustomGameSaveDescription.clear();
		}
		else if (Phobos::Config::SaveGameOnScenarioStart && SessionClass::IsCampaign())
			return InitialSave;
	}

	return SkipSave;
}
