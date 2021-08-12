#include <Helpers/Macro.h>

#include "Body.h"

#include <TagClass.h>

DEFINE_HOOK(0x689910, ScenarioClass_SetLocalToByID, 0x5)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(const char, bState, 0x8);

	auto itr = ScenarioExt::Global()->LocalVariables.find(nIndex);

	if (itr != ScenarioExt::Global()->LocalVariables.end() && itr->second.Value != bState)
	{
		itr->second.Value = bState;
		ScenarioClass::Instance->VariablesChanged = true;
		TagClass::NotifyLocalChanged(nIndex);
	}

	return 0x689955;
}

DEFINE_HOOK(0x689A00, ScenarioClass_GetLocalStateByID, 0x6)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(char*, pOut, 0x8);

	auto itr = ScenarioExt::Global()->LocalVariables.find(nIndex);
	if (itr != ScenarioExt::Global()->LocalVariables.end())
		*pOut = itr->second.Value;

	return 0x689A26;
}

DEFINE_HOOK(0x689B20, ScenarioClass_ReadLocalVariables, 0x6)
{
	GET_STACK(CCINIClass*, pINI, 0x4);

	int nCount = pINI->GetKeyCount("VariableNames");
	for (int i = 0; i < nCount; ++i)
	{
		auto pKey = pINI->GetKeyName("VariableNames", i);
		int nIndex;
		if (sscanf_s(pKey, "%d", &nIndex) == 1)
		{
			auto& var = ScenarioExt::Global()->LocalVariables[nIndex];
			pINI->ReadString("VariableNames", pKey, pKey, Phobos::readBuffer);
			strcpy(var.Name, strtok(Phobos::readBuffer, ","));
			if (auto pState = strtok(nullptr, ","))
				var.Value = atoi(pState) != 0;
			else
				var.Value = 0;
		}
	}

	return 0x689C4B;
}

DEFINE_HOOK(0x689670, ScenarioClass_SetGlobalToByID, 0x5)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(const char, bState, 0x8);

	auto itr = ScenarioExt::Global()->GlobalVariables.find(nIndex);

	if (itr != ScenarioExt::Global()->GlobalVariables.end() && itr->second.Value != bState)
	{
		itr->second.Value = bState;
		ScenarioClass::Instance->VariablesChanged = true;
		TagClass::NotifyGlobalChanged(nIndex);
	}

	return 0x6896AF;
}

DEFINE_HOOK(0x689760, ScenarioClass_GetGlobalStateByID, 0x6)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(char*, pOut, 0x8);

	auto itr = ScenarioExt::Global()->GlobalVariables.find(nIndex);
	if (itr != ScenarioExt::Global()->GlobalVariables.end())
		*pOut = itr->second.Value;

	return 0x689786;
}

DEFINE_HOOK(0x689880, ScenarioClass_ReadGlobalVariables, 0x6)
{
	GET_STACK(CCINIClass*, pINI, 0x4);

	int nCount = pINI->GetKeyCount("VariableNames");
	for (int i = 0; i < nCount; ++i)
	{
		auto pKey = pINI->GetKeyName("VariableNames", i);
		int nIndex;
		if (sscanf_s(pKey, "%d", &nIndex) == 1)
		{
			auto& var = ScenarioExt::Global()->GlobalVariables[nIndex];
			pINI->ReadString("VariableNames", pKey, pKey, Phobos::readBuffer);
			strcpy(var.Name, strtok(Phobos::readBuffer, ","));
			if (auto pState = strtok(nullptr, ","))
				var.Value = atoi(pState) != 0;
			else
				var.Value = 0;
		}
	}

	return 0x6898FF;
}