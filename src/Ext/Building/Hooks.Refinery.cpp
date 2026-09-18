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

DEFINE_HOOK(0x450CD7, BuildingClass_UpdateAnimations_SiloDamage, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	auto const pType = pThis->Type;
	int frame = 0;
	if (pType && pType->Storage > 0)
	{
		auto const pExt = TechnoExt::Fetch(pThis);
		float const total = pExt ? pExt->GetTotalTiberium() : pThis->Tiberium.GetTotalAmount();
		int const ratio = static_cast<int>((static_cast<double>(static_cast<int>(total) * 4) / static_cast<double>(pType->Storage)) + 0.5);
		frame = std::clamp(ratio, 0, 3);
	}

	R->EDI(frame);
	return 0x450D1D;
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

	for (size_t i = 0; i < pExt->TiberiumStorage.size(); ++i)
	{
		float const stored = pExt->TiberiumStorage[i];
		if (stored > 0.0f)
		{
			pExt->TiberiumStorage[i] = 0.0f;
			if (pHouseExt && i < pHouseExt->TiberiumStorage.size())
			{
				float const removed = std::min(pHouseExt->TiberiumStorage[i], stored);
				pHouseExt->TiberiumStorage[i] -= removed;
			}

			if (i >= 4 && stored >= 1.0f)
			{
				auto const pTib = TiberiumClass::Array.GetItemOrDefault(static_cast<int>(i));
				if (pTib && pTib->Image)
				{
					int const pips = static_cast<int>(stored);
					for (int j = pips; j > 0; --j)
					{
						auto const dist = ScenarioClass::Instance->Random.RandomRanged(256, 768);
						auto const crd = MapClass::GetRandomCoordsNear(pThis->Location, dist, true);
						if (auto const pCell = MapClass::Instance.GetCellAt(crd))
							pCell->IncreaseTiberium(static_cast<int>(i), 1);
					}
				}
			}
		}
	}

	pHouse->UpdateAllSilos(0, 0);

	return 0;
}

DEFINE_HOOK(0x44A232, BuildingClass_Mission_Selling_RedistributeTiberium, 0x6)
{
	enum { SkipRefundLoop = 0x44A287 };

	GET(BuildingClass*, pThis, EBP);

	if (!pThis || !pThis->Owner)
		return SkipRefundLoop;

	auto const pHouse = pThis->Owner;
	auto const pHouseExt = HouseExt::Fetch(pHouse);
	auto const pExt = TechnoExt::Fetch(pThis);

	// Collect and extract all stored resources from the building being sold
	std::vector<std::pair<int, float>> resources;
	size_t const maxSlots = std::max(pExt ? pExt->TiberiumStorage.size() : static_cast<size_t>(0), static_cast<size_t>(4));

	for (size_t i = 0; i < maxSlots; ++i)
	{
		float amount = 0.0f;
		if (pExt && i < pExt->TiberiumStorage.size() && pExt->TiberiumStorage[i] > 0.0f)
			amount = pExt->TiberiumStorage[i];
		else if (i < 4)
			amount = pThis->Tiberium.GetAmount(static_cast<int>(i));

		if (amount > 0.0f)
		{
			resources.emplace_back(static_cast<int>(i), amount);

			// Extract from the sold building
			if (pExt && i < pExt->TiberiumStorage.size())
				pExt->TiberiumStorage[i] = 0.0f;

			if (i < 4)
			{
				pThis->Tiberium.RemoveAmount(amount, static_cast<int>(i));
				pHouse->OwnedTiberium.RemoveAmount(amount, static_cast<int>(i));
			}

			if (pHouseExt && i < pHouseExt->TiberiumStorage.size())
			{
				float const removed = std::min(pHouseExt->TiberiumStorage[i], amount);
				pHouseExt->TiberiumStorage[i] -= removed;
			}
		}
	}

	// Try to redistribute resources into other available storage structures
	for (auto const& [idxType, amount] : resources)
	{
		float remaining = amount;

		for (auto const pBld : pHouse->Buildings)
		{
			if (!pBld || pBld == pThis || !pBld->IsOnMap || pBld->InLimbo || pBld->CurrentMission == Mission::Selling)
				continue;

			if (pBld->Type->Storage <= 0)
				continue;

			auto const pBldExt = TechnoExt::Fetch(pBld);
			float const curStored = pBldExt ? pBldExt->GetTotalTiberium() : pBld->Tiberium.GetTotalAmount();
			float const freeSpace = static_cast<float>(pBld->Type->Storage) - curStored;

			if (freeSpace > 0.0f)
			{
				float const toStore = std::min(remaining, freeSpace);

				if (pBldExt)
					pBldExt->AddTiberium(toStore, idxType);

				if (idxType < 4)
				{
					pBld->Tiberium.AddAmount(toStore, idxType);
					pHouse->OwnedTiberium.AddAmount(toStore, idxType);
				}

				if (pHouseExt)
					pHouseExt->AddTiberiumStorage(toStore, idxType);

				pBld->Mark(MarkType::Change);
				remaining -= toStore;

				if (remaining <= 0.0f)
					break;
			}
		}

		// Any excess 'remaining' that couldn't fit into storage is LOST (never converted to credits).
	}

	pHouse->UpdateAllSilos(0, 0);

	return SkipRefundLoop;
}
