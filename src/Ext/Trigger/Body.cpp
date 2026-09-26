#include "Body.h"

#include <BuildingClass.h>
#include <InfantryClass.h>
#include <HouseClass.h>

#include <Ext/Scenario/Body.h>
#include <Ext/Techno/Body.h>

#include <ScenarioClass.h>
#include <TEventClass.h>

//Static init
TriggerExt::ExtContainer TriggerExt::ExtMap;

CDTimerClass* TriggerExt::GetTimerForEvent(int eventIndex, TEventClass* pEvent, bool isParallel)
{
	if (!pEvent || (pEvent->EventKind != TriggerEvent::ElapsedTime && pEvent->EventKind != TriggerEvent::RandomDelay))
	{
		return &this->OwnerObject()->Timer;
	}

	auto& entry = this->EventTimers[eventIndex];
	if (!entry.Started)
	{
		entry.IsRandom = (pEvent->EventKind == TriggerEvent::RandomDelay);
		entry.Duration = pEvent->Value;

		int duration = pEvent->Value;
		if (entry.IsRandom && ScenarioClass::Instance)
		{
			duration = ScenarioClass::Instance->Random.RandomRanged(
				static_cast<int>(pEvent->Value * 0.5),
				static_cast<int>(pEvent->Value * 1.5));
		}

		entry.Timer.Start(15 * duration);
		entry.Started = true;
	}

	return &entry.Timer;
}

void TriggerExt::ResetAllTimers()
{
	this->EventTimers.clear();
}

// =============================
// load / save

template <typename T>
void TriggerExt::Serialize(T& Stm)
{
	Stm
		.Process(this->EventTimers)
		;
}

void TriggerExt::LoadFromStream(PhobosStreamReader& Stm)
{
	AbstractExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TriggerExt::SaveToStream(PhobosStreamWriter& Stm)
{
	AbstractExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

TriggerExt::ExtContainer::ExtContainer() : Container("TriggerClass") { }

TriggerExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x7260C8, TriggerClass_CTOR, 0x8)
{
	GET(TriggerClass*, pItem, ESI);

	TriggerExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x72617D, TriggerClass_DTOR, 0xF)
{
	GET(TriggerClass*, pItem, ESI);

	TriggerExt::ExtMap.Remove(pItem);

	return 0;
}
