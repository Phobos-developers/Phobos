#include "TogglePassengers.h"

#include <Utilities/GeneralUtils.h>
#include <MessageListClass.h>
#include <HouseClass.h>

const char* TogglePassengersCommandClass::GetName() const
{
	return "Toggle Passengers";
}

const wchar_t* TogglePassengersCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_PASSENGERCAMEO", L"Toggle Passenger Cameos");
}

const wchar_t* TogglePassengersCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* TogglePassengersCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_PASSENGERCAMEO_DESC", L"Show/hide passenger cameos display.");
}

void TogglePassengersCommandClass::Execute(WWKey eInput) const
{
	Phobos::Config::ShowPassengers_Enable = !Phobos::Config::ShowPassengers_Enable;

	auto PrintMessage = [](const wchar_t* pMessage)
	{
		MessageListClass::Instance.PrintMessage(
			pMessage,
			RulesClass::Instance->MessageDelay,
			HouseClass::CurrentPlayer->ColorSchemeIndex,
			true
		);
	};

	if (Phobos::Config::ShowPassengers_Enable)
		PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:PassengerCameoDisplayEnabled", L"Passenger Cameos Display: Enabled"));
	else
		PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:PassengerCameoDisplayDisabled", L"Passenger Cameos Display: Disabled"));
}