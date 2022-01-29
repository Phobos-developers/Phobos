#pragma once

#include <AircraftClass.h>
#include <HouseClass.h>

// ScoreStruct in YRPP has a buffer with the size of 512
// Nowadays, most mods have exceeding this limit already

class PhobosUnitTrackerClass
{
public:
	PhobosUnitTrackerClass* Initialize();
	void Clear();

	void Save(IStream* pStm);
	void Load(IStream* pStm);

	void ToPCFormat();
	void ToNetworkFormat();
	void DecrementUnitTotal(int nUnit);
	void ClearUnitTotal();
	void IncrementUnitTotal(int nUnit);
	int* GetAllTotals();

	void PopulateUnitCounts(int nCount);
	int GetUnitCounts() const;

private:
	static u_long __stdcall htonl(u_long hostlong) JMP_STD(0x7C8962);
	static u_long __stdcall ntohl(u_long netlong) JMP_STD(0x7C896E);

	int* Array;
	int ElementCount;
	bool IsNetworkFormat;

private:
	char gap[sizeof(ScoreStruct) - sizeof(Array) - sizeof(ElementCount) - sizeof(IsNetworkFormat)];
};