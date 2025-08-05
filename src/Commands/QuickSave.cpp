#include "QuickSave.h"

#include <ScenarioClass.h>
#include <HouseClass.h>
#include <SessionClass.h>
#include <EventClass.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/SpawnerHelper.h>

const char* QuickSaveCommandClass::GetName() const
{
	return "Quicksave";
}

const wchar_t* QuickSaveCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE", L"Quicksave");
}

const wchar_t* QuickSaveCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* QuickSaveCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_DESC", L"Save the current game.");
}

void QuickSaveCommandClass::Execute(WWKey eInput) const
{
	auto PrintMessage = [](const wchar_t* pMessage)
	{
		MessageListClass::Instance.PrintMessage(
			pMessage,
			RulesClass::Instance->MessageDelay,
			HouseClass::CurrentPlayer->ColorSchemeIndex,
			true
		);
	};

	if (SessionClass::IsSingleplayer())
	{
		Phobos::ScheduleGameSave(GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_SUFFIX", L"Quicksaved"));
	}
	else if (SpawnerHelper::IsSaveGameEventHooked())
	{
		// Relinquish handling of the save game to spawner
		EventClass::OutList.Add(EventClass { HouseClass::CurrentPlayer->ArrayIndex, EventType::SaveGame });
	}
	else
	{
		PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:NotAvailableInMultiplayer", L"QuickSave is not available in multiplayer"));
	}
}
