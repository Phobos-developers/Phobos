#include "UnitTrackerClass.h"

#include <Helpers/Macro.h>

PhobosUnitTrackerClass* PhobosUnitTrackerClass::Initialize()
{
	this->Array = nullptr;
	this->ElementCount = 0;
	this->IsNetworkFormat = false;

	return this;
}

void PhobosUnitTrackerClass::Clear()
{
	if (this->Array)
		GameDeleteArray(this->Array, this->ElementCount);
}

void PhobosUnitTrackerClass::Save(IStream* pStm)
{
	pStm->Write(this->Array, sizeof(int) * this->ElementCount, nullptr);
}

void PhobosUnitTrackerClass::Load(IStream* pStm)
{
	this->Array = nullptr;
	this->PopulateUnitCounts(this->ElementCount);

	pStm->Read(this->Array, sizeof(int) * this->ElementCount, nullptr);
}

void PhobosUnitTrackerClass::ToPCFormat()
{
	if (this->IsNetworkFormat)
	{
		for (int i = 0; i < this->ElementCount; ++i)
			this->Array[i] = PhobosUnitTrackerClass::ntohl(this->Array[i]);
	}

	this->IsNetworkFormat = false;
}

void PhobosUnitTrackerClass::ToNetworkFormat()
{
	if (!this->IsNetworkFormat)
	{
		for (int i = 0; i < this->ElementCount; ++i)
			this->Array[i] = PhobosUnitTrackerClass::htonl(this->Array[i]);
	}

	this->IsNetworkFormat = true;
}

void PhobosUnitTrackerClass::DecrementUnitTotal(int nUnit)
{
	if (nUnit < this->ElementCount)
		--this->Array[nUnit];
}

void PhobosUnitTrackerClass::ClearUnitTotal()
{
	memset(this->Array, 0, sizeof(int) * this->ElementCount);
}

void PhobosUnitTrackerClass::IncrementUnitTotal(int nUnit)
{
	if (nUnit < this->ElementCount)
		++this->Array[nUnit];
}

int* PhobosUnitTrackerClass::GetAllTotals()
{
	return this->Array;
}

void PhobosUnitTrackerClass::PopulateUnitCounts(int nCount)
{
	this->Clear();

	this->ElementCount = nCount;
	this->Array = GameCreateArray<int>(nCount);
}

int PhobosUnitTrackerClass::GetUnitCounts() const
{
	int nRet = 0;
	for (int i = 0; i < this->ElementCount; ++i)
		nRet += this->Array[i];
	return nRet;
}

DEFINE_HOOK(0x748FD0, UnitTrackerClass_CTOR, 0x5)
{
	GET(PhobosUnitTrackerClass*, pThis, ECX);

	R->EAX(pThis->Initialize());

	return 0x749000;
}

DEFINE_HOOK(0x749010, UnitTrackerClass_DTOR, 0x0)
{
	GET(PhobosUnitTrackerClass*, pThis, ECX);

	pThis->Clear();

	return 0;
}

DEFINE_HOOK(0x749150, UnitTrackerClass_ToPCFormat, 0x5)
{
	GET(PhobosUnitTrackerClass*, pThis, ECX);

	pThis->ToPCFormat();

	return 0x74919A;
}

DEFINE_HOOK(0x749100, UnitTrackerClass_ToNetworkFormat, 0xA)
{
	GET(PhobosUnitTrackerClass*, pThis, ECX);

	pThis->ToNetworkFormat();

	return 0x749142;
}

DEFINE_HOOK(0x749040, UnitTrackerClass_DecrementUnitTotal, 0xA)
{
	GET(PhobosUnitTrackerClass*, pThis, ECX);
	GET_STACK(int, nUnit, 0x4);

	pThis->DecrementUnitTotal(nUnit);

	return 0x749051;
}

DEFINE_HOOK(0x7490D0, UnitTrackerClass_ClearUnitTotal, 0x6)
{
	GET(PhobosUnitTrackerClass*, pThis, ECX);

	pThis->ClearUnitTotal();

	return 0x7490F4;
}

DEFINE_HOOK(0x749020, UnitTrackerClass_IncrementUnitTotal, 0xA)
{
	GET(PhobosUnitTrackerClass*, pThis, ECX);
	GET_STACK(int, nUnit, 0x4);

	pThis->IncrementUnitTotal(nUnit);

	return 0x749031;
}

DEFINE_HOOK(0x7490C2, UnitTrackerClass_GetAllTotals, 0x0)
{
	GET(PhobosUnitTrackerClass*, pThis, EAX);

	R->EAX(pThis->GetAllTotals());

	return 0;
}

DEFINE_HOOK(0x749060, UnitTrackerClass_PopulateUnitCounts, 0xB)
{
	GET(PhobosUnitTrackerClass*, pThis, ECX);
	GET_STACK(int, nCount, 0x4);

	pThis->PopulateUnitCounts(nCount);

	return 0x74908E;
}

DEFINE_HOOK(0x7490A0, UnitTrackerClass_GetUnitCounts, 0x6)
{
	GET(PhobosUnitTrackerClass*, pThis, ECX);

	R->EAX(pThis->GetUnitCounts());

	return 0x7490B8;
}

DEFINE_HOOK(0x5031E6, HouseClass_Load_UnitTrackers, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	GET(IStream*, pStm, EDI);

	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltInfantryTypes)->Load(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltUnitTypes)->Load(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltAircraftTypes)->Load(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltBuildingTypes)->Load(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledInfantryTypes)->Load(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledUnitTypes)->Load(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledAircraftTypes)->Load(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledBuildingTypes)->Load(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->UnknownScores)->Load(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->CollectedPowerups)->Load(pStm);

	return 0;
}

DEFINE_HOOK(0x5040A2, HouseClass_Save_UnitTrackers, 0x6)
{
	GET(HouseClass*, pThis, EDI);
	GET(IStream*, pStm, ESI);

	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltInfantryTypes)->Save(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltUnitTypes)->Save(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltAircraftTypes)->Save(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltBuildingTypes)->Save(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledInfantryTypes)->Save(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledUnitTypes)->Save(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledAircraftTypes)->Save(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledBuildingTypes)->Save(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->UnknownScores)->Save(pStm);
	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->CollectedPowerups)->Save(pStm);

	return 0;
}