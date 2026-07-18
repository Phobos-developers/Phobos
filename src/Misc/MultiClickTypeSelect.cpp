#include <Phobos.h>

#include <FootClass.h>
#include <TacticalClass.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

// Multi-click type selection.
//
// Vanilla Yuri's Revenge only offers "type select" through the type-select hotkey (hold T,
// then click/drag) - there is no double-click equivalent; a double-click in the tactical
// area falls through to the message handler's default case. This adds the modern-RTS gesture:
//   * double-click a unit  -> select every unit of the same selection group on screen;
//   * triple-click a unit  -> select every unit of the same selection group across the map.
//
// "Selection group" reuses Phobos' existing concept (TechnoTypeExt::GetSelectionGroupID /
// HasSelectionGroupID, i.e. the [TechnoType]GroupAs tag with the type ID as fallback), so it
// stays consistent with the hotkey-driven type select in Selection.cpp.
//
// Gated behind [Phobos]TypeSelectByMultiClick (default off).

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

	// Add to the current selection every own, selectable mobile unit sharing the just-clicked
	// unit's selection group. onScreenOnly limits it to units drawn in the tactical viewport.
	static void SelectSameGroup(bool onScreenOnly)
	{
		if (ObjectClass::CurrentObjects.Count < 1)
			return;

		// The single click that preceded this streak left the clicked unit as the selection.
		const auto pClicked = abstract_cast<FootClass*, true>(ObjectClass::CurrentObjects.GetItem(0));
		if (!pClicked)
			return; // only mobile units drive type select

		const char* groupID = TechnoTypeExt::GetSelectionGroupID(pClicked->GetTechnoType());

		for (auto const pTechno : TechnoClass::Array)
		{
			const auto pFoot = abstract_cast<FootClass*, true>(pTechno);

			if (!pFoot || pFoot->IsSelected || !IsOwnSelectable(pFoot))
				continue;

			if (!TechnoTypeExt::HasSelectionGroupID(pFoot->GetTechnoType(), groupID))
				continue;

			if (onScreenOnly && !TacticalClass::Instance->CoordsToClient(pFoot->GetCoords()).second)
				continue;

			pFoot->Select();
		}
	}

	// Update the click streak from the current cursor time/position and act on it.
	static void HandleClick()
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

		if (ClickStreak == 2)
			SelectSameGroup(true);  // same group, on screen
		else if (ClickStreak == 3)
			SelectSameGroup(false); // same group, whole map
	}
}

// Tactical LBUTTONUP handler, just after the click's own selection has been applied (call to
// 0x4AB9B0) and before the drag flag is cleared. This point is only reached by a genuine
// single click - a completed rubber-band selection returns earlier - so the preceding click
// has already made the clicked unit the current selection. Stolen bytes:
// mov byte ptr [esi+0x555A], bl (absolute operand, safe to relocate).
DEFINE_HOOK(0x693290, TacticalClass_LButtonUp_MultiClickTypeSelect, 0x6)
{
	if (Phobos::Config::TypeSelectByMultiClick)
		MultiClickTypeSelect::HandleClick();

	return 0;
}
