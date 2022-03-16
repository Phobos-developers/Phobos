#include "Body.h"

#include <InfantryClass.h>

#include <Ext/Building/Body.h>

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

DEFINE_HOOK(0x51F0AF, InfantryClass_WhatAction_Grinding, 0x0)
{
	enum { Skip = 0x51F05E, ReturnValue = 0x51F17E };

	GET(InfantryClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(Action, action, EBP);

	if (auto pBuilding = static_cast<BuildingClass*>(pTarget))
	{
		if (const auto pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
		{
			if (pBuilding->Type->Grinding && pThis->Owner->IsPlayerControl() && !pBuilding->IsBeingWarpedOut() &&
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

	if (auto pBuilding = static_cast<BuildingClass*>(pTarget))
	{
		if (const auto pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
		{
			bool canBeGrinded = BuildingExt::CanGrindTechno(pBuilding, pThis);
			R->EBP(canBeGrinded ? Action::Repair : Action::NoGRepair);
			return ReturnValue;
		}
	}

	return 0;
}

DEFINE_HOOK(0x740134, UnitClass_WhatAction_Grinding, 0x0)
{
	enum { Continue = 0x7401C1 };

	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EDI);
	GET(Action, action, EBX);

	if (auto pBuilding = static_cast<BuildingClass*>(pTarget))
	{
		if (const auto pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
		{
			if (pThis->Owner->IsPlayerControl() && !pBuilding->IsBeingWarpedOut() &&
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

DEFINE_HOOK(0x5198B3, InfantryClass_PerCellProcess_Grinding, 0x5)
{
	enum { Continue = 0x5198CE };

	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	return BuildingExt::DoGrindingExtras(pBuilding, pThis) ? Continue : 0;
}

DEFINE_HOOK(0x73A1C3, UnitClass_PerCellProcess_Grinding, 0x5)
{
	enum { Continue = 0x73A1DE};

	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pBuilding, EBX);

	return BuildingExt::DoGrindingExtras(pBuilding, pThis) ? Continue : 0;
}