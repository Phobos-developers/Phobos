#include "SaveVariablesToFile.h"

#include <Ext/Scenario/Body.h>
#include <HouseClass.h>

const char* SaveVariablesToFileCommandClass::GetName() const
{
	return "Save Variables to File";
}

const wchar_t* SaveVariablesToFileCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SAVE_VARIABLES", L"Save Variables to File");
}

const wchar_t* SaveVariablesToFileCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* SaveVariablesToFileCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SAVE_VARIABLES_DESC", L"Save local & global variables to an INI file.");
}

void SaveVariablesToFileCommandClass::Execute(WWKey eInput) const
{
	MessageListClass::Instance->PrintMessage(
		L"Variables saved.",
		RulesClass::Instance->MessageDelay,
		HouseClass::CurrentPlayer->ColorSchemeIndex,
		true
	);

	ScenarioExt::ExtData::SaveVariablesToFile(false);
	ScenarioExt::ExtData::SaveVariablesToFile(true);
}
