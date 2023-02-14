#include "Body.h"
#include <GameStrings.h>
#include <Misc/FlyingStrings.h>

#pragma region Harvesting
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

	if (auto pBldExt = BuildingExt::ExtMap.Find(pDock))
	{
		if (pBldExt->TypeExtData->DisplayIncome.Get(RulesExt::Global()->DisplayIncome.Get()))
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

	int money = slaveMiner->Owner->Available_Money() - OwnerBalanceBefore::SlaveComesBack;

	if (auto pBld = abstract_cast<BuildingClass*>(slaveMiner))
	{
		auto pBldExt = BuildingExt::ExtMap.Find(pBld);
		if (pBldExt->TypeExtData->DisplayIncome.Get(RulesExt::Global()->DisplayIncome.Get()))
			pBldExt->AccumulatedIncome += money;
	}
	else if (auto pBldTypeExt = BuildingTypeExt::ExtMap.Find(slaveMiner->GetTechnoType()->DeploysInto))
	{
		if (pBldTypeExt->DisplayIncome.Get(RulesExt::Global()->DisplayIncome.Get()))
			FlyingStrings::AddMoneyString(money, slaveMiner->Owner, RulesExt::Global()->DisplayIncome_Houses.Get(), slaveMiner->Location);
	}

	return 0;
}
#pragma endregion

#pragma region Selling
// SellSound and EVA dehardcode
DEFINE_HOOK(0x4D9F7B, FootClass_Sell, 0x6)
{
	enum { ReadyToVanish = 0x4D9FCB };
	GET(FootClass*, pThis, ESI);

	int money = pThis->GetRefund();
	pThis->Owner->GiveMoney(money);

	if (pThis->Owner->IsControlledByCurrentPlayer())
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_UnitSold)));
		//WW used VocClass::PlayGlobal to play the SellSound, why did they do that?
		VocClass::PlayAt(pTypeExt->SellSound.Get(RulesClass::Instance->SellSound), pThis->Location);
	}

	if (RulesExt::Global()->DisplayIncome.Get())
		FlyingStrings::AddMoneyString(money, pThis->Owner, RulesExt::Global()->DisplayIncome_Houses.Get(), pThis->Location);

	return ReadyToVanish;
}

//TODO: For my future PR
DEFINE_HOOK(0x449CC1, BuildingClass_Mission_Selling_EVA_Sold_1, 0x6)
{
	enum { SkipVoxPlay = 0x449CEA };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->IsOwnedByCurrentPlayer && !pThis->Focus) // For future use
		VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_StructureSold)));

	return SkipVoxPlay;

}

DEFINE_HOOK(0x44AB22, BuildingClass_Mission_Selling_EVA_Sold_2, 0x6)
{
	enum { SkipVoxPlay = 0x44AB3B };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->IsOwnedByCurrentPlayer)
		VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_StructureSold)));

	return SkipVoxPlay;
}

//TODO: For my future PR
DEFINE_HOOK(0x44A827, BuildingClass_Mission_Selling_Sellsound, 0x6)
{
	enum { FinishPlaying = 0x44A85B };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	VocClass::PlayAt(pTypeExt->SellSound.Get(RulesClass::Instance->SellSound), pThis->Location);
	return FinishPlaying;
}
#pragma endregion

