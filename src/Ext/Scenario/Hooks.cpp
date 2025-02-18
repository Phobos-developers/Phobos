#include <Ext/Scenario/Body.h>
#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

DEFINE_HOOK(0x6870D7, ReadScenario_LoadingScreens, 0x5)
{
	enum { SkipGameCode = 0x6873AB };

	LEA_STACK(CCINIClass*, pINI, STACK_OFFSET(0x174, -0x158));

	auto const pScenario = ScenarioClass::Instance();
	auto const scenarioName = pScenario->FileName;
	auto const defaultsSection = "Defaults";

	pScenario->LS640BriefLocX = pINI->ReadInteger(scenarioName, "LS640BriefLocX", pINI->ReadInteger(defaultsSection, "DefaultLS640BriefLocX", 0));
	pScenario->LS640BriefLocY = pINI->ReadInteger(scenarioName, "LS640BriefLocY", pINI->ReadInteger(defaultsSection, "DefaultLS640BriefLocY", 0));
	pScenario->LS800BriefLocX = pINI->ReadInteger(scenarioName, "LS800BriefLocX", pINI->ReadInteger(defaultsSection, "DefaultLS800BriefLocX", 0));
	pScenario->LS800BriefLocY = pINI->ReadInteger(scenarioName, "LS800BriefLocY", pINI->ReadInteger(defaultsSection, "DefaultLS800BriefLocY", 0));
	pINI->ReadString(defaultsSection, "DefaultLS640BkgdName", pScenario->LS640BkgdName, pScenario->LS640BkgdName, 64);
	pINI->ReadString(scenarioName, "LS640BkgdName", pScenario->LS640BkgdName, pScenario->LS640BkgdName, 64);
	pINI->ReadString(defaultsSection, "DefaultLS800BkgdName", pScenario->LS800BkgdName, pScenario->LS800BkgdName, 64);
	pINI->ReadString(scenarioName, "LS800BkgdName", pScenario->LS800BkgdName, pScenario->LS800BkgdName, 64);
	pINI->ReadString(defaultsSection, "DefaultLS800BkgdPal", pScenario->LS800BkgdPal, pScenario->LS800BkgdPal, 64);
	pINI->ReadString(scenarioName, "LS800BkgdPal", pScenario->LS800BkgdPal, pScenario->LS800BkgdPal, 64);

	return SkipGameCode;
}

#pragma region PlayerAtX

// Map <Player @ X> as object owner name to correct HouseClass index.
DEFINE_HOOK(0x50C186, GetHouseIndexFromName_PlayerAtX, 0x6)
{
	enum { ReturnFromFunction = 0x50C203 };

	GET(const char*, name, ECX);

	// Bail out early in campaign mode or if the name does not start with <
	if (SessionClass::IsCampaign() || *name != '<')
		return 0;

	int playerAtIndex = HouseClass::GetPlayerAtFromString(name);

	if (playerAtIndex != -1)
	{
		auto const pHouse = HouseClass::FindByPlayerAt(playerAtIndex);

		if (pHouse)
		{
			R->EDX(pHouse->ArrayIndex);
			return ReturnFromFunction;
		}
	}

	return 0;
}

// Skip check that prevents buildings from being created for local player.
DEFINE_JUMP(LJMP, 0x44F8D5, 0x44F8E1);

#pragma endregion
