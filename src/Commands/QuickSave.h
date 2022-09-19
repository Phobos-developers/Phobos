#pragma once
#include <ScenarioClass.h>
#include <SessionClass.h>
#include <HouseClass.h>

#include "Commands.h"
#include <Utilities/GeneralUtils.h>

class QuickSaveCommandClass : public PhobosCommandClass
{
public:
	// CommandClass
	virtual const char* GetName() const override
	{
		return "Quicksave";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE", L"Quicksave");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_INTERFACE", L"Interface");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_DESC", L"Save the current game (Singleplayer only).");
	}

	virtual void Execute(WWKey eInput) const override
	{
		auto PrintMessage = [](const wchar_t* pMessage)
		{
			MessageListClass::Instance->PrintMessage(
				pMessage,
				RulesClass::Instance->MessageDelay,
				HouseClass::CurrentPlayer->ColorSchemeIndex,
				true
			);
		};

		if (SessionClass::Instance->GameMode == GameMode::Campaign || SessionClass::Instance->GameMode == GameMode::Skirmish)
		{
			char fName[0x80];

			SYSTEMTIME time;
			GetLocalTime(&time);

			_snprintf_s(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.sav",
				time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

			PrintMessage(StringTable::LoadString("TXT_SAVING_GAME"));

			wchar_t fDescription[0x80] = { 0 };
			if (SessionClass::Instance->GameMode == GameMode::Campaign)
				wcscpy_s(fDescription, ScenarioClass::Instance->UINameLoaded);
			else
				wcscpy_s(fDescription, ScenarioClass::Instance->Name);
			wcscat_s(fDescription, L" - ");
			wcscat_s(fDescription, GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_SUFFIX", L"Quicksaved"));

			if (ScenarioClass::SaveGame(fName, fDescription))
				PrintMessage(StringTable::LoadString("TXT_GAME_WAS_SAVED"));
			else
				PrintMessage(StringTable::LoadString("TXT_ERROR_SAVING_GAME"));
		}
		else
		{
			PrintMessage(StringTable::LoadString("MSG:NotAvailableInMultiplayer"));
		}
	}
};
