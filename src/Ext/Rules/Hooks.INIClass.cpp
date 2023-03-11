#include <Utilities/Macro.h>
#include <LocomotionClass.h>
#include "Body.h"

// Reenable obsolete [JumpjetControls] in RA2/YR
// Author: Uranusian
DEFINE_HOOK(0x7115AE, TechnoTypeClass_CTOR_JumpjetControls, 0xA)
{
	GET(TechnoTypeClass*, pThis, ESI);
	auto pRules = RulesClass::Instance();
	auto pRulesExt = RulesExt::Global();

	pThis->JumpjetTurnRate = pRules->TurnRate;
	pThis->JumpjetSpeed = pRules->Speed;
	pThis->JumpjetClimb = static_cast<float>(pRules->Climb);
	pThis->JumpjetCrash = static_cast<float>(pRulesExt->JumpjetCrash);
	pThis->JumpjetHeight = pRules->CruiseHeight;
	pThis->JumpjetAccel = static_cast<float>(pRules->Acceleration);
	pThis->JumpjetWobbles = static_cast<float>(pRules->WobblesPerSecond);
	pThis->JumpjetNoWobbles = pRulesExt->JumpjetNoWobbles;
	pThis->JumpjetDeviation = pRules->WobbleDeviation;

	return 0x711601;
}

// skip vanilla JumpjetControls and make it earlier load
DEFINE_JUMP(LJMP, 0x668EB5, 0x668EBD); // RulesClass_Process_SkipJumpjetControls

DEFINE_HOOK(0x52D0F9, InitRules_EarlyLoadJumpjetControls, 0x6)
{
	GET(RulesClass*, pThis, ECX);
	GET(CCINIClass*, pINI, EAX);

	pThis->Read_JumpjetControls(pINI);

	return 0;
}

DEFINE_HOOK(0x6744E4, RulesClass_ReadJumpjetControls_Extra, 0x7)
{
	auto pRulesExt = RulesExt::Global();
	if (!pRulesExt)
		return 0;

	GET(CCINIClass*, pINI, EDI);
	INI_EX exINI(pINI);

	pRulesExt->JumpjetCrash.Read(exINI, GameStrings::JumpjetControls, "Crash");
	pRulesExt->JumpjetNoWobbles.Read(exINI, GameStrings::JumpjetControls, "NoWobbles");

	return 0;
}
