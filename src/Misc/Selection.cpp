#include "Phobos.h"
#include "Utilities/Macro.h"
#include "Ext/TechnoType/Body.h"

#include <HouseClass.h>
#include <Unsorted.h>

class ExtSelection
{
public:
	using callback_type = bool(__fastcall*)(ObjectClass*);

	static inline class TacticalSelectablesHelper
	{
	public:
		inline size_t size()
		{
			return TacticalClass::Instance->SelectableCount;
		}

		inline TacticalSelectableStruct* begin()
		{
			return &Unsorted::TacticalSelectables[0];
		}

		inline TacticalSelectableStruct* end()
		{
			return &Unsorted::TacticalSelectables[size()];
		}
	} Array {};

	// Reversed from Is_Selectable, w/o Select call
	static bool ObjectClass_IsSelectable(ObjectClass* pThis)
	{
		const auto pOwner = pThis->GetOwningHouse();
		return pOwner && pOwner->ControlledByPlayer()
			&& pThis->CanBeSelected() && pThis->CanBeSelectedNow()
			&& !pThis->InLimbo;
	}

	// Reversed from Tactical::Select
	static bool Tactical_IsInSelectionRect(TacticalClass* pThis, RECT* pRect, const TacticalSelectableStruct& selectable)
	{
		if (selectable.Techno && selectable.Techno->IsAlive)
		{
			LONG localX = selectable.X - pThis->TacticalPos.X;
			LONG localY = selectable.Y - pThis->TacticalPos.Y;

			if ((localX >= pRect->left && localX < pRect->right + pRect->left) &&
				(localY >= pRect->top && localY < pRect->bottom + pRect->top)) {
				return true;
			}
		}
		return false;
	}

	static bool Tactical_IsHighPriorityInRect(TacticalClass* pThis, RECT* rect)
	{
		for (const auto& selected : Array)
			if (Tactical_IsInSelectionRect(pThis, rect, selected) && ObjectClass_IsSelectable(selected.Techno))
				if (!TechnoTypeExt::ExtMap.Find(selected.Techno->GetTechnoType())->LowSelectionPriority)
					return true;

		return false;
	}

	static // Reversed from Tactical::Select
	void Tactical_SelectFiltered(TacticalClass* pThis, RECT* pRect, callback_type check_callback, bool bPriorityFiltering)
	{
		Unsorted::MoveFeedback = true;

		if (pRect->right <= 0 || pRect->bottom <= 0 || pThis->SelectableCount <= 0)
			return;

		for (const auto& selected : Array)
			if (Tactical_IsInSelectionRect(pThis, pRect, selected))
			{
				const auto pTechno = selected.Techno;
				auto pTechnoType = pTechno->GetTechnoType();
				auto TypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (bPriorityFiltering && TypeExt && TypeExt->LowSelectionPriority)
					continue;

				if (TypeExt && Game::IsTypeSelecting())
					Game::UICommands_TypeSelect_7327D0(TypeExt->GetSelectionGroupID());
				else if (check_callback)
					(*check_callback)(pTechno);
				else
				{
					const auto pBldType = abstract_cast<BuildingTypeClass*>(pTechnoType);
					const auto pOwner = pTechno->GetOwningHouse();

					if (pOwner && pOwner->ControlledByPlayer() && pTechno->CanBeSelected()
						&& (!pBldType || (pBldType && pBldType->UndeploysInto && pBldType->IsUndeployable())))
					{
						Unsorted::MoveFeedback = !pTechno->Select();
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

			bool bPriorityFiltering = Phobos::Config::PrioritySelectionFiltering && Tactical_IsHighPriorityInRect(pThis, &rect);
			Tactical_SelectFiltered(pThis, &rect, check_callback, bPriorityFiltering);

			pThis->Band.left = 0;
			pThis->Band.top = 0;
		}
	}
};

// Replace single call
DEFINE_POINTER_CALL(0x4ABCEB, ExtSelection::Tactical_MakeFilteredSelection);

// Replace vanilla function. For in case another module tries to call the vanilla function at offset
DEFINE_POINTER_LJMP(0x6D9FF0, ExtSelection::Tactical_MakeFilteredSelection)
