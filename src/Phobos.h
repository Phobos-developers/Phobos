#pragma once
#include "Phobos.version.h"

class CCINIClass;

constexpr auto TOOLTIPS_SECTION = "ToolTips";
constexpr auto SIDEBAR_SECTION = "Sidebar";

class Phobos
{
public:
	static void CmdLineParse(char**, int);

	static CCINIClass* OpenConfig(const char*);
	static void CloseConfig(CCINIClass*&);

	//variables
	static const size_t readLength = 2048;
	static char readBuffer[readLength];
	static wchar_t wideBuffer[readLength];
	static const char readDelims[4];

	static const char* AppIconPath;
	static const wchar_t* VersionDescription;

	class UI
	{
	public:
		static bool DisableEmptySpawnPositions;
		static bool ExtendedToolTips;
		static int MaxToolTipWidth;
		static bool ShowHarvesterCounter;

		static const wchar_t* CostLabel;
		static const wchar_t* PowerLabel;
		static const wchar_t* TimeLabel;
		static const wchar_t* HarvesterLabel;
	};

	class Config
	{
	public:
		static bool ToolTipDescriptions;
		static bool PrioritySelectionFiltering;
		static bool DevelopmentCommands;
	};
};
