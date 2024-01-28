#pragma once
#include <Phobos.version.h>
#include <Windows.h>

#define CAN_USE_ARES 1

class CCINIClass;
class AbstractClass;

constexpr auto NONE_STR = "<none>";
constexpr auto NONE_STR2 = "none";
constexpr auto TOOLTIPS_SECTION = "ToolTips";
constexpr auto SIDEBAR_SECTION = "Sidebar";

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
	static const char readDelims[4];

	static const char* AppIconPath;
	static const wchar_t* VersionDescription;
	static bool DisplayDamageNumbers;
	static bool IsLoadingSaveGame;

#ifdef DEBUG
	static bool DetachFromDebugger();
#endif

	class UI
	{
	public:
		static bool DisableEmptySpawnPositions;
		static bool ExtendedToolTips;
		static int MaxToolTipWidth;
		static bool ShowHarvesterCounter;
		static double HarvesterCounter_ConditionYellow;
		static double HarvesterCounter_ConditionRed;
		static bool ShowProducingProgress;
		static bool ShowPowerDelta;
		static double PowerDelta_ConditionYellow;
		static double PowerDelta_ConditionRed;
		static bool CenterPauseMenuBackground;

		static const wchar_t* CostLabel;
		static const wchar_t* PowerLabel;
		static const wchar_t* PowerBlackoutLabel;
		static const wchar_t* TimeLabel;
		static const wchar_t* HarvesterLabel;
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
