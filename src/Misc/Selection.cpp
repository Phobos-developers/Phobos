#include "../Phobos.h"
#include <TacticalClass.h>
#include <TechnoClass.h>
#include <UnitClass.h>
#include <UnitTypeClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <ObjectClass.h>
#include "../Ext/TechnoType/Body.h"
#include <Unsorted.h>

class ExtSelection
{
public:
	// Reversed from Is_Selectable, w/o Select call
	static bool ObjectClass_IsSelectable(ObjectClass* pThis)
	{
		HouseClass* owner = pThis->GetOwningHouse();

		return owner && owner->ControlledByPlayer()
			&& pThis->CanBeSelected() && pThis->CanBeSelectedNow()
			&& !pThis->InLimbo;
	}

	// Reversed from Tactical::Select
	static bool Tactical_IsInSelectionRect(TacticalClass* pThis, RECT* rect, TacticalSelectableStruct* selectable)
	{
		if (selectable->Techno && selectable->Techno->IsAlive)
		{
			LONG localX = selectable->X - pThis->TacticalPos0.X;
			LONG localY = selectable->Y - pThis->TacticalPos0.Y;

			if ((localX >= rect->left && localX < rect->right + rect->left) &&
				(localY >= rect->top && localY < rect->bottom + rect->top)) {
				return true;
			}
		}
		return false;
	}

	static bool Tactical_IsHighPriorityInRect(TacticalClass* pThis, RECT* rect)
	{
		TacticalSelectableStruct* selected = Unsorted::TacticalSelectables;

		for (int i = 0; i < pThis->SelectableCount; i++, selected++) {
			if (Tactical_IsInSelectionRect(pThis, rect, selected) && ObjectClass_IsSelectable(selected->Techno)) {
				auto technoTypeExt = TechnoTypeExt::ExtMap.Find(selected->Techno->GetTechnoType());

				if (!technoTypeExt->LowSelectionPriority)
					return true;
			}
		}

		return false;
	}

	static // Reversed from Tactical::Select
		void Tactical_SelectFiltered(TacticalClass* pThis, RECT* rect, bool(__fastcall* check_callback)(ObjectClass*), bool priorityFiltering)
	{
		Unsorted::MoveFeedback = true;

		if (rect->right <= 0 || rect->bottom <= 0 || pThis->SelectableCount <= 0)
			return;

		TacticalSelectableStruct* selected = Unsorted::TacticalSelectables;
		for (int i = 0; i < pThis->SelectableCount; i++, selected++) {
			if (Tactical_IsInSelectionRect(pThis, rect, selected)) {
				TechnoClass* techno = selected->Techno;
				TechnoTypeClass* technoType = techno->GetTechnoType();
				auto technoTypeExt = TechnoTypeExt::ExtMap.Find(technoType);

				if (priorityFiltering && technoTypeExt->LowSelectionPriority)
					continue;

				if (Game::IsTypeSelecting()) {
					Game::UICommands_TypeSelect_7327D0(technoTypeExt->GetSelectionGroupID());
				}
				else if (check_callback) {
					(*check_callback)(techno);
				}
				else {
					bool isDeployedBuilding = false;
					if (techno->WhatAmI() == AbstractType::Building) {
						BuildingTypeClass* buildingType = (BuildingTypeClass*)techno->GetType();

						if (buildingType->UndeploysInto && buildingType->IsUndeployable()) {
							isDeployedBuilding = true;
						}
					}

					HouseClass* owner = techno->GetOwningHouse();
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
		void Tactical_MakeFilteredSelection(TacticalClass* pThis, bool(__fastcall* check_callback)(ObjectClass*))
	{
		if (pThis->Band.left || pThis->Band.top) {
			LONG left = pThis->Band.left;
			LONG right = pThis->Band.right;

			if (left > right) {
				LONG temp = left;
				left = right;
				right = temp;
			}

			LONG top = pThis->Band.top;
			LONG bottom = pThis->Band.bottom;

			if (top > bottom) {
				LONG temp = top;
				top = bottom;
				bottom = temp;
			}

			RECT rect;
			rect.right = right - left + 1;
			rect.bottom = bottom - top + 1;
			rect.left = left;
			rect.top = top;

			bool priorityFiltering = Phobos::Config::PrioritySelectionFiltering && Tactical_IsHighPriorityInRect(pThis, &rect);
			Tactical_SelectFiltered(pThis, &rect, check_callback, priorityFiltering);

			pThis->Band.left = 0;
			pThis->Band.top = 0;
		}
	}
};

DEFINE_HOOK(6D9FF0, Tactical_MakeSelection_FilterSelection, 0)
{
	auto IsSelectable = R->Stack<bool(__fastcall*)(ObjectClass*)>(4);
	GET(TacticalClass*, pThis, ECX);
	ExtSelection::Tactical_MakeFilteredSelection(pThis, IsSelectable);
	return 0x6DA075;
}