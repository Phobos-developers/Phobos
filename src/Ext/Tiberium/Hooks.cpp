#include "Body.h"

#include <CellClass.h>
#include <MapClass.h>
#include <OverlayClass.h>
#include <ScenarioClass.h>

namespace
{
	// West=1, North=2, East=3, South=4. Higher indices (corners, steep ramps) lack Tiberium overlay art.
	constexpr BYTE MaxCardinalRampIndex = 4;
	constexpr int RequiredRampOverlays = 8;

	static_assert(offsetof(TiberiumClass, SpreadLogic) == 0xF0, "TiberiumClass::SpreadLogic offset mismatch");
	static_assert(offsetof(TiberiumClass, GrowthLogic) == 0x10C, "TiberiumClass::GrowthLogic offset mismatch");

	void ReindexGrowth(TiberiumClass* pThis)
	{
		reinterpret_cast<void(__thiscall*)(TiberiumClass*)>(0x7233A0)(pThis);
	}

	void ReindexSpread(TiberiumClass* pThis)
	{
		reinterpret_cast<void(__thiscall*)(TiberiumClass*)>(0x7228B0)(pThis);
	}

	bool CanResourceGerminateOnRamp(TiberiumClass* pTib, BYTE slopeIndex)
	{
		if (slopeIndex == 0)
			return true;

		if (slopeIndex > MaxCardinalRampIndex)
			return false;

		if (pTib)
		{
			const auto pExt = TiberiumExt::TryFetch(pTib);
			if (pExt && pExt->AllowRamps)
			{
				if (pTib->NumSlopes < RequiredRampOverlays)
					pTib->NumSlopes = RequiredRampOverlays;

				return true;
			}

			return false;
		}

		for (const auto pItem : TiberiumClass::Array)
		{
			if (const auto pExt = TiberiumExt::TryFetch(pItem))
			{
				if (pExt->AllowRamps)
				{
					if (pItem->NumSlopes < RequiredRampOverlays)
						pItem->NumSlopes = RequiredRampOverlays;

					return true;
				}
			}
		}

		return false;
	}
}

// =============================================================================
// Radar / Minimap Colors
// =============================================================================

DEFINE_HOOK(0x47C210, CellClass_CellColor_TiberiumRadarColor, 0x6)
{
	enum { ReturnFromFunction = 0x47C23F };

	GET(CellClass*, pThis, ESI);
	GET_STACK(ColorStruct*, arg0, STACK_OFFSET(0x14, 0x4));
	GET_STACK(ColorStruct*, arg4, STACK_OFFSET(0x14, 0x8));

	const int tiberiumType = OverlayClass::GetTiberiumType(pThis->OverlayTypeIndex);

	if (tiberiumType < 0)
		return 0;

	const auto pTiberium = TiberiumClass::Array.GetItem(tiberiumType);

	if (const auto pTiberiumExt = TiberiumExt::TryFetch(pTiberium))
	{
		if (pTiberiumExt->MinimapColor.isset())
		{
			auto& color = pTiberiumExt->MinimapColor.Get();

			arg0->R = color.R;
			arg0->G = color.G;
			arg0->B = color.B;

			arg4->R = color.R;
			arg4->G = color.G;
			arg4->B = color.B;

			R->ECX(arg4);
			R->AL(color.B);

			return ReturnFromFunction;
		}
	}

	return 0;
}

// =============================================================================
// Tiberium Growth & Spread Heap Overflow / Capacity Guards
// =============================================================================

DEFINE_HOOK(0x7235CE, TiberiumClass_Queue_Growth_At_Cell_CapacityGuard, 0x5)
{
	enum { ReturnEarly = 0x7236C2 };

	GET(TiberiumClass*, pThis, ESI);
	const int surfaceIdx = R->Stack<int>(0x0C);
	const auto& logic = pThis->GrowthLogic;
	const int maxCount = PriorityQueueClassNode::SurfaceDataCount();

	// Prevent duplicate queue entries for cells already awaiting growth
	if (logic.CellIndexesWithTiberium && surfaceIdx >= 0 && surfaceIdx < maxCount)
	{
		if (logic.CellIndexesWithTiberium[surfaceIdx])
			return ReturnEarly;
	}

	// Replicate stolen instruction: call 0x42B1F0 (returns surface count in EAX)
	R->EAX(maxCount);

	return 0;
}

DEFINE_HOOK(0x72302E, TiberiumClass_Grow_RequeueGuard, 0x6)
{
	GET(TiberiumClass*, pThis, ESI);
	const int maxCount = PriorityQueueClassNode::SurfaceDataCount();

	// Compact and re-index active cells before buffer overflows
	if (pThis->GrowthLogic.Count >= maxCount - 20)
		ReindexGrowth(pThis);

	// Replicate stolen instruction: mov eax, dword ptr [esi + 0x10C]
	R->EAX(pThis->GrowthLogic.Count);

	return 0;
}

DEFINE_HOOK(0x722FF2, TiberiumClass_Grow_NodeSanityCheck, 0x8)
{
	enum { ExitLoop = 0x72324E, NextIteration = 0x72312F };

	GET(PriorityQueueClassNode*, pNode, EBX);

	if (!pNode)
		return ExitLoop;

	if (!MapClass::Instance.CellExists(pNode->MapCoord))
		return NextIteration;

	return 0;
}

DEFINE_HOOK(0x722B48, TiberiumClass_Queue_Spread_At_Cell_CapacityGuard, 0x6)
{
	GET(TiberiumClass*, pThis, ESI);
	const int maxCount = PriorityQueueClassNode::SurfaceDataCount();

	if (pThis->SpreadLogic.Count >= maxCount - 20)
		ReindexSpread(pThis);

	// Replicate stolen instruction: mov ecx, dword ptr [esi + 0xF0]
	R->ECX(pThis->SpreadLogic.Count);

	return 0;
}

DEFINE_HOOK(0x72252E, TiberiumClass_Spread_NodeSanityCheck, 0x8)
{
	enum { ExitLoop = 0x72275F, NextIteration = 0x722657 };

	GET(PriorityQueueClassNode*, pNode, EAX);

	if (!pNode)
		return ExitLoop;

	if (!MapClass::Instance.CellExists(pNode->MapCoord))
		return NextIteration;

	return 0;
}

DEFINE_HOOK(0x722586, TiberiumClass_Spread_RequeueGuard, 0x6)
{
	GET(TiberiumClass*, pThis, EBX);
	const int maxCount = PriorityQueueClassNode::SurfaceDataCount();

	// Compact and re-index active cells before buffer overflows
	if (pThis->SpreadLogic.Count >= maxCount - 20)
		ReindexSpread(pThis);

	// Replicate stolen instruction: mov eax, dword ptr [ebx + 0xF0]
	R->EAX(pThis->SpreadLogic.Count);

	return 0;
}

DEFINE_HOOK(0x722574, TiberiumClass_Spread_StallFix, 0x5)
{
	enum { Requeue = 0x722586, ClearFlagAndFinish = 0x722645, Finish = 0x722657 };

	const bool spreadSuccess = (R->AL() != 0);
	GET(int, eligibleNeighbors, EBP);

	// Replicate stolen instructions that increment loop counter
	const int loopCount = R->Stack<int>(0x10) + 1;
	R->Stack<int>(0x10, loopCount);
	R->ECX(loopCount);

	// When spread fails due to temporary obstacles, re-queue the cell so expansion does not stall
	if (!spreadSuccess)
		return Requeue;

	// When all available neighbors have been successfully seeded, clear tracking flag and complete
	if (eligibleNeighbors <= 1)
		return ClearFlagAndFinish;

	// Additional unseeded neighbors remain
	return Requeue;
}

// =============================================================================
// Ramp Tiberium Support & Division by Zero Safety
// =============================================================================

DEFINE_HOOK(0x4839B6, CellClass_CanTiberiumGerminate_RampSupport, 0xA)
{
	enum { Disallow = 0x4839E9, ContinueChecks = 0x4839C0, AllowRamp = 0x4839E2 };

	GET(CellClass*, pThis, EDI);

	if (pThis->SlopeIndex != 0)
	{
		TiberiumClass* pTib = nullptr;

		const auto pEbx = reinterpret_cast<TiberiumClass*>(R->EBX());
		if (TiberiumClass::Array.FindItemIndex(pEbx) != -1)
			pTib = pEbx;
		else
		{
			const auto pArg = R->Stack<TiberiumClass*>(0x0C);
			if (TiberiumClass::Array.FindItemIndex(pArg) != -1)
				pTib = pArg;
			else
			{
				const auto pCallerTib = R->Stack<TiberiumClass*>(0x30);
				if (TiberiumClass::Array.FindItemIndex(pCallerTib) != -1)
					pTib = pCallerTib;
			}
		}

		if (CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
			return AllowRamp;

		return Disallow;
	}

	return ContinueChecks;
}

DEFINE_HOOK(0x47D36E, CellClass_RecalcAttributes_PreserveRampTiberium, 0x18)
{
	GET(CellClass*, pThis, ESI);
	GET(OverlayTypeClass*, pOverlayType, EBP);

	if (pOverlayType && pOverlayType->Tiberium)
	{
		const int tibType = OverlayClass::GetTiberiumType(pOverlayType->ArrayIndex);
		const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibType);

		if (!CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
		{
			pThis->OverlayTypeIndex = -1;
			pThis->OverlayData = 0;
		}
	}

	return 0x47D386;
}

DEFINE_HOOK(0x483650, CellClass_CanTiberiumGrow_RampSupport, 0x6)
{
	enum { Disallow = 0x48365A, ContinueChecks = 0x48365E };

	GET(CellClass*, pThis, ESI);

	if (pThis->SlopeIndex != 0)
	{
		const int tibType = OverlayClass::GetTiberiumType(pThis->OverlayTypeIndex);
		const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibType);

		if (!CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
			return Disallow;
	}

	return ContinueChecks;
}

DEFINE_HOOK(0x4836CF, CellClass_CanTiberiumSpread_RampSupport, 0xA)
{
	enum { Disallow = 0x4836D9, ContinueChecks = 0x4836DE };

	GET(CellClass*, pThis, ESI);

	if (pThis->SlopeIndex != 0)
	{
		GET(int, tibIndex, EDI);
		const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibIndex);

		if (!CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
			return Disallow;
	}

	return ContinueChecks;
}

DEFINE_HOOK(0x483738, CellClass_CanTiberiumGrow_RampCheck, 0xA)
{
	enum { Disallow = 0x48377C, ContinueChecks = 0x483742 };

	GET(CellClass*, pThis, ESI);
	GET(TiberiumClass*, pTib, EAX);

	if (pThis->SlopeIndex != 0)
	{
		if (!CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
			return Disallow;
	}

	return ContinueChecks;
}

DEFINE_HOOK(0x4837BB, CellClass_SpreadTiberium_RampSupport, 0xA)
{
	enum { Disallow = 0x483800, ContinueSpread = 0x4837C5 };

	GET(CellClass*, pThis, EDI);

	if (pThis->SlopeIndex != 0)
	{
		GET(int, tibIndex, ESI);
		const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibIndex);

		if (!CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
			return Disallow;
	}

	return ContinueSpread;
}

DEFINE_HOOK(0x4873A7, CellClass_IncreaseTiberium_RampStageSupport, 0x11)
{
	enum { Disallow = 0x48761E, ContinueGrowth = 0x4873B8 };

	GET(CellClass*, pThis, ESI);
	GET(int, tibIndex, EAX);

	const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibIndex);
	R->EAX(pTib);

	if (pThis->SlopeIndex != 0)
	{
		if (!CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
			return Disallow;
	}

	return ContinueGrowth;
}

DEFINE_HOOK(0x47F87D, CellClass_DrawOverlay_DivZeroGuard, 0x9)
{
	enum { DrawFlat = 0x47F8D7, DrawRamp = 0x47F888 };

	const BYTE slopeIndex = static_cast<BYTE>(R->CL());
	GET(int, tibIndex, EAX);

	const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibIndex);
	R->EBX(pTib);
	R->Stack<TiberiumClass*>(0x28, pTib);

	if (slopeIndex != 0 && CanResourceGerminateOnRamp(pTib, slopeIndex))
		return DrawRamp;

	return DrawFlat;
}

DEFINE_HOOK(0x47FBE5, CellClass_GetContainingRect_DivZeroGuard, 0xA)
{
	enum { RectFlat = 0x47FC49, RectRamp = 0x47FBEF };

	GET(CellClass*, pThis, ESI);
	GET(TiberiumClass*, pTib, EDI);

	R->CL(pThis->SlopeIndex);

	if (pThis->SlopeIndex != 0 && CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
		return RectRamp;

	return RectFlat;
}

