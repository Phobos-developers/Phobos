#pragma once

#include "../Phobos.h"

class Debug
{
public:
    enum class ExitCode : int
    {
        SLFail = 114514

    };

    static void __cdecl Log(const char* pFormat, ...);
    static void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);
    static void FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...);
};
