#pragma once
#include <Helpers/Macro.h>
#include <CCINIClass.h>

#include "Utilities/Debug.h"
#include "Phobos.version.h"

#define TOOLTIPS_SECTION "ToolTips"

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

	static const char* AppIconPath;

	static class UI
	{
	public:
		static bool DisableEmptySpawnPositions;
		static bool ExtendedToolTips;

		static const wchar_t* CostLabel;
		static const wchar_t* PowerLabel;
		static const wchar_t* TimeLabel;
	};
};


