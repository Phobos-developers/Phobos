#include "GeneralUtils.h"
#include <string.h>
#include <StringTable.h>

bool GeneralUtils::IsValidString(const char* str)
{
    return str != nullptr
        && strlen(str) != 0
        && _stricmp(str, NONE_STR) != 0
        && _stricmp(str, NONE_STR2) != 0;
}

const wchar_t* GeneralUtils::LoadStringOrDefault(char* key, const wchar_t* defaultValue)
{
	if (GeneralUtils::IsValidString(key))
		return StringTable::LoadStringA(key);
	else
		return defaultValue;
}
