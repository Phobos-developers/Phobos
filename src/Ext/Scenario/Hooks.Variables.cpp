#include "Body.h"
#include <Misc/MapSelectionClass.h>

DEFINE_HOOK(0x689910, ScenarioClass_SetLocalToByID, 0x5)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(const char, bState, 0x8);

	ScenarioExt::Global()->SetVariableToByID(false, nIndex, bState);

	return 0x689955;
}

DEFINE_HOOK(0x689A00, ScenarioClass_GetLocalStateByID, 0x6)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(char*, pOut, 0x8);

	ScenarioExt::Global()->GetVariableStateByID(false, nIndex, pOut);

	return 0x689A26;
}

DEFINE_HOOK(0x689B20, ScenarioClass_ReadLocalVariables, 0x6)
{
	GET_STACK(CCINIClass*, pINI, 0x4);

	ScenarioExt::Global()->ReadVariables(false, pINI);

	return 0x689C4B;
}

DEFINE_HOOK(0x689670, ScenarioClass_SetGlobalToByID, 0x5)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(const char, bState, 0x8);

	ScenarioExt::Global()->SetVariableToByID(true, nIndex, bState);

	return 0x6896AF;
}

DEFINE_HOOK(0x689760, ScenarioClass_GetGlobalStateByID, 0x6)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(char*, pOut, 0x8);

	ScenarioExt::Global()->GetVariableStateByID(true, nIndex, pOut);

	return 0x689786;
}

// Called by MapGeneratorClass
DEFINE_HOOK(0x689880, ScenarioClass_ReadGlobalVariables, 0x6)
{
	GET_STACK(CCINIClass*, pINI, 0x4);

	ScenarioExt::Global()->ReadVariables(true, pINI);

	return 0x6898FF;
}

DEFINE_HOOK(0x685354, ClearLotsOfShit_GlobalVariable, 0x9)
{
	for (auto& [idx, var] : ScenarioExt::Global()->Variables[1])
	{
		if (var.Value)
		{
			var.Value = 0;
			ScenarioClass::Instance->VariablesChanged = true;
			TagClass::NotifyGlobalChanged(idx);
		}
	}
	return 0x68538D;
}

std::vector<std::pair<int, int>> CarryOverGlobalsBuffer {};

DEFINE_HOOK(0x4C6217, EvadeClass_DoShit_Globals, 0x0)
{
	for (auto const& [idx, var] : CarryOverGlobalsBuffer)
		ScenarioExt::Global()->SetVariableToByID(true, idx, static_cast<char>(var));
	return 0x4C622F;
}

DEFINE_HOOK(0x4C6185, EvadeClass_CarryOverShit_Globals, 0x0)
{
	CarryOverGlobalsBuffer.clear();
	for (auto const& [idx, var] : ScenarioExt::Global()->Variables[1])
		CarryOverGlobalsBuffer.emplace_back(idx, var.Value);
	return 0x4C61A3;
}

// ScenarioClass_ReadGlobalVariables inlined in Read_Scenario_INI
DEFINE_HOOK(0x6876C2, ReadScenarioINI_Inlined_ReadGlobalVariables, 0x6)
{
	ScenarioExt::Global()->ReadVariables(true, CCINIClass::INI_Rules);

	// Stupid inline
	R->ESI(GameMode::Campaign);

	return 0x68773F;
}

//IDK why but you can't do it at the beginning, just random place in the middle
DEFINE_HOOK_AGAIN(0x6857EA, PhobosSaveVariables, 0x5)//Win
DEFINE_HOOK(0x685EB1, PhobosSaveVariables, 0x5)//Lose
{
	if (Phobos::Config::SaveVariablesOnScenarioEnd)
	{
		ScenarioExt::ExtData::SaveVariablesToFile(false);
		ScenarioExt::ExtData::SaveVariablesToFile(true);
	}

	return 0;
}

#pragma region MapSelectExtension

static bool __fastcall MapSelectClass_SetNextScenario_CustomMission(MapSelectClass* pThis, void* _, ScenarioClass* pItem)
{
	// It can be directly filled in the map file name without being restricted by mapselmd.ini.
	if (pItem->SkipMapSelect)
	{
		std::string scenario {};
		auto const pGlobal = ScenarioExt::Global();

		do
		{
			if (strcmp(ScenarioClass::Instance->AltNextScenario, ""))
			{
				const auto& localVariables = pGlobal->Variables[false];
				const auto& globalVariables = pGlobal->Variables[true];
				bool success = false;

				if (!localVariables.empty())
				{
					for (const auto& itr : localVariables)
					{
						if (strcmp(itr.second.Name, "<Alternate Next Scenario>") || itr.second.Value <= 0)
							continue;

						success = true;
						break;
					}
				}

				if (!success && !globalVariables.empty())
				{
					for (const auto& itr : globalVariables)
					{
						if (strcmp(itr.second.Name, "<Alternate Next Scenario>") || itr.second.Value <= 0)
							continue;

						success = true;
						break;
					}

					if (!success && globalVariables.contains(1) && globalVariables.find(1)->second.Value > 0)
						success = true;
				}

				if (success)
				{
					scenario = pItem->AltNextScenario;
					break;
				}
			}

			scenario = pItem->NextScenario;
		}
		while (false);

		if (!scenario.empty())
		{
			// Does this Stage have any function ? If anyone knows, please let me know.
			pItem->Stage = 0;
			strncpy(pItem->FileName, scenario.c_str(), 0x104u);
			pItem->FileName[259] = '\0';

			return true;
		}

		return false;
	}

	// Try opening our interactive Map Selection Screen if SkipMapSelect is false
	if (MapSelectionClass::OpenMapSelectionWindow(pItem))
	{
		return true;
	}

	// Safe fallback to NextScenario if map selection was cancelled or empty (prevents crashing on dead vanilla code)
	const char* nextScenario = (pItem->NextScenario && pItem->NextScenario[0]) ? pItem->NextScenario : pItem->FileName;
	if (nextScenario && nextScenario[0])
	{
		pItem->Stage = 0;
		strncpy(pItem->FileName, nextScenario, 0x104u);
		pItem->FileName[259] = '\0';
		return true;
	}

	return false;
}

DEFINE_JUMP(LJMP, 0x685A38, 0x685A63)	// Skip the original code
DEFINE_FUNCTION_JUMP(CALL, 0x5ADD63, MapSelectClass_SetNextScenario_CustomMission)

#pragma endregion
