#pragma once

#include <AnimTypeClass.h>
#include <FootClass.h>
#include <CaptureManagerClass.h>

class CaptureManagerExt
{
public:
	static bool CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget);
	static bool FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool silent = false);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bRemoveFirst,
		AnimTypeClass* pControlledAnimType = RulesClass::Instance->ControlledAnimationType, bool silent = false, int threatDelay = 0);
	static bool CaptureUnit(CaptureManagerClass* pManager, AbstractClass* pTechno,
		AnimTypeClass* pControlledAnimType = RulesClass::Instance->ControlledAnimationType, int threatDelay = 0);
	static void DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot);
};
