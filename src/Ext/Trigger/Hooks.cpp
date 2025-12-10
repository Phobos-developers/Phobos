#include <TriggerClass.h>
#include <TriggerTypeClass.h>
#include <HouseClass.h>

#include <Helpers/Macro.h>

#include <Ext/TEvent/Body.h>
#include <Ext/Trigger/Body.h>

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

DEFINE_HOOK(0x72612C, TriggerClass_CTOR_ForceSequentialEvents, 0x7)
{
	GET(TriggerClass*, pThis, ESI);

	if (!pThis->Type)
		return 0;

	auto pExt = TriggerExt::ExtMap.Find(pThis);
	auto pCurrentEvent = pThis->Type->FirstEvent;

	while (pCurrentEvent)
	{
		pExt->SortedEventsList.emplace_back(pCurrentEvent);
		pCurrentEvent = pCurrentEvent->NextEvent;
	}

	std::reverse(pExt->SortedEventsList.begin(), pExt->SortedEventsList.end());

	for (std::size_t i = 0; i < pExt->SortedEventsList.size(); i++)
	{
		pCurrentEvent = pExt->SortedEventsList[i];

		if (static_cast<int>(pCurrentEvent->EventKind) == PhobosTriggerEvent::ForceSequentialEvents)
		{
			pExt->SequentialSwitchModeIndex = i;
			continue;
		}

		if (pCurrentEvent->EventKind != TriggerEvent::ElapsedTime && pCurrentEvent->EventKind != TriggerEvent::RandomDelay)
			continue;

		int countdown = 0;

		if (pCurrentEvent->EventKind == TriggerEvent::ElapsedTime) // Event 13 "Elapsed Time..."
			countdown = pCurrentEvent->Value;
		else // Event 51 "Random delay..."
			countdown = ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(pCurrentEvent->Value * 0.5), static_cast<int>(pCurrentEvent->Value * 1.5));

		if (pExt->SequentialSwitchModeIndex >= 0)
		{
			pExt->SequentialTimersOriginalValue[i] = pCurrentEvent->EventKind == TriggerEvent::ElapsedTime ? pCurrentEvent->Value : pCurrentEvent->Value * -1;
			pExt->SequentialTimers[i].Start(15 * countdown);
			pExt->SequentialTimers[i].Pause();
		}
		else
		{
			pExt->ParallelTimersOriginalValue[i] = pCurrentEvent->EventKind == TriggerEvent::ElapsedTime ? pCurrentEvent->Value : pCurrentEvent->Value * -1;
			pExt->ParallelTimers[i].Start(15 * countdown);
		}
	}

	return 0;
}

// TriggerClass::RegisterEvent(...) rewrite
DEFINE_HOOK(0x7264C0, TriggerClass_RegisterEvent_ForceSequentialEvents, 0x0)
{
	enum { SkipGameCode = 0x7265B1 };

	GET(TriggerClass*, pThis, ECX);
	GET_STACK(TriggerEvent, nEvent, 0x4);
	GET_STACK(TechnoClass*, pTechno, 0x8);
	GET_STACK(bool, skipStuff, 0xC);
	GET_STACK(bool, isPersistant, 0x10);
	GET_STACK(ObjectClass*, pPayback, 0x14); // <-- Warning! YRpp call HasOccured(...) doesn't have the last argument...

	if (!pThis->Enabled || pThis->Destroyed)
	{
		R->AL(false);
		return SkipGameCode;
	}

	auto pExt = TriggerExt::ExtMap.Find(pThis);
	bool isSequentialMode = false; // Flag: Controls if short-circuit is active for subsequent events
	bool allEventsSuccessful = true;
	int nPredecessorEventsCompleted = 0;

	if (!skipStuff)
	{
		// Check status of the trigger events in sequential logic (INI order)
		for (std::size_t i = 0; i < pExt->SortedEventsList.size(); i++)
		{
			const auto pCurrentEvent = pExt->SortedEventsList[i];
			bool alreadyOccured = pThis->HasEventOccured(i);
			bool triggeredNow = false;
			auto eventTimer = pThis->Timer; // Fallback

			if (pExt->ParallelTimers.contains(i))
			{
				eventTimer = pExt->ParallelTimers[i];
			}
			else if (pExt->SequentialTimers.contains(i))
			{
				if (pExt->SequentialTimers[i].HasTimeLeft()
				&& !pExt->SequentialTimers[i].InProgress()
				&& !pExt->SequentialTimers[i].Completed())
				{
					pExt->SequentialTimers[i].Resume();
				}

				eventTimer = pExt->SequentialTimers[i];
			}

			if (static_cast<int>(pCurrentEvent->EventKind) == PhobosTriggerEvent::ForceSequentialEvents)
			{
				bool predecessorsCompleted = false;

				if (nPredecessorEventsCompleted >= pExt->SequentialSwitchModeIndex)
					predecessorsCompleted = true;

				if (predecessorsCompleted)
				{
					pThis->MarkEventAsOccured(i);
					alreadyOccured = true;
					triggeredNow = true;
					isSequentialMode = true; // Activate sequential mode for the rest of the INI events
				}
				else
				{
					allEventsSuccessful = false;
					R->AL(false);
					return SkipGameCode; // Short-circuit
				}
			}

			if (!alreadyOccured)
			{
				HouseClass* pEventOwner = HouseClass::FindByCountryName(pThis->Type->House->ID);
				TechnoClass* pPaybackTechno = static_cast<TechnoClass*>(pPayback);

				triggeredNow = pCurrentEvent->HasOccured(
									static_cast<int>(nEvent),
									pEventOwner,
									pTechno,
									&eventTimer,
									&isPersistant);
				// Note: I think HasOccured in YRpp is wrong, where is the last parameter for "pPaybackTechno"? I mean this "payback1":
				// TEventClass::operator()(v8, tevent, v9, &techno->r.m.o, &this->Event.Timer, &is_persistant, payback1)) )
				// In pseudocode:
				// bool __thiscall TEventClass::operator()(TEventClass *this, int event, HouseClass *house, ObjectClass *obj, CDTimerClass *td, bool *bool1, TechnoClass *source)
			}

			if (alreadyOccured || triggeredNow)
			{
				HouseClass* pNewHouse = pCurrentEvent->House;

				if (pNewHouse)
					pThis->House = pNewHouse;

				if (isPersistant && pCurrentEvent->GetStateA() && pCurrentEvent->GetStateB())
					pThis->MarkEventAsOccured(i); //pThis->OccuredEvents |= eventBit;

				nPredecessorEventsCompleted++;
			}
			else
			{
				// Conditional short-circuit on Failure
				allEventsSuccessful = false;

				if (isSequentialMode)
				{
					R->AL(false);
					return SkipGameCode;
				}
			}
		}
	}

	if (allEventsSuccessful || skipStuff)
	{
		if (isPersistant)
		{
			pThis->ResetTimers(); // Is really needed now? Maybe, because YRpp is incomplete and looks that each event have its own timer inside a struct... or something similar. I'll preserve this for now that doesn't hurt having this here...

			for (std::size_t i = 0; i < pExt->ParallelTimersOriginalValue.size(); i++)
			{
				int timerValue = pExt->ParallelTimersOriginalValue[i];

				if (timerValue < 0)
				{
					// Generate random value for event 51 "Delayed timer"
					timerValue = ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(std::abs(timerValue) * 0.5), static_cast<int>(std::abs(timerValue) * 1.5));
				}

				pExt->ParallelTimers[i].Start(15 * timerValue);
			}

			for (std::size_t i = 0; i < pExt->SequentialTimersOriginalValue.size(); i++)
			{
				int timerValue = pExt->SequentialTimersOriginalValue[i];

				if (timerValue < 0)
				{
					// Generate random value for event 51 "Delayed timer"
					timerValue = ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(std::abs(timerValue) * 0.5), static_cast<int>(std::abs(timerValue) * 1.5));
				}

				pExt->SequentialTimers[i].Start(15 * timerValue);
				pExt->SequentialTimers[i].Pause();
			}
		}
	}

	if (allEventsSuccessful)
		Debug::Log("Starting actions of the trigger: [%s] - %s\n", pThis->Type->ID, pThis->Type->Name);

	R->AL(allEventsSuccessful);

	return SkipGameCode;
}
