#pragma once
#include "DeselectObject5.h"
#include <Utilities/GeneralUtils.h>

const char* DeselectObject5CommandClass::GetName() const
{
	return "DeselectFive";
}

const wchar_t* DeselectObject5CommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DESELECT5", L"Deselect 5 Objects");
}

const wchar_t* DeselectObject5CommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* DeselectObject5CommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DESELECT5_DESC", L"Deselect 5 objects from current selection.");
}

void DeselectObject5CommandClass::Execute(WWKey eInput) const
{
	int nCount = ObjectClass::CurrentObjects.Count;

	if (nCount > 0)
	{
		int max = nCount >= 5 ? 5 : nCount;
		for (int i = max - 1; i >= 0; i--)
		{
			ObjectClass::CurrentObjects.GetItem(i)->Deselect();
		}
	}
}
