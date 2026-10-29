#include "Body.h"

#include <CCINIClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <OverlayClass.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <Theater.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace
{
	// West=1, North=2, East=3, South=4. Higher indices (corners, steep ramps) lack Tiberium overlay art.
	constexpr BYTE MaxCardinalRampIndex = 4;
	constexpr int RequiredRampOverlays = 8;
	constexpr int RampOverlayZAdjustOffset = 30;

	int GetRampSlopeOffset(const CellClass* pCell, const TiberiumClass* pTib)
	{
		const int numPerSlope = (pTib && pTib->NumSlopes >= 4) ? (pTib->NumSlopes / 4) : 2;
		const int variation = (static_cast<int>(pCell->MapCoords.X) * static_cast<int>(pCell->MapCoords.Y)) % numPerSlope;
		return (static_cast<int>(pCell->SlopeIndex) - 1) * numPerSlope + variation;
	}

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

	int GetMaxValidCells(int maxSurfaceCount)
	{
		return maxSurfaceCount - 20;
	}

	bool HasValidSHPData(SHPStruct* pSHP)
	{
		if (!pSHP)
			return false;

		pSHP->Load();
		return pSHP->GetData() != nullptr;
	}

	bool HasRampArt(TiberiumClass* pTib)
	{
		if (!pTib)
			return false;

		if (const auto pExt = TiberiumExt::TryFetch(pTib))
			return pExt->HasRampOverlays();

		if (!pTib->Image)
			return false;

		const int rampOverlayIdx = pTib->Image->ArrayIndex + pTib->NumImages;
		if (rampOverlayIdx >= OverlayTypeClass::Array.Count)
			return false;

		const auto pRampOverlay = OverlayTypeClass::Array.GetItem(rampOverlayIdx);
		return pRampOverlay && pRampOverlay->Tiberium;
	}

	OverlayTypeClass* GetRampOverlay(TiberiumClass* pTib, int slopeOffset)
	{
		if (!pTib)
			return nullptr;

		OverlayTypeClass* pRamp = nullptr;

		if (const auto pExt = TiberiumExt::TryFetch(pTib))
		{
			pRamp = pExt->GetRampOverlay(slopeOffset);
		}
		else if (pTib->Image)
		{
			const int rampOverlayIdx = pTib->Image->ArrayIndex + pTib->NumImages + slopeOffset;
			pRamp = OverlayTypeClass::Array.GetItemOrDefault(rampOverlayIdx);
		}

		if (pRamp)
		{
			pRamp->Tiberium = true;

			if (!HasValidSHPData(pRamp->Image))
			{
				std::vector<std::string> candidateNames;
				auto addCandidate = [&candidateNames](const char* pName)
				{
					if (!pName || !pName[0])
						return;

					for (const auto& existing : candidateNames)
					{
						if (_stricmp(existing.c_str(), pName) == 0)
							return;
					}

					candidateNames.emplace_back(pName);
				};

				addCandidate(pRamp->ImageFile);
				addCandidate(pRamp->ID);

				if (pTib->Image && pTib->Image->ImageFile && pTib->Image->ImageFile[0])
				{
					std::string basePrefix = pTib->Image->ImageFile;
					while (!basePrefix.empty() && std::isdigit(static_cast<unsigned char>(basePrefix.back())))
						basePrefix.pop_back();

					if (!basePrefix.empty())
					{
						const int variety = pTib->NumImages > 0 ? pTib->NumImages : 12;
						char derived[64];
						sprintf_s(derived, "%s%02d", basePrefix.c_str(), variety + 1 + slopeOffset);
						addCandidate(derived);

						sprintf_s(derived, "%s%02d", basePrefix.c_str(), 13 + slopeOffset);
						addCandidate(derived);
					}
				}

				const size_t initialCount = candidateNames.size();
				for (size_t i = 0; i < initialCount; ++i)
				{
					char artImage[64] = { 0 };
					CCINIClass::INI_Art.ReadString(candidateNames[i].c_str(), "Image", "", artImage, sizeof(artImage));
					addCandidate(artImage);
				}

				for (const auto& name : candidateNames)
				{
					if (const auto pOther = OverlayTypeClass::Find(name.c_str()))
					{
						pOther->Tiberium = true;

						if (HasValidSHPData(pOther->Image))
						{
							pRamp->Image = pOther->Image;
							break;
						}
					}
				}

				if (!HasValidSHPData(pRamp->Image))
				{
					std::vector<const char*> extensions;
					if (ScenarioClass::Instance)
					{
						const auto theater = ScenarioClass::Instance->Theater;
						const auto& ext = Theater::GetTheater(theater).Extension;
						if (ext[0] != '\0')
							extensions.push_back(ext);
					}

					extensions.push_back("shp");
					extensions.push_back("DES");
					extensions.push_back("SNO");
					extensions.push_back("LUN");
					extensions.push_back("UBN");
					extensions.push_back("TEM");
					extensions.push_back("URB");

					for (const auto& name : candidateNames)
					{
						bool loaded = false;
						for (const char* pExtStr : extensions)
						{
							char filename[64];
							sprintf_s(filename, "%s.%s", name.c_str(), pExtStr);
							if (auto pSHP = FileSystem::LoadSHPFile(filename))
							{
								if (HasValidSHPData(pSHP))
								{
									pRamp->Image = pSHP;
									loaded = true;
									break;
								}
							}
						}

						if (loaded)
							break;
					}
				}
			}
		}

		return pRamp;
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
			if (pExt && pExt->AllowRamps && HasRampArt(pTib))
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
				if (pExt->AllowRamps && HasRampArt(pItem))
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
	GET(CellStruct*, pCellCoords, EDI);
	const auto& logic = pThis->GrowthLogic;
	const int maxCount = PriorityQueueClassNode::SurfaceDataCount();
	const int maxCells = GetMaxValidCells(maxCount);

	if (pThis->GrowthLogic.Count >= maxCells)
		ReindexGrowth(pThis);

	// Prevent duplicate queue entries for cells already awaiting growth
	if (logic.CellIndexesWithTiberium && pCellCoords)
	{
		const int surfaceIdx = PriorityQueueClassNode::ToSurfaceIndex(*pCellCoords);
		if (surfaceIdx >= 0 && surfaceIdx < maxCount && logic.CellIndexesWithTiberium[surfaceIdx])
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
	const int maxCells = GetMaxValidCells(maxCount);

	// Compact and re-index active cells before buffer overflows
	if (pThis->GrowthLogic.Count >= maxCells)
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
	enum { ReturnEarly = 0x722C2A };

	GET(TiberiumClass*, pThis, ESI);
	GET(CellStruct*, pCellCoords, EBP);
	const auto& logic = pThis->SpreadLogic;
	const int maxCount = PriorityQueueClassNode::SurfaceDataCount();
	const int maxCells = GetMaxValidCells(maxCount);

	if (pThis->SpreadLogic.Count >= maxCells)
		ReindexSpread(pThis);

	// Prevent duplicate queue entries for cells already awaiting spread
	if (logic.CellIndexesWithTiberium && pCellCoords)
	{
		const int surfaceIdx = PriorityQueueClassNode::ToSurfaceIndex(*pCellCoords);
		if (surfaceIdx >= 0 && surfaceIdx < maxCount && logic.CellIndexesWithTiberium[surfaceIdx])
			return ReturnEarly;
	}

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
	const int maxCells = GetMaxValidCells(maxCount);

	// Compact and re-index active cells before buffer overflows
	if (pThis->SpreadLogic.Count >= maxCells)
		ReindexSpread(pThis);

	// Replicate stolen instruction: mov eax, dword ptr [ebx + 0xF0]
	R->EAX(pThis->SpreadLogic.Count);

	return 0;
}

DEFINE_HOOK(0x722574, TiberiumClass_Spread_StallFix, 0x5)
{
	enum { Requeue = 0x722586, Finish = 0x722657 };

	const bool spreadSuccess = (R->AL() != 0);
	GET(int, eligibleNeighbors, EBP);

	// Replicate stolen instructions that increment loop counter
	const int loopCount = R->Stack<int>(0x10) + 1;
	R->Stack<int>(0x10, loopCount);
	R->ECX(loopCount);

	// When spread succeeds, 1 neighbor was seeded. If more candidates remain, re-queue with delay
	if (spreadSuccess && eligibleNeighbors > 1)
		return Requeue;

	// Completed or spread failed (e.g. cell occupied by unit/structure) - do not spin
	return Finish;
}

DEFINE_HOOK(0x7225AB, TiberiumClass_Spread_RequeueScore, 0x8)
{
	GET(DWORD, nodesBase, ECX);
	GET(DWORD, nodeIndex, EAX);

	const int delay = ScenarioClass::Instance
		? ScenarioClass::Instance->Random.RandomRanged(1, 50)
		: 25;
	const float score = static_cast<float>(Unsorted::CurrentFrame + delay);

	auto pScore = reinterpret_cast<float*>(nodesBase + nodeIndex * 8 + 4);
	*pScore = score;

	return 0x7225B3;
}

DEFINE_HOOK(0x72311E, TiberiumClass_GrowthAI_MaxStageSpread, 0x7)
{
	GET(TiberiumClass*, pThis, ESI);
	GET(CellStruct*, pCellCoords, EBX);

	if (pThis && pCellCoords)
	{
		const int surfaceIdx = PriorityQueueClassNode::ToSurfaceIndex(*pCellCoords);
		const int maxCount = PriorityQueueClassNode::SurfaceDataCount();
		const auto& logic = pThis->SpreadLogic;

		if (surfaceIdx >= 0 && surfaceIdx < maxCount)
		{
			if (!logic.CellIndexesWithTiberium || !logic.CellIndexesWithTiberium[surfaceIdx])
				pThis->RegisterForSpread(pCellCoords);
		}

		R->EAX(surfaceIdx);
	}

	return 0;
}

// =============================================================================
// Ramp Tiberium Support & Division by Zero Safety
// =============================================================================

DEFINE_HOOK(0x48399C, CellClass_CanTiberiumGerminate_RampSupport, 0x6)
{
	enum { Disallow = 0x4839E9, AllowRamp = 0x4839E2, ContinueFlat = 0x4839A2 };

	GET(CellClass*, pThis, EDI);

	if (pThis->SlopeIndex != 0)
	{
		if (pThis->OverlayTypeIndex != -1)
			return Disallow;

		TiberiumClass* pTib = nullptr;

		const auto pEbx = reinterpret_cast<TiberiumClass*>(R->EBX());
		if (TiberiumClass::Array.FindItemIndex(pEbx) != -1)
		{
			pTib = pEbx;
		}
		else
		{
			const auto pArg = R->Stack<TiberiumClass*>(0x0C);
			if (TiberiumClass::Array.FindItemIndex(pArg) != -1)
			{
				pTib = pArg;
			}
			else
			{
				const auto pCallerTib = R->Stack<TiberiumClass*>(0x30);
				if (TiberiumClass::Array.FindItemIndex(pCallerTib) != -1)
					pTib = pCallerTib;
			}
		}

		const bool allowed = CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex);

		if (allowed)
			return AllowRamp;

		return Disallow;
	}

	R->EAX(static_cast<DWORD>(pThis->LandType));

	return ContinueFlat;
}

DEFINE_HOOK(0x47D36E, CellClass_RecalcAttributes_PreserveRampTiberium, 0x18)
{
	GET(CellClass*, pThis, ESI);
	GET(OverlayTypeClass*, pOverlayType, EBP);

	if (pOverlayType && pOverlayType->Tiberium)
	{
		const int tibType = OverlayClass::GetTiberiumType(pOverlayType->ArrayIndex);
		const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibType);
		const bool keep = CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex);

		if (!keep)
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

DEFINE_HOOK(0x4819D1, CellClass_TiberiumAdjust_Stage, 0x7)
{
	enum { SetStage = 0x4819D8 };

	GET(TiberiumClass*, pTib, ECX);

	static constexpr int AdjTable[9] = { 0, 1, 3, 4, 6, 7, 8, 10, 11 };

	const int numFrames = (pTib && pTib->NumFrames > 0) ? pTib->NumFrames : 12;
	const int count = std::clamp(static_cast<int>(R->EAX()) * numFrames + static_cast<int>(R->EDX()), 0, 8);
	int stage = AdjTable[count];

	if (numFrames != 12)
		stage = (stage * (numFrames - 1)) / 11;

	R->AL(static_cast<BYTE>(stage));
	return SetStage;
}

DEFINE_HOOK(0x4836B7, CellClass_CanTiberiumSpread_StageAndRampSupport, 0xA)
{
	enum { Disallow = 0x4836D9, ContinueChecks = 0x4836DE };

	GET(CellClass*, pThis, ESI);
	GET(int, tibIndex, EDI);

	const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibIndex);
	if (!pTib)
		return Disallow;

	const int minStage = std::min(tibIndex < 4 ? tibIndex / 2 : 1, pTib->NumFrames - 2);
	if (static_cast<int>(pThis->OverlayData) <= minStage)
		return Disallow;

	if (pThis->SlopeIndex != 0 && !CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
		return Disallow;

	return ContinueChecks;
}

DEFINE_HOOK(0x483738, CellClass_CanTiberiumGrow_RampCheck, 0xA)
{
	enum { Disallow = 0x48377C, ContinueChecks = 0x483742 };

	GET(CellClass*, pThis, ESI);
	GET(TiberiumClass*, pTib, EAX);

	if (pThis->SlopeIndex != 0 && !CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
		return Disallow;

	return ContinueChecks;
}

DEFINE_HOOK(0x4837AA, CellClass_SpreadTiberium_StageAndRampSupport, 0xB)
{
	enum { Disallow = 0x483800, ContinueSpread = 0x4837C5 };

	GET(CellClass*, pThis, EDI);
	GET(int, tibIndex, ESI);

	const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibIndex);
	if (!pTib)
		return Disallow;

	const int minStage = std::min(tibIndex < 4 ? tibIndex / 2 : 1, pTib->NumFrames - 2);
	if (static_cast<int>(pThis->OverlayData) <= minStage)
		return Disallow;

	if (pThis->SlopeIndex != 0 && !CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
		return Disallow;

	return ContinueSpread;
}

DEFINE_HOOK(0x4871B6, CellClass_IncreaseTiberium_ClampStage, 0x6)
{
	enum { ContinuePlace = 0x4871C2, Abort = 0x48761E };

	GET(TiberiumClass*, pTib, EDI);
	GET(int, stage, EBX);

	if (!pTib || !pTib->Image || pTib->NumFrames <= 0)
		return Abort;

	R->EBX(std::clamp(stage, 0, pTib->NumFrames - 1));
	return ContinuePlace;
}

DEFINE_HOOK(0x487228, CellClass_IncreaseTiberium_RampOverlayType, 0x9)
{
	enum { SetOverlayAndJump = 0x487289 };

	GET(CellClass*, pThis, ESI);
	GET(TiberiumClass*, pTib, EDI);

	const int slopeOffset = GetRampSlopeOffset(pThis, pTib);
	const auto pRampType = GetRampOverlay(pTib, slopeOffset);

	if (pRampType)
	{
		R->EAX(pRampType);
		return SetOverlayAndJump;
	}

	// Replicate stolen instructions if overlay not found
	R->ECX(R->EDX() + static_cast<int>(pThis->SlopeIndex) * 2);
	R->EDX(pTib->NumImages);

	return 0x487231;
}

DEFINE_HOOK(0x48725C, CellClass_IncreaseTiberium_FlatVariation, 0xA)
{
	enum { SkipRandomCall = 0x48726B };

	GET(TiberiumClass*, pTib, EDI);

	const int maxVar = (pTib && pTib->NumImages > 1) ? (pTib->NumImages - 1) : 0;
	const int variation = ScenarioClass::Instance
		? ScenarioClass::Instance->Random.RandomRanged(0, maxVar)
		: 0;

	R->EAX(variation);
	return SkipRandomCall;
}

DEFINE_HOOK(0x4872A0, CellClass_IncreaseTiberium_Germinate_RegisterSpread, 0x6)
{
	GET(CellClass*, pThis, ESI);
	GET(TiberiumClass*, pTib, EDI);
	const auto stage = static_cast<BYTE>(R->BL());

	pThis->OverlayData = stage;

	if (pThis && pTib && stage >= pTib->NumFrames - 1)
	{
		CellStruct mapCoords = pThis->MapCoords;
		const int surfaceIdx = PriorityQueueClassNode::ToSurfaceIndex(mapCoords);
		const int maxCount = PriorityQueueClassNode::SurfaceDataCount();
		const auto& logic = pTib->SpreadLogic;

		if (surfaceIdx >= 0 && surfaceIdx < maxCount)
		{
			if (!logic.CellIndexesWithTiberium || !logic.CellIndexesWithTiberium[surfaceIdx])
				pTib->RegisterForSpread(&mapCoords);
		}
	}

	return 0x4872A6;
}

DEFINE_HOOK(0x4873A7, CellClass_IncreaseTiberium_RampStageSupport, 0x11)
{
	enum { Disallow = 0x48761E, ContinueGrowth = 0x4873B8 };

	GET(CellClass*, pThis, ESI);
	GET(int, tibIndex, EAX);

	const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibIndex);
	R->EAX(pTib);

	if (pThis->SlopeIndex != 0 && !CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
		return Disallow;

	return ContinueGrowth;
}

DEFINE_HOOK(0x47F532, CellClass_DrawOverlayShadow_RampOverlayType, 0x6)
{
	GET(CellClass*, pThis, ESI);
	GET(OverlayTypeClass*, pOverlayType, EDI);
	GET(int, levelZ, EAX);

	if (pThis->SlopeIndex != 0 && pOverlayType && pOverlayType->Tiberium)
	{
		const int tibIndex = OverlayClass::GetTiberiumType(pThis->OverlayTypeIndex);
		const auto pTib = TiberiumClass::Array.GetItemOrDefault(tibIndex);

		if (CanResourceGerminateOnRamp(pTib, pThis->SlopeIndex))
		{
			const int slopeOffset = GetRampSlopeOffset(pThis, pTib);
			if (const auto pRampType = GetRampOverlay(pTib, slopeOffset))
			{
				if (const auto pImage = pRampType->GetImage())
				{
					if (pImage->GetData())
					{
						pOverlayType = pRampType;
						R->EDI(pOverlayType);
						levelZ += RampOverlayZAdjustOffset;
					}
				}
			}
		}
	}

	R->ECX(pOverlayType);
	R->Stack<int>(0x10, levelZ);

	return 0x47F538;
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

DEFINE_HOOK(0x47F8B7, CellClass_DrawOverlay_RampOverlayType, 0x6)
{
	enum { JumpToDirectDraw = 0x47F957, JumpToFlat = 0x47F8D7 };

	GET(TiberiumClass*, pTib, EAX);
	GET(int, slopeOffset, EDX);

	const auto pRampType = GetRampOverlay(pTib, slopeOffset);
	const auto pImage = pRampType ? pRampType->GetImage() : nullptr;
	const auto pData = pImage ? pImage->GetData() : nullptr;

	if (!pRampType || !pImage || !pData)
	{
		R->EBX(pTib);
		return JumpToFlat;
	}

	// Bypass the broken 0x47F912 SlopeZShape (SLOP01Z..SLOP04Z) branch:
	// 1. SLOP01Z..SLOP04Z in ra2md.mix/ra2.mix use RLE-Zero compression (Flags=3),
	//    which DSurface::DrawSHP (0x4AEF71) misinterprets as uncompressed raw bytes.
	// 2. Passing XOffset=0, YOffset=0 clips off the left half of the 60x60 frame (X < 30).
	// 3. BlitRLE (0x437BBE & 0x437E80) anchors baseZ at FrameBounds.Y and skips per-scanline
	//    baseZ decrements when ZShape != nullptr, pushing uncropped 60x60 frames (GEM13..20,
	//    TIB13..20) +25..+45 Z-units behind the slope tile in the Z-buffer.
	R->EAX(pImage);
	R->EDI(R->EDI() + RampOverlayZAdjustOffset);

	return JumpToDirectDraw;
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

DEFINE_HOOK(0x47FC20, CellClass_GetOverlayRect_RampOverlayType, 0x6)
{
	enum { JumpToCall = 0x47FC39, JumpToFlat = 0x47FC49 };

	GET(TiberiumClass*, pTib, EDI);
	GET(int, slopeOffset, EDX);

	const auto pRampType = GetRampOverlay(pTib, slopeOffset);
	const auto pImage = pRampType ? pRampType->GetImage() : nullptr;
	const auto pData = pImage ? pImage->GetData() : nullptr;

	if (!pRampType || !pImage || !pData)
		return JumpToFlat;

	R->ECX(pRampType);

	return JumpToCall;
}

DEFINE_HOOK(0x5FDD28, OverlayClass_GetTiberiumType_RampOverlays, 0x6)
{
	enum { FoundTiberium = 0x5FDDCF, ReturnInvalid = 0x5FDDD5, FallbackLog = 0x5FDDA4 };

	GET(int, overlayTypeIndex, ECX);

	if (overlayTypeIndex < 0)
		return ReturnInvalid;

	const auto pOverlay = OverlayTypeClass::Array.GetItemOrDefault(overlayTypeIndex);
	if (!pOverlay)
		return ReturnInvalid;

	for (const auto pTib : TiberiumClass::Array)
	{
		if (!pTib)
			continue;

		if (pOverlay->Tiberium && pTib->Image)
		{
			const int baseIdx = pTib->Image->ArrayIndex;
			if (overlayTypeIndex >= baseIdx && overlayTypeIndex < baseIdx + pTib->NumImages)
			{
				R->EAX(pTib->ArrayIndex);
				return FoundTiberium;
			}
		}

		if (const auto pExt = TiberiumExt::TryFetch(pTib))
		{
			if (pExt->HasRampOverlays())
			{
				for (const auto pRampOverlay : pExt->RampOverlays)
				{
					if (!pRampOverlay)
						continue;

					if (pRampOverlay->ArrayIndex == overlayTypeIndex || pRampOverlay == pOverlay)
					{
						pOverlay->Tiberium = true;
						R->EAX(pTib->ArrayIndex);
						return FoundTiberium;
					}

					if (pOverlay->ImageFile && pOverlay->ImageFile[0])
					{
						if (_stricmp(pOverlay->ImageFile, pRampOverlay->ID) == 0 ||
							(pRampOverlay->ImageFile && _stricmp(pOverlay->ImageFile, pRampOverlay->ImageFile) == 0))
						{
							pOverlay->Tiberium = true;
							R->EAX(pTib->ArrayIndex);
							return FoundTiberium;
						}
					}

					if (pRampOverlay->ImageFile && pRampOverlay->ImageFile[0])
					{
						if (_stricmp(pRampOverlay->ImageFile, pOverlay->ID) == 0)
						{
							pOverlay->Tiberium = true;
							R->EAX(pTib->ArrayIndex);
							return FoundTiberium;
						}
					}
				}
			}
			continue;
		}

		if (pOverlay->Tiberium && pTib->Image && pTib->NumSlopes > 0)
		{
			const int slopeBase = pTib->Image->ArrayIndex + pTib->NumImages;
			if (overlayTypeIndex >= slopeBase && overlayTypeIndex < slopeBase + pTib->NumSlopes)
			{
				R->EAX(pTib->ArrayIndex);
				return FoundTiberium;
			}
		}
	}

	if (!pOverlay->Tiberium)
		return ReturnInvalid;

	R->EAX(pOverlay);
	return FallbackLog;
}
