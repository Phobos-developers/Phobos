#include <array>
#include <TriggerClass.h>
#include <TriggerTypeClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <Ext/Scenario/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Trigger/Body.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x727064, TriggerTypeClass_HasLocalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	// 512-529 compare a local variable against a local or global one, or a global variable against a local one
	return nIndex >= PhobosTriggerEvent::LocalVariableGreaterThan && nIndex <= PhobosTriggerEvent::LocalVariableAndIsTrue
		|| nIndex >= PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable && nIndex <= PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable
		|| nIndex == static_cast<int>(TriggerEvent::LocalSet)
		? 0x72706E
		: 0x727069;
}

DEFINE_HOOK(0x727024, TriggerTypeClass_HasGlobalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	// 518-535 compare a global variable against a local or global one, or a local variable against a global one
	return nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThan && nIndex <= PhobosTriggerEvent::GlobalVariableAndIsTrue
		|| nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable && nIndex <= PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable
		|| nIndex == static_cast<int>(TriggerEvent::GlobalSet)
		? 0x72702E
		: 0x727029;
}

static bool __fastcall TriggerClass_RegisterEvent_Wrapper(
	TriggerClass* pThis,
	void* _,
	TriggerEvent nEvent,
	ObjectClass* pObject,
	bool forceFire,
	bool isPersistent,
	TechnoClass* pSource)
{
	if (!pThis || !pThis->Enabled || pThis->Destroyed || !pThis->Type)
		return false;

	if (forceFire)
	{
		if (isPersistent)
		{
			pThis->ResetTimers();
			if (auto pExt = TriggerExt::TryFetch(pThis))
				pExt->ResetAllTimers();
		}
		return true;
	}

	auto const pFirstEvent = pThis->Type->FirstEvent;
	if (!pFirstEvent)
		return false;

	// Collect all events in original INI order
	// In YR, OccuredEvents is a 32-bit bitfield, so max 32 events per trigger
	constexpr size_t MaxEvents = 32;
	std::array<TEventClass*, MaxEvents> events;
	size_t eventCount = 0;
	for (auto pEvent = pFirstEvent; pEvent && eventCount < MaxEvents; pEvent = pEvent->NextEvent)
		events[eventCount++] = pEvent;
	std::reverse(events.begin(), events.begin() + eventCount);

	enum class EventBlockType
	{
		Parallel,
		Sequential
	};

	struct EventBlock
	{
		EventBlockType Type { EventBlockType::Parallel };
		int StartIndex { 0 };
		int EndIndex { 0 };
		int ControlEventIndex { -1 };
	};

	bool hasControlEvents = false;
	for (size_t i = 0; i < eventCount; ++i)
	{
		int const kind = static_cast<int>(events[i]->EventKind);
		if (kind == PhobosTriggerEvent::ForceSequentialEvents || kind == PhobosTriggerEvent::ForceParallelEvents)
		{
			hasControlEvents = true;
			break;
		}
	}

	auto const pExt = TriggerExt::Fetch(pThis);
	HouseClass* pEventOwner = nullptr;
	if (pThis->Type)
	{
		if (!SessionClass::IsCampaign())
		{
			if (auto const pScenarioExt = ScenarioExt::Global())
			{
				auto const& triggerOwners = pScenarioExt->TriggerTypePlayerAtXOwners;
				auto it = triggerOwners.find(pThis->Type->ArrayIndex);
				if (it != triggerOwners.end())
					pEventOwner = HouseClass::FindByPlayerAt(it->second);
			}
		}

		if (!pEventOwner && pThis->Type->House)
			pEventOwner = HouseClass::FindByCountryName(pThis->Type->House->ID);
	}

	bool allEventsOccurred = true;

	if (!hasControlEvents)
	{
		// Standard parallel evaluation (vanilla)
		for (size_t i = 0; i < eventCount; ++i)
		{
			auto const pEvent = events[i];
			const DWORD eventBit = 1u << i;
			bool occurred = (pThis->OccuredEvents & eventBit) != 0;

			if (!occurred)
			{
				bool repeatingFlag = isPersistent;
				occurred = pEvent->HasOccured(
					static_cast<int>(nEvent),
					pEventOwner,
					pObject,
					&pThis->Timer,
					&repeatingFlag
				);

				if (!occurred)
					allEventsOccurred = false;
			}

			if (occurred)
			{
				if (pEvent->House)
					pThis->House = pEvent->House;

				if (isPersistent && pEvent->GetStateA() && pEvent->GetStateB())
					pThis->OccuredEvents |= eventBit;
			}
		}
	}
	else
	{
		// Multi-block evaluation (alternating Parallel and Sequential blocks)
		std::array<EventBlock, MaxEvents> blocks;
		size_t blockCount = 0;

		EventBlock currentBlock;
		currentBlock.Type = EventBlockType::Parallel;
		currentBlock.StartIndex = 0;
		currentBlock.ControlEventIndex = -1;

		for (size_t i = 0; i < eventCount; ++i)
		{
			int const kind = static_cast<int>(events[i]->EventKind);
			if (kind == PhobosTriggerEvent::ForceSequentialEvents)
			{
				currentBlock.EndIndex = static_cast<int>(i) - 1;
				currentBlock.ControlEventIndex = static_cast<int>(i);
				if (blockCount < MaxEvents)
					blocks[blockCount++] = currentBlock;

				// Start new sequential block
				currentBlock.Type = EventBlockType::Sequential;
				currentBlock.StartIndex = static_cast<int>(i) + 1;
				currentBlock.ControlEventIndex = -1;
			}
			else if (kind == PhobosTriggerEvent::ForceParallelEvents)
			{
				currentBlock.EndIndex = static_cast<int>(i) - 1;
				currentBlock.ControlEventIndex = static_cast<int>(i);
				if (blockCount < MaxEvents)
					blocks[blockCount++] = currentBlock;

				// Start new parallel block
				currentBlock.Type = EventBlockType::Parallel;
				currentBlock.StartIndex = static_cast<int>(i) + 1;
				currentBlock.ControlEventIndex = -1;
			}
		}
		currentBlock.EndIndex = static_cast<int>(eventCount) - 1;
		if (blockCount < MaxEvents)
			blocks[blockCount++] = currentBlock;

		for (size_t b = 0; b < blockCount; ++b)
		{
			const auto& block = blocks[b];
			if (block.StartIndex <= block.EndIndex)
			{
				if (block.Type == EventBlockType::Parallel)
				{
					bool blockDone = true;
					for (int i = block.StartIndex; i <= block.EndIndex; ++i)
					{
						auto const pEvent = events[i];
						const DWORD eventBit = 1u << i;
						bool occurred = (pThis->OccuredEvents & eventBit) != 0;

						if (!occurred)
						{
							CDTimerClass* pTimer = pExt->GetTimerForEvent(i, pEvent, true);
							bool repeatingFlag = isPersistent;
							occurred = pEvent->HasOccured(
								static_cast<int>(nEvent),
								pEventOwner,
								pObject,
								pTimer,
								&repeatingFlag
							);

							if (!occurred)
								blockDone = false;
						}

						if (occurred)
						{
							if (pEvent->House)
								pThis->House = pEvent->House;

							pThis->OccuredEvents |= eventBit;
						}
					}

					if (!blockDone)
					{
						// Parallel block incomplete: short-circuit!
						return false;
					}
				}
				else // Sequential block
				{
					bool blockDone = true;
					for (int i = block.StartIndex; i <= block.EndIndex; ++i)
					{
						auto const pEvent = events[i];
						const DWORD eventBit = 1u << i;
						bool occurred = (pThis->OccuredEvents & eventBit) != 0;

						if (!occurred)
						{
							// Active sequential step in this block
							CDTimerClass* pTimer = pExt->GetTimerForEvent(i, pEvent, false);
							bool repeatingFlag = isPersistent;
							occurred = pEvent->HasOccured(
								static_cast<int>(nEvent),
								pEventOwner,
								pObject,
								pTimer,
								&repeatingFlag
							);

							if (occurred)
							{
								if (pEvent->House)
									pThis->House = pEvent->House;

								pThis->OccuredEvents |= eventBit;
							}
							else
							{
								// Sequential step failed: stop evaluating this block and subsequent blocks!
								blockDone = false;
								break;
							}
						}
						else
						{
							if (pEvent->House)
								pThis->House = pEvent->House;
						}
					}

					if (!blockDone)
						return false;
				}
			}

			// Block fully satisfied! Mark closing control event as passed
			if (block.ControlEventIndex >= 0)
				pThis->OccuredEvents |= (1u << block.ControlEventIndex);
		}
	}

	if (allEventsOccurred)
	{
		if (isPersistent)
		{
			pThis->ResetTimers();
			pExt->ResetAllTimers();
		}
		return true;
	}

	return false;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7264C0, TriggerClass_RegisterEvent_Wrapper);

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
