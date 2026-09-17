#include "Body.h"

#include <Misc/FlyingStrings.h>
#include <ScenarioClass.h>
#include <MapClass.h>
#include <CellClass.h>
#include <Ext/House/Body.h>

// The method of calculating the income is subject to each specific situation,
// which may probably subject to further changes if anyone wants to extend the harvesting logic in the future.
// I don't want to investigate the details so I check the balance difference directly. --Trsdy
namespace OwnerBalanceBefore
{
	long HarversterUnloads;
	long SlaveComesBack;
}

// Unload more than once per ore dump if the harvester contains more than 1 tiberium type
DEFINE_HOOK(0x73E3DB, UnitClass_Mission_Unload_NoteBalanceBefore, 0x6)
{
	GET(HouseClass* const, pHouse, EBX); // this is the house of the refinery, not the harvester
	// GET(BuildingClass* const, pDock, EDI);
	OwnerBalanceBefore::HarversterUnloads = pHouse->Available_Money();// Available_Money takes silos into account
	return 0;
}

DEFINE_HOOK(0x73E4D0, UnitClass_Mission_Unload_CheckBalanceAfter, 0xA)
{
	GET(HouseClass* const, pHouse, EBX);
	GET(BuildingClass* const, pDock, EDI);

	if (auto const pBldExt = BuildingExt::TryFetch(pDock))
	{
		pBldExt->AccumulatedIncome += pHouse->Available_Money() - OwnerBalanceBefore::HarversterUnloads;
	}

	return 0;
}

DEFINE_HOOK(0x522D50, InfantryClass_SlaveGiveMoney_RecordBalanceBefore, 0x5)
{
	GET_STACK(TechnoClass* const, slaveMiner, 0x4);
	OwnerBalanceBefore::SlaveComesBack = slaveMiner->Owner->Available_Money();
	return 0;
}

DEFINE_HOOK(0x522E4F, InfantryClass_SlaveGiveMoney_CheckBalanceAfter, 0x6)
{
	GET_STACK(TechnoClass* const, slaveMiner, STACK_OFFSET(0x18, 0x4));

	const int money = slaveMiner->Owner->Available_Money() - OwnerBalanceBefore::SlaveComesBack;

	if (auto const pBld = abstract_cast<BuildingClass*>(slaveMiner))
	{
		auto const pBldExt = BuildingExt::Fetch(pBld);
		pBldExt->AccumulatedIncome += money;
	}
	else if (auto const pBldTypeExt = BuildingTypeExt::TryFetch(slaveMiner->GetTechnoType()->DeploysInto))
	{
		if (pBldTypeExt->DisplayIncome.Get(RulesExt::Global()->DisplayIncome.Get()))
			FlyingStrings::AddMoneyString(money, slaveMiner, slaveMiner->Owner, RulesExt::Global()->DisplayIncome_Houses.Get(), slaveMiner->Location);
	}

	return 0;
}

DEFINE_HOOK(0x445FE4, BuildingClass_Place_RefineryActiveAnim, 0x6)
{
	GET(BuildingTypeClass*, pType, ESI);

	return BuildingTypeExt::Fetch(pType)->Refinery_UseNormalActiveAnim ? 0x446183 : 0;
}

DEFINE_HOOK(0x450DAA, BuildingClass_UpdateAnimations_RefineryActiveAnim, 0x6)
{
	GET(BuildingTypeClass*, pType, EDX);

	return BuildingTypeExt::Fetch(pType)->Refinery_UseNormalActiveAnim ? 0x450F9E : 0;
}

DEFINE_HOOK(0x441C0C, BuildingClass_Destroy_CustomTiberiumSpill, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);

	if (!pThis || !pThis->Type || !pThis->Owner)
		return 0;

	auto const pExt = TechnoExt::Fetch(pThis);
	if (!pExt)
		return 0;

	auto const pHouse = pThis->Owner;
	auto const pHouseExt = HouseExt::Fetch(pHouse);

	for (size_t i = 4; i < pExt->TiberiumStorage.size(); ++i)
	{
		float const stored = pExt->TiberiumStorage[i];
		if (stored >= 1.0f)
		{
			auto const amount = std::ceil(stored);
			pExt->RemoveTiberium(amount, static_cast<int>(i));
			if (pHouseExt)
				pHouseExt->RemoveTiberiumStorage(amount, static_cast<int>(i));

			auto const pTib = TiberiumClass::Array.GetItemOrDefault(static_cast<int>(i));
			if (!pTib || !pTib->Image)
				continue;

			for (int j = static_cast<int>(amount); j > 0; --j)
			{
				auto const dist = ScenarioClass::Instance->Random.RandomRanged(256, 768);
				auto const crd = MapClass::GetRandomCoordsNear(pThis->Location, dist, true);
				if (auto const pCell = MapClass::Instance.GetCellAt(crd))
					pCell->IncreaseTiberium(static_cast<int>(i), 1);
			}
		}
	}

	return 0;
}
