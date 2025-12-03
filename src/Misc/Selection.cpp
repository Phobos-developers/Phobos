#include "Phobos.h"
#include "Utilities/Macro.h"
#include <Utilities/AresHelper.h>
#include "Ext/Techno/Body.h"
#include "Ext/TechnoType/Body.h"

#include <TacticalClass.h>
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

	static inline bool ProcessingIDMatches = false;
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
					auto const& pExt = TechnoExt::ExtMap.Find(static_cast<TechnoClass*>(selected.Object));
					auto const& pTypeExt = TechnoTypeExt::ExtMap.Find(selected.Object->GetTechnoType());

					bool isLowPriorityByAttachment = pExt->ParentAttachment && pExt->ParentAttachment->GetType()->LowSelectionPriority;
					if (!pTypeExt->LowSelectionPriority && !isLowPriorityByAttachment)
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
					if (bPriorityFiltering)
					{
						auto const& pExt = TechnoExt::ExtMap.Find(static_cast<TechnoClass*>(pObject));
						// Attached units shouldn't be selected regardless of the setting
						bool isLowPriorityByAttachment = pExt->ParentAttachment && pExt->ParentAttachment->GetType()->LowSelectionPriority;
						bool isLowPriorityByTechno = Phobos::Config::PrioritySelectionFiltering && pTypeExt->LowSelectionPriority;

						if (isLowPriorityByAttachment || isLowPriorityByTechno)
							continue;
					}

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

	static bool __fastcall TechnoClass_IDMatches(TechnoClass* pTechno, DynamicVectorClass<const char*>& names)
	{
		bool result = false;

		do
		{
			const auto pTechnoType = pTechno->GetTechnoType();
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
			const char* id = pTypeExt->GetSelectionGroupID();

			if (std::ranges::none_of(names, [id](const char* pID) { return !_stricmp(pID, id); }))
				break;

			if (pTechnoType->Gunner && !ExtSelection::IFVGroups.empty())
			{
				char* gunnerID = pTypeExt->WeaponGroupAs[pTechno->CurrentWeaponNumber];

				if (!GeneralUtils::IsValidString(gunnerID))
					sprintf_s(gunnerID, 0x20, "%d", RulesExt::Global()->TypeSelectUseIFVMode ? pTechno->CurrentWeaponNumber + 1 : 0);

				if (std::ranges::none_of(ExtSelection::IFVGroups, [gunnerID](const char* pID) { return !_stricmp(pID, gunnerID); }))
					break;
			}

			result = pTechno->CanBeSelectedNow() || (pTechno->WhatAmI() == BuildingClass::AbsID && pTechnoType->UndeploysInto);
		}
		while (false);

		ExtSelection::ProcessingIDMatches = false;
		return result;
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

			bool bPriorityFiltering = Tactical_IsHighPriorityInRect(pThis, &rect);
			Tactical_SelectFiltered(pThis, &rect, check_callback, bPriorityFiltering);

			pThis->Band.Left = 0;
			pThis->Band.Top = 0;
		}
	}
};

DEFINE_FUNCTION_JUMP(LJMP, 0x732C30, ExtSelection::TechnoClass_IDMatches)

// Replace single call
DEFINE_FUNCTION_JUMP(CALL, 0x4ABCEB, ExtSelection::Tactical_MakeFilteredSelection)

// Replace vanilla function. For in case another module tries to call the vanilla function at offset
DEFINE_FUNCTION_JUMP(LJMP, 0x6D9FF0, ExtSelection::Tactical_MakeFilteredSelection)

DEFINE_HOOK(0x73298D, TypeSelectExecute_UseIFVMode, 0x5)
{
	const bool useIFVMode = RulesExt::Global()->TypeSelectUseIFVMode;

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
			sprintf_s(gunnerID, 0x20, "%d", useIFVMode ? pTechno->CurrentWeaponNumber + 1 : 0);

		if (std::ranges::none_of(ExtSelection::IFVGroups, [gunnerID](const char* pID) { return !_stricmp(pID, gunnerID); }))
			ExtSelection::IFVGroups.emplace_back(gunnerID);
	}

	return 0;
}

DEFINE_HOOK(0x732C06, TypeSelectExecute_Clear, 0x6)
{
	ExtSelection::IFVGroups.clear();
	return 0;
}

#pragma region BuildingTypeSelectable

DEFINE_HOOK_AGAIN(0x732B28, TypeSelectExecute_SetContext, 0x6)
DEFINE_HOOK(0x732A85, TypeSelectExecute_SetContext, 0x7)
{
	ExtSelection::ProcessingIDMatches = true;
	return 0;
}

// If the context is set as well as the flags is enabled, this will make the vfunc CanBeSelectedNow return true to enable the type selection.
DEFINE_HOOK(0x465D40, BuildingClass_Is1x1AndUndeployable_BuildingMassSelectable, 0x6)
{
	enum { SkipGameCode = 0x465D6A };

	// Since Ares hooks around, we have difficulty juggling Ares and no Ares.
	// So we simply disable this feature if no Ares.
	if (!AresHelper::CanUseAres)
		return 0;

	if (!ExtSelection::ProcessingIDMatches || !RulesExt::Global()->BuildingTypeSelectable)
		return 0;

	R->EAX(true);
	return SkipGameCode;
}

#pragma endregion
