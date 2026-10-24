#include "Body.h"

#include <SessionClass.h>
#include <Misc/FlyingStrings.h>

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

DEFINE_HOOK(0x522D6A, InfantryClass_SlaveGiveMoney_Storage, 0x6)
{
	enum { NothingToUnload = 0x522E5B, UnloadedAtBuilding = 0x522E4F, ContinueDefault = 0x522D70 };

	GET(int const, firstSlot, ESI);
	if (firstSlot == -1)
		return NothingToUnload;

	GET(StorageClass* const, pSlaveStorage, EBP);
	GET_STACK(TechnoClass* const, pMaster, STACK_OFFSET(0x18, 0x4));

	if (!pMaster || !pMaster->Owner)
		return ContinueDefault;

	int const storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

	if (auto const pBld = abstract_cast<BuildingClass*>(pMaster))
	{
		auto const pBldTypeExt = BuildingTypeExt::Fetch(pBld->Type);
		if (pBldTypeExt->Refinery_UseStorage)
		{
			auto const pHouse = pBld->Owner;
			bool unloaded = false;

			int purifiers = pHouse->NumOrePurifiers;
			if (!pHouse->IsControlledByHuman() && SessionClass::Instance.GameMode != GameMode::Campaign)
				purifiers += RulesClass::Instance->AIVirtualPurifiers.GetItemOrDefault(pHouse->GetAIDifficultyIndex());

			for (int idx = 0; idx < 4; ++idx)
			{
				float const amount = pSlaveStorage->GetAmount(idx);
				if (amount > 0.0f)
				{
					pSlaveStorage->RemoveAmount(amount, idx);
					unloaded = true;

					float const purifierBails = static_cast<float>(purifiers) * RulesClass::Instance->PurifierBonus * amount;
					if (purifierBails > 0.0f)
					{
						if (auto const pTib = TiberiumClass::Array.GetItemOrDefault(idx))
						{
							int const purifierMoney = static_cast<int>(purifierBails * pTib->Value * pHouse->Type->IncomeMult);
							if (purifierMoney > 0)
								pHouse->GiveMoney(purifierMoney);
						}
					}

					int const idxStorage = storageTiberiumIndex >= 0 ? storageTiberiumIndex : idx;
					BuildingExt::StoreTiberium(pBld, amount, idx, idxStorage);
				}
			}

			return unloaded ? UnloadedAtBuilding : NothingToUnload;
		}
	}
	else if (auto const pUnit = abstract_cast<UnitClass*>(pMaster))
	{
		auto const pDeploysInto = pUnit->Type->DeploysInto;
		if (pDeploysInto && BuildingTypeExt::Fetch(pDeploysInto)->Refinery_UseStorage)
		{
			int const maxStorage = pUnit->Type->Storage > 0 ? pUnit->Type->Storage : pDeploysInto->Storage;

			for (int idx = 0; idx < 4; ++idx)
			{
				float const amount = pSlaveStorage->GetAmount(idx);
				if (amount > 0.0f)
				{
					pSlaveStorage->RemoveAmount(amount, idx);

					if (maxStorage > 0)
					{
						int const idxStorage = storageTiberiumIndex >= 0 ? storageTiberiumIndex : idx;
						auto const pTiberium = TiberiumClass::Array.GetItem(idx);
						auto const pDepositableTiberium = TiberiumClass::Array.GetItem(idxStorage);
						float const depositableAmount = (amount * pTiberium->Value) / pDepositableTiberium->Value;

						float const freeSpace = static_cast<float>(maxStorage) - pUnit->Tiberium.GetTotalAmount();
						if (freeSpace > 0.0f)
						{
							float const toStore = std::min(depositableAmount, freeSpace);
							pUnit->Tiberium.AddAmount(toStore, idxStorage);
						}
					}
				}
			}

			return NothingToUnload;
		}
	}

	return ContinueDefault;
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
