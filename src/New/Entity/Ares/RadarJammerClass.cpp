#include "RadarJammerClass.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/EnumFunctions.h>

void RadarJammerClass::Update()
{
	const auto pTechno = this->Techno;
	const auto pTypeExt = TechnoExt::ExtMap.Find(pTechno)->TypeExtData;

	if (Unsorted::CurrentFrame - this->LastScan < pTypeExt->RadarJamDelay)
		return;

	this->LastScan = Unsorted::CurrentFrame;
	const auto pOwner = pTechno->Owner;
	const int jamRange = pTypeExt->RadarJamRadius * Unsorted::LeptonsPerCell;

	for (const auto pHouse : HouseClass::Array)
	{
		if (!EnumFunctions::CanTargetHouse(pTypeExt->RadarJamHouses, pOwner, pHouse))
			continue;

		for (const auto pBuilding : pHouse->Buildings)
		{
			const auto pBuildingType = pBuilding->Type;

			if (!pBuildingType->Radar && !pBuildingType->SpySat)
				continue;

			if (pTechno->DistanceFrom(pBuilding) <= jamRange
				&& !pTypeExt->RadarJamIgnore.Contains(pBuildingType)
				&& (pTypeExt->RadarJamAffect.empty() || pTypeExt->RadarJamAffect.Contains(pBuildingType)))
			{
				this->Jam(pBuilding);
			}
			else
			{
				this->Unjam(pBuilding);
			}
		}
	}
}

struct AresBuildingExtHere
{
	char _[0x40];
	PhobosMap<TechnoClass*, bool> RegisteredJammers;
};

void RadarJammerClass::Jam(BuildingClass* pBuilding)
{
	const auto pBuildingExt_Ares = reinterpret_cast<AresBuildingExtHere*>(*(uintptr_t*)((char*)pBuilding + 0x71C));

	if (pBuildingExt_Ares->RegisteredJammers.insert(this->Techno, true))
	{
		if (pBuildingExt_Ares->RegisteredJammers.size() == 1u)
			pBuilding->Owner->RecheckRadar = true;

		this->Registered = true;
	}
}

void RadarJammerClass::Unjam(BuildingClass* pBuilding)
{
	const auto pBuildingExt_Ares = reinterpret_cast<AresBuildingExtHere*>(*(uintptr_t*)((char*)pBuilding + 0x71C));

	if (pBuildingExt_Ares->RegisteredJammers.erase(this->Techno) && pBuildingExt_Ares->RegisteredJammers.empty())
		pBuilding->Owner->RecheckRadar = true;
}
