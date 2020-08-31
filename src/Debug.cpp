#include "Debug.h"

void(__cdecl *Debug::Log)(const char *pFormat, ...) = (void(__cdecl *)(const char *pFormat, ...))0x4068E0;
