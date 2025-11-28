#include "Phobos.h"
#include "Utilities/Macro.h"
#include "Ext/Techno/Body.h"
#include "Ext/TechnoType/Body.h"

#include <TacticalClass.h>
#include <HouseClass.h>
#include <Unsorted.h>

#include <unordered_set>

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

	static inline std::vector<const char*> IFVGroups;

	// Reversed from Is_Selectable, w/o Select call
	static bool ObjectClass_IsSelectable(ObjectClass* pThis)
	{
		const auto pOwner = pThis->GetOwningHouse();
		return pOwner && pOwner->IsControlledByCurrentPlayer()
			&& pThis->CanBeSelected() && pThis->CanBeSelectedNow()
			&& !pThis->InLimbo;
	}

	// Reversed from Tactical::Select
	static bool Tactical_IsInSelectionRect(TacticalClass* pThis, LTRBStruct* pRect, const TacticalSelectableStruct& selectable)
	{
		if (selectable.Object && selectable.Object->IsAlive)
		{
			const int nLocalX = selectable.X - pThis->TacticalPos.X;
			const int nLocalY = selectable.Y - pThis->TacticalPos.Y;

			if ((nLocalX >= pRect->Left && nLocalX < pRect->Right + pRect->Left)
				&& (nLocalY >= pRect->Top && nLocalY < pRect->Bottom + pRect->Top))
			{
				return true;
			}
		}

		return false;
	}

	static bool Tactical_IsHighPriorityInRect(TacticalClass* pThis, LTRBStruct* rect)
	{
		for (const auto& selected : Array)
		{
			if (Tactical_IsInSelectionRect(pThis, rect, selected) && ObjectClass_IsSelectable(selected.Object))
			{
				if ((selected.Object->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
				{
					if (!TechnoExt::ExtMap.Find(static_cast<TechnoClass*>(selected.Object))->TypeExtData->LowSelectionPriority)
						return true;
				}
			}
		}

		return false;
	}

	// Reversed from Tactical::Select
	static void Tactical_SelectFiltered(TacticalClass* pThis, LTRBStruct* pRect, callback_type check_callback, bool bPriorityFiltering)
	{
		Unsorted::MoveFeedback = true;

		if (pRect->Right <= 0 || pRect->Bottom <= 0 || pThis->SelectableCount <= 0)
			return;

		for (const auto& selected : Array)
		{
			if (Tactical_IsInSelectionRect(pThis, pRect, selected))
			{
				const auto pObject = selected.Object;
				const auto pTechnoType = pObject->GetTechnoType(); // Returns nullptr on non techno objects

				if (auto const pTypeExt = TechnoTypeExt::ExtMap.TryFind(pTechnoType)) // If pTechnoType is nullptr so will be pTypeExt
				{
					if (bPriorityFiltering && pTypeExt->LowSelectionPriority)
						continue;

					if (Game::IsTypeSelecting())
					{
						Game::UICommands_TypeSelect_7327D0(pTypeExt->GetSelectionGroupID());
						continue;
					}
				}

				if (check_callback)
				{
					(*check_callback)(pObject);
				}
				else
				{
					const auto pBldType = abstract_cast<BuildingTypeClass*, true>(pTechnoType);
					const auto pOwner = pObject->GetOwningHouse();

					if (pOwner && pOwner->IsControlledByCurrentPlayer() && pObject->CanBeSelected()
						&& (!pBldType || (pBldType->UndeploysInto && pBldType->IsVehicle())))
					{
						Unsorted::MoveFeedback = !pObject->Select();
					}
				}
			}
		}

		Unsorted::MoveFeedback = true;
	}

	static bool __fastcall TypeSelectCommand_Execute_Filter(TechnoClass* pTechno, DynamicVectorClass<const char*>& names)
	{
		const auto pTechnoType = pTechno->GetTechnoType();
		const char* id = TechnoTypeExt::GetSelectionGroupID(pTechnoType);

		if (std::ranges::none_of(names, [id](const char* pID) { return !_stricmp(pID, id); }))
			return false;

		if (pTechnoType->Gunner && !ExtSelection::IFVGroups.empty())
		{
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
			char* gunnerID = pTypeExt->WeaponGroupAs[pTechno->CurrentWeaponNumber];

			if (!GeneralUtils::IsValidString(gunnerID))
				sprintf_s(gunnerID, 0x20, "%d", pTechno->CurrentWeaponNumber + 1);

			if (std::ranges::none_of(ExtSelection::IFVGroups, [gunnerID](const char* pID) { return !_stricmp(pID, gunnerID); }))
				return false;
		}

		return pTechno->CanBeSelectedNow() || (pTechno->WhatAmI() == BuildingClass::AbsID && pTechnoType->UndeploysInto);
	}

	// Reversed from Tactical::MakeSelection
	static void __fastcall Tactical_MakeFilteredSelection(TacticalClass* pThis, void* _, callback_type check_callback)
	{
		if (pThis->Band.Left || pThis->Band.Top)
		{
			int nLeft = pThis->Band.Left;
			int nRight = pThis->Band.Right;
			int nTop = pThis->Band.Top;
			int nBottom = pThis->Band.Bottom;

			if (nLeft > nRight)
				std::swap(nLeft, nRight);
			if (nTop > nBottom)
				std::swap(nTop, nBottom);

			LTRBStruct rect { nLeft , nTop, nRight - nLeft + 1, nBottom - nTop + 1 };

			const bool bPriorityFiltering = Phobos::Config::PrioritySelectionFiltering && Tactical_IsHighPriorityInRect(pThis, &rect);
			Tactical_SelectFiltered(pThis, &rect, check_callback, bPriorityFiltering);

			pThis->Band.Left = 0;
			pThis->Band.Top = 0;
		}
	}
};

DEFINE_FUNCTION_JUMP(LJMP, 0x732C30, ExtSelection::TypeSelectCommand_Execute_Filter)

// Replace single call
DEFINE_FUNCTION_JUMP(CALL, 0x4ABCEB, ExtSelection::Tactical_MakeFilteredSelection)

// Replace vanilla function. For in case another module tries to call the vanilla function at offset
DEFINE_FUNCTION_JUMP(LJMP, 0x6D9FF0, ExtSelection::Tactical_MakeFilteredSelection)

DEFINE_HOOK(0x73298D, TypeSelectCommand_Execute_UseIFVMode, 0x5)
{
	if (!RulesExt::Global()->TypeSelectUseIFVMode)
		return 0;

	ExtSelection::IFVGroups.clear();

	for (const auto pObject : ObjectClass::CurrentObjects)
	{
		const auto pTechno = abstract_cast<TechnoClass*, true>(pObject);

		if (!pTechno)
			continue;

		const auto pTechnoType = pTechno->GetTechnoType();

		if (!pTechnoType->Gunner)
			continue;

		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
		char* gunnerID = pTypeExt->WeaponGroupAs[pTechno->CurrentWeaponNumber];

		if (!GeneralUtils::IsValidString(gunnerID))
			sprintf_s(gunnerID, 0x20, "%d", pTechno->CurrentWeaponNumber + 1);

		if (std::ranges::none_of(ExtSelection::IFVGroups, [gunnerID](const char* pID) { return !_stricmp(pID, gunnerID); }))
			ExtSelection::IFVGroups.emplace_back(gunnerID);
	}

	return 0;
}
