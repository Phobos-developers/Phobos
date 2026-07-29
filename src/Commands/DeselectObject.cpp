#pragma once
#include "DeselectObject.h"
#include <Utilities/GeneralUtils.h>

const char* DeselectObjectCommandClass::GetName() const
{
	return "DeselectOne";
}

const wchar_t* DeselectObjectCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DESELECT", L"Deselect 1 Object");
}

const wchar_t* DeselectObjectCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* DeselectObjectCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DESELECT_DESC", L"Deselect 1 object from current selection.");
}

void DeselectObjectCommandClass::Execute(WWKey eInput) const
{
	const int nCount = ObjectClass::CurrentObjects.Count;

	if (nCount > 0)
		ObjectClass::CurrentObjects.GetItem(0)->Deselect();
}
