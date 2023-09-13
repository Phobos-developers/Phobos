#include "Body.h"

#include <SuperClass.h>
#include <MessageListClass.h>
#include <Utilities/AresFunctions.h>

//Ares hooked at 0x6CC390 and jumped to 0x6CDE40
// If a super is not handled by Ares however, we do it at the original entry point
DEFINE_HOOK_AGAIN(0x6CC390, SuperClass_Place_FireExt, 0x6)
DEFINE_HOOK(0x6CDE40, SuperClass_Place_FireExt, 0x4)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	// GET_STACK(bool const, isPlayer, 0x8);

	// Check if the SuperClass pointer is valid and not corrupted.
	if (pSuper && VTable::Get(pSuper) == SuperClass::AbsVTable)
		SWTypeExt::FireSuperWeaponExt(pSuper, *pCell);
	else
		Debug::Log("SuperClass_Place_FireExt: Hook entered with an invalid or corrupt SuperClass pointer.");

	return 0;
}

DEFINE_HOOK(0x6CB5EB, SuperClass_Grant_ShowTimer, 0x5)
{
	GET(SuperClass*, pThis, ESI);

	if (SuperClass::ShowTimers->AddItem(pThis))
		std::sort(SuperClass::ShowTimers->begin(), SuperClass::ShowTimers->end(),
			[](SuperClass* a, SuperClass* b){
				auto aExt = SWTypeExt::ExtMap.Find(a->Type);
				auto bExt = SWTypeExt::ExtMap.Find(b->Type);
				return aExt->ShowTimer_Priority.Get() > bExt->ShowTimer_Priority.Get();
			});

	return 0x6CB63E;
}

DEFINE_HOOK(0x6DBE74, Tactical_SuperLinesCircles_ShowDesignatorRange, 0x7)
{
	if (!Phobos::Config::ShowDesignatorRange || !(RulesExt::Global()->ShowDesignatorRange) || Unsorted::CurrentSWType == -1)
		return 0;

	const auto pSuperType = SuperWeaponTypeClass::Array()->GetItem(Unsorted::CurrentSWType);
	const auto pExt = SWTypeExt::ExtMap.Find(pSuperType);

	if (!pExt->ShowDesignatorRange)
		return 0;

	for (const auto pCurrentTechno : *TechnoClass::Array)
	{
		const auto pCurrentTechnoType = pCurrentTechno->GetTechnoType();
		const auto pOwner = pCurrentTechno->Owner;

		if (!pCurrentTechno->IsAlive
			|| pCurrentTechno->InLimbo
			|| (pOwner != HouseClass::CurrentPlayer && pOwner->IsAlliedWith(HouseClass::CurrentPlayer))                  // Ally objects are never designators or inhibitors
			|| (pOwner == HouseClass::CurrentPlayer && !pExt->SW_Designators.Contains(pCurrentTechnoType))               // Only owned objects can be designators
			|| (!pOwner->IsAlliedWith(HouseClass::CurrentPlayer) && !pExt->SW_Inhibitors.Contains(pCurrentTechnoType)))  // Only enemy objects can be inhibitors
		{
			continue;
		}

		const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pCurrentTechnoType);

		const float radius = pOwner == HouseClass::CurrentPlayer
			? (float)(pTechnoTypeExt->DesignatorRange.Get(pCurrentTechnoType->Sight))
			: (float)(pTechnoTypeExt->InhibitorRange.Get(pCurrentTechnoType->Sight));

		CoordStruct coords = pCurrentTechno->GetCenterCoords();
		coords.Z = MapClass::Instance->GetCellFloorHeight(coords);
		const auto color = pOwner->Color;
		Game::DrawRadialIndicator(false, true, coords, color, radius, false, true);
	}

	return 0;
}

DEFINE_HOOK(0x6AAEAC, SuperClass_Place_ResetTimer, 0x6)
{
	GET(int, idxSW, EDX);

	SuperClass* pSuper = HouseClass::CurrentPlayer->Supers.GetItem(idxSW);
	if (!pSuper)
		return 0;

	SuperExt::ExtData* pExt = SuperExt::ExtMap.Find(pSuper);
	if (!pExt)
		return 0;

	SWTypeExt::ExtData* pTypeData = SWTypeExt::ExtMap.Find(pSuper->Type);
	if (!pTypeData)
		return 0;

	if (!pSuper->IsCharged)
		return 0;

	if (!pTypeData->SW_FirstClickRestartsTimer)
		return 0;

	if (!pExt->TimerRestarted)
	{
		// In case of require money prevent firing the SW if the player doesn't have sufficient funds
		int cost = pTypeData->SW_FirstClickRestartsTimer_Cost;

		if (cost != 0)
		{
			if (!HouseClass::CurrentPlayer->CanTransactMoney(cost))
			{
				VoxClass::Play(GameStrings::EVA_InsufficientFunds);
				MessageListClass::Instance->PrintMessage(pTypeData->Message_InsufficientFunds.Get(), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);

				return 0x6AB95A;
			}

			HouseClass::CurrentPlayer->TransactMoney(cost);
		}

		if (pTypeData->EVA_RestartedTimer.isset())
			VoxClass::PlayIndex(pTypeData->EVA_RestartedTimer.Get());

		MessageListClass::Instance->PrintMessage(pTypeData->Message_RestartedTimer.Get(), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);

		pExt->TimerRestarted = true;
		pSuper->Reset();

		return 0x6AB95A;
	}

	return 0;
}

DEFINE_HOOK(0x6CBD13, SuperClass_Place_ResetTimer_AutoFire, 0x6)
{
	GET(SuperClass*, pSuper, ESI);

	SWTypeExt::ExtData* pTypeData = SWTypeExt::ExtMap.Find(pSuper->Type);
	if (!pTypeData)
		return 0;

	if (!pSuper->IsCharged)
		return 0;

	if (!pTypeData->SW_FirstClickRestartsTimer)
		return 0;

	SuperExt::ExtData* pExt = SuperExt::ExtMap.Find(pSuper);
	if (!pExt)
		return 0;

	if (!pExt->TimerRestarted)
		return 0;

	if (pTypeData->SW_FirstClickRestartsTimer_AutoFire)
	{
		pSuper->Launch(CellStruct::Empty, HouseClass::CurrentPlayer->IsCurrentPlayer());
		SWTypeExt::FireSuperWeaponExt(pSuper, CellStruct::Empty);
		pSuper->Reset();
		pSuper->SetRechargeTime(pSuper->Type->RechargeTime * 15);
		pSuper->SetReadiness(false);

		return 0x6CBD2F;
	}

	return 0;
}

DEFINE_HOOK(0x6CBCD4, SuperClass_AI_ResetTimer_Update, 0x5)
{
	GET(SuperClass*, pSuper, ESI);

	SuperExt::ExtData* pExt = SuperExt::ExtMap.Find(pSuper);
	if (!pExt)
		return 0;

	SWTypeExt::ExtData* pTypeData = SWTypeExt::ExtMap.Find(pSuper->Type);
	if (!pTypeData)
		return 0;

	if (pTypeData->SW_FirstClickRestartsTimer && pExt->TimerRestarted && !pSuper->Granted)
	{
		pExt->TimerRestarted = false;

		if (pTypeData->SW_FirstClickRestartsTimer_RefundIfAborted)
		{
			int cost = -1 * pTypeData->SW_FirstClickRestartsTimer_Cost;

			if (HouseClass::CurrentPlayer->CanTransactMoney(cost))
				HouseClass::CurrentPlayer->TransactMoney(cost);
		}
	}

	return 0;
}
