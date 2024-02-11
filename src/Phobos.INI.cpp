#include "Phobos.h"

#include <GameStrings.h>
#include <CCINIClass.h>

#include <Utilities/Parser.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include "Misc/BlittersFix.h"

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
bool Phobos::Config::DigitalDisplay_Enable = false;
bool Phobos::Config::RealTimeTimers = false;
bool Phobos::Config::RealTimeTimers_Adaptive = false;
int Phobos::Config::CampaignDefaultGameSpeed = 2;
bool Phobos::Config::SkirmishUnlimitedColors = false;
bool Phobos::Config::ShowDesignatorRange = false;
bool Phobos::Config::SaveVariablesOnScenarioEnd = false;
bool Phobos::Config::FixUnitLightingTint = true;
bool Phobos::Config::FixTiberiumLightingTint = true;

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

	CCINIClass* pINI_UIMD = CCINIClass::LoadINIFile(GameStrings::UIMD_INI);

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

	CCINIClass::UnloadINIFile(pINI_UIMD);

	CCINIClass* pINI_RULESMD = CCINIClass::LoadINIFile(GameStrings::RULESMD_INI);

	Phobos::Config::ArtImageSwap = pINI_RULESMD->ReadBool(GameStrings::General, "ArtImageSwap", false);
	Phobos::Config::FixUnitLightingTint = pINI_RULESMD->ReadBool(GameStrings::General, "FixUnitLightingTint", true);
	Phobos::Config::FixTiberiumLightingTint = pINI_RULESMD->ReadBool(GameStrings::General, "FixTiberiumLightingTint", true);

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
	if (Phobos::Config::SkirmishUnlimitedColors)
	{
		// Game_GetLinkedColor converts vanilla dropdown color index into color scheme index ([Colors] from rules)
		// What we want to do is to restore vanilla from Ares hook, and immediately return arg
		// So if spawner feeds us a number, it will be used to look up color scheme directly
		Patch::Apply_RAW(0x69A310,
			{
				0x8B, 0x44, 0x24, 0x04, // mov eax, [esp+4]
				0xD1, 0xE0,             // shl eax, 1
				0x40,                   // inc eax
				0xC2, 0x04, 0x00        // retn 4
			}
		);
	}

	Phobos::Config::SaveVariablesOnScenarioEnd = pINI_RULESMD->ReadBool(GameStrings::General, "SaveVariablesOnScenarioEnd", false);

	CCINIClass::UnloadINIFile(pINI_RULESMD);

	return 0;
}

DEFINE_HOOK(0x66E9DF, RulesClass_Process_Phobos, 0x8)
{
#ifndef DEBUG
	GET(CCINIClass*, rulesINI, EDI);

	Phobos::Config::DevelopmentCommands = rulesINI->ReadBool("GlobalControls", "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);
#endif

	return 0;
}
