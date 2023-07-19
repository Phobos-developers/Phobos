#include "Body.h"

#include <EventClass.h>
#include <WarheadTypeClass.h>
#include <TacticalClass.h>

#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>


DEFINE_HOOK(0x707CB3, TechnoClass_KillCargo_HandleAttachments, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0x4, 0x4));

	TechnoExt::DestroyAttachments(pThis, pSource);

	return 0;
}

DEFINE_HOOK(0x5F6609, ObjectClass_RemoveThis_TechnoClass_NotifyParent, 0x9)
{
	GET(TechnoClass*, pThis, ESI);

	pThis->KillPassengers(nullptr);  // restored code
	TechnoExt::HandleDestructionAsChild(pThis);

	return 0x5F6612;
}

DEFINE_HOOK(0x4DEBB4, FootClass_OnDestroyed_NotifyParent, 0x8)
{
	GET(FootClass*, pThis, ESI);

	TechnoExt::HandleDestructionAsChild(pThis);

	return 0;
}


DEFINE_HOOK(0x6F6F20, TechnoClass_Unlimbo_UnlimboAttachments, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::UnlimboAttachments(pThis);

	return 0;
}

DEFINE_HOOK(0x6F6B1C, TechnoClass_Limbo_LimboAttachments, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::LimboAttachments(pThis);

	return 0;
}

#pragma region Cell occupation handling

// DEFINE_HOOK(0x7441B6, UnitClass_SetOccupyBit_SkipVirtual, 0x6)
// {
// 	enum { Skip = 0x7441F6, Continue = 0 };

// 	GET(UnitClass*, pThis, ECX);

// 	if (TechnoExt::DoesntOccupyCellAsChild(pThis))
// 		return Skip;

// 	return Continue;
// }

// DEFINE_HOOK(0x744216, UnitClass_ClearOccupyBit_SkipVirtual, 0x6)
// {
// 	enum { Skip = 0x74425E, Continue = 0 };

// 	GET(UnitClass*, pThis, ECX);

// 	if (TechnoExt::DoesntOccupyCellAsChild(pThis))
// 		return Skip;

// 	return Continue;
// }

// screw Ares

void __fastcall UnitClass_SetOccupyBit_SkipVirtual(UnitClass* pThis, discard_t, const CoordStruct& coords)
{
	if (!TechnoExt::DoesntOccupyCellAsChild(pThis))
		pThis->UnitClass::MarkAllOccupationBits(coords);
}

void __fastcall UnitClass_ClearOccupyBit_SkipVirtual(UnitClass* pThis, discard_t, const CoordStruct& coords)
{
	if (!TechnoExt::DoesntOccupyCellAsChild(pThis))
		pThis->UnitClass::UnmarkAllOccupationBits(coords);
}

DEFINE_JUMP(VTABLE, 0x7F5D60, GET_OFFSET(UnitClass_SetOccupyBit_SkipVirtual))
DEFINE_JUMP(VTABLE, 0x7F5D64, GET_OFFSET(UnitClass_ClearOccupyBit_SkipVirtual))

// TODO ^ same for non-UnitClass, cba for now

// TODO now children block the parent from moving, should not happen

namespace TechnoAttachmentTemp
{
	// no idea what Ares or w/e else is doing with occupation flags,
	// so just to be safe assume it can be null and store it
	byte storedVehicleFlag;
}

// Game assumes cell is occupied by a vehicle by default and if this vehicle
// turns out to be self, then it un-assumes the occupancy. Because with techno
// attachment logic it's possible to have multiple vehicles on the same cell,
// we flip the logic from "passable if special case is found" to "impassable if
// non-special case is found" - Kerbiter

DEFINE_HOOK(0x73F520, UnitClass_CanEnterCell_AssumeNoVehicleByDefault, 0x0)
{
	enum { Check = 0x73F528, Skip = 0x73FA92 };

	REF_STACK(byte, occupyFlags, STACK_OFFSET(0x90, -0x7C));
	REF_STACK(bool, isVehicleFlagSet, STACK_OFFSET(0x90, -0x7B));

	GET(TechnoClass*, pOccupier, ESI);

	if (!pOccupier)  // stolen code
		return Skip;

	TechnoAttachmentTemp::storedVehicleFlag = occupyFlags & 0x20;

	occupyFlags &= ~0x20;
	isVehicleFlagSet = false;

	return Check;
}

DEFINE_HOOK(0x73F528, UnitClass_CanEnterCell_SkipChildren, 0x0)
{
	enum { IgnoreOccupier = 0x73FA87, Continue = 0x73F530 };

	GET(UnitClass*, pThis, EBX);
	GET(TechnoClass*, pOccupier, ESI);

	REF_STACK(byte, occupyFlags, STACK_OFFSET(0x90, -0x7C));
	REF_STACK(bool, isVehicleFlagSet, STACK_OFFSET(0x90, -0x7B));

	if (pThis == pOccupier
		|| TechnoExt::DoesntOccupyCellAsChild(pOccupier)
		|| TechnoExt::IsChildOf(pOccupier, pThis))
	{
		return IgnoreOccupier;
	}

	if (abstract_cast<UnitClass*>(pOccupier))
	{
		occupyFlags |= TechnoAttachmentTemp::storedVehicleFlag;
		isVehicleFlagSet = TechnoAttachmentTemp::storedVehicleFlag != 0;
	}

	return Continue;
}

DEFINE_HOOK(0x51C249, InfantryClass_CanEnterCell_AssumeNoVehicleByDefault, 0x0)
{
	enum { Check = 0x51C251, Skip = 0x51C78F };

	REF_STACK(byte, occupyFlags, STACK_OFFSET(0x34, -0x21));
	REF_STACK(bool, isVehicleFlagSet, STACK_OFFSET(0x34, -0x22));

	GET(TechnoClass*, pOccupier, ESI);

	if (!pOccupier)  // stolen code
		return Skip;

	TechnoAttachmentTemp::storedVehicleFlag = occupyFlags & 0x20;

	occupyFlags &= ~0x20;
	isVehicleFlagSet = false;

	return Check;
}

DEFINE_HOOK(0x51C251, InfantryClass_CanEnterCell_SkipChildren, 0x0)
{
	enum { IgnoreOccupier = 0x51C70F, Continue = 0x51C259 };

	GET(InfantryClass*, pThis, EBP);
	GET(TechnoClass*, pOccupier, ESI);

	REF_STACK(byte, occupyFlags, STACK_OFFSET(0x34, -0x21));
	REF_STACK(bool, isVehicleFlagSet, STACK_OFFSET(0x34, -0x22));

	if ((TechnoClass*)pThis == pOccupier
		|| TechnoExt::DoesntOccupyCellAsChild(pOccupier)
		|| TechnoExt::IsChildOf(pOccupier, (TechnoClass*)pThis))
	{
		return IgnoreOccupier;
	}

	if (abstract_cast<UnitClass*>(pOccupier))
	{
		occupyFlags |= TechnoAttachmentTemp::storedVehicleFlag;
		isVehicleFlagSet = TechnoAttachmentTemp::storedVehicleFlag != 0;
	}

	return Continue;
}

enum CellTechnoMode
{
	NoAttachments,
	NoVirtualOrRelatives,
	NoVirtual,
	NoRelatives,
	All,

	DefaultBehavior = All,
};

namespace TechnoAttachmentTemp
{
	CellTechnoMode currentMode = DefaultBehavior;
}

#define DEFINE_CELLTECHNO_WRAPPER(mode) \
TechnoClass* __fastcall CellTechno_##mode(CellClass* pThis, discard_t, Point2D *a2, bool check_alt, TechnoClass* techno) \
{ \
	TechnoAttachmentTemp::currentMode = CellTechnoMode::mode; \
	auto const retval = pThis->FindTechnoNearestTo(*a2, check_alt, techno); \
	TechnoAttachmentTemp::currentMode = CellTechnoMode::DefaultBehavior; \
	return retval; \
}

DEFINE_CELLTECHNO_WRAPPER(NoAttachments);
DEFINE_CELLTECHNO_WRAPPER(NoVirtualOrRelatives);
DEFINE_CELLTECHNO_WRAPPER(NoVirtual);
DEFINE_CELLTECHNO_WRAPPER(NoRelatives);
DEFINE_CELLTECHNO_WRAPPER(All);

#undef DEFINE_CELLTECHNO_WRAPPER

DEFINE_HOOK(0x47C432, CellClass_CellTechno_HandleAttachments, 0x0)
{
	enum { Continue = 0x47C437, IgnoreOccupier = 0x47C4A7 };

	GET(TechnoClass*, pOccupier, ESI);
	GET_BASE(TechnoClass*, pSelf, 0x10);

	using namespace TechnoAttachmentTemp;
	const bool noAttachments =
		currentMode == CellTechnoMode::NoAttachments;
	const bool noVirtual =
		currentMode == CellTechnoMode::NoVirtual ||
		currentMode == CellTechnoMode::NoVirtualOrRelatives;
	const bool noRelatives =
		currentMode == CellTechnoMode::NoRelatives ||
		currentMode == CellTechnoMode::NoVirtualOrRelatives;

	if (pOccupier == pSelf  // restored code
		|| noAttachments && TechnoExt::IsAttached(pOccupier)
		|| noVirtual && TechnoExt::DoesntOccupyCellAsChild(pOccupier)
		|| noRelatives && TechnoExt::IsChildOf(pOccupier, (TechnoClass*)pSelf))
	{
		return IgnoreOccupier;
	}

	return Continue;
}

// skip building placement occupation checks for virtuals
DEFINE_JUMP(CALL, 0x47C805, GET_OFFSET(CellTechno_NoVirtual));
DEFINE_JUMP(CALL, 0x47C738, GET_OFFSET(CellTechno_NoVirtual));

// skip building attachments in bib check
DEFINE_JUMP(CALL, 0x4495F2, GET_OFFSET(CellTechno_NoVirtualOrRelatives));
DEFINE_JUMP(CALL, 0x44964E, GET_OFFSET(CellTechno_NoVirtualOrRelatives));

DEFINE_HOOK(0x4495F7, BuildingClass_ClearFactoryBib_SkipCreatedUnitAttachments, 0x0)
{
	enum { BibClear = 0x44969B, NotClear = 0x4495FF };

	GET(TechnoClass*, pBibTechno, EAX);

	if (!pBibTechno)
		return BibClear;

	GET(BuildingClass*, pThis, ESI);

	TechnoClass* pBuiltTechno = pThis->GetNthLink(0);
	if (TechnoExt::IsChildOf(pBibTechno, pBuiltTechno))
		return BibClear;

	return NotClear;
}

#pragma endregion

#pragma region InAir/OnGround

bool __fastcall TechnoClass_OnGround(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritHeightStatus
		? pExt->ParentAttachment->Parent->IsOnFloor()
		: pThis->ObjectClass::IsOnFloor();
}

bool __fastcall TechnoClass_InAir(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritHeightStatus
		? pExt->ParentAttachment->Parent->IsInAir()
		: pThis->ObjectClass::IsInAir();
}

bool __fastcall TechnoClass_IsSurfaced(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritHeightStatus
		? pExt->ParentAttachment->Parent->IsSurfaced()
		: pThis->ObjectClass::IsSurfaced();
}

// TechnoClass
DEFINE_JUMP(VTABLE, 0x7F49B0, GET_OFFSET(TechnoClass_OnGround));
DEFINE_JUMP(VTABLE, 0x7F49B4, GET_OFFSET(TechnoClass_InAir));
DEFINE_JUMP(VTABLE, 0x7F49DC, GET_OFFSET(TechnoClass_IsSurfaced));

// BuildingClass
DEFINE_JUMP(VTABLE, 0x7E3F0C, GET_OFFSET(TechnoClass_OnGround));
DEFINE_JUMP(VTABLE, 0x7E3F10, GET_OFFSET(TechnoClass_InAir));
DEFINE_JUMP(VTABLE, 0x7E3F38, GET_OFFSET(TechnoClass_IsSurfaced));

// FootClass
DEFINE_JUMP(VTABLE, 0x7E8CE4, GET_OFFSET(TechnoClass_OnGround));
DEFINE_JUMP(VTABLE, 0x7E8CE8, GET_OFFSET(TechnoClass_InAir));
DEFINE_JUMP(VTABLE, 0x7E8D10, GET_OFFSET(TechnoClass_IsSurfaced));

// UnitClass
DEFINE_JUMP(VTABLE, 0x7F5CC0, GET_OFFSET(TechnoClass_OnGround));
DEFINE_JUMP(VTABLE, 0x7F5CC4, GET_OFFSET(TechnoClass_InAir));
DEFINE_JUMP(VTABLE, 0x7F5CEC, GET_OFFSET(TechnoClass_IsSurfaced));

// InfantryClass
DEFINE_JUMP(VTABLE, 0x7EB0A8, GET_OFFSET(TechnoClass_OnGround));
DEFINE_JUMP(VTABLE, 0x7EB0AC, GET_OFFSET(TechnoClass_InAir));
DEFINE_JUMP(VTABLE, 0x7EB0D4, GET_OFFSET(TechnoClass_IsSurfaced));

// AircraftClass has it's own logic, who would want to attach aircrafts anyways

#pragma endregion

DEFINE_HOOK(0x6CC763, SuperClass_Place_ChronoWarp_SkipChildren, 0x6)
{
	enum { Skip = 0x6CCCCA, Continue = 0 };

	GET(FootClass* const, pFoot, ESI);
	auto const pExt = TechnoExt::ExtMap.Find(pFoot);

	return pExt->ParentAttachment ? Skip : Continue;
}

void ParentClickedWaypoint(TechnoClass* pThis, int idxPath, signed char idxWP)
{
	// Rewrite of the original code
	pThis->AssignPlanningPath(idxPath, idxWP);

	if ((pThis->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::Foot)
		pThis->unknown_bool_430 = false;

	// Children handling
	if (auto const& pExt = TechnoExt::ExtMap.Find(pThis))
	{
		for (auto const& pAttachment : pExt->ChildAttachments)
		{
			if (pAttachment->Child && pAttachment->GetType()->InheritCommands)
				ParentClickedWaypoint(pAttachment->Child, idxPath, idxWP);
		}
	}
}

void ParentClickedAction(TechnoClass* pThis, ObjectClass* pTarget, CellStruct* pCell, CellStruct* pSecondCell)
{
	// Rewrite of the original code
	if (pTarget)
	{
		Action whatAction = pThis->MouseOverObject(pTarget, false);
		pThis->ObjectClickedAction(whatAction, pTarget, false);
	}
	else
	{
		Action whatAction = pThis->MouseOverCell(pCell, false, false);
		pThis->CellClickedAction(whatAction, pCell, pSecondCell, false);
	}

	Unsorted::MoveFeedback = false;

	// Children handling
	if (auto const& pExt = TechnoExt::ExtMap.Find(pThis))
	{
		for (auto const& pAttachment : pExt->ChildAttachments)
		{
			if (pAttachment->Child && pAttachment->GetType()->InheritCommands)
				ParentClickedAction(pAttachment->Child, pTarget, pCell, pSecondCell);
		}
	}
}

DEFINE_HOOK(0x4AE7B3, DisplayClass_ActiveClickWith_Iterate, 0x0)
{
	REF_STACK(int, idxPath, STACK_OFFSET(0x18, -0x8));
	REF_STACK(unsigned char, idxWP, STACK_OFFSET(0x18, -0xC));

	for (auto const& pObject : ObjectClass::CurrentObjects.get())
	{
		if (auto pTechno = abstract_cast<TechnoClass*>(pObject))
			ParentClickedWaypoint(pTechno, idxPath, idxWP);
	}

	GET_STACK(ObjectClass* const, pTarget, STACK_OFFSET(0x18, +0x4));
	LEA_STACK(CellStruct* const, pCell, STACK_OFFSET(0x18, +0x8));
	GET_STACK(Action const, action, STACK_OFFSET(0x18, +0xC));

	CellStruct invalidCell { -1, -1 };
	CellStruct* pSecondCell = &invalidCell;

	if (action == Action::Move || action == Action::PatrolWaypoint || action == Action::NoMove)
		pSecondCell = pCell;

	for (auto const& pObject : ObjectClass::CurrentObjects.get())
	{
		if (auto pTechno = abstract_cast<TechnoClass*>(pObject))
			ParentClickedAction(pTechno, pTarget, pCell, pSecondCell);
	}

	Unsorted::MoveFeedback = true;

	return 0x4AE99B;
}

namespace TechnoAttachmentTemp
{
	bool stopPressed = false;
}

DEFINE_HOOK(0x730EA0, StopCommand_Context_Set, 0x5)
{
	TechnoAttachmentTemp::stopPressed = true;
	return 0;
}

namespace TechnoAttachmentTemp
{
	TechnoClass* pParent = nullptr;
}

DEFINE_HOOK(0x6FFE00, TechnoClass_ClickedEvent_Context_Set, 0x5)
{
	TechnoAttachmentTemp::pParent = R->ECX<TechnoClass*>();
	return 0;
}

DEFINE_HOOK_AGAIN(0x6FFEB1, TechnoClass_ClickedEvent_HandleChildren, 0x6)
DEFINE_HOOK(0x6FFE4F, TechnoClass_ClickedEvent_HandleChildren, 0x6)
{
	if (TechnoAttachmentTemp::stopPressed && TechnoAttachmentTemp::pParent)
	{
		if (auto const& pExt = TechnoExt::ExtMap.Find(TechnoAttachmentTemp::pParent))
		{
			for (auto const& pAttachment : pExt->ChildAttachments)
			{
				if (pAttachment->Child && pAttachment->GetType()->InheritCommands)
					pAttachment->Child->ClickedEvent(NetworkEvents::Idle);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x730F1C, StopCommand_Context_Unset, 0x5)
{
	TechnoAttachmentTemp::stopPressed = false;
	return 0;
}

/*
namespace TechnoAttachmentTemp
{
	TechnoClass* pParent;
}

DEFINE_HOOK(0x6FFBE0, TechnoClass_PlayerAssignMission_Context_Set, 0x6)
{
	TechnoAttachmentTemp::pParent = R->ECX<TechnoClass*>();

	return 0;
}

DEFINE_HOOK_AGAIN(0x6FFDEB, TechnoClass_PlayerAssignMission_HandleChildren, 0x5)
DEFINE_HOOK(0x6FFCAE, TechnoClass_PlayerAssignMission_HandleChildren, 0x5)
{
	GET_STACK(Mission, mission, STACK_OFFSET(0x98, +0x4));

	switch (mission)
	{
	case Mission::Move:
	case Mission::AttackMove:
	case Mission::Enter:
	case Mission::QMove:
		return 0;
	}

	GET_STACK(ObjectClass* const, pTarget, STACK_OFFSET(0x98, +0x8));
	GET_STACK(CellClass* const, pTargetCell, STACK_OFFSET(0x98, +0xC));
	GET_STACK(CellClass* const, pCellNearTarget, STACK_OFFSET(0x98, +0x10));
	auto const& pExt = TechnoExt::ExtMap.Find(TechnoAttachmentTemp::pParent);

	bool oldFeedback = Unsorted::MoveFeedback;
	Unsorted::MoveFeedback = false;

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		// Recursive call, PlayerAssignMission == ClickedMission
		if (pAttachment->Child && pAttachment->GetType()->InheritCommands)
			pAttachment->Child->ClickedMission(mission, pTarget, pTargetCell, pCellNearTarget);
	}

	Unsorted::MoveFeedback = oldFeedback;

	// PlayerAssignMission returns bool which indicates whether the player is in planning mode.
	// This can't change when we handle children so we don't adjust the return value - Kerbiter
	return 0;
}
*/

// DEFINE_HOOK(0x6CCCCA, SuperClass_Place_ChronoWarp_HandleAttachment, 0x0)
// {
// 	enum { Loop = 0x6CC742, Break = 0x6CCCD5 };
//
// 	GET(FootClass*, pFoot, ESI)
//
// 	pFoot = abstract_cast<FootClass*>(pFoot->NextObject);
//
// 	return pFoot ? Loop : Break;
// }

// TODO
// 0x4DEAE0 IC for footclass
// 0x457C90 IC (forceshield) for buildings
// 0x6CCCCA Chrono Warp
// 0x4694BB Temporal warhead
// ...

DEFINE_HOOK(0x469672, BulletClass_Logics_Locomotor_CheckIfAttached, 0x6)
{
	enum { SkipInfliction = 0x469AA4, ContinueCheck = 0x0 };

	GET(FootClass*, pThis, EDI);
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment
		? SkipInfliction
		: ContinueCheck;
}

DEFINE_HOOK(0x6FC3F4, TechnoClass_CanFire_HandleAttachmentLogics, 0x6)
{
	enum { ReturnFireErrorIllegal = 0x6FC86A, ContinueCheck = 0x0 };

	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EBP);
	GET(WeaponTypeClass*, pWeapon, EDI);

	//auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	//auto const& pTargetExt = TechnoExt::ExtMap.Find(pTarget);

	bool illegalParentTargetWarhead = pWeapon->Warhead
		&& pWeapon->Warhead->IsLocomotor;

	if (illegalParentTargetWarhead && TechnoExt::IsChildOf(pThis, pTarget))
		return ReturnFireErrorIllegal;

	return ContinueCheck;
}

// TODO WhatWeaponShouldIUse

DEFINE_HOOK(0x6F3283, TechnoClass_CanScatter_CheckIfAttached, 0x8)
{
	enum { ReturnFalse = 0x6F32C5, ContinueCheck = 0x0 };

	GET(TechnoClass*, pThis, ECX);
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment
		? ReturnFalse
		: ContinueCheck;
}

// TODO maybe? handle scatter for InfantryClass and possibly AircraftClass

DEFINE_HOOK(0x736FB6, UnitClass_FiringAI_ForbidAttachmentRotation, 0x6)
{
	enum { SkipBodyRotation = 0x737063, ContinueCheck = 0x0 };

	GET(UnitClass*, pThis, ESI);
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment
		? SkipBodyRotation
		: ContinueCheck;
}

DEFINE_HOOK(0x736A2F, UnitClass_RotationAI_ForbidAttachmentRotation, 0x7)
{
	enum { SkipBodyRotation = 0x736A8E, ContinueCheck = 0x0 };

	GET(UnitClass*, pThis, ESI);
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment
		? SkipBodyRotation
		: ContinueCheck;
}
/*
Action __fastcall UnitClass_MouseOverCell_Wrapper(UnitClass* pThis, discard_t, CellStruct const* pCell, bool checkFog, bool ignoreForce)
{
	Action result = pThis->UnitClass::MouseOverCell(pCell, checkFog, ignoreForce);

	auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt->ParentAttachment)
		return result;

	switch (result)
	{
		case Action::GuardArea:
		case Action::AttackMoveNav:
		case Action::PatrolWaypoint:
		case Action::Harvest:
		case Action::Move:
			result = Action::NoMove;
			break;
		case Action::EnterTunnel:
			result = Action::NoEnterTunnel;
			break;
	}

	return result;
}

// TODO MouseOverObject for entering bunkers, grinder, buildings etc

DEFINE_JUMP(VTABLE, 0x7F5CE0, GET_OFFSET(UnitClass_MouseOverCell_Wrapper))
*/
/*
DEFINE_HOOK(0x4D74EC, FootClass_ObjectClickedAction_HandleAttachment, 0x6)
{
	enum { ReturnFalse = 0x4D77EC, Continue = 0x0 };

	GET(FootClass*, pThis, ESI);
	GET_STACK(Action, action, STACK_OFFSET(0x108, +0x4));
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->ParentAttachment)
		return Continue;

	switch (action)
	{
		case Action::Move:
		case Action::AttackMoveNav:
		case Action::NoMove:
		case Action::Enter:
		case Action::NoEnter:
		case Action::Capture:
		case Action::Repair:
		case Action::Sabotage:
		case Action::GuardArea:
			return ReturnFalse;
		default:
			return Continue;
	}
}

DEFINE_HOOK(0x4D7D58, FootClass_CellClickedAction_HandleAttachment, 0x6)
{
	enum { ReturnFalse = 0x4D7D62, Continue = 0x0 };

	GET(FootClass*, pThis, ESI);
	GET_STACK(Action, action, STACK_OFFSET(0x24, +0x4));
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->ParentAttachment)
		return Continue;

	switch (action)
	{
		case Action::Move:
		case Action::AttackMoveNav:
		case Action::NoMove:
		case Action::Enter:
		case Action::NoEnter:
		case Action::Harvest:
		case Action::Capture:
		case Action::Sabotage:
		case Action::GuardArea:
		case Action::EnterTunnel:
		case Action::NoEnterTunnel:
		case Action::PatrolWaypoint:
			return ReturnFalse;
		default:
			return Continue;
	}
}
*/

// DEFINE_HOOK(0x41BE02, AbstractClass_RenderCoord_AttachedCoord, 0x7)
// {
// 	enum { Return = 0x41BE2A, Continue = 0x0 };

// 	GET(AbstractClass* const, pThis, ECX);

// 	if (auto const& pThisAsTechno = abstract_cast<TechnoClass*>(pThis))
// 	{
// 		auto const& pExt = TechnoExt::ExtMap.Find(pThisAsTechno);
// 		if (pExt && pExt->ParentAttachment)
// 		{
// 			R->EAX<CoordStruct*>(&pExt->ParentAttachment->GetChildLocation());
// 			return Return;
// 		}
// 	}

// 	return Continue;
// }

CoordStruct __fastcall TechnoClass_GetRenderCoords(TechnoClass* pThis)
{
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt && pExt->ParentAttachment)
	{
		// The parent origin is our origin, we will offset later in draw function
		return pExt->ParentAttachment->Cache.TopLevelParent->GetRenderCoords();
	}

	return pThis->ObjectClass::GetRenderCoords();
}

// TODO hook matrix
// 6F3B88 DONE
// 6F3DA4 DONE
// 73B5B5 DONE
// 73C864 DONE

// DEFINE_HOOK(0x6F3B88, TechnoClass_FireCoord_AttachmentAdjust, 0x6)
// {
// 	enum { Skip = 0x6F3B9E };

// 	GET(TechnoClass*, pThis, EBX);

// 	R->EAX(&TechnoExt::GetAttachmentTransform(pThis));
// 	return Skip;
// }

// DEFINE_HOOK(0x6F3DA4, TechnoClass_firecoord_6F3D60_AttachmentAdjust, 0x6)
// {
// 	enum { Skip = 0x6F3DBA };

// 	GET(TechnoClass*, pThis, EBX);

// 	R->EAX(&TechnoExt::GetAttachmentTransform(pThis));
// 	return Skip;
// }

// WIP jitterless drawing. requires a lot of work so wasn't finished
/*
DEFINE_HOOK(0x73B5B5, UnitClass_DrawVoxel_AttachmentAdjust, 0x6)
{
	enum { Skip = 0x73B5CE };

	GET(UnitClass*, pThis, EBP);
	LEA_STACK(VoxelIndexKey*, pKey, STACK_OFFSET(0x1C4, -0x1B0));

	Matrix3D transform = TechnoExt::GetAttachmentTransform(pThis, pKey);
	R->EAX(&transform);

	return Skip;
}

DEFINE_HOOK(0x73C864, UnitClass_drawcode_AttachmentAdjust, 0x6)
{
	enum { Skip = 0x73C87D };

	GET(UnitClass*, pThis, EBP);
	LEA_STACK(VoxelIndexKey*, pKey, STACK_OFFSET(0x128, -0xC8));

	Matrix3D transform = TechnoExt::GetAttachmentTransform(pThis, pKey);
	R->EAX(&transform);

	return Skip;
}

DEFINE_HOOK(0x73C29F, UnitClass_DrawVoxel_AttachmentAdjustShadow, 0x6)
{
	enum { Skip = 0x73C2B8 };

	GET(UnitClass*, pThis, EBP);
	LEA_STACK(VoxelIndexKey*, pKey, STACK_OFFSET(0x1F0, -0x1DC));

	Matrix3D transform = TechnoExt::GetAttachmentTransform(pThis, pKey, true);
	R->EAX(&transform);

	return Skip;
}

DEFINE_HOOK(0x414953, AircraftClass_DrawIt_AttachmentAdjust, 0x6)
{
	enum { Skip = 0x41496C };

	GET(AircraftClass*, pThis, EBP);
	LEA_STACK(VoxelIndexKey*, pKey, STACK_OFFSET(0xD8, -0xC8));

	Matrix3D transform = TechnoExt::GetAttachmentTransform((TechnoClass*)pThis, pKey);
	R->EAX(&transform);

	return Skip;
}

DEFINE_HOOK(0x41480D, AircraftClass_DrawIt_AttachmentAdjustShadow, 0x6)
{
	enum { Skip = 0x414826 };

	GET(AircraftClass*, pThis, EBP);
	LEA_STACK(VoxelIndexKey*, pKey, STACK_OFFSET(0xD4, -0xC4));

	Matrix3D transform = TechnoExt::GetAttachmentTransform((TechnoClass*)pThis, pKey, true);
	R->EAX(&transform);

	return Skip;
}

// TODO merge hooks

// TODO aircraft
// normal matrix 0x414969
// shadow matrix 0x414823

// TODO hook shadow matrix

// BuildingClass::GetRenderCoords already has it's own override,
// should hook it if you ever want to dip into that - Kerbiter

// FIXME it's probably not a good idea to mix vtable replacements with manual hooks
// DEFINE_JUMP(VTABLE, 0x7F4A0C, GET_OFFSET(TechnoClass_GetRenderCoords)) // TechnoClass
// DEFINE_JUMP(VTABLE, 0x7E8D40, GET_OFFSET(TechnoClass_GetRenderCoords)) // FootClass
// DEFINE_JUMP(VTABLE, 0x7F5D1C, GET_OFFSET(TechnoClass_GetRenderCoords)) // UnitClass
// DEFINE_JUMP(VTABLE, 0x7EB104, GET_OFFSET(TechnoClass_GetRenderCoords)) // InfantryClass
// DEFINE_JUMP(VTABLE, 0x7E2350, GET_OFFSET(TechnoClass_GetRenderCoords)) // AircraftClass


// // DEFINE_JUMP(VTABLE, 0x7F4A74, GET_OFFSET(TechnoClass_DrawIt))

// // skip children before parent drawn
// DEFINE_HOOK(0x73CEC0, UnitClass_DrawIt_HandleChildren , 0x5)
// {
// 	GET(UnitClass*, pThis, ECX);

// 	if (TechnoExt::IsAttached(pThis))
// }

namespace TechnoAttachmentTemp
{
	ObjectClass* pThis;
	Point2D realDrawPoint = Point2D::Empty;
}

DEFINE_HOOK(0x5F4B13, ObjectClass_Render_Context_Set, 0x5)
{
	GET(ObjectClass*, pThis, ECX);

	TechnoAttachmentTemp::pThis = pThis;

	return 0;
}

// Swap the render coord after the draw rect is calculated
DEFINE_HOOK(0x5F4CC3, ObjectClass_Render_AttachmentAdjust, 0x6)
{
	LEA_STACK(Point2D*, drawPoint, STACK_OFFSET(0x30, -0x18));
	TechnoAttachmentTemp::realDrawPoint = *drawPoint;

	ObjectClass* pThis = TechnoAttachmentTemp::pThis;
	if (pThis->AbstractFlags & AbstractFlags::Techno)
		TacticalClass::Instance->CoordsToClient(TechnoExt::GetTopLevelParent((TechnoClass*)pThis)->GetRenderCoords(), drawPoint);

	return 0;
}

DEFINE_HOOK(0x6D9EF0, TacticalClass_AddSelectable_SwapCoord, 0x6)
{
	REF_STACK(TechnoClass*, pTechno, STACK_OFFSET(0x0, 0x4));
	REF_STACK(int, x, STACK_OFFSET(0x0, 0x8));
	REF_STACK(int, y, STACK_OFFSET(0x0, 0xC));

	if (pTechno == TechnoAttachmentTemp::pThis)
	{
		x = TechnoAttachmentTemp::realDrawPoint.X;
		y = TechnoAttachmentTemp::realDrawPoint.Y;
	}

	return 0;
}
*/

// offsets the voxel visual, needs additional handling
// DEFINE_HOOK(0x73B140, UnitClass_DrawObject_SwapCoord, 0x5)
// {
// 	GET(UnitClass*, pThis, ECX);
// 	REF_STACK(int, x, STACK_OFFSET(0x0, 0x8));
// 	REF_STACK(int, y, STACK_OFFSET(0x0, 0xC));

// 	if (pThis == TechnoAttachmentTemp::pThis)
// 	{
// 		x = TechnoAttachmentTemp::realDrawPoint.X;
// 		y = TechnoAttachmentTemp::realDrawPoint.Y;
// 	}

// 	return 0;
// }

// YSort for attachments
int __fastcall TechnoClass_SortY_Wrapper(ObjectClass* pThis)
{
	auto const pTechno = abstract_cast<TechnoClass*>(pThis);

	if (pTechno)
	{
		const auto pExt = TechnoExt::ExtMap.Find(pTechno);

		if (pExt->ParentAttachment)
		{
			const auto ySortPosition = pExt->ParentAttachment->GetType()->YSortPosition.Get();
			const auto pParentTechno = pExt->ParentAttachment->Parent;

			if (ySortPosition != AttachmentYSortPosition::Default && pParentTechno)
			{
				int parentYSort = pParentTechno->GetYSort();

				return parentYSort + (ySortPosition == AttachmentYSortPosition::OverParent ? 1 : -1);
			}
		}
	}

	return pThis->ObjectClass::GetYSort();
}

DEFINE_JUMP(CALL, 0x449413, GET_OFFSET(TechnoClass_SortY_Wrapper))   // BuildingClass
DEFINE_JUMP(VTABLE, 0x7E235C, GET_OFFSET(TechnoClass_SortY_Wrapper)) // AircraftClass
DEFINE_JUMP(VTABLE, 0x7EB110, GET_OFFSET(TechnoClass_SortY_Wrapper)) // InfantryClass
DEFINE_JUMP(VTABLE, 0x7F5D28, GET_OFFSET(TechnoClass_SortY_Wrapper)) // UnitClass

DEFINE_JUMP(LJMP, 0x568831, 0x568841); // Skip locomotion layer check in MapClass::PickUp
DEFINE_JUMP(LJMP, 0x4D37A2, 0x4D37AE); // Skip locomotion layer check in FootClass::Mark
/*
// Handle waypoint mode command inheritance
DEFINE_HOOK(0x4C73D1, EventClass_Execute_MegaMission_HandleChildren, 0x6)
{
	GET(EventClass*, pThis, ESI);
	GET(TechnoClass*, pParent, EDI);
	GET(AbstractClass*, pArchiveTarget, EBP);
	GET(Mission, mission, EBX);

	auto const pExt = TechnoExt::ExtMap.Find(pParent);
	auto const pTarget = pThis->Data.MegaMission.Target.As_Abstract();
	auto const pDestination = pThis->Data.MegaMission.Destination.As_Abstract();
	bool isGuard = pThis->Data.MegaMission.Mission == static_cast<char>(Mission::Area_Guard) && pParent->AbstractFlags & AbstractFlags::Foot;

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (!pAttachment->Child || !pAttachment->GetType()->InheritCommands)
			continue;

		auto const pChild = pAttachment->Child;
		pChild->QueueMission(mission, false);

		if (isGuard)
		{
			pChild->SetTarget(pChild);
			pChild->SetDestination(pTarget, true);
			pChild->SetFocus(pTarget);
		}
		else
		{
			if (pArchiveTarget)
			{
				pChild->SetFocus(pArchiveTarget);

				if (auto const pFoot = abstract_cast<FootClass*>(pChild))
					pFoot->ClearNavQueue();
			}

			pChild->SetTarget(pTarget);
			pChild->SetDestination(pDestination, true);

			if (pTarget && pChild->GetTechnoType()->OpenTopped)
				pChild->SetTargetForPassengers(pTarget);
		}
	}

	return 0;
}
*/

// TODO review following 2 hooks from Kratos

DEFINE_HOOK(0x69251A, ScrollClass_ProcessClickCoords_TransparentToMouse, 0x6)
{
	GET(TechnoClass*, pTechno, EAX);

	if (!pTechno)
		return 0x0;

	auto const pExt = TechnoExt::ExtMap.Find(pTechno);
	if (pExt && pExt->ParentAttachment && pExt->ParentAttachment->GetType()->TransparentToMouse)
		R->EAX<TechnoClass*>(nullptr);

	return 0x0;
}

DEFINE_HOOK(0x6DA3FF, TacticalClass_SelectAt_TransparentToMouse, 0x6)
{
	enum { SkipTechno = 0x6DA440, ContinueCheck = 0x0 };

	GET(TechnoClass*, pTechno, EAX);

	auto const pExt = TechnoExt::ExtMap.Find(pTechno);
	if (pExt && pExt->ParentAttachment && pExt->ParentAttachment->GetType()->TransparentToMouse)
		return SkipTechno;

	return ContinueCheck;
}
