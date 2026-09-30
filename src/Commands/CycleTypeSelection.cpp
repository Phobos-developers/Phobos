#include "CycleTypeSelection.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/TechnoType/Body.h>

namespace CycleTypeSelection
{
	constexpr int NavCycleMode_CycleTypeSelection = 7;

	// One entry per distinct type found in the selection the cycle was started with.
	struct TypeGroup
	{
		std::string ID;        // regist game
		int Priority { 0 };    // TechnoType's TypeCyclePriority
		int UnitCost { 0 };    // the type's raw cost as registered in the INI
		int TypeIndex { 0 };   // the type's index in the game's TechnoType array
	};

	// The objects the cycle was started with.
	std::vector<ObjectClass*> Objects;

	// Objects' types, ordered from the highest TypeCyclePriority, down to the lowest,
	std::vector<TypeGroup> Types;

	int Index = -1;

	// Vanilla's type selection predicate - alive, player-controlled, type-matching - extended
	// by Ares and Phobos GroupAs definition.
	bool Matches(ObjectClass* pObject, const char* pID)
	{
		const auto pTechno = abstract_cast<TechnoClass*>(pObject);

		if (!pTechno || pTechno->Health <= 0 || !pTechno->IsAlive || pTechno->InLimbo)
			return false;

		const auto pOwner = pTechno->GetOwningHouse();

		if (!pOwner || !pOwner->IsControlledByCurrentPlayer())
			return false;

		if (pTechno->IsPlayerAliveUnitOf(pID))
			return true;

		return TechnoTypeExt::HasSelectionGroupID(pTechno->GetTechnoType(), pID);
	}

	void CollectFromCurrentSelection()
	{
		Objects.clear();
		Types.clear();
		Index = -1;

		for (const auto pObject : ObjectClass::CurrentObjects)
		{
			Objects.push_back(pObject);

			const auto pTechno = abstract_cast<TechnoClass*, true>(pObject);

			if (!pTechno)
				continue;

			const auto pType = pTechno->GetTechnoType();

			if (!pType)
				continue;

			const auto pID = TechnoTypeExt::GetSelectionGroupID(pType);

			// Only the first object of a type contributes its type's properties
			const auto it = std::ranges::find_if(Types,
				[pID](const TypeGroup& group) { return _stricmp(group.ID.c_str(), pID) == 0; });

			if (it != Types.end())
				continue;

			const int nTypeIndex = TechnoTypeClass::Array.FindItemIndex(pType);
			Types.emplace_back(TypeGroup { pID, TechnoTypeExt::Fetch(pType)->TypeCyclePriority, pType->Cost, nTypeIndex });
		}

		// Cycle by:
		// TypeCyclePriority descending, larger first then
		// Cost descending, gerater first then
		// Registered order (Array index) descending, later first
		std::ranges::sort(Types, [](const TypeGroup& lhs, const TypeGroup& rhs)
		{
			if (lhs.Priority != rhs.Priority)
				return lhs.Priority > rhs.Priority;

			if (lhs.UnitCost != rhs.UnitCost)
				return lhs.UnitCost > rhs.UnitCost;

			return lhs.TypeIndex > rhs.TypeIndex;
		});
	}

	// Mirrors the summary the game's own type cycle prints via FormatSelectionSummary (0x731D90):
	// the type's name when the current selection consists solely of that type,
	// and the number of selected objects of the type together with their total cost,
	// formatted into the vanilla MSG:UnitsWorth string. The costs are added up the way the game
	// does it there, cost multipliers of that house are taken into account.
	void PrintTypeSummary(const char* pID)
	{
		constexpr const char* pKeyName = "MSG:UnitsWorth";
		constexpr const char* pKeyNoUnits = "MSG:NoUnitsSel";

		const wchar_t* pFormat = StringTable::LoadString(pKeyName);

		if (!pFormat)
			return;

		const wchar_t* pString = nullptr;
		int count = 0;
		int value = 0;

		for (const auto pObject : ObjectClass::CurrentObjects)
		{
			++count;

			const auto pType = pObject->GetType();
			const auto pOwner = pObject->GetOwningHouse();

			if (pType && pOwner)
				value += pType->GetActualCost(pOwner);
		}

		// Everything selected is of the cycled type: label the summary with the type's name.
		if (ObjectClass::CurrentObjects.Count > 0)
			pString = ObjectClass::CurrentObjects.GetItem(0)->GetUIName();

		// Vanilla only formats anything if it has both a selection set and a name to report;
		// without a selection there is nothing this summary could be about.
		// TXT_NOTHING_SELECTED was printed already
		if (!pString)
			return;

		wchar_t buffer[256];

		if (count > 0)
		{
			swprintf_s(buffer, pFormat, count, pString, value);
		}
		else if (const wchar_t* const pFormatEmpty = StringTable::LoadString(pKeyNoUnits))
		{
			swprintf_s(buffer, pFormatEmpty, pString);
		}
		else
		{
			return;
		}

		MessageListClass::Instance.PrintMessage(buffer, RulesClass::Instance->MessageDelay,
			HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	}
}

const char* CycleTypeSelectionCommandClass::GetName() const
{
	return "Cycle Type Selection";
}

const wchar_t* CycleTypeSelectionCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CYCLE_TYPE_SELECTION", L"Cycle Type Selection");
}

const wchar_t* CycleTypeSelectionCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* CycleTypeSelectionCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CYCLE_TYPE_SELECTION_DESC", L"Cycle through the types in the current selection.");
}

void CycleTypeSelectionCommandClass::Execute(WWKey eInput) const
{
	// Same guard vanilla's navigation commands use.
	if (Unsorted::MuteSWLaunches)
		return;

	if (Unsorted::NavCycleMode != CycleTypeSelection::NavCycleMode_CycleTypeSelection)
	{
		CycleTypeSelection::CollectFromCurrentSelection();
	}

	const int count = static_cast<int>(CycleTypeSelection::Types.size());

	if (count <= 0)
	{
		MessageListClass::Instance.PrintMessage(StringTable::LoadString(GameStrings::TXT_NOTHING_SELECTED),
			RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
		return;
	}

	CycleTypeSelection::Index = (CycleTypeSelection::Index + 1) % count;

	const auto pID = CycleTypeSelection::Types[CycleTypeSelection::Index].ID.c_str();

	// This also zeroes Unsorted::NavCycleMode.
	MapClass::UnselectAll();

	// Follows the same pattern.
	// Exactly one selection voice per type switch instead of one per object.
	const char prev = Unsorted::MoveFeedback;
	Unsorted::MoveFeedback = true;

	for (const auto pObject : CycleTypeSelection::Objects)
	{
		if (CycleTypeSelection::Matches(pObject, pID))
			Unsorted::MoveFeedback = !pObject->Select();
	}

	Unsorted::MoveFeedback = prev;

	MapClass::Instance.MarkNeedsRedraw(1);
	Unsorted::NavCycleMode = CycleTypeSelection::NavCycleMode_CycleTypeSelection;

	// Vanilla's own type cycle prints a selection summary on every step.
	if (Phobos::Config::CycleTypeSelectionPrintSummary)
		CycleTypeSelection::PrintTypeSummary(pID);
}
