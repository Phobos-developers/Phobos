#pragma once

#include "../Phobos.h"

class Debug
{
public:
    static void __cdecl Log(const char* pFormat, ...);
    static void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);
};
