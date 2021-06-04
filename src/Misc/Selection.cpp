#include "Phobos.h"
#include "Utilities/Macro.h"
#include "Ext/TechnoType/Body.h"

#include <HouseClass.h>
#include <Unsorted.h>

class ExtSelection
{
public:
	using callback_type = bool(__fastcall*)(ObjectClass*);

	// Reversed from Is_Selectable, w/o Select call
	static bool ObjectClass_IsSelectable(ObjectClass* pThis)
	{
		auto owner = pThis->GetOwningHouse();
		return owner && owner->ControlledByPlayer()
			&& pThis->CanBeSelected() && pThis->CanBeSelectedNow()
			&& !pThis->InLimbo;
	}

	// Reversed from Tactical::Select
	static bool Tactical_IsInSelectionRect(TacticalClass* pThis, RECT* rect, TacticalSelectableStruct* selectable)
	{
		if (selectable->Techno && selectable->Techno->IsAlive)
		{
			LONG localX = selectable->X - pThis->VisibleArea.X;
			LONG localY = selectable->Y - pThis->VisibleArea.Y;

			if ((localX >= rect->left && localX < rect->right + rect->left) &&
				(localY >= rect->top && localY < rect->bottom + rect->top)) {
				return true;
			}
		}
		return false;
	}

	static bool Tactical_IsHighPriorityInRect(TacticalClass* pThis, RECT* rect)
	{
		auto selected = Unsorted::TacticalSelectables;

		for (int i = 0; i < pThis->SelectableCount; i++, selected++) {
			if (Tactical_IsInSelectionRect(pThis, rect, selected) && ObjectClass_IsSelectable(selected->Techno)) {
				auto technoTypeExt = TechnoTypeExt::ExtMap.Find(selected->Techno->GetTechnoType());

				if (technoTypeExt && !technoTypeExt->LowSelectionPriority)
					return true;
			}
		}

		return false;
	}

	static // Reversed from Tactical::Select
	void Tactical_SelectFiltered(TacticalClass* pThis, RECT* rect, callback_type check_callback, bool priorityFiltering)
	{
		Unsorted::MoveFeedback = true;

		if (rect->right <= 0 || rect->bottom <= 0 || pThis->SelectableCount <= 0)
			return;

		auto selected = Unsorted::TacticalSelectables;
		for (int i = 0; i < pThis->SelectableCount; i++, selected++) {
			if (Tactical_IsInSelectionRect(pThis, rect, selected)) {
				auto techno = selected->Techno;
				auto technoType = techno->GetTechnoType();
				auto technoTypeExt = TechnoTypeExt::ExtMap.Find(technoType);

				if (priorityFiltering && technoTypeExt && technoTypeExt->LowSelectionPriority)
					continue;

				if (technoTypeExt && Game::IsTypeSelecting()) {
					Game::UICommands_TypeSelect_7327D0(technoTypeExt->GetSelectionGroupID());
				}
				else if (check_callback) {
					(*check_callback)(techno);
				}
				else {
					bool isDeployedBuilding = false;
					if (techno->WhatAmI() == AbstractType::Building) {
						auto buildingType = abstract_cast<BuildingTypeClass*>(techno->GetType());

						if (buildingType->UndeploysInto && buildingType->IsUndeployable()) {
							isDeployedBuilding = true;
						}
					}

					auto owner = techno->GetOwningHouse();
					if (owner && owner->ControlledByPlayer() && techno->CanBeSelected()
						&& (techno->WhatAmI() != AbstractType::Building || isDeployedBuilding)) {
						Unsorted::MoveFeedback = !techno->Select();
					}
				}
			}
		}

		Unsorted::MoveFeedback = true;
	}

	static // Reversed from Tactical::MakeSelection
	void __fastcall Tactical_MakeFilteredSelection(TacticalClass* pThis, void*_, callback_type check_callback)
	{
		if (pThis->Band.left || pThis->Band.top) {
			LONG left = pThis->Band.left;
			LONG right = pThis->Band.right;
			LONG top = pThis->Band.top;
			LONG bottom = pThis->Band.bottom;

			if (left > right)
				std::swap(left, right);
			if (top > bottom)
				std::swap(top, bottom);

			RECT rect{ left , top, right - left + 1, bottom - top + 1 };

			bool priorityFiltering = Phobos::Config::PrioritySelectionFiltering && Tactical_IsHighPriorityInRect(pThis, &rect);
			Tactical_SelectFiltered(pThis, &rect, check_callback, priorityFiltering);

			pThis->Band.left = 0;
			pThis->Band.top = 0;
		}
	}
};

// Replace single call
DEFINE_POINTER_CALL(0x4ABCEB, ExtSelection::Tactical_MakeFilteredSelection);

// Replace vanilla function. For in case another module tries to call the vanilla function at offset
DEFINE_POINTER_LJMP(0x6D9FF0, ExtSelection::Tactical_MakeFilteredSelection)
