#pragma once

#include "../Phobos.h"

class Debug
{
public:
    enum class ExitCode : int
    {
        Undefined = -1,
        SLFail = 114514

    };

    static void __cdecl Log(const char* pFormat, ...);
    static void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);
    static void FatalErrorAndExit(const char* pFormat, ...);
    static void FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...);
};
