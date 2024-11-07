#include "Body.h"

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

// Rewrite 0x449BC0
// true: undeploy into vehicle; false: sell
bool __forceinline BuildingExt::CanUndeployOnSell(BuildingClass* pThis)
{
	auto pType = pThis->Type;

	if (!pType->UndeploysInto)
		return false;

	if (pType->ConstructionYard)
	{
		// Conyards can't undeploy if MCVRedeploy=no
		if (!GameModeOptionsClass::Instance->MCVRedeploy)
			return false;
		// or MindControlledBy YURIX (why? for balance?)
		if (pThis->MindControlledBy || !pThis->Owner->IsControlledByHuman())
			return false;
	}

	// Move ArchiveTarget check outside Conyard check to allow generic Unsellable=no buildings to be sold
	return pThis->ArchiveTarget;
}

// Skip SessionClass::IsCampaign() checks, where inlined not exactly the function above but sth similar
DEFINE_JUMP(LJMP, 0x443A9A, 0x443AA3); //BuildingClass_SetRallyPoint
DEFINE_JUMP(LJMP, 0x44375E, 0x443767); //BuildingClass_CellClickedAction
DEFINE_JUMP(LJMP, 0x44F602, 0x44F60B); //BuildingClass_IsControllable

DEFINE_HOOK(0x449CC1, BuildingClass_Mi_Selling_EVASold_UndeploysInto, 0x6)
{
	enum { CreateUnit = 0x449D5E, SkipTheEntireShit = 0x44A1E8 };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	// Fix Conyards can't play EVA_StructureSold
	if (pThis->IsOwnedByCurrentPlayer && (!pThis->ArchiveTarget || !pThis->Type->UndeploysInto))
		VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_StructureSold)));

	return BuildingExt::CanUndeployOnSell(pThis) ? CreateUnit : SkipTheEntireShit;
}

DEFINE_HOOK(0x44A7CF, BuildingClass_Mi_Selling_PlaySellSound, 0x6)
{
	enum { FinishPlaying = 0x44A85B };
	GET(BuildingClass*, pThis, EBP);

	if (!BuildingExt::CanUndeployOnSell(pThis))
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
		VocClass::PlayAt(pTypeExt->SellSound.Get(RulesClass::Instance->SellSound), pThis->Location);
	}

	return FinishPlaying;
}

DEFINE_HOOK(0x44A8E5, BuildingClass_Mi_Selling_SetTarget, 0x6)
{
	enum { ResetTarget = 0x44A937, SkipShit = 0x44A95E };
	GET(BuildingClass*, pThis, EBP);

	return BuildingExt::CanUndeployOnSell(pThis) ? ResetTarget : SkipShit;
}

DEFINE_HOOK(0x44A964, BuildingClass_Mi_Selling_VoiceDeploy, 0x6)
{
	enum { CanDeploySound = 0x44A9CA, SkipShit = 0x44AA3D };
	GET(BuildingClass*, pThis, EBP);

	return BuildingExt::CanUndeployOnSell(pThis) ? CanDeploySound : SkipShit;
}

DEFINE_HOOK(0x44AB22, BuildingClass_Mi_Selling_EVASold_Plug, 0x6)
{
	enum { SkipVoxPlay = 0x44AB3B };
#if ANYBODY_NOTICED_THIS
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->IsOwnedByCurrentPlayer)
		VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_StructureSold)));
#endif
	return SkipVoxPlay;
}
