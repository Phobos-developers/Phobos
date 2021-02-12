#include "GeneralUtils.h"
#include <string.h>
#include <StringTable.h>

bool IsValidString(const char* str)
{
    return str != nullptr
        && strlen(str) != 0
        && _stricmp(str, NONE_STR) != 0
        && _stricmp(str, NONE_STR2) != 0;
}

const wchar_t* LoadStringOrDefault(char* key, const wchar_t* defaultValue)
{
	if (IsValidString(key))
		return StringTable::LoadStringA(key);
	else
		return defaultValue;
}
