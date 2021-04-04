#pragma once

#include <CaptureManagerClass.h>

class CaptureManager
{
public:
    static bool FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bSilent = false);
    static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bRemoveFirst = false);
};