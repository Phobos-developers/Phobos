#include "GeneralUtils.h"
#include <string.h>

bool IsValidString(const char* str)
{
    return str != nullptr
        && strlen(str) != 0
        && _stricmp(str, NONE_STR) != 0
        && _stricmp(str, NONE_STR) != 0;
}
