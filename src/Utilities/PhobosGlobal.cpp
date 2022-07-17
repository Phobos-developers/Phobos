#include "PhobosGlobal.h"

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <Ext/WarheadType/Body.h>

//GlobalObject initial
PhobosGlobal PhobosGlobal::GlobalObject;

PhobosGlobal* PhobosGlobal::Global()
{
	return &GlobalObject;
}

void PhobosGlobal::Clear()
{
	GlobalObject.Reset();
}

void PhobosGlobal::Reset()
{
	RandomTriggerPool.clear();
}

//Save/Load
#pragma region save/load

template <typename T>
bool PhobosGlobal::Serialize(T& stm)
{
	return stm
		.Process(this->RandomTriggerPool)
		.Success();
}

template <typename T>
bool PhobosGlobal::SerializeGlobal(T& stm)
{
	ProcessTechnoType(stm);
	ProcessTechno(stm);
	return stm.Success();
}

template <typename T>
bool Process(T& stm, TechnoTypeClass* pItem)
{
//	TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pItem);
//	stm
//		;
	return stm.Success();
}

template <typename T>
bool Process(T& stm, TechnoClass* pItem)
{
//	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pItem);
//	stm
//		;
	return stm.Success();
}

template <typename T>
bool PhobosGlobal::ProcessTechnoType(T& stm)
{
	/*for (auto pItem : *UnitTypeClass::Array)
	{
		Process(stm, pItem);
	}
	for (auto pItem : *InfantryTypeClass::Array)
	{
		Process(stm, pItem);
	}
	for (auto pItem : *AircraftTypeClass::Array)
	{
		Process(stm, pItem);
	}
	for (auto pItem : *BuildingTypeClass::Array)
	{
		Process(stm, pItem);
	}*/
	return stm.Success();
}

template <typename T>
bool PhobosGlobal::ProcessTechno(T& stm)
{
	/*for (auto pItem : *UnitClass::Array)
	{
		Process(stm, pItem);
	}
	for (auto pItem : *InfantryClass::Array)
	{
		Process(stm, pItem);
	}
	for (auto pItem : *AircraftClass::Array)
	{
		Process(stm, pItem);
	}
	for (auto pItem : *BuildingClass::Array)
	{
		Process(stm, pItem);
	}*/
	return stm.Success();
}

bool PhobosGlobal::Save(PhobosStreamWriter& stm)
{
	return Serialize(stm);
}

bool PhobosGlobal::Load(PhobosStreamReader& stm)
{
	return Serialize(stm);
}

bool PhobosGlobal::SaveGlobals(PhobosStreamWriter& stm)
{
	SerializeGlobal(stm);
	GlobalObject.Save(stm);
	return stm.Success();
}

bool PhobosGlobal::LoadGlobals(PhobosStreamReader& stm)
{
	SerializeGlobal(stm);
	GlobalObject.Load(stm);
	return stm.Success();
}

#pragma endregion save/load
