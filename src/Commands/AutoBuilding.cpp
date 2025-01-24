#include "AutoBuilding.h"

#include <HouseClass.h>
#include <MessageListClass.h>
#include <Utilities/GeneralUtils.h>

const char* AutoBuildingCommandClass::GetName() const
{
	return "Auto Building";
}

const wchar_t* AutoBuildingCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_BUILD", L"Toggle Auto Building");
}

const wchar_t* AutoBuildingCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* AutoBuildingCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_BUILD_DESC", L"Toggle on/off automatically place building");
}

void AutoBuildingCommandClass::Execute(WWKey eInput) const
{
	Phobos::Config::AutoBuilding_Enable = !Phobos::Config::AutoBuilding_Enable;

	MessageListClass::Instance->PrintMessage(
		Phobos::Config::AutoBuilding_Enable ? L"Auto Building Switch On." : L"Auto Building Switch Off.",
		150,
		HouseClass::CurrentPlayer->ColorSchemeIndex,
		true
	);
}
