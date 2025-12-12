#include "Body.h"

#include <BuildingClass.h>
#include <InfantryClass.h>
#include <HouseClass.h>

#include <Ext/Scenario/Body.h>
#include <Ext/Techno/Body.h>

//Static init
TriggerExt::ExtContainer TriggerExt::ExtMap;

// =============================
// load / save

template <typename T>
void TriggerExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->SortedEventsList)
		.Process(this->SequentialTimers)
		.Process(this->SequentialTimersOriginalValue)
		.Process(this->ParallelTimers)
		.Process(this->ParallelTimersOriginalValue)
		.Process(this->SequentialSwitchModeIndex)
		;
}

void TriggerExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TriggerClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TriggerExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TriggerClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

DEFINE_HOOK(0x7260C8, TriggerClass_CTOR, 0x8)
{
	GET(TriggerClass*, pItem, ESI);

	TriggerExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x72617D, TriggerClass_DTOR, 0xF)
{
	GET(TriggerClass*, pItem, ESI);

	TriggerExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK(0x726860, TriggerClass_Load_Prefix, 0x5)
{
	GET_STACK(TriggerClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TriggerExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x7268CB, TriggerClass_Load_Suffix, 0x4)
{
	TriggerExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x7268D0, TriggerClass_Save_Prefix, 0x8)
{
	GET_STACK(TriggerClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TriggerExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x7268EA, TriggerClass_Save_Suffix, 0x5)
{
	TriggerExt::ExtMap.SaveStatic();

	return 0;
}

// =============================
// container

TriggerExt::ExtContainer::ExtContainer() : Container("TriggerClass") { }

TriggerExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
