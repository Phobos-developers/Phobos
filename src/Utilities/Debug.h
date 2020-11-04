#pragma once

class Debug
{
public:
    static void(__cdecl *Log)(const char *pFormat, ...);
};
