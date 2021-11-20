#include "Body.h"

#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x4DA86E, FootClass_AI_UpdateAttachedLocomotion, 0x0)
{
	GET(FootClass* const, pThis, ESI);
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->ParentAttachment)
		pThis->Locomotor->Process();

	return 0x4DA87A;
}

DEFINE_HOOK(0x710460, TechnoClass_Destroy_HandleAttachments, 0x6)
{
	GET(TechnoClass*, pThis, ECX);

	TechnoExt::HandleHostDestruction(pThis);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt->ParentAttachment)
		pExt->ParentAttachment->ChildDestroyed();

	pExt->ParentAttachment = nullptr;

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

DEFINE_HOOK(0x73F528, UnitClass_CanEnterCell_SkipChildren, 0x0)
{
	enum { IgnoreOccupier = 0x73FC10, Continue = 0x73F530 };

	GET(UnitClass*, pThis, EBX);
	GET(TechnoClass*, pOccupier, ESI);

	if (pThis == pOccupier || TechnoExt::IsParentOf(pThis, pOccupier))
		return IgnoreOccupier;

	return Continue;
}

DEFINE_HOOK(0x51C251, InfantryClass_CanEnterCell_SkipChildren, 0x0)
{
	enum { IgnoreOccupier = 0x51C70F, Continue = 0x51C259 };

	GET(InfantryClass*, pThis, EBP);
	GET(TechnoClass*, pOccupier, ESI);

	if ((TechnoClass*)pThis == pOccupier || TechnoExt::IsParentOf((TechnoClass*)pThis, pOccupier))
		return IgnoreOccupier;

	return Continue;
}

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
	GET_STACK(int, idxPath, STACK_OFFS(0x18, +0x8));
	GET_STACK(unsigned char, idxWP, STACK_OFFS(0x18, +0xC));

	for (auto const& pObject : ObjectClass::CurrentObjects.get())
	{
		if (auto pTechno = abstract_cast<TechnoClass*>(pObject))
			ParentClickedWaypoint(pTechno, idxPath, idxWP);
	}

	GET_STACK(ObjectClass* const, pTarget, STACK_OFFS(0x18, -0x4));
	LEA_STACK(CellStruct* const, pCell, STACK_OFFS(0x18, -0x8));
	GET_STACK(Action const, action, STACK_OFFS(0x18, -0xC));

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
	GET_STACK(Mission, mission, STACK_OFFS(0x98, -0x4));

	switch (mission)
	{
	case Mission::Move:
	case Mission::AttackMove:
	case Mission::Enter:
	case Mission::QMove:
		return 0;
	}

	GET_STACK(ObjectClass* const, pTarget, STACK_OFFS(0x98, -0x8));
	GET_STACK(CellClass* const, pTargetCell, STACK_OFFS(0x98, -0xC));
	GET_STACK(CellClass* const, pCellNearTarget, STACK_OFFS(0x98, -0x10));
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
// 0x4696FB Locomotor warhead
// ...