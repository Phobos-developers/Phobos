#include <TriggerClass.h>
#include <TriggerTypeClass.h>

#include <Helpers/Macro.h>

#include <Ext/Scenario/Body.h>
#include <Ext/TEvent/Body.h>

DEFINE_HOOK(0x727064, TriggerTypeClass_HasLocalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return nIndex >= PhobosTriggerEvent::LocalVariableGreaterThan && nIndex <= PhobosTriggerEvent::LocalVariableAndIsTrue
		|| nIndex >= PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable && nIndex >= PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable
		|| nIndex >= PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable && nIndex >= PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable
		|| nIndex == static_cast<int>(TriggerEvent::LocalSet)
		? 0x72706E
		: 0x727069;
}

DEFINE_HOOK(0x727024, TriggerTypeClass_HasGlobalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThan && nIndex <= PhobosTriggerEvent::GlobalVariableAndIsTrue
		|| nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable && nIndex >= PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable
		|| nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable && nIndex >= PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable
		|| nIndex == static_cast<int>(TriggerEvent::GlobalSet)
		? 0x72702E
		: 0x727029;
}

#pragma region PlayerAtX

// Store player slot index for trigger type if such value is used in scenario INI.
DEFINE_HOOK(0x727292, TriggerTypeClass_ReadINI_PlayerAtX, 0x5)
{
	GET(TriggerTypeClass*, pThis, EBP);
	GET(const char*, pID, ESI);

	// Bail out early in campaign mode or if the name does not start with <
	if (SessionClass::IsCampaign() || *pID != '<')
		return 0;

	const int playerAtIndex = HouseClass::GetPlayerAtFromString(pID);

	if (playerAtIndex != -1)
	{
		ScenarioExt::Global()->TriggerTypePlayerAtXOwners.emplace(pThis->ArrayIndex, playerAtIndex);

		// Override the name to prevent Ares whining about non-existing HouseType names.
		R->ESI(NONE_STR);
	}

	return 0;
}

// Handle mapping player slot index for trigger to HouseClass pointer in logic.
DEFINE_HOOK_AGAIN(0x7265F7, TriggerClass_Logic_PlayerAtX, 0x6)
DEFINE_HOOK(0x72652D, TriggerClass_Logic_PlayerAtX, 0x6)
{
	enum { SkipGameCode1 = 0x726538, SkipGameCode2 = 0x726602};

	GET(TriggerTypeClass*, pType, EDX);

	if (SessionClass::IsCampaign())
		return 0;

	auto const& triggerOwners = ScenarioExt::Global()->TriggerTypePlayerAtXOwners;
	auto it = triggerOwners.find(pType->ArrayIndex);

	if (it != triggerOwners.end())
	{
		if (auto const pHouse = HouseClass::FindByPlayerAt(it->second))
		{
			R->EAX(pHouse);
			return R->Origin() == 0x72652D ? SkipGameCode1 : SkipGameCode2;
		}
	}
	
	return 0;
}

// Destroy triggers with Player @ X owners if they are not present in scenario.
DEFINE_HOOK(0x725FC7, TriggerClass_CTOR_PlayerAtX, 0x7)
{
	GET(TriggerClass*, pThis, ESI);

	if (SessionClass::IsCampaign())
		return 0;

	auto& triggerOwners = ScenarioExt::Global()->TriggerTypePlayerAtXOwners;
	auto it = triggerOwners.find(pThis->Type->ArrayIndex);

	if (it != triggerOwners.end())
	{
		if (!HouseClass::FindByPlayerAt(it->second))
			pThis->Destroy();
	}

	return 0;
}

// Remove destroyed triggers from the map.
DEFINE_HOOK(0x726727, TriggerClass_Destroy_PlayerAtX, 0x5)
{
	GET(TriggerClass*, pThis, ESI);

	if (SessionClass::IsCampaign())
		return 0;

	auto& triggerOwners = ScenarioExt::Global()->TriggerTypePlayerAtXOwners;
	auto it = triggerOwners.find(pThis->Type->ArrayIndex);

	if (it != triggerOwners.end())
		triggerOwners.erase(it);

	return 0;
}

#pragma endregion
