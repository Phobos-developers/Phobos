#include <Utilities/Macro.h>
#include <LocomotionClass.h>
#include "Body.h"

DEFINE_HOOK(0x527AE2, INIClass__Get_UUID, 0x7)
{
	LEA_STACK(const char*, buffer, STACK_OFFSET(0x1B8, -0x180));

	if (buffer[0] != '{')
	{
		REF_STACK(CLSID, loco, STACK_OFFSET(0x1B8, -0x1A0));

#define PARSE_IF_IS_LOCO(name)\
if(_strcmpi(buffer, #name) == 0){ loco = LocomotionClass::CLSIDs::name; return 0x527B16;}

		PARSE_IF_IS_LOCO(Drive);
		PARSE_IF_IS_LOCO(Jumpjet);
		PARSE_IF_IS_LOCO(Hover);
		PARSE_IF_IS_LOCO(Rocket);
		PARSE_IF_IS_LOCO(Tunnel);
		PARSE_IF_IS_LOCO(Walk);
		PARSE_IF_IS_LOCO(Fly);
		PARSE_IF_IS_LOCO(Teleport);
		PARSE_IF_IS_LOCO(Mech);
		PARSE_IF_IS_LOCO(Ship);
		PARSE_IF_IS_LOCO(Droppod);

#undef PARSE_IF_IS_LOCO
	}

	return 0;
}

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
