#pragma once
#include "Phobos.version.h"

#include <Helpers/Macro.h>
#include "Misc/Debug.h"

class CCINIClass;
class AbstractClass;

constexpr auto NONE_STR = "<none>";
constexpr auto NONE_STR2 = "none";
constexpr auto TOOLTIPS_SECTION = "ToolTips";

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

	static void Clear();
	static void PointerGotInvalid(AbstractClass* const pInvalid, bool const removed);
	static HRESULT SaveGameData(IStream* pStm);
	static void LoadGameData(IStream* pStm);

	class UI
	{
	public:
		static bool DisableEmptySpawnPositions;
		static bool ExtendedToolTips;
		static int MaxToolTipWidth;

		static const wchar_t* CostLabel;
		static const wchar_t* PowerLabel;
		static const wchar_t* TimeLabel;
	};

	class Config
	{
	public:
		static bool ToolTipDescriptions;
		static bool PrioritySelectionFiltering;
		static bool DevelopmentCommands;
	};
};
