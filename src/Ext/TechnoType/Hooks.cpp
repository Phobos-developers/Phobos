#include "Body.h"
#include <Ext/House/Body.h>
#include <Ext/AnimType/Body.h>

DEFINE_HOOK(0x73D223, UnitClass_DrawIt_OreGath, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(const int, nFacing, EDI);
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFSET(0x50, 0x8));
	LEA_STACK(Point2D*, pLocation, STACK_OFFSET(0x50, -0x18));
	GET_STACK(const int, nBrightness, STACK_OFFSET(0x50, 0x4));

	auto const pData = TechnoTypeExt::ExtMap.Find(pThis->Type);

	ConvertClass* pDrawer = FileSystem::ANIM_PAL;
	SHPStruct* pSHP = FileSystem::OREGATH_SHP;
	int idxFrame;

	const int idxTiberium = pThis->GetCell()->GetContainedTiberiumIndex();
	const int idxArray = pData->OreGathering_Tiberiums.size() > 0 ? pData->OreGathering_Tiberiums.IndexOf(idxTiberium) : 0;
	if (idxTiberium != -1 && idxArray != -1)
	{
		auto const pAnimType = pData->OreGathering_Anims.size() > 0 ? pData->OreGathering_Anims[idxArray] : nullptr;
		auto const nFramesPerFacing = pData->OreGathering_FramesPerDir.size() > 0 ? pData->OreGathering_FramesPerDir[idxArray] : 15;
		auto const pAnimExt = AnimTypeExt::ExtMap.TryFind(pAnimType);
		if (pAnimType)
		{
			pSHP = pAnimType->GetImage();
			if (auto const pPalette = pAnimExt->Palette.GetConvert())
				pDrawer = pPalette;
		}
		idxFrame = nFramesPerFacing * nFacing + (Unsorted::CurrentFrame + pThis->WalkedFramesSoFar) % nFramesPerFacing;
	}
	else
	{
		idxFrame = 15 * nFacing + (Unsorted::CurrentFrame + pThis->WalkedFramesSoFar) % 15;
	}

	DSurface::Temp->DrawSHP(
		pDrawer, pSHP, idxFrame, pLocation, pBounds,
		BlitterFlags::Flat | BlitterFlags::Alpha | BlitterFlags::Centered,
		0, pThis->GetZAdjustment() - 2, ZGradient::Ground, nBrightness,
		0, nullptr, 0, 0, 0
	);

	R->EBP(nBrightness);
	R->EBX(pBounds);

	return 0x73D28C;
}

// Issue #503
// Author : Otamaa
DEFINE_HOOK(0x4AE670, DisplayClass_GetToolTip_EnemyUIName, 0x8)
{
	enum { SetUIName = 0x4AE678 };

	GET(ObjectClass*, pObject, ECX);

	auto pDecidedUIName = pObject->GetUIName();
	const auto pFoot = generic_cast<FootClass*, true>(pObject);
	const auto pTechnoType = pObject->GetTechnoType();

	if (pFoot && pTechnoType && !pObject->IsDisguised())
	{
		const auto pOwnerHouse = pFoot->Owner;
		const bool IsAlly = pOwnerHouse->IsAlliedWith(HouseClass::CurrentPlayer);
		const bool IsCivilian = (pOwnerHouse == HouseClass::FindCivilianSide()) || pOwnerHouse->IsNeutral();
		const bool IsObserver = HouseClass::Observer || HouseClass::IsCurrentPlayerObserver();

		if (!IsAlly && !IsCivilian && !IsObserver)
		{
			const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

			if (const auto pEnemyUIName = pTechnoTypeExt->EnemyUIName.Get().Text)
			{
				pDecidedUIName = pEnemyUIName;
			}
		}
	}

	R->EAX(pDecidedUIName);
	return SetUIName;
}

DEFINE_HOOK(0x711F39, TechnoTypeClass_CostOf_FactoryPlant, 0x8)
{
	GET(TechnoTypeClass*, pThis, ESI);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(float, mult, STACK_OFFSET(0x10, -0x8));

	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);

	if (pHouseExt->RestrictedFactoryPlants.size() > 0)
		mult *= pHouseExt->GetRestrictedFactoryPlantMult(pThis);

	return 0;
}

DEFINE_HOOK(0x711FDF, TechnoTypeClass_RefundAmount_FactoryPlant, 0x8)
{
	GET(TechnoTypeClass*, pThis, ESI);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(float, mult, STACK_OFFSET(0x10, -0x4));

	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);

	if (pHouseExt->RestrictedFactoryPlants.size() > 0)
		mult *= pHouseExt->GetRestrictedFactoryPlantMult(pThis);

	return 0;
}


DEFINE_HOOK(0x71464A, TechnoTypeClass_ReadINI_Speed, 0x7)
{
	enum { SkipGameCode = 0x71469F };

	GET(TechnoTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, ESI);
	GET(char*, pSection, EBX);
	GET(int, eliteAirstrikeRechargeTime, EAX);

	pThis->EliteAirstrikeRechargeTime = eliteAirstrikeRechargeTime; // Restore overridden instructions.
	INI_EX exINI(pINI);
	exINI.ReadSpeed(pSection, "Speed", &pThis->Speed);

	return SkipGameCode;
}
