#include "Body.h"

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
		pBldExt->AccumulatedIncome += money;
	}
	else if (auto pBldTypeExt = BuildingTypeExt::ExtMap.Find(slaveMiner->GetTechnoType()->DeploysInto))
	{
		if (pBldTypeExt->DisplayIncome.Get(RulesExt::Global()->DisplayIncome.Get()))
			FlyingStrings::AddMoneyString(money, slaveMiner->Owner, RulesExt::Global()->DisplayIncome_Houses.Get(), slaveMiner->Location);
	}

	return 0;
}
