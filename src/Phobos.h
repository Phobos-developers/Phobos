#pragma once

#include "Debug.h"
#include <Helpers/Macro.h>
#include <CCINIClass.h>

class Phobos
{
public:
	static const char* AppIconPath;

	static void CmdLineParse(char**, int);

	static CCINIClass* OpenConfig(const char*);
	static void CloseConfig(CCINIClass*&);
};
