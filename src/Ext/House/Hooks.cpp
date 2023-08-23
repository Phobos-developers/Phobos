#include "Body.h"
#include "../Techno/Body.h"
#include "../Building/Body.h"
#include <unordered_map>

DEFINE_HOOK(0x4F8440, HouseClass_Update_Beginning, 0x5)
{
	GET(HouseClass* const, pThis, ECX);

	auto pExt = HouseExt::ExtMap.Find(pThis);

	pExt->UpdateAutoDeathObjectsInLimbo();

	return 0;
}

DEFINE_HOOK(0x508C30, HouseClass_UpdatePower_UpdateCounter, 0x5)
{
	GET(HouseClass*, pThis, ECX);
	auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	pHouseExt->PowerPlantEnhancers.clear();

	// This pre-iterating ensure our process to be done in O(NM) instead of O(N^2),
	// as M should be much less than N, this will be a great improvement. - secsome
	for (auto& pBld : pThis->Buildings)
	{
		if (TechnoExt::IsActive(pBld) && pBld->IsOnMap && pBld->HasPower)
		{
			const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

			if (pExt->PowerPlantEnhancer_Buildings.size() &&
				(pExt->PowerPlantEnhancer_Amount != 0 || pExt->PowerPlantEnhancer_Factor != 1.0f))
			{
				++pHouseExt->PowerPlantEnhancers[pExt];
			}
		}
	}

	return 0;
}

// Power Plant Enhancer #131
DEFINE_HOOK(0x508CF2, HouseClass_UpdatePower_PowerOutput, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	pThis->PowerOutput += BuildingTypeExt::GetEnhancedPower(pBld, pThis);

	return 0x508D07;
}

DEFINE_HOOK(0x73E474, UnitClass_Unload_Storage, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amount, 0x1C);

	auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
	if (!pTypeExt)
		return 0;

	if (!pBuilding->Owner)
		return 0;

	auto storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

	if (pTypeExt->Refinery_UseStorage && storageTiberiumIndex >= 0)
	{
		BuildingExt::StoreTiberium(pBuilding, amount, idxTiberium, storageTiberiumIndex);
		amount = 0.0f;
	}

	return 0;
}

namespace RecalcCenterTemp
{
	HouseExt::ExtData* pExtData;
}

DEFINE_HOOK(0x4FD166, HouseClass_RecalcCenter_SetContext, 0x5)
{
	GET(HouseClass* const, pThis, EDI);

	RecalcCenterTemp::pExtData = HouseExt::ExtMap.Find(pThis);

	return 0;
}

DEFINE_HOOK_AGAIN(0x4FD463, HouseClass_RecalcCenter_LimboDelivery, 0x6)
DEFINE_HOOK(0x4FD1CD, HouseClass_RecalcCenter_LimboDelivery, 0x6)
{
	enum { SkipBuilding1 = 0x4FD23B, SkipBuilding2 = 0x4FD4D5 };

	GET(BuildingClass* const, pBuilding, ESI);

	auto const pExt = RecalcCenterTemp::pExtData;

	if (pExt && pExt->OwnsLimboDeliveredBuilding(pBuilding))
		return R->Origin() == 0x4FD1CD ? SkipBuilding1 : SkipBuilding2;

	return 0;
}

#pragma region LimboTracking

namespace LimboTrackingTemp
{
	bool IsBeingDeleted = false;
}

void __fastcall TechnoClass_UnInit_Wrapper(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();

	if (pThis->InLimbo && !pType->Insignificant && !pType->DontScore)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		pOwnerExt->RemoveFromLimboTracking(pType);
	}

	LimboTrackingTemp::IsBeingDeleted = true;
	pThis->ObjectClass::UnInit();
	LimboTrackingTemp::IsBeingDeleted = false;
}

DEFINE_JUMP(CALL, 0x4DE60B, GET_OFFSET(TechnoClass_UnInit_Wrapper));   // FootClass
DEFINE_JUMP(VTABLE, 0x7E3FB4, GET_OFFSET(TechnoClass_UnInit_Wrapper)); // BuildingClass

DEFINE_HOOK(0x4FF754, HouseClass_TrackingAdd_AddTracking, 0x6)
{
	GET(HouseClass* const, pThis, EDI);
	GET(TechnoClass* const, pTechno, ESI);
	GET_STACK(unsigned int, callerAddress, STACK_OFFSET(0xC, 0x0));

	auto const pType = pTechno->GetTechnoType();

	if (callerAddress != 0x7015EB)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis);
		pOwnerExt->AddToLimboTracking(pType);
	}

	return 0;
}

DEFINE_HOOK(0x6F6BC9, TechnoClass_Limbo_AddTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (!LimboTrackingTemp::IsBeingDeleted && !pType->Insignificant && !pType->DontScore)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		pOwnerExt->AddToLimboTracking(pType);
	}

	return 0;
}

DEFINE_HOOK(0x6F6D85, TechnoClass_Unlimbo_RemoveTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (!pType->Insignificant && !pType->DontScore)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		pOwnerExt->RemoveFromLimboTracking(pType);
	}

	return 0;
}

DEFINE_HOOK(0x7015C9, TechnoClass_Captured_UpdateTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EBP);

	auto const pType = pThis->GetTechnoType();

	if (!pType->Insignificant && !pType->DontScore)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		pOwnerExt->RemoveFromLimboTracking(pType);
		auto const pNewOwnerExt = HouseExt::ExtMap.Find(pNewOwner);
		pNewOwnerExt->AddToLimboTracking(pType);
	}

	return 0;
}

#pragma endregion
