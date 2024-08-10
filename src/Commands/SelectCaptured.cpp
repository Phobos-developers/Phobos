#include "SelectCaptured.h"

#include <BuildingTypeClass.h>
#include <MessageListClass.h>
#include <MapClass.h>
#include <ObjectClass.h>
#include <TacticalClass.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

const char* SelectCapturedCommandClass::GetName() const
{
	return "Select Captured Units";
}

const wchar_t* SelectCapturedCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECT_CAPTURED", L"Select Captured Units");
}

const wchar_t* SelectCapturedCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* SelectCapturedCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECT_CAPTURED_DESC", L"Select the captured units in the current screen.");
}

void SelectCapturedCommandClass::Execute(WWKey eInput) const
{
	
	MapClass::Instance->SetTogglePowerMode(0);
	MapClass::Instance->SetWaypointMode(0, false);
	MapClass::Instance->SetRepairMode(0);
	MapClass::Instance->SetSellMode(0);

	auto pFirstObject = MapClass::Instance->NextObject(
		ObjectClass::CurrentObjects->Count ? ObjectClass::CurrentObjects->GetItem(0) : nullptr);

	bool CapturedPresent = false;
	auto pCurrentObject = pFirstObject;

	do
	{
		if (auto pTechno = abstract_cast<TechnoClass*>(pCurrentObject))
		{
			Point2D coordInScreen { 0 , 0 };
			CoordStruct coordInMap { 0 , 0 , 0 };
			pTechno->GetCoords( &coordInMap );
			TacticalClass::Instance->CoordsToScreen( &coordInScreen , &coordInMap );
			coordInScreen -= TacticalClass::Instance->TacticalPos;
			RectangleStruct screenArea = DSurface::Composite->GetRect();
			if (screenArea.Width >= coordInScreen.X && screenArea.Height >= coordInScreen.Y && coordInScreen.X >=0 && coordInScreen.Y >= 0 && // the unit is in the current screen
				pTechno->IsMindControlled() && pTechno->IsSelectable() && !pTechno->MindControlledByAUnit) // the unit is mc by non-perma mc, and selectable.
			{
				if (!CapturedPresent)
				{
					CapturedPresent = true;
					MapClass::UnselectAll();
				}
				pCurrentObject->Select();
			}
		}

		pCurrentObject = MapClass::Instance->NextObject(pCurrentObject);
	}
	while (pCurrentObject != pFirstObject);

	if (CapturedPresent)
	{
		MapClass::Instance->MarkNeedsRedraw(1);
		MessageListClass::Instance->PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:SelectCaptured", L"Captured units selected."), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	}
	else
	{
		MessageListClass::Instance->PrintMessage(StringTable::LoadString("MSG:NothingSelected"), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	}
}
