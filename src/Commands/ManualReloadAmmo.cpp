#include "ManualReloadAmmo.h"

#include <HouseClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Event/Body.h>

const char* ManualReloadAmmoCommandClass::GetName() const
{
	return "Manual Reload Ammo";
}

const wchar_t* ManualReloadAmmoCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_MANUAL_RELOAD", L"Manual Reload Ammo");
}

const wchar_t* ManualReloadAmmoCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* ManualReloadAmmoCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_MANUAL_RELOAD_DESC", L"Manual Reload Ammo");
}

void ManualReloadAmmoCommandClass::Execute(WWKey eInput) const
{
	for (const auto& pObj : ObjectClass::CurrentObjects)
	{
		if (const auto pTechno = abstract_cast<TechnoClass*>(pObj))
		{
			if (pTechno->Owner->IsControlledByCurrentPlayer())
				EventExt::RaiseManualReloadEvent(pTechno);
		}
	}
}
