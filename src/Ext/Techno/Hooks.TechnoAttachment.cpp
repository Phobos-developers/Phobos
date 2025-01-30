#include "Body.h"

#include <EventClass.h>
#include <WarheadTypeClass.h>
#include <TacticalClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Cell/Body.h>

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

// because Ares hooks in the single usable position we need to do a detour instead
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

// TODO ^ same for non-UnitClass, not needed so cba for now

namespace TechnoAttachmentTemp
{
	// no idea what Ares or w/e else is doing with occupation flags,
	// so just to be safe assume it can be nothing and store it
	byte storedVehicleFlag;
}

// Game assumes cell is occupied by a vehicle by default and if this vehicle
// turns out to be self, then it un-assumes the occupancy. Because with techno
// attachment logic it's possible to have multiple vehicles on the same cell,
// we flip the logic from "passable if special case is found" to "impassable if
// non-special case is found" - Kerbiter

void AssumeNoVehicleByDefault(byte& occupyFlags, bool& isVehicleFlagSet)
{
	TechnoAttachmentTemp::storedVehicleFlag = occupyFlags & 0x20;

	occupyFlags &= ~0x20;
	isVehicleFlagSet = false;
}

DEFINE_HOOK(0x73F520, UnitClass_CanEnterCell_AssumeNoVehicleByDefault, 0x0)
{
	enum { Check = 0x73F528, Skip = 0x73FA92 };

	REF_STACK(byte, occupyFlags, STACK_OFFSET(0x90, -0x7C));
	REF_STACK(bool, isVehicleFlagSet, STACK_OFFSET(0x90, -0x7B));

	GET(TechnoClass*, pOccupier, ESI);

	if (!pOccupier)  // stolen code
		return Skip;

	AssumeNoVehicleByDefault(occupyFlags, isVehicleFlagSet);

	return Check;
}

bool IsOccupierIgnorable(TechnoClass* pThis, ObjectClass* pOccupier, byte& occupyFlags, bool& isVehicleFlagSet)
{
	if (pThis == pOccupier)
		return true;

	auto const pTechno = abstract_cast<TechnoClass*>(pOccupier);
	if (pTechno &&
		(TechnoExt::DoesntOccupyCellAsChild(pTechno) || TechnoExt::IsChildOf(pTechno, pThis)))
	{
		return true;
	}

	if (abstract_cast<UnitClass*>(pOccupier))
	{
		occupyFlags |= TechnoAttachmentTemp::storedVehicleFlag;
		isVehicleFlagSet = TechnoAttachmentTemp::storedVehicleFlag != 0;
	}

	return false;
}

DEFINE_HOOK(0x73F528, UnitClass_CanEnterCell_SkipChildren, 0x0)
{
	enum { SkipToNextOccupier = 0x73FA87, ContinueCheck = 0x73F530 };

	GET(UnitClass*, pThis, EBX);
	GET(ObjectClass*, pOccupier, ESI);

	REF_STACK(byte, occupyFlags, STACK_OFFSET(0x90, -0x7C));
	REF_STACK(bool, isVehicleFlagSet, STACK_OFFSET(0x90, -0x7B));

	return IsOccupierIgnorable(pThis, pOccupier, occupyFlags, isVehicleFlagSet)
		? SkipToNextOccupier : ContinueCheck;
}

void AccountForMovingInto(CellClass* into, bool isAlt, TechnoClass* pThis, byte& occupyFlags, bool& isVehicleFlagSet)
{
	auto const pCellExt = CellExt::ExtMap.Find(into);
	auto const& pIncoming = isAlt ? pCellExt->IncomingUnitAlt : pCellExt->IncomingUnit;

	// Non-occupiers shouldn't be inserted as incoming units anyways so don't check that
	if (pIncoming && pIncoming != pThis &&
		!TechnoExt::IsChildOf(pIncoming, pThis))
	{
		occupyFlags |= TechnoAttachmentTemp::storedVehicleFlag;
		isVehicleFlagSet = TechnoAttachmentTemp::storedVehicleFlag != 0;
	}
}

DEFINE_HOOK(0x73FA92, UnitClass_CanEnterCell_CheckMovingInto, 0x0)
{
	GET_STACK(CellClass*, into, STACK_OFFSET(0x90, 0x4));
	GET_STACK(bool const, isAlt, STACK_OFFSET(0x90, -0x7D));
	GET(UnitClass*, pThis, EBX);

	REF_STACK(byte, occupyFlags, STACK_OFFSET(0x90, -0x7C));
	REF_STACK(bool, isVehicleFlagSet, STACK_OFFSET(0x90, -0x7B));

	AccountForMovingInto(into, isAlt, pThis, occupyFlags, isVehicleFlagSet);

	// stolen code ahead
	if (!isAlt)
		return 0x73FA9E;

	return 0x73FC24;
}

DEFINE_HOOK(0x51C249, InfantryClass_CanEnterCell_AssumeNoVehicleByDefault, 0x0)
{
	enum { Check = 0x51C251, Skip = 0x51C78F };

	REF_STACK(byte, occupyFlags, STACK_OFFSET(0x34, -0x21));
	REF_STACK(bool, isVehicleFlagSet, STACK_OFFSET(0x34, -0x22));

	GET(TechnoClass*, pOccupier, ESI);

	if (!pOccupier)  // stolen code
		return Skip;

	AssumeNoVehicleByDefault(occupyFlags, isVehicleFlagSet);

	return Check;
}

DEFINE_HOOK(0x51C251, InfantryClass_CanEnterCell_SkipChildren, 0x0)
{
	enum { IgnoreOccupier = 0x51C70F, Continue = 0x51C259 };

	GET(InfantryClass*, pThis, EBP);
	GET(ObjectClass*, pOccupier, ESI);

	REF_STACK(byte, occupyFlags, STACK_OFFSET(0x34, -0x21));
	REF_STACK(bool, isVehicleFlagSet, STACK_OFFSET(0x34, -0x22));

	return IsOccupierIgnorable(pThis, pOccupier, occupyFlags, isVehicleFlagSet)
		? IgnoreOccupier : Continue;
}
/*
DEFINE_HOOK(0x51C78F, InfantryClass_CanEnterCell_CheckMovingInto, 0x6)
{
	GET_STACK(CellClass*, into, STACK_OFFSET(0x34, 0x4));
	GET_STACK(bool const, isAlt, STACK_OFFSET(0x34, -0x23));
	GET(InfantryClass*, pThis, EBP);

	REF_STACK(byte, occupyFlags, STACK_OFFSET(0x34, -0x21));
	REF_STACK(bool, isVehicleFlagSet, STACK_OFFSET(0x34, -0x22));

	AccountForMovingInto(into, isAlt, pThis, occupyFlags, isVehicleFlagSet);

	return 0;
}
*/
enum class CellTechnoMode
{
	NoAttachments,
	NoVirtualOrRelatives,
	NoVirtual,
	NoRelatives, // misleading name but I think doesn't matter for the use case for now
	All,

	DefaultBehavior = All,
};

namespace TechnoAttachmentTemp
{
	CellTechnoMode currentMode = CellTechnoMode::DefaultBehavior;
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

// original code doesn't account for multiple possible technos on the cell
DEFINE_HOOK(0x73A5EA, UnitClass_PerCellProcess_EntryLoopTechnos, 0x0)
{
	enum { SkipEntry = 0x73A7D2, TryEnterTarget = 0x73A6D1 };

	GET(UnitClass*, pThis, EBP);

	if (pThis->GetCurrentMission() != Mission::Enter)
		return SkipEntry;

	CellClass* pCell = pThis->GetCell();
	ObjectClass*& pFirst = pThis->OnBridge
		? pCell->AltObject : pCell->FirstObject;

	for (ObjectClass* pObject = pFirst; pObject; pObject = pObject->NextObject)
	{
		auto pEntryTarget = abstract_cast<TechnoClass*>(pObject);

		if (pEntryTarget
			&& pEntryTarget != pThis
			&& pEntryTarget->GetMapCoords() == pThis->GetMapCoords()
			&& pThis->ContainsLink(pEntryTarget)
			&& pEntryTarget->GetTechnoType()->Passengers > 0)
		{
			R->ESI<TechnoClass*>(pEntryTarget);
			return TryEnterTarget;
		}
	}

	return SkipEntry;
}

enum class AttachCargoMode
{
	SingleObject,
	ObjectChain,

	DefaultBehavior = SingleObject,
};

namespace TechnoAttachmentTemp
{
	AttachCargoMode currentAttachMode = AttachCargoMode::DefaultBehavior;
}

#define DEFINE_ATTACH_WRAPPER(mode) \
void __fastcall CargoClass_Attach_##mode(PassengersClass* pThis, discard_t, FootClass* pThat) \
{ \
	TechnoAttachmentTemp::currentAttachMode = AttachCargoMode::mode; \
	pThis->AddPassenger(pThat); \
	TechnoAttachmentTemp::currentAttachMode = AttachCargoMode::DefaultBehavior; \
}

DEFINE_ATTACH_WRAPPER(SingleObject);
DEFINE_ATTACH_WRAPPER(ObjectChain);

DEFINE_JUMP(CALL, 0x65DF88, GET_OFFSET(CargoClass_Attach_ObjectChain));  // Create_Group
DEFINE_JUMP(CALL, 0x65DCF0, GET_OFFSET(CargoClass_Attach_ObjectChain));  // Do_Reinforcements, paradrop loading

DEFINE_HOOK(0x4733BD, CargoClass_Attach_HandleCurrentAttachMode, 0x6)
{
	enum { SkipAttachingChain = 0x4733FA, Continue = 0x0 };

	return TechnoAttachmentTemp::currentAttachMode == AttachCargoMode::SingleObject
		? SkipAttachingChain
		: Continue;
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

	return TechnoExt::IsAttached(pFoot) ? Skip : Continue;
}

#pragma region Command inheritance

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
	bool deployPressed = false;
}

DEFINE_HOOK(0x730EA0, StopCommand_Context_Set, 0x5)
{
	TechnoAttachmentTemp::stopPressed = true;
	return 0;
}

DEFINE_HOOK(0x730AF0, DeployCommand_Context_Set, 0x8)
{
	TechnoAttachmentTemp::deployPressed = true;
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
	if ((TechnoAttachmentTemp::stopPressed || TechnoAttachmentTemp::deployPressed)
		&& TechnoAttachmentTemp::pParent)
	{
		if (auto const& pExt = TechnoExt::ExtMap.Find(TechnoAttachmentTemp::pParent))
		{
			for (auto const& pAttachment : pExt->ChildAttachments)
			{
				if (!pAttachment->Child)
					continue;

				if (pAttachment->GetType()->InheritCommands_StopCommand)
					pAttachment->Child->ClickedEvent(EventType::Idle);

				if (pAttachment->GetType()->InheritCommands_DeployCommand)
					pAttachment->Child->ClickedEvent(EventType::Deploy);
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

DEFINE_HOOK(0x730D55, DeployCommand_Context_Unset, 0x7)
{
	TechnoAttachmentTemp::deployPressed = false;
	return 0;
}


#pragma endregion

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

DEFINE_HOOK(0x4817A8, CellClass_Incoming_CheckIfTechnoOccupies, 0x6)
{
	enum { ConditionIsTrue = 0x4817C3, ContinueCheck = 0x0 };

	GET(TechnoClass*, pTechno, ESI);
	auto const& pExt = TechnoExt::ExtMap.Find(pTechno);

	return pExt->ParentAttachment && pExt->ParentAttachment->GetType()->OccupiesCell
		? ConditionIsTrue
		: ContinueCheck;
}

DEFINE_HOOK(0x4817C3, CellClass_Incoming_HandleScatterWithAttachments, 0x0)
{
	GET(TechnoClass*, pTechno, ESI);

	GET(CoordStruct*, pThreatCoord, EBP);
	GET(bool, isForced, EBX);
	GET_STACK(bool, isNoKidding, STACK_OFFSET(0x2C, 0xC));  // direct all complaints to tomsons26 for the variable naming
	CoordStruct const& threatCoord = *pThreatCoord;

	// we already checked that this is something that occupies the cell, see the hook above - Kerbiter
	TechnoExt::GetTopLevelParent(pTechno)->Scatter(threatCoord, isForced, isNoKidding);

	return 0x4817D9;
}

DEFINE_HOOK(0x51D0DD, InfantryClass_Scatter_CheckAttachments, 0x6)
{
	enum { Bail = 0x51D6E6, Continue = 0x0 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExt::HasAttachmentLoco(pThis)
		? Bail
		: Continue;
}


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

// MouseOverObject for entering bunkers, grinder, buildings etc
// is handled along with the shield logics in another file

DEFINE_JUMP(VTABLE, 0x7F5CE0, GET_OFFSET(UnitClass_MouseOverCell_Wrapper))

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

DEFINE_HOOK(0x6DA3FF, TacticalClass_SelectAt_TransparentToMouse_TacticalSelectable, 0x6)
{
	enum { SkipTechno = 0x6DA440, ContinueCheck = 0x0 };

	GET(TechnoClass*, pTechno, EAX);

	auto const pExt = TechnoExt::ExtMap.Find(pTechno);
	if (pExt && pExt->ParentAttachment && pExt->ParentAttachment->GetType()->TransparentToMouse)
		return SkipTechno;

	return ContinueCheck;
}

DEFINE_HOOK(0x6DA4FB, TacticalClass_SelectAt_TransparentToMouse_OccupierPtr, 0x6)
{
	GET(CellClass*, pCell, EAX);

	ObjectClass* pFoundObject = nullptr;
	for (ObjectClass* pOccupier = pCell->FirstObject; pOccupier; pOccupier = pOccupier->NextObject)
	{
		// find first non-transparent to mouse techno and return it
		if (auto const pOccupierAsTechno = abstract_cast<TechnoClass*>(pOccupier))
		{
			auto const pExt = TechnoExt::ExtMap.Find(pOccupierAsTechno);
			if (pExt && pExt->ParentAttachment && pExt->ParentAttachment->GetType()->TransparentToMouse)
				continue;
		}

		pFoundObject = pOccupier;
		break;
	}

	R->EAX<ObjectClass*>(pFoundObject);
	return 0x6DA501;
}

// this is probably not the best way to implement sight since we may be hijacking
// into some undesirable side effects, cause this is intended for air units that
// don't run Per Cell Process function, ergo, don't update their sight - Kerbiter
DEFINE_HOOK(0x4DA6A0, FootClass_AI_CheckLocoForSight, 0x0)
{
	enum { ContinueCheck = 0x4DA6AF, NoSightUpdate = 0x4DA7B0 };

	GET(FootClass*, pThis, ESI);

	return pThis->IsInAir() || TechnoExt::HasAttachmentLoco(pThis)
		? ContinueCheck
		: NoSightUpdate;
}

DEFINE_HOOK(0x440951, BuildingClass_Unlimbo_AttachmentsFromUpgrade, 0x6)
{
	GET(BuildingClass*, pBuilding, EDI);
	GET(BuildingClass*, pUpgrade, ESI);

	TechnoExt::TransferAttachments(pUpgrade, pBuilding);

	return 0;
}
