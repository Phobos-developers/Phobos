#include <Phobos.h>

#include <FootClass.h>
#include <TacticalClass.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

// Multi-click type selection:
//   * double-click a unit -> select every unit of its selection group nearby;
//   * triple-click a unit -> select that group across the whole map.
//
// Vanilla only has the type-select hotkey (hold T, then click or drag); there is no
// double-click equivalent, a double click in the tactical area falls through to the message
// handler's default case. Grouping reuses TechnoTypeExt::GetSelectionGroupID /
// HasSelectionGroupID (the GroupAs tag, with the type ID as fallback), so it stays consistent
// with the hotkey type select in Selection.cpp.
//
// This needs RightClickCommand. With the vanilla left button a click on an already selected
// unit is a command, so a double click would deploy an MCV or an Allied GI instead of
// selecting. Phobos.INI.cpp turns the setting off if RightClickCommand is not enabled.

namespace RightClickCommand
{
	// Defined in RightClickCommand.cpp. Set by the RMB command redirect, cleared here.
	extern bool RmbCommandInProgress;
}

namespace MultiClickTypeSelect
{
	// Clicks within GetDoubleClickTime() and this many screen pixels of the previous one
	// count as part of the same streak.
	static constexpr int PositionTolerance = 4;

	static DWORD LastClickTick = 0;
	static POINT LastClickPos = { -9999, -9999 };
	static int ClickStreak = 0;

	// Mirrors ExtSelection::ObjectClass_IsSelectable (Selection.cpp): an own, alive, currently
	// selectable object.
	static bool IsOwnSelectable(TechnoClass* pTechno)
	{
		const auto pOwner = pTechno->GetOwningHouse();
		return pOwner && pOwner->IsControlledByCurrentPlayer()
			&& pTechno->CanBeSelected() && pTechno->CanBeSelectedNow()
			&& !pTechno->InLimbo;
	}

	// Add to the current selection every own, selectable mobile unit sharing the clicked
	// unit's selection group. wholeMap takes the whole map; otherwise the reach is
	// TypeSelectByMultiClick.Range cells around the clicked unit, or, if that is negative,
	// everything drawn in the tactical viewport.
	static void SelectSameGroup(FootClass* pClicked, bool wholeMap)
	{
		const char* groupID = TechnoTypeExt::GetSelectionGroupID(pClicked->GetTechnoType());
		const int rangeCells = Phobos::Config::TypeSelectByMultiClick_Range;

		for (auto const pTechno : TechnoClass::Array)
		{
			const auto pFoot = abstract_cast<FootClass*, true>(pTechno);

			if (!pFoot || pFoot->IsSelected || !IsOwnSelectable(pFoot))
				continue;

			if (!TechnoTypeExt::HasSelectionGroupID(pFoot->GetTechnoType(), groupID))
				continue;

			if (!wholeMap)
			{
				if (rangeCells >= 0)
				{
					if (pClicked->DistanceFrom(pFoot) > rangeCells * Unsorted::LeptonsPerCell)
						continue;
				}
				else if (!TacticalClass::Instance->CoordsToClient(pFoot->GetCoords()).second)
				{
					continue;
				}
			}

			pFoot->Select();
		}
	}

	// Update the click streak from the current cursor time/position and act on it. pClicked is
	// the object the click landed on, or null for empty ground.
	static void HandleClick(FootClass* pClicked)
	{
		POINT pos { 0, 0 };
		GetCursorPos(&pos);
		const DWORD now = GetTickCount();

		const int dx = pos.x - LastClickPos.x;
		const int dy = pos.y - LastClickPos.y;
		const bool sameSpot = dx >= -PositionTolerance && dx <= PositionTolerance
			&& dy >= -PositionTolerance && dy <= PositionTolerance;

		if (now - LastClickTick <= GetDoubleClickTime() && sameSpot)
			ClickStreak = ClickStreak < 3 ? ClickStreak + 1 : 3;
		else
			ClickStreak = 1;

		LastClickTick = now;
		LastClickPos = pos;

		if (!pClicked) // only mobile units drive type select
			return;

		if (ClickStreak == 2)
			SelectSameGroup(pClicked, false);
		else if (ClickStreak == 3)
			SelectSameGroup(pClicked, true);
	}
}

// Tactical LBUTTONUP handler, just after the click's own selection has been applied (call to
// 0x4AB9B0) and before the drag flag is cleared. This point is only reached by a genuine
// single click - a completed rubber-band selection returns earlier. Stolen bytes:
// mov byte ptr [esi+0x555A], bl (absolute operand, safe to relocate).
DEFINE_HOOK(0x693290, TacticalMsgHandler_LButtonUp_MultiClickTypeSelect, 0x6)
{
	// The RMB command redirect (RightClickCommand.cpp) ends here too. Consume the flag and
	// skip, so an RMB order is not counted as a left click for the streak.
	if (RightClickCommand::RmbCommandInProgress)
	{
		RightClickCommand::RmbCommandInProgress = false;
		return 0;
	}

	if (!Phobos::Config::TypeSelectByMultiClick)
		return 0;

	// The object this click landed on, as ProcessClickCoords resolved it at 0x69325E and
	// DecideAction and the applier were given it. ESP here equals ESP at the start of the
	// command dispatch (0x69323E), which is where that output slot is addressed from.
	// Not CurrentObjects[0]: with several units selected that is not the clicked one.
	// Null when the click landed on empty ground, so this cast must keep its null check.
	const auto pClicked = abstract_cast<FootClass*>(R->Stack<ObjectClass*>(0x2C));

	MultiClickTypeSelect::HandleClick(pClicked);

	return 0;
}
