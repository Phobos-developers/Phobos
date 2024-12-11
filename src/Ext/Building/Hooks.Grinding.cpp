#include "Body.h"

#include <InfantryClass.h>
#include <InputManagerClass.h>

DEFINE_HOOK(0x43C30A, BuildingClass_ReceiveMessage_Grinding, 0x6)
{
	enum { ReturnStatic = 0x43C31A, ReturnNegative = 0x43CB68, ReturnRoger = 0x43CCF2 };

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pFrom, EDI);

	if (pThis->Type->Grinding)
	{
		if (!pThis->Owner->IsAlliedWith(pFrom))
			return ReturnStatic;

		if (pThis->GetCurrentMission() == Mission::Construction || pThis->GetCurrentMission() == Mission::Selling ||
			pThis->BState == 0 || !pThis->HasPower || pFrom->GetTechnoType()->BalloonHover)
		{
			return ReturnNegative;
		}

		bool isAmphibious = pFrom->GetTechnoType()->MovementZone == MovementZone::Amphibious || pFrom->GetTechnoType()->MovementZone == MovementZone::AmphibiousCrusher ||
			pFrom->GetTechnoType()->MovementZone == MovementZone::AmphibiousDestroyer;

		if (!isAmphibious && (pThis->GetTechnoType()->Naval && !pFrom->GetTechnoType()->Naval ||
			!pThis->GetTechnoType()->Naval && pFrom->GetTechnoType()->Naval))
		{
			return ReturnNegative;
		}

		return BuildingExt::CanGrindTechno(pThis, pFrom) ? ReturnRoger : ReturnNegative;
	}

	return 0;
}


DEFINE_HOOK(0x4D4CD3, FootClass_Mission_Eaten_Grinding, 0x6)
{
	enum { LoseDestination = 0x4D4D43 };

	GET(FootClass*, pThis, ESI);

	if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis->Destination))
	{
		if (pBuilding->Type->Grinding && !BuildingExt::CanGrindTechno(pBuilding, pThis))
		{
			pThis->SetDestination(nullptr, false);
			return LoseDestination;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4D4B43, FootClass_Mission_Capture_ForbidUnintended, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	enum { LosesDestination = 0x4D4BD1 };

	if (!pThis || pThis->Target)
		return 0;

	auto pBld = specific_cast<BuildingClass*>(pThis->Destination);
	if (!pBld)
		return 0;

	if (pThis->Type->Engineer)
		return 0;

	// interaction issues with Ares, no more further checking to make life easier. If someone still try to abuse the bug I won't try to stop them
	if (pThis->Type->Infiltrate && !pThis->Owner->IsAlliedWith(pBld->Owner))
		return 0;
	if (pBld->IsStrange())
		return 0;

	if (pBld->Type->CanBeOccupied && (pThis->Type->Occupier || pThis->Type->Assaulter))
		return 0;

	if (pThis->Type->C4 || pThis->HasAbility(Ability::C4))
		return 0;

	// If you can't do any of these then why are you here?
	pThis->SetDestination(nullptr, false);
	return LosesDestination;
}

DEFINE_HOOK(0x51F0AF, InfantryClass_WhatAction_Grinding, 0x0)
{
	enum { Skip = 0x51F05E, ReturnValue = 0x51F17E };

	GET(InfantryClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(Action, action, EBP);

	if (auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
	{
		if (const auto pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
		{
			if (pBuilding->Type->Grinding && pThis->Owner->IsControlledByCurrentPlayer() && !pBuilding->IsBeingWarpedOut() &&
				pThis->Owner->IsAlliedWith(pTarget) && (pExt->Grinding_AllowAllies || action == Action::Select))
			{
				action = BuildingExt::CanGrindTechno(pBuilding, pThis) ? Action::Repair : Action::NoEnter;
				R->EBP(action);
				return ReturnValue;
			}
		}
	}

	return Skip;
}

DEFINE_HOOK(0x51E63A, InfantryClass_WhatAction_Grinding_Engineer, 0x6)
{
	enum { ReturnValue = 0x51F17E };

	GET(InfantryClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);

	if (auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
	{
		bool canBeGrinded = BuildingExt::CanGrindTechno(pBuilding, pThis);
		R->EBP(canBeGrinded ? Action::Repair : Action::NoGRepair);
		return ReturnValue;
	}

	return 0;
}

DEFINE_HOOK(0x740134, UnitClass_WhatAction_Grinding, 0x0)
{
	enum { Continue = 0x7401C1 };

	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EDI);
	GET(Action, action, EBX);

	if (InputManagerClass::Instance->IsForceFireKeyPressed() && pThis->IsArmed())
		return Continue;

	if (auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
	{
		if (pThis->Owner->IsControlledByCurrentPlayer() && !pBuilding->IsBeingWarpedOut() &&
			pThis->Owner->IsAlliedWith(pTarget) && (pBuilding->Type->Grinding || action == Action::Select))
		{
			if (pThis->SendCommand(RadioCommand::QueryCanEnter, pTarget) == RadioCommand::AnswerPositive)
			{
				bool isFlying = pThis->GetTechnoType()->MovementZone == MovementZone::Fly;
				bool canBeGrinded = BuildingExt::CanGrindTechno(pBuilding, pThis);
				action = pBuilding->Type->Grinding ? canBeGrinded && !isFlying ? Action::Repair : Action::NoEnter : !isFlying ? Action::Enter : Action::NoEnter;
				R->EBX(action);
			}
			else if (pBuilding->Type->Grinding)
			{
				R->EBX(Action::NoEnter);
			}
		}
	}

	return Continue;
}

DEFINE_HOOK(0x4DFABD, FootClass_Try_Grinding_CheckIfAllowed, 0x8)
{
	enum { Skip = 0x4DFB30 };

	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	if (!BuildingExt::CanGrindTechno(pBuilding, pThis))
		return Skip;

	return 0;
}

namespace GrinderRefundTemp
{
	int BalanceBefore;
}

DEFINE_HOOK(0x519790, InfantryClass_PerCellProcess_SkipDieSoundBeforeGrinding, 0xA)
{
	enum { SkipVoiceDie = 0x51986A };
	GET(BuildingClass*, pBuilding, EBX);

	if (BuildingTypeExt::ExtMap.Find(pBuilding->Type)->Grinding_PlayDieSound.Get())
		return 0;

	return SkipVoiceDie;
}

DEFINE_HOOK(0x5198B3, InfantryClass_PerCellProcess_DoGrindingExtras, 0x5)
{
	enum { Continue = 0x5198CE };

	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	return BuildingExt::DoGrindingExtras(pBuilding, pThis, pThis->GetRefund()) ? Continue : 0;
}

DEFINE_HOOK(0x739FBC, UnitClass_PerCellProcess_BeforeGrinding, 0x5)
{
	enum { SkipDieSound = 0x73A0A5 };
	GET(BuildingClass*, pBuilding, EBX);

	GrinderRefundTemp::BalanceBefore = pBuilding->Owner->Balance;

	if (BuildingTypeExt::ExtMap.Find(pBuilding->Type)->Grinding_PlayDieSound.Get())
		return 0;

	return SkipDieSound;
}

DEFINE_HOOK(0x73A1C3, UnitClass_PerCellProcess_DoGrindingExtras, 0x5)
{
	enum { Continue = 0x73A1DE };

	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pBuilding, EBX);

	// Calculated like this because it is easier than tallying up individual refunds for passengers and parasites.
	int totalRefund = pBuilding->Owner->Balance - GrinderRefundTemp::BalanceBefore;

	return BuildingExt::DoGrindingExtras(pBuilding, pThis, totalRefund) ? Continue : 0;
}
