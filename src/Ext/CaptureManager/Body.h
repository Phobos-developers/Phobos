#pragma once

#include <CaptureManagerClass.h>
#include <AnimTypeClass.h>
#include <RulesClass.h>
#include <FootClass.h>
#include <HouseClass.h>
#include <AnimClass.h>

#include <Ext/TechnoType/Body.h>

class CaptureManagerExt
{
public:
	static bool CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget);
	static bool FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool silent = false);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget,
		bool bRemoveFirst, AnimTypeClass* pControlledAnimType = RulesClass::Instance->ControlledAnimationType, bool silent = false);
	static bool CaptureUnit(CaptureManagerClass* pManager, AbstractClass *pTechno,
		AnimTypeClass* pControlledAnimType = RulesClass::Instance->ControlledAnimationType);
	static void DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot);
};
