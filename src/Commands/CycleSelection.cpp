#include "CycleSelection.h"

#include <Utilities/GeneralUtils.h>

namespace CycleSelection
{
	constexpr int NavCycleMode_CycleSelection = 6;

	// Not persisted in savegames.
	std::vector<ObjectClass*> Objects;
	int Index = -1;
}

const char* CycleSelectionCommandClass::GetName() const
{
	return "Cycle Selection";
}

const wchar_t* CycleSelectionCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CYCLE_SELECTION", L"Cycle Selection");
}

const wchar_t* CycleSelectionCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* CycleSelectionCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CYCLE_SELECTION_DESC", L"Cycle through the current selection, selecting one object at a time.");
}

void CycleSelectionCommandClass::Execute(WWKey eInput) const
{
	// Same guard vanilla's navigation commands use
	if (Unsorted::MuteSWLaunches)
		return;

	if (Unsorted::NavCycleMode != CycleSelection::NavCycleMode_CycleSelection)
	{
		CycleSelection::Objects.clear();

		for (const auto pObject : ObjectClass::CurrentObjects)
		{
			CycleSelection::Objects.push_back(pObject);
		}

		CycleSelection::Index = -1;
	}

	const int count = static_cast<int>(CycleSelection::Objects.size());

	if (count <= 0)
	{
		MessageListClass::Instance.PrintMessage(StringTable::LoadString("MSG:NothingSelected"),
			RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
		return;
	}

	ObjectClass* pTarget = nullptr;
	int index = CycleSelection::Index;

	for (int i = 0; i < count; ++i)
	{
		index = (index + 1) % count;

		if (const auto pObject = abstract_cast<ObjectClass*>(CycleSelection::Objects[index])) // in case of wild pointer
		{
			if (pObject->Health > 0 && pObject->IsAlive && !pObject->InLimbo)
			{
				pTarget = pObject;
				break;
			}
		}
	}

	if (!pTarget)
	{
		CycleSelection::Objects.clear();
		MessageListClass::Instance.PrintMessage(StringTable::LoadString(GameStrings::TXT_NOTHING_SELECTED),
			RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
		return;
	}

	MapClass::UnselectAll();

	if (pTarget->Select())
	{
		CycleSelection::Index = index;
		MapClass::Instance.MarkNeedsRedraw(1);
		// UnselectAll and Select sets NavCycleMode to 0
		Unsorted::NavCycleMode = CycleSelection::NavCycleMode_CycleSelection;
	}
}
