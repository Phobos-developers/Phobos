#include "Body.h"

#include <SuperClass.h>

// Ares hooked at 0x6CC390 and jumped to 0x6CDE40
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
		Debug::Log(__FUNCTION__": Hook entered with an invalid or corrupt SuperClass pointer.");

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

DEFINE_HOOK(0x6CBEF4, SuperClass_AnimStage_UseWeeds, 0x6)
{
	enum
	{
		Ready = 0x6CBFEC,
		NotReady = 0x6CC064,
		ProgressInEax = 0x6CC066
	};

	constexpr int maxCounterFrames = 54;

	GET(SuperClass*, pSuper, ECX);
	GET(SuperWeaponTypeClass*, pSWType, EBX);

	auto pExt = SWTypeExt::ExtMap.Find(pSWType);

	if (pExt->UseWeeds)
	{
		if (pSuper->IsReady)
			return Ready;

		if (pExt->UseWeeds_StorageTimer)
		{
			int progress = static_cast<int>(pSuper->Owner->OwnedWeed.GetTotalAmount() * maxCounterFrames / pExt->UseWeeds_Amount);
			if (progress > maxCounterFrames)
				progress = maxCounterFrames;

			R->EAX(progress);
			return ProgressInEax;
		}
		else
		{
			return NotReady;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6CBD2C, SuperClass_AI_UseWeeds, 0x6)
{
	enum
	{
		NothingChanged = 0x6CBE9D,
		SomethingChanged = 0x6CBD48,
		Charged = 0x6CBD73
	};

	enum
	{
		SWReadyTimer = 0,
		SWAlmostReadyTimer = 15,
		SWNotReadyTimer = 915
	};

	GET(SuperClass*, pSuper, ESI);

	auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);

	if (pExt->UseWeeds)
	{
		if (pSuper->Type->ShowTimer)
			pSuper->Type->ShowTimer = false;

		if (pSuper->Owner->OwnedWeed.GetTotalAmount() >= pExt->UseWeeds_Amount)
		{
			pSuper->Owner->OwnedWeed.RemoveAmount(static_cast<float>(pExt->UseWeeds_Amount), 0);
			pSuper->RechargeTimer.Start(SWReadyTimer); // The Armageddon is here
			return Charged;
		}

		if (pSuper->Owner->OwnedWeed.GetTotalAmount() >= pExt->UseWeeds_ReadinessAnimationPercentage / 100.0 * pExt->UseWeeds_Amount)
		{
			pSuper->RechargeTimer.Start(SWAlmostReadyTimer); // The end is nigh!
		}
		else
		{
			pSuper->RechargeTimer.Start(SWNotReadyTimer); // 61 seconds > 60 seconds (animation activation threshold)
		}

		int animStage = pSuper->AnimStage();
		if (pSuper->CameoChargeState != animStage)
		{
			pSuper->CameoChargeState = animStage;
			return SomethingChanged;
		}

		return NothingChanged;
	}

	return 0;
}

// This is pointless for SWs using weeds because their charge is tied to weed storage.
DEFINE_HOOK(0x6CC1E6, SuperClass_SetSWCharge_UseWeeds, 0x5)
{
	enum { Skip = 0x6CC251 };

	GET(SuperClass*, pSuper, EDI);

	auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);

	if (pExt->UseWeeds)
		return Skip;

	return 0;
}
