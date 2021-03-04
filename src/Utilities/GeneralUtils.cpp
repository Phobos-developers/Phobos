#include "GeneralUtils.h"
#include <string.h>
#include <StringTable.h>
#include <CCINIClass.h>

bool GeneralUtils::IsValidString(const char* str)
{
    return str != nullptr
        && strlen(str) != 0
        && !INIClass::IsBlank(str);
}

const wchar_t* GeneralUtils::LoadStringOrDefault(char* key, const wchar_t* defaultValue)
{
	if (GeneralUtils::IsValidString(key))
		return StringTable::LoadStringA(key);
	else
		return defaultValue;
}
