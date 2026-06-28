#include "ToggleUnitPassengers.h"

#include <Utilities/GeneralUtils.h>
#include <MessageListClass.h>
#include <HouseClass.h>

const char* ToggleUnitPassengersCommandClass::GetName() const
{
	return "Toggle Unit Passengers";
}

const wchar_t* ToggleUnitPassengersCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_UNIT_PASSENGERS", L"Toggle Unit Passenger Icons");
}

const wchar_t* ToggleUnitPassengersCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleUnitPassengersCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_UNIT_PASSENGERS_DESC", L"Show/hide unit passenger icons display.");
}

void ToggleUnitPassengersCommandClass::Execute(WWKey eInput) const
{
	Phobos::Config::UnitPassengers_Enable = !Phobos::Config::UnitPassengers_Enable;

	auto PrintMessage = [](const wchar_t* pMessage)
	{
		MessageListClass::Instance.PrintMessage(
			pMessage,
			RulesClass::Instance->MessageDelay,
			HouseClass::CurrentPlayer->ColorSchemeIndex,
			true
		);
	};

	if (Phobos::Config::UnitPassengers_Enable)
		PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:UnitPassengersEnabled", L"Unit Passenger Icons Display: Enabled"));
	else
		PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:UnitPassengersDisabled", L"Unit Passenger Icons Display: Disabled"));
}