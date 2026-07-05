#include "ToggleCameos.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>
#include <MessageListClass.h>
#include <HouseClass.h>

const char* ToggleCameosCommandClass::GetName() const
{
	return "Toggle Cameos";
}

const wchar_t* ToggleCameosCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SHOWCAMEO", L"Toggle Cameos");
}

const wchar_t* ToggleCameosCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleCameosCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SHOWCAMEO_DESC", L"Show/hide cameo display.");
}

void ToggleCameosCommandClass::Execute(WWKey eInput) const
{
	// Ignore hotkey in permanent mode
	if (RulesExt::Global()->ShowCameo)
		return;

	// Ignore hotkey if toggleable is off
	if (!RulesExt::Global()->ShowCameo_Toggleable)
		return;

	Phobos::Config::ShowCameo_Enable = !Phobos::Config::ShowCameo_Enable;

	auto PrintMessage = [](const wchar_t* pMessage)
	{
		MessageListClass::Instance.PrintMessage(
			pMessage,
			RulesClass::Instance->MessageDelay,
			HouseClass::CurrentPlayer->ColorSchemeIndex,
			true
		);
	};

	if (Phobos::Config::ShowCameo_Enable)
		PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:ShowCameoEnabled", L"Cameos Display: Enabled"));
	else
		PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:ShowCameoDisabled", L"Cameos Display: Disabled"));
}