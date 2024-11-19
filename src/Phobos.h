#pragma once
#include <Phobos.version.h>
#include <Windows.h>

#include <string>

class CCINIClass;
class AbstractClass;

constexpr auto NONE_STR = "<none>";
constexpr auto NONE_STR2 = "none";
constexpr auto SIDEBAR_SECTION = "Sidebar";
constexpr auto UISETTINGS_SECTION = "UISettings";

class Phobos
{
public:
	static void CmdLineParse(char**, int);

	static void ExeRun();
	static void ExeTerminate();

	//variables
	static HANDLE hInstance;

	static const size_t readLength = 2048;
	static char readBuffer[readLength];
	static wchar_t wideBuffer[readLength];
	static constexpr auto readDelims = ",";

	static const char* AppIconPath;
	static const wchar_t* VersionDescription;
	static bool DisplayDamageNumbers;
	static bool IsLoadingSaveGame;
	static bool ShouldQuickSave;
	static std::wstring CustomGameSaveDescription;
	static void PassiveSaveGame();
#ifdef DEBUG
	static bool DetachFromDebugger();
#endif

	class UI
	{
	public:
		static bool DisableEmptySpawnPositions;
		static bool ExtendedToolTips;
		static int MaxToolTipWidth;
		static bool HarvesterCounter_Show;
		static double HarvesterCounter_ConditionYellow;
		static double HarvesterCounter_ConditionRed;
		static bool ProducingProgress_Show;
		static bool PowerDelta_Show;
		static double PowerDelta_ConditionYellow;
		static double PowerDelta_ConditionRed;
		static bool CenterPauseMenuBackground;
		static bool WeedsCounter_Show;
		static bool AnchoredToolTips;

		static const wchar_t* CostLabel;
		static const wchar_t* PowerLabel;
		static const wchar_t* PowerBlackoutLabel;
		static const wchar_t* TimeLabel;
		static const wchar_t* HarvesterLabel;
		static const wchar_t* ShowBriefingResumeButtonLabel;
		static const wchar_t* SWShotsFormat;
		static char ShowBriefingResumeButtonStatusLabel[0x20];
	};

	class Config
	{
	public:
		static bool ToolTipDescriptions;
		static bool ToolTipBlur;
		static bool PrioritySelectionFiltering;
		static bool DevelopmentCommands;
		static bool ArtImageSwap;
		static bool ShowPlacementPreview;
		static bool EnableBuildingPlacementPreview;
		static bool DigitalDisplay_Enable;
		static bool RealTimeTimers;
		static bool RealTimeTimers_Adaptive;
		static int CampaignDefaultGameSpeed;
		static bool SkirmishUnlimitedColors;
		static bool ShowDesignatorRange;
		static bool SaveVariablesOnScenarioEnd;
		static bool SaveGameOnScenarioStart;
		static bool ShowBriefing;
		static bool ShowPowerDelta;
		static bool ShowHarvesterCounter;
		static bool ShowWeedsCounter;
		static bool ShowPlanningPath;
		static bool HideLightFlashEffects;
	};

	class Misc
	{
	public:
		static bool CustomGS;
		static int CustomGS_ChangeInterval[7];
		static int CustomGS_ChangeDelay[7];
		static int CustomGS_DefaultDelay[7];
	};
};
