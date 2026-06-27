#include "ToggleUnitPassengers.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>
#include <MessageListClass.h>
#include <HouseClass.h>

const char* ToggleUnitPassengersCommandClass::GetName() const
{
	return "Toggle Unit Passengers";
}

const wchar_t* ToggleUnitPassengersCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_UNIT_PASSENGERS", L"Toggle Unit Passengers");
}

const wchar_t* ToggleUnitPassengersCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleUnitPassengersCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_UNIT_PASSENGERS_DESC", L"Show/hide unit passengers display.");
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

	const auto pRulesExt = RulesExt::Global();

	if (Phobos::Config::UnitPassengers_Enable)
	{
		const auto& msg = pRulesExt->ShowUnitPassengers_EnabledMessage;
		if (msg.isset())
			PrintMessage(GeneralUtils::LoadStringUnlessMissing(msg.Get().Label, L"Unit Passengers display: Enabled"));
	}
	else
	{
		const auto& msg = pRulesExt->ShowUnitPassengers_DisabledMessage;
		if (msg.isset())
			PrintMessage(GeneralUtils::LoadStringUnlessMissing(msg.Get().Label, L"Unit Passengers display: Disabled"));
	}
}