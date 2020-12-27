#pragma once
#include <Helpers/Macro.h>
#include <CCINIClass.h>

#include "Utilities/Debug.h"
#include "Phobos.version.h"

class Phobos
{
public:
	static void CmdLineParse(char**, int);

	static CCINIClass* OpenConfig(const char*);
	static void CloseConfig(CCINIClass*&);

	//variables
	static const size_t readLength = 2048;
	static char readBuffer[readLength];
	static const char readDelims[4];

	static const char* AppIconPath;
};
