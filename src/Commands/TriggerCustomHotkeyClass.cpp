#include "TriggerCustomHotkeyClass.h"
#include <Utilities/GeneralUtils.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Event/Body.h>

void TriggerCustomHotkeyClass::SetNumeralSequence(int numeralSequence)
{
	NumeralSequence = numeralSequence;
	_snprintf_s(ButtonName, sizeof(ButtonName), "Trigger.CustomHotkey_%i", numeralSequence);
	_snprintf_s(UIName, sizeof(UIName), "TXT_CUSTOM_HOTKEY_%i", numeralSequence);
	_snprintf_s(UIDescription, sizeof(UIDescription), "TXT_CUSTOM_HOTKEY_%i_DESC", numeralSequence);
	mbstowcs(UINameFallback, UIName, sizeof(UIName));
	mbstowcs(UIDescriptionFallback, UIDescription, sizeof(UIDescription));
}

const char* TriggerCustomHotkeyClass::GetName() const
{
	return ButtonName;
}

const wchar_t* TriggerCustomHotkeyClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing(UIName, UINameFallback);
}

const wchar_t* TriggerCustomHotkeyClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* TriggerCustomHotkeyClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing(UIDescription, UIDescriptionFallback);
}

void TriggerCustomHotkeyClass::Execute(WWKey eInput) const
{
	static PhobosMap<int, EventTypeClass*> CachedEventTypeMap;

	if (!CachedEventTypeMap.contains(NumeralSequence))
	{
		static char tempBuffer[32];
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "CustomHotkey_%i", NumeralSequence);
		// Since if this event type is not in use, there is no need to fire it in the first place.
		// Thus, we only find here, we don't allocate.
		auto pEventType = EventTypeClass::Find(tempBuffer);
		CachedEventTypeMap.insert(NumeralSequence, pEventType);
	}

	auto pEventType = CachedEventTypeMap.get_or_default(NumeralSequence, nullptr);

	// If this event type is not in use, we don't need to do anything.
	if (!pEventType)
		return;

	for (const auto& pUnit : ObjectClass::CurrentObjects())
	{
		// try to cast to TechnoClass
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pUnit);

		// if not a techno or is in berserk or is not controlled by the local player then ignore it
		if (!pTechno || pTechno->Berzerk || !pTechno->Owner->IsControlledByCurrentPlayer())
			continue;

		if (auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno))
		{
			// if it can't handle the event why bother to send a network event
			if (pTechnoExt->CanHandleEvent(pEventType))
				EventExt::RaiseTriggerCustomHotkey(pTechno, HouseClass::CurrentPlayer, NumeralSequence);
		}
	}

	for (HouseClass* pHouse : *HouseClass::Array)
	{
		if (pHouse->IsControlledByCurrentPlayer())
		{
			if (auto pHouseExt = HouseExt::ExtMap.Find(pHouse))
			{
				// if it can't handle the event why bother to send a network event
				if (pHouseExt->CanHandleEvent(pEventType))
					EventExt::RaiseTriggerCustomHotkey(pHouse, HouseClass::CurrentPlayer, NumeralSequence);
			}
		}
	}
}
