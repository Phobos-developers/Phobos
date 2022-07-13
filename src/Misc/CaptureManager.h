#pragma once

#include <CaptureManagerClass.h>
#include <AnimTypeClass.h>
#include <RulesClass.h>
#include <FootClass.h>

class CaptureManager
{
public:
	static bool CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget);
	static bool FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bSilent = false);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget,
		bool bRemoveFirst, AnimTypeClass* pControlledAnimType = RulesClass::Instance->ControlledAnimationType);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTechno,
		AnimTypeClass* pControlledAnimType = RulesClass::Instance->ControlledAnimationType);
	static void DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot);
};
