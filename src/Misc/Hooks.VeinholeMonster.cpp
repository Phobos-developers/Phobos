#include <Ext/Anim/Body.h>

#include <CellClass.h>
#include <ColorScheme.h>
#include <HouseClass.h>
#include <IonBlastClass.h>
#include <MapClass.h>
#include <OverlayTypeClass.h>
#include <PriorityQueueClass.h>
#include <RulesClass.h>
#include <ScenarioClass.h>
#include <VeinholeMonsterClass.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace
{
	constexpr int VeinOverlayIndex = 0x7E; // 126 (VEINS)
	constexpr int VeinholeOverlayIndex = 0xA7; // 167 (VEINHOLE)
	constexpr int VeinholeDummyOverlayIndex = 0xB2; // 178 (VEINHOLEDUMMY)
	constexpr unsigned char MatureVeinStartFrame = 0x30; // 48 (frames 48..52 flat mature, 53..60 ramp mature)
	constexpr unsigned char MaxCardRampSlopeIndex = 4; // SlopeIndex 1..4 are cardinal ramps
	constexpr int RampOverlayZAdjustOffset = 30;

	// Note: YRpp VeinholeMonsterClass::IsCurrentPosAffected has an outdated address (0xA83DC8);
	// the actual global in YR 1.001 is at 0xB1D2F0.
	inline bool*& GetAffectedMap()
	{
		return Make_Global<bool*>(0xB1D2F0);
	}

	inline int GetVeinNeighborMask(const CellClass* pCell)
	{
		return reinterpret_cast<int(__thiscall*)(const CellClass*)>(0x485390)(pCell);
	}

	inline int SpreadVeinsToNeighbors(CellClass* pCell)
	{
		return reinterpret_cast<int(__thiscall*)(CellClass*)>(0x485AF0)(pCell);
	}

	inline bool CanVeinsGrowOnLandType(LandType land)
	{
		return land != LandType::Water
			&& land != LandType::Rock
			&& land != LandType::Tiberium
			&& land != LandType::Beach;
	}

	bool CanVeinsGrowHere(const CellClass* pCell)
	{
		if (!pCell)
			return false;

		if (pCell->SlopeIndex > MaxCardRampSlopeIndex)
			return false;

		if (!CanVeinsGrowOnLandType(pCell->LandType))
			return false;

		if (pCell->OverlayTypeIndex != -1 && pCell->OverlayTypeIndex != VeinOverlayIndex)
			return false;

		// Cardinal ramp cells (SlopeIndex 1..4) use dedicated self-contained ramp frames (53..60)
		// and are naturally flanked by cliff rocks or steep side-slopes (SlopeIndex 5..20).
		if (pCell->SlopeIndex != 0)
			return true;

		for (int dir = 0; dir < 8; dir += 2)
		{
			const CellClass* pAdj = pCell->GetNeighbourCell(static_cast<FacingType>(dir));
			if (!pAdj)
				continue;

			// Steep side-slopes (SlopeIndex 5..20) and Veinhole dummy cliff tiles cannot host
			// fringe veins, so they must not block a flat cell at a ramp entrance/exit from maturing.
			if (pAdj->SlopeIndex > MaxCardRampSlopeIndex)
				continue;

			if (!CanVeinsGrowOnLandType(pAdj->LandType))
				return false;

			if (pAdj->OverlayTypeIndex != -1)
			{
				const auto pOverlayType = OverlayTypeClass::Array.GetItemOrDefault(pAdj->OverlayTypeIndex);
				if (!pOverlayType || !pOverlayType->IsVeins)
					return false;
			}
		}

		return true;
	}

	float ComputeCellPriority(const VeinholeMonsterClass* pMonster, const CellClass* pCell)
	{
		const CellStruct monsterPos = pMonster->Position;
		const CellStruct cellPos = pCell->MapCoords;
		const double dist = std::hypot(
			static_cast<double>(cellPos.X - monsterPos.X),
			static_cast<double>(cellPos.Y - monsterPos.Y));
		const int jitter = ScenarioClass::Instance->Random.RandomRanged(-5, 5);
		return static_cast<float>(dist + static_cast<double>(jitter));
	}

	void SafeRemoveNodeFromHeap(PriorityQueueClass<PriorityQueueClassNode>* pQueue, int heapIdx)
	{
		if (!pQueue || heapIdx < 1 || heapIdx > pQueue->Count)
			return;

		PriorityQueueClassNode* const pLast = pQueue->Nodes[pQueue->Count];
		--pQueue->Count;

		if (heapIdx > pQueue->Count)
		{
			pQueue->Nodes[heapIdx] = nullptr;
			return;
		}

		const float lastVal = pLast->Score;
		int cur = heapIdx;

		while (cur > 1)
		{
			const int parent = cur / 2;
			if (lastVal >= pQueue->Nodes[parent]->Score)
				break;

			pQueue->Nodes[cur] = pQueue->Nodes[parent];
			cur = parent;
		}

		if (cur == heapIdx)
		{
			while (true)
			{
				int child = cur * 2;
				if (child > pQueue->Count)
					break;

				if (child + 1 <= pQueue->Count
					&& pQueue->Nodes[child + 1]->Score < pQueue->Nodes[child]->Score)
				{
					++child;
				}

				if (lastVal <= pQueue->Nodes[child]->Score)
					break;

				pQueue->Nodes[cur] = pQueue->Nodes[child];
				cur = child;
			}
		}

		pQueue->Nodes[cur] = pLast;
		pQueue->Nodes[pQueue->Count + 1] = nullptr;
	}

	// Compacts the pre-allocated node pool when NextFreeNodeIndex approaches MaxVeinholeGrowth
	// while the actual live heap Count is much smaller (e.g. after Weeders harvest and veins regrow).
	void CompactGrowthQueue(VeinholeMonsterClass* pMonster)
	{
		if (!pMonster)
			return;

		auto& logic = pMonster->GrowthLogic;
		auto const pQueue = logic.Queue;
		if (!pQueue || !logic.Nodes)
			return;

		const int maxGrowth = RulesClass::Instance->MaxVeinholeGrowth;
		if (logic.NextFreeNodeIndex < maxGrowth - 40)
			return;

		const int liveCount = pQueue->Count;
		if (liveCount <= 0)
		{
			logic.NextFreeNodeIndex = 0;
			return;
		}

		if (liveCount >= maxGrowth - 40)
			return;

		std::vector<PriorityQueueClassNode> liveNodes(static_cast<size_t>(liveCount));
		for (int i = 1; i <= liveCount; ++i)
			liveNodes[static_cast<size_t>(i - 1)] = *pQueue->Nodes[i];

		for (int i = 0; i < liveCount; ++i)
		{
			logic.Nodes[i] = liveNodes[static_cast<size_t>(i)];
			pQueue->Nodes[i + 1] = &logic.Nodes[i];
		}

		logic.NextFreeNodeIndex = liveCount;
	}

	bool EnqueueGrowthCandidate(VeinholeMonsterClass* pMonster, const CellClass* pCell, float priority)
	{
		if (!pMonster || !pCell)
			return false;

		auto& logic = pMonster->GrowthLogic;
		auto const pQueue = logic.Queue;
		if (!pQueue || !logic.Nodes || !logic.CellIndexesWithVeins)
			return false;

		const int maxGrowth = RulesClass::Instance->MaxVeinholeGrowth;
		const int surfCap = PriorityQueueClassNode::SurfaceDataCount();
		const int surfIdx = PriorityQueueClassNode::ToSurfaceIndex(pCell->MapCoords);
		if (surfIdx < 0 || surfIdx >= surfCap)
			return false;

		for (int i = 1; i <= pQueue->Count; ++i)
		{
			if (pQueue->Nodes[i] && pQueue->Nodes[i]->MapCoord == pCell->MapCoords)
				return false;
		}

		if (logic.NextFreeNodeIndex >= maxGrowth - 40)
			CompactGrowthQueue(pMonster);

		if (logic.NextFreeNodeIndex >= maxGrowth - 2)
			return false;

		bool* const pAffected = GetAffectedMap();
		logic.CellIndexesWithVeins[surfIdx] = true;
		if (pAffected)
			pAffected[surfIdx] = true;

		PriorityQueueClassNode* const pNode = &logic.Nodes[logic.NextFreeNodeIndex];
		pNode->MapCoord = pCell->MapCoords;
		pNode->Score = priority;

		if (pQueue->Push(pNode))
		{
			++logic.NextFreeNodeIndex;
			return true;
		}

		return false;
	}

	// Seeds the 12 cardinal border cells around a living Veinhole Monster's 3x3 footprint
	// so a monster with no pre-placed surrounding veins (or after all veins around it are cleared)
	// can start producing veins on its own.
	void SeedMonsterBorderCells(VeinholeMonsterClass* pMonster)
	{
		if (!pMonster || pMonster->IsDead)
			return;

		const CellStruct center = pMonster->Position;
		const CellClass* pCenterCell = MapClass::Instance.TryGetCellAt(center);
		if (!pCenterCell)
			return;

		bool* const pAffected = GetAffectedMap();
		const int surfCap = PriorityQueueClassNode::SurfaceDataCount();

		for (short dy = -2; dy <= 2; ++dy)
		{
			for (short dx = -2; dx <= 2; ++dx)
			{
				const int absDx = std::abs(static_cast<int>(dx));
				const int absDy = std::abs(static_cast<int>(dy));
				if (std::max(absDx, absDy) != 2 || absDx + absDy > 3)
					continue;

				const CellStruct pos { static_cast<short>(center.X + dx), static_cast<short>(center.Y + dy) };
				if (!MapClass::Instance.CoordinatesLegal(pos))
					continue;

				const CellClass* pCell = MapClass::Instance.GetCellAt(pos);
				if (!pCell)
					continue;

				const short refDx = static_cast<short>(dx / 2);
				const short refDy = static_cast<short>(dy / 2);
				const CellStruct refPos { static_cast<short>(center.X + refDx), static_cast<short>(center.Y + refDy) };
				const CellClass* pRefCell = MapClass::Instance.TryGetCellAt(refPos);
				const int refLevel = pRefCell ? pRefCell->Level : pCenterCell->Level;

				if (std::abs(static_cast<int>(pCell->Level) - refLevel) > 2)
					continue;

				if (pCell->OverlayTypeIndex != -1
					&& (pCell->OverlayTypeIndex != VeinOverlayIndex || pCell->OverlayData >= MatureVeinStartFrame))
				{
					continue;
				}

				if (!CanVeinsGrowHere(pCell))
					continue;

				const int surfIdx = PriorityQueueClassNode::ToSurfaceIndex(pos);
				if (surfIdx < 0 || surfIdx >= surfCap)
					continue;

				if (pAffected && pAffected[surfIdx] && !pMonster->GrowthLogic.CellIndexesWithVeins[surfIdx])
					continue;

				EnqueueGrowthCandidate(pMonster, pCell, ComputeCellPriority(pMonster, pCell));
			}
		}
	}

	bool IsOwnedByAnyLivingMonster(int surfIdx, const VeinholeMonsterClass* pExclude = nullptr)
	{
		const int surfCap = PriorityQueueClassNode::SurfaceDataCount();
		if (surfIdx < 0 || surfIdx >= surfCap)
			return false;

		for (int i = 0; i < VeinholeMonsterClass::Array.Count; ++i)
		{
			const VeinholeMonsterClass* pMonster = VeinholeMonsterClass::Array.GetItem(i);
			if (!pMonster || pMonster == pExclude || pMonster->IsDead)
				continue;

			if (pMonster->GrowthLogic.CellIndexesWithVeins
				&& pMonster->GrowthLogic.CellIndexesWithVeins[surfIdx])
			{
				return true;
			}
		}

		return false;
	}

	bool IsInsideMonsterPit(const VeinholeMonsterClass* pMonster, const CellStruct& pos)
	{
		if (!pMonster)
			return false;

		return std::abs(static_cast<int>(pos.X) - static_cast<int>(pMonster->Position.X)) <= 1
			&& std::abs(static_cast<int>(pos.Y) - static_cast<int>(pMonster->Position.Y)) <= 1;
	}

	// Simultaneous multi-source BFS across all living Veinhole Monsters.
	// Fairly partitions shared vein networks by distance, avoids double-counting VeinCount,
	// queues empty/fringe border cells, and optionally cleans up unattached veins on map load.
	void IndexMonstersBFS(bool cleanupDisconnectedVeins, const VeinholeMonsterClass* pDyingMonster = nullptr)
	{
		const int surfCap = PriorityQueueClassNode::SurfaceDataCount();
		if (surfCap <= 0)
			return;

		bool* const pAffected = GetAffectedMap();
		if (!pAffected)
			return;

		std::memset(pAffected, 0, static_cast<size_t>(surfCap));

		struct BFSItem
		{
			VeinholeMonsterClass* Owner;
			CellStruct Pos;
		};

		std::vector<BFSItem> bfsQueue;
		bfsQueue.reserve(1024);

		for (int i = 0; i < VeinholeMonsterClass::Array.Count; ++i)
		{
			VeinholeMonsterClass* pMonster = VeinholeMonsterClass::Array.GetItem(i);
			if (!pMonster)
				continue;

			if (pMonster->IsDead || pMonster == pDyingMonster)
			{
				if (cleanupDisconnectedVeins && pMonster->GrowthLogic.CellIndexesWithVeins)
				{
					for (int idx = 0; idx < surfCap; ++idx)
					{
						if (pMonster->GrowthLogic.CellIndexesWithVeins[idx])
							pAffected[idx] = true;
					}
				}
				continue;
			}

			auto& logic = pMonster->GrowthLogic;
			if (!logic.Queue || !logic.Nodes || !logic.CellIndexesWithVeins)
				continue;

			logic.Queue->Clear();
			logic.Queue->Count = 0;
			logic.NextFreeNodeIndex = 0;
			std::memset(logic.CellIndexesWithVeins, 0, static_cast<size_t>(surfCap));
			pMonster->VeinCount = 0;

			for (short dy = -1; dy <= 1; ++dy)
			{
				for (short dx = -1; dx <= 1; ++dx)
				{
					const CellStruct pos {
						static_cast<short>(pMonster->Position.X + dx),
						static_cast<short>(pMonster->Position.Y + dy)
					};

					if (!MapClass::Instance.CoordinatesLegal(pos))
						continue;

					const int surfIdx = PriorityQueueClassNode::ToSurfaceIndex(pos);
					if (surfIdx < 0 || surfIdx >= surfCap)
						continue;

					logic.CellIndexesWithVeins[surfIdx] = true;
					pAffected[surfIdx] = true;
				}
			}

			for (short dy = -2; dy <= 2; ++dy)
			{
				for (short dx = -2; dx <= 2; ++dx)
				{
					const int absDx = std::abs(static_cast<int>(dx));
					const int absDy = std::abs(static_cast<int>(dy));
					if (std::max(absDx, absDy) != 2 || absDx + absDy > 3)
						continue;

					const CellStruct pos {
						static_cast<short>(pMonster->Position.X + dx),
						static_cast<short>(pMonster->Position.Y + dy)
					};

					if (!MapClass::Instance.CoordinatesLegal(pos))
						continue;

					const CellClass* pCell = MapClass::Instance.GetCellAt(pos);
					if (!pCell)
						continue;

					const int surfIdx = PriorityQueueClassNode::ToSurfaceIndex(pos);
					if (surfIdx < 0 || surfIdx >= surfCap || pAffected[surfIdx])
						continue;

					if (pDyingMonster && IsInsideMonsterPit(pDyingMonster, pos))
						continue;

					if (pCell->OverlayTypeIndex == VeinOverlayIndex)
					{
						logic.CellIndexesWithVeins[surfIdx] = true;
						pAffected[surfIdx] = true;

						if (pCell->OverlayData >= MatureVeinStartFrame)
						{
							++pMonster->VeinCount;
							bfsQueue.push_back({ pMonster, pos });
						}
						else if (CanVeinsGrowHere(pCell))
						{
							EnqueueGrowthCandidate(pMonster, pCell, ComputeCellPriority(pMonster, pCell));
						}
					}
				}
			}
		}

		for (size_t head = 0; head < bfsQueue.size(); ++head)
		{
			const BFSItem cur = bfsQueue[head];
			VeinholeMonsterClass* const pOwner = cur.Owner;
			const CellClass* pCurCell = MapClass::Instance.TryGetCellAt(cur.Pos);
			if (!pCurCell)
				continue;

			for (int dir = 0; dir < 8; dir += 2)
			{
				const CellClass* pAdj = pCurCell->GetNeighbourCell(static_cast<FacingType>(dir));
				if (!pAdj || !MapClass::Instance.CoordinatesLegal(pAdj->MapCoords))
					continue;

				if (std::abs(static_cast<int>(pAdj->Level) - static_cast<int>(pCurCell->Level)) >= 2)
					continue;

				if (pDyingMonster && IsInsideMonsterPit(pDyingMonster, pAdj->MapCoords))
					continue;

				const int adjIdx = PriorityQueueClassNode::ToSurfaceIndex(pAdj->MapCoords);
				if (adjIdx < 0 || adjIdx >= surfCap)
					continue;

				if (pAffected[adjIdx])
					continue;

				if (pAdj->OverlayTypeIndex == VeinOverlayIndex)
				{
					pOwner->GrowthLogic.CellIndexesWithVeins[adjIdx] = true;
					pAffected[adjIdx] = true;

					if (pAdj->OverlayData >= MatureVeinStartFrame)
					{
						++pOwner->VeinCount;
						bfsQueue.push_back({ pOwner, pAdj->MapCoords });
					}
					else if (CanVeinsGrowHere(pAdj))
					{
						EnqueueGrowthCandidate(pOwner, pAdj, ComputeCellPriority(pOwner, pAdj));
					}
				}
				else if (pAdj->OverlayTypeIndex == -1 && CanVeinsGrowHere(pAdj))
				{
					EnqueueGrowthCandidate(pOwner, pAdj, ComputeCellPriority(pOwner, pAdj));
				}
			}
		}

		for (int i = 0; i < VeinholeMonsterClass::Array.Count; ++i)
		{
			VeinholeMonsterClass* pMonster = VeinholeMonsterClass::Array.GetItem(i);
			if (!pMonster || pMonster->IsDead || pMonster == pDyingMonster)
				continue;

			SeedMonsterBorderCells(pMonster);
		}

		if (cleanupDisconnectedVeins)
		{
			for (int i = 0; i < MapClass::Instance.Cells.Capacity; ++i)
			{
				CellClass* pCell = MapClass::Instance.Cells.Items[i];
				if (!pCell || pCell->OverlayTypeIndex != VeinOverlayIndex)
					continue;

				const int surfIdx = PriorityQueueClassNode::ToSurfaceIndex(pCell->MapCoords);
				if (surfIdx >= 0 && surfIdx < surfCap && !pAffected[surfIdx])
				{
					pCell->OverlayTypeIndex = -1;
					pCell->OverlayData = 0;
					pCell->RecalcAttributes(-1);
				}
			}
		}
	}

	// Single-monster BFS rebuild used by Func_74E1C0 without double-counting VeinCount.
	void RebuildSingleMonsterBFS(VeinholeMonsterClass* pMonster)
	{
		if (!pMonster || pMonster->IsDead)
			return;

		const int surfCap = PriorityQueueClassNode::SurfaceDataCount();
		bool* const pAffected = GetAffectedMap();
		auto& logic = pMonster->GrowthLogic;
		if (surfCap <= 0 || !pAffected || !logic.Queue || !logic.Nodes || !logic.CellIndexesWithVeins)
			return;

		for (int idx = 0; idx < surfCap; ++idx)
		{
			if (logic.CellIndexesWithVeins[idx] && !IsOwnedByAnyLivingMonster(idx, pMonster))
				pAffected[idx] = false;
		}

		logic.Queue->Clear();
		logic.Queue->Count = 0;
		logic.NextFreeNodeIndex = 0;
		std::memset(logic.CellIndexesWithVeins, 0, static_cast<size_t>(surfCap));
		pMonster->VeinCount = 0;

		std::vector<CellStruct> bfsQueue;
		bfsQueue.reserve(512);

		for (short dy = -1; dy <= 1; ++dy)
		{
			for (short dx = -1; dx <= 1; ++dx)
			{
				const CellStruct pos {
					static_cast<short>(pMonster->Position.X + dx),
					static_cast<short>(pMonster->Position.Y + dy)
				};

				if (!MapClass::Instance.CoordinatesLegal(pos))
					continue;

				const int surfIdx = PriorityQueueClassNode::ToSurfaceIndex(pos);
				if (surfIdx >= 0 && surfIdx < surfCap)
				{
					logic.CellIndexesWithVeins[surfIdx] = true;
					pAffected[surfIdx] = true;
				}
			}
		}

		for (short dy = -2; dy <= 2; ++dy)
		{
			for (short dx = -2; dx <= 2; ++dx)
			{
				const int absDx = std::abs(static_cast<int>(dx));
				const int absDy = std::abs(static_cast<int>(dy));
				if (std::max(absDx, absDy) != 2 || absDx + absDy > 3)
					continue;

				const CellStruct pos {
					static_cast<short>(pMonster->Position.X + dx),
					static_cast<short>(pMonster->Position.Y + dy)
				};

				if (!MapClass::Instance.CoordinatesLegal(pos))
					continue;

				const CellClass* pCell = MapClass::Instance.GetCellAt(pos);
				if (!pCell)
					continue;

				const int surfIdx = PriorityQueueClassNode::ToSurfaceIndex(pos);
				if (surfIdx < 0 || surfIdx >= surfCap || pAffected[surfIdx])
					continue;

				if (pCell->OverlayTypeIndex == VeinOverlayIndex)
				{
					logic.CellIndexesWithVeins[surfIdx] = true;
					pAffected[surfIdx] = true;

					if (pCell->OverlayData >= MatureVeinStartFrame)
					{
						++pMonster->VeinCount;
						bfsQueue.push_back(pos);
					}
					else if (CanVeinsGrowHere(pCell))
					{
						EnqueueGrowthCandidate(pMonster, pCell, ComputeCellPriority(pMonster, pCell));
					}
				}
			}
		}

		for (size_t head = 0; head < bfsQueue.size(); ++head)
		{
			const CellClass* pCurCell = MapClass::Instance.TryGetCellAt(bfsQueue[head]);
			if (!pCurCell)
				continue;

			for (int dir = 0; dir < 8; dir += 2)
			{
				const CellClass* pAdj = pCurCell->GetNeighbourCell(static_cast<FacingType>(dir));
				if (!pAdj || !MapClass::Instance.CoordinatesLegal(pAdj->MapCoords))
					continue;

				if (std::abs(static_cast<int>(pAdj->Level) - static_cast<int>(pCurCell->Level)) >= 2)
					continue;

				const int adjIdx = PriorityQueueClassNode::ToSurfaceIndex(pAdj->MapCoords);
				if (adjIdx < 0 || adjIdx >= surfCap || pAffected[adjIdx])
					continue;

				if (pAdj->OverlayTypeIndex == VeinOverlayIndex)
				{
					logic.CellIndexesWithVeins[adjIdx] = true;
					pAffected[adjIdx] = true;

					if (pAdj->OverlayData >= MatureVeinStartFrame)
					{
						++pMonster->VeinCount;
						bfsQueue.push_back(pAdj->MapCoords);
					}
					else if (CanVeinsGrowHere(pAdj))
					{
						EnqueueGrowthCandidate(pMonster, pAdj, ComputeCellPriority(pMonster, pAdj));
					}
				}
				else if (pAdj->OverlayTypeIndex == -1 && CanVeinsGrowHere(pAdj))
				{
					EnqueueGrowthCandidate(pMonster, pAdj, ComputeCellPriority(pMonster, pAdj));
				}
			}
		}

		SeedMonsterBorderCells(pMonster);
	}

	// Handles VeinholeMonsterClass::Recalculate (0x74E930) when a monster dies:
	// 1. Replaces only the 3x3 pit dummy cells with veins (without spawning new mature veins on empty outer cells).
	// 2. Transfers any connected veins to surviving Veinhole Monsters sharing the network.
	// 3. Schedules only truly disconnected veins (and the dead monster's pit) for progressive shrinkage.
	void HandleMonsterDeath(VeinholeMonsterClass* pDead)
	{
		if (!pDead)
			return;

		const int surfCap = PriorityQueueClassNode::SurfaceDataCount();
		bool* const pAffected = GetAffectedMap();

		for (short dy = -2; dy <= 2; ++dy)
		{
			for (short dx = -2; dx <= 2; ++dx)
			{
				const CellStruct pos {
					static_cast<short>(pDead->Position.X + dx),
					static_cast<short>(pDead->Position.Y + dy)
				};

				if (!MapClass::Instance.CoordinatesLegal(pos))
					continue;

				CellClass* pCell = MapClass::Instance.GetCellAt(pos);
				if (!pCell)
					continue;

				const int absDx = std::abs(static_cast<int>(dx));
				const int absDy = std::abs(static_cast<int>(dy));
				const bool isPitCenterOrCardinal = (absDx + absDy <= 1);
				const bool isPitCorner = (absDx == 1 && absDy == 1);

				if (isPitCenterOrCardinal || isPitCorner)
				{
					if (pCell->OverlayTypeIndex == VeinholeOverlayIndex
						|| pCell->OverlayTypeIndex == VeinholeDummyOverlayIndex)
					{
						pCell->OverlayTypeIndex = -1;
						pCell->OverlayData = 0;
					}
				}

				if (isPitCenterOrCardinal)
				{
					SpreadVeinsToNeighbors(pCell);
				}
				else if (pCell->OverlayTypeIndex == VeinOverlayIndex)
				{
					if (pCell->OverlayData >= MatureVeinStartFrame)
					{
						SpreadVeinsToNeighbors(pCell);
					}
					else
					{
						const int mask = GetVeinNeighborMask(pCell);
						if (mask == 0)
						{
							pCell->OverlayTypeIndex = -1;
							pCell->OverlayData = 0;
						}
						else
						{
							pCell->OverlayData = static_cast<unsigned char>(
								mask * 3 + ScenarioClass::Instance->Random.RandomRanged(0, 2));
						}
					}
				}

				pCell->MarkForRedraw();
				pCell->RecalcAttributes(-1);
			}
		}

		pDead->IsDead = 1;

		bool hasLivingMonsters = false;
		for (int i = 0; i < VeinholeMonsterClass::Array.Count; ++i)
		{
			const VeinholeMonsterClass* pOther = VeinholeMonsterClass::Array.GetItem(i);
			if (pOther && pOther != pDead && !pOther->IsDead)
			{
				hasLivingMonsters = true;
				break;
			}
		}

		if (hasLivingMonsters && surfCap > 0 && pAffected && pDead->GrowthLogic.CellIndexesWithVeins)
		{
			IndexMonstersBFS(false, pDead);

			for (int idx = 0; idx < surfCap; ++idx)
			{
				if (!pDead->GrowthLogic.CellIndexesWithVeins[idx])
					continue;

				if (IsOwnedByAnyLivingMonster(idx, pDead))
				{
					pDead->GrowthLogic.CellIndexesWithVeins[idx] = false;
				}
				else
				{
					pAffected[idx] = true;
				}
			}
		}

		pDead->RecalculateSpread();
	}

	// Safe replacement for VeinholeMonsterClass::RemoveFrom (0x74EF10):
	// - Avoids deliberate divide-by-zero crash traps in vanilla heap removal
	// - Recognizes ramp mature veins (OverlayData 53..60) as mature
	// - Re-queues ramp cells when harvested so they can regrow
	// - Transfers border ownership cleanly when a dying monster's shrinking veins meet a living monster's network
	void RemoveFromSafe(VeinholeMonsterClass* pThis, CellClass* pCell)
	{
		if (!pThis || !pCell)
			return;

		if (pCell->OverlayTypeIndex != VeinOverlayIndex)
			return;

		auto& logic = pThis->GrowthLogic;
		auto const pQueue = logic.Queue;
		if (!pQueue || !logic.Nodes || !logic.CellIndexesWithVeins)
			return;

		bool* const pAffected = GetAffectedMap();
		const int surfCap = PriorityQueueClassNode::SurfaceDataCount();
		const int cellSurfIdx = PriorityQueueClassNode::ToSurfaceIndex(pCell->MapCoords);
		if (cellSurfIdx < 0 || cellSurfIdx >= surfCap)
			return;

		// If this monster is dead and another living monster already adopted this cell, do not shrink it.
		if (pThis->IsDead && IsOwnedByAnyLivingMonster(cellSurfIdx, pThis))
		{
			logic.CellIndexesWithVeins[cellSurfIdx] = false;
			return;
		}

		const bool wasMature = (pCell->OverlayData >= MatureVeinStartFrame);
		if (wasMature && pThis->VeinCount > 0)
			--pThis->VeinCount;

		const int selfMask = GetVeinNeighborMask(pCell);
		VeinholeMonsterClass* pAdoptingMonster = nullptr;

		if (pThis->IsDead && selfMask != 0)
		{
			for (int dir = 0; dir < 8; dir += 2)
			{
				const CellClass* pAdj = pCell->GetNeighbourCell(static_cast<FacingType>(dir));
				if (!pAdj || pAdj->OverlayTypeIndex != VeinOverlayIndex || pAdj->OverlayData < MatureVeinStartFrame)
					continue;

				const int adjIdx = PriorityQueueClassNode::ToSurfaceIndex(pAdj->MapCoords);
				if (adjIdx < 0 || adjIdx >= surfCap)
					continue;

				for (int m = 0; m < VeinholeMonsterClass::Array.Count; ++m)
				{
					VeinholeMonsterClass* pOther = VeinholeMonsterClass::Array.GetItem(m);
					if (pOther && pOther != pThis && !pOther->IsDead
						&& pOther->GrowthLogic.CellIndexesWithVeins
						&& pOther->GrowthLogic.CellIndexesWithVeins[adjIdx])
					{
						pAdoptingMonster = pOther;
						break;
					}
				}

				if (pAdoptingMonster)
					break;
			}
		}

		if (selfMask == 0 || pCell->SlopeIndex != 0)
		{
			pCell->OverlayTypeIndex = -1;
			pCell->OverlayData = 0;

			if (!pThis->IsDead && pCell->SlopeIndex != 0 && selfMask != 0 && CanVeinsGrowHere(pCell))
			{
				EnqueueGrowthCandidate(pThis, pCell, ComputeCellPriority(pThis, pCell));
			}
			else if (pAdoptingMonster && pCell->SlopeIndex != 0 && selfMask != 0 && CanVeinsGrowHere(pCell))
			{
				logic.CellIndexesWithVeins[cellSurfIdx] = false;
				EnqueueGrowthCandidate(pAdoptingMonster, pCell, ComputeCellPriority(pAdoptingMonster, pCell));
			}
			else
			{
				logic.CellIndexesWithVeins[cellSurfIdx] = false;
				if (pAffected && !IsOwnedByAnyLivingMonster(cellSurfIdx, pThis))
					pAffected[cellSurfIdx] = false;
			}
		}
		else
		{
			const int newFrame = selfMask * 3 + ScenarioClass::Instance->Random.RandomRanged(0, 2);
			pCell->OverlayData = static_cast<unsigned char>(newFrame);

			if (!pThis->IsDead)
			{
				EnqueueGrowthCandidate(pThis, pCell, ComputeCellPriority(pThis, pCell));
			}
			else if (pAdoptingMonster)
			{
				logic.CellIndexesWithVeins[cellSurfIdx] = false;
				if (CanVeinsGrowHere(pCell))
					EnqueueGrowthCandidate(pAdoptingMonster, pCell, ComputeCellPriority(pAdoptingMonster, pCell));
			}
		}

		pCell->MarkForRedraw();
		pCell->RecalcAttributes(-1);

		for (int dir = 0; dir < 8; dir += 2)
		{
			CellClass* pAdj = pCell->GetNeighbourCell(static_cast<FacingType>(dir));
			if (!pAdj || pAdj->OverlayTypeIndex != VeinOverlayIndex)
				continue;

			if (std::abs(static_cast<int>(pAdj->Level) - static_cast<int>(pCell->Level)) >= 2)
				continue;

			const int adjSurfIdx = PriorityQueueClassNode::ToSurfaceIndex(pAdj->MapCoords);
			if (adjSurfIdx < 0 || adjSurfIdx >= surfCap)
				continue;

			if (pThis->IsDead && IsOwnedByAnyLivingMonster(adjSurfIdx, pThis))
				continue;

			const int adjMask = GetVeinNeighborMask(pAdj);
			if (adjMask == 0)
			{
				for (int i = pQueue->Count; i >= 1; --i)
				{
					if (pQueue->Nodes[i] && pQueue->Nodes[i]->MapCoord == pAdj->MapCoords)
						SafeRemoveNodeFromHeap(pQueue, i);
				}

				if (pAdj->OverlayData >= MatureVeinStartFrame && pThis->VeinCount > 0)
					--pThis->VeinCount;

				pAdj->OverlayTypeIndex = -1;
				pAdj->OverlayData = 0;
				pAdj->MarkForRedraw();
				pAdj->RecalcAttributes(-1);

				logic.CellIndexesWithVeins[adjSurfIdx] = false;
				if (pAffected && !IsOwnedByAnyLivingMonster(adjSurfIdx, pThis))
					pAffected[adjSurfIdx] = false;
			}
			else if (pAdj->OverlayData < MatureVeinStartFrame)
			{
				pAdj->OverlayData = static_cast<unsigned char>(
					adjMask * 3 + ScenarioClass::Instance->Random.RandomRanged(0, 2));
				pAdj->MarkForRedraw();
				pAdj->RecalcAttributes(-1);
			}
			else if (!pThis->IsDead)
			{
				for (int subDir = 0; subDir < 8; subDir += 2)
				{
					const CellClass* pSubAdj = pAdj->GetNeighbourCell(static_cast<FacingType>(subDir));
					if (!pSubAdj)
						continue;

					if (std::abs(static_cast<int>(pSubAdj->Level) - static_cast<int>(pAdj->Level)) >= 2)
						continue;

					if (pSubAdj->OverlayTypeIndex == -1
						|| (pSubAdj->OverlayTypeIndex == VeinOverlayIndex && pSubAdj->OverlayData < MatureVeinStartFrame))
					{
						if (CanVeinsGrowHere(pSubAdj))
							EnqueueGrowthCandidate(pThis, pSubAdj, ComputeCellPriority(pThis, pSubAdj));
					}
				}
			}
			else if (pThis->IsDead && adjMask < 15)
			{
				const float dist = static_cast<float>(std::hypot(
					static_cast<double>(pAdj->MapCoords.X - pThis->Position.X),
					static_cast<double>(pAdj->MapCoords.Y - pThis->Position.Y)));
				EnqueueGrowthCandidate(pThis, pAdj, -dist);
			}
		}
	}
}

///
/// Veinhole Monster & Veins Fixes
///

// Fixes ramp vein rendering: bypasses the broken SLOP01Z..SLOP04Z z-mask + YOffset=30 branch
// and adjusts ZAdjust by +30 so ramp veins render cleanly on cardinal slopes (1..4).
DEFINE_HOOK(0x47F9D3, CellClass_DrawOverlay_VeinsRamp, 0x6)
{
	GET(CellClass* const, pCell, ESI);
	GET(int const, levelZ, EDI);

	if (pCell && pCell->SlopeIndex != 0)
	{
		R->EDI(levelZ + RampOverlayZAdjustOffset);
		return 0x47FA2C;
	}

	return 0;
}

// Sets the drawer for veins overlay to match OpenTS and the Veinhole Monster
DEFINE_HOOK_AGAIN(0x47FA5C, CellClass_Draw_Overlay_VeinsPalette, 0x6)
DEFINE_HOOK(0x47FA1F, CellClass_Draw_Overlay_VeinsPalette, 0x6)
{
	GET(CellClass* const, pCell, ESI);

	if (pCell)
	{
		auto const pOverlay = OverlayTypeClass::Array.GetItemOrDefault(pCell->OverlayTypeIndex);
		auto const pPlayer = HouseClass::CurrentPlayer;

		if (pOverlay && pOverlay->IsVeins && pPlayer)
		{
			if (auto const pScheme = ColorScheme::Array.GetItemOrDefault(pPlayer->ColorSchemeIndex))
			{
				R->EDX(pScheme->LightConvert);
				return R->Origin() + 0x6;
			}
		}
	}

	return 0;
}

// Fixes CellClass::CanVeinsGrowHere (0x485460) so:
// 1. Flat cells adjacent to steep side-slopes (SlopeIndex 5..20) at ramp entrances/exits can mature.
// 2. Cardinal ramp cells (SlopeIndex 1..4) can mature even when flanked by cliff rocks or steep side-slopes.
DEFINE_HOOK(0x485460, CellClass_CanVeinsGrowHere, 0x7)
{
	GET(CellClass* const, pThis, ECX);

	R->EAX(CanVeinsGrowHere(pThis));
	return 0x485587;
}

// Fixes CellClass::SpreadVeinsToNeighbors (0x485AF0) when called on a cardinal ramp cell (SlopeIndex 1..4):
// assigns its ramp mature frame (53..60) AND continues into the 4-neighbor fringe loop + ActivateVeins
// so veins can cross multi-tile ramps and exit at the top/bottom of ramps.
DEFINE_HOOK(0x485CA8, CellClass_SpreadVeinsToNeighbors_RampSelf, 0x8)
{
	GET(CellClass* const, pThis, EBP);

	if (pThis->SlopeIndex != 0)
	{
		const int randBit = ScenarioClass::Instance->Random.Random() & 1;
		pThis->OverlayData = static_cast<unsigned char>(0x33 + pThis->SlopeIndex * 2 + randBit);
		R->EBX(0);
		R->Stack<int>(0x34, 0);
		return 0x485CDC;
	}

	return 0x485CB0;
}

// Prevents CellClass::SpreadVeinsToNeighbors from prematurely placing mature ramp frames on adjacent
// ramp cells before they are popped from GrowthLogic.Queue (which caused UpdateGrowth to skip them).
DEFINE_HOOK(0x485CE4, CellClass_SpreadVeinsToNeighbors_NeighborCheck, 0x8)
{
	GET(CellClass* const, pThis, EBP);
	GET(CellClass* const, pAdj, EAX);

	R->ESI(pAdj);

	if (!pAdj
		|| pAdj->SlopeIndex != 0
		|| std::abs(static_cast<int>(pAdj->Level) - static_cast<int>(pThis->Level)) >= 2
		|| !CanVeinsGrowOnLandType(pAdj->LandType)
		|| (pAdj->OverlayTypeIndex != -1
			&& (pAdj->OverlayTypeIndex != VeinOverlayIndex || pAdj->OverlayData >= MatureVeinStartFrame)))
	{
		return 0x486283;
	}

	return 0x485D29;
}

// Preserves pre-placed veins on cardinal ramps (SlopeIndex 1..4) during map load (0x5FDDF0).
DEFINE_HOOK(0x5FDE4D, OverlayClass_PostReadINI_RampVeins, 0x7)
{
	GET(CellClass* const, pCell, ESI);

	if (pCell && (pCell->OverlayData >= MatureVeinStartFrame
		|| (pCell->SlopeIndex >= 1 && pCell->SlopeIndex <= MaxCardRampSlopeIndex)))
	{
		return 0x5FDE56;
	}

	return 0x5FDE8F;
}

// Allows mature veins on cardinal ramps (SlopeIndex 1..4) to spawn vein attack animations on units.
DEFINE_HOOK(0x48693E, CellClass_ActivateVeins_SlopeCheck, 0x6)
{
	GET(CellClass* const, pThis, EBX);

	return (pThis && pThis->SlopeIndex <= MaxCardRampSlopeIndex) ? 0x48694C : 0x486A66;
}

// Compacts the growth node pool when needed and seeds border cells when a living Veinhole Monster
// has no veins touching it.
DEFINE_HOOK(0x74D7DC, VeinholeMonsterClass_UpdateGrowth_CompactAndSeed, 0x6)
{
	GET(VeinholeMonsterClass* const, pThis, ESI);

	CompactGrowthQueue(pThis);

	auto const pQueue = pThis->GrowthLogic.Queue;
	if (!pQueue || pQueue->Count <= 0)
	{
		if (!pThis->IsDead && pThis->VeinCount < RulesClass::Instance->MaxVeinholeGrowth - 100)
			SeedMonsterBorderCells(pThis);

		if (!pQueue || pQueue->Count <= 0)
			return 0x74DBE9;
	}

	R->EDX(pThis->GrowthLogic.NextFreeNodeIndex);
	R->EAX(RulesClass::Instance->MaxVeinholeGrowth);
	return 0x74D7E8;
}

// Replaces the per-monster DFS in InitVeinGrowthData with a simultaneous multi-source BFS
// that seeds border cells for bare monsters and calls RecalcAttributes(-1) on removed unattached veins.
DEFINE_HOOK(0x74E04D, VeinholeMonsterClass_InitVeinGrowthData_BuildIndex, 0x6)
{
	IndexMonstersBFS(true);
	return 0x74E0F0;
}

// Replaces Func_74E1C0 with a clean BFS that does not double-count VeinCount and seeds bare monsters.
DEFINE_HOOK(0x74E1C0, VeinholeMonsterClass_Func_74E1C0_RecalculateCellVector, 0x6)
{
	GET(VeinholeMonsterClass* const, pThis, ECX);

	RebuildSingleMonsterBFS(pThis);
	return 0x74E6A1;
}

// Fixes VeinholeMonsterClass::Recalculate (called on death) so shared vein networks are adopted
// by surviving monsters and no new veins are spawned around the dying monster.
DEFINE_HOOK(0x74E930, VeinholeMonsterClass_Recalculate_Death, 0x6)
{
	GET(VeinholeMonsterClass* const, pThis, ECX);

	HandleMonsterDeath(pThis);
	return 0x74EA23;
}

// Fixes ~VeinholeMonsterClass 5x5 cleanup so it does not wipe veins owned by another living monster
// and properly updates CellClass::RecalcAttributes(-1) when clearing overlays.
DEFINE_HOOK(0x74CA18, VeinholeMonsterClass_DTOR_CleanupArea, 0x8)
{
	GET(VeinholeMonsterClass* const, pThis, EBX);

	bool* const pAffected = GetAffectedMap();
	const int surfCap = PriorityQueueClassNode::SurfaceDataCount();

	for (short dy = -2; dy <= 2; ++dy)
	{
		for (short dx = -2; dx <= 2; ++dx)
		{
			const CellStruct pos {
				static_cast<short>(pThis->Position.X + dx),
				static_cast<short>(pThis->Position.Y + dy)
			};

			if (!MapClass::Instance.CoordinatesLegal(pos))
				continue;

			CellClass* pCell = MapClass::Instance.GetCellAt(pos);
			if (!pCell)
				continue;

			const int surfIdx = PriorityQueueClassNode::ToSurfaceIndex(pos);
			const bool ownedByOther = IsOwnedByAnyLivingMonster(surfIdx, pThis);

			if (pAffected && surfIdx >= 0 && surfIdx < surfCap && !ownedByOther)
				pAffected[surfIdx] = false;

			if (pCell->OverlayTypeIndex == VeinholeOverlayIndex
				|| pCell->OverlayTypeIndex == VeinholeDummyOverlayIndex
				|| (pCell->OverlayTypeIndex == VeinOverlayIndex && !ownedByOther))
			{
				pCell->OverlayTypeIndex = -1;
				pCell->OverlayData = 0;
				pCell->MarkForRedraw();
				pCell->RecalcAttributes(-1);
			}
		}
	}

	return 0x74CC1C;
}

// Replaces VeinholeMonsterClass::RemoveFrom (0x74EF10) to fix divide-by-zero crash traps,
// ramp vein removal/regrowth, and multi-monster network border handoff.
DEFINE_HOOK(0x74EF10, VeinholeMonsterClass_RemoveFrom_Full, 0x6)
{
	GET(VeinholeMonsterClass* const, pThis, ECX);
	GET_STACK(CellClass* const, pCell, 0x4);

	RemoveFromSafe(pThis, pCell);
	return 0x74F2C4;
}

// Loads the veinhole monster art
// Call removed from YR by WW
DEFINE_HOOK(0x4AD097, DisplayClass_ReadIni_LoadVeinholeArt, 0x6)
{
	const int theater = static_cast<int>(ScenarioClass::Instance->Theater);
	VeinholeMonsterClass::LoadVeinholeArt(theater);
	if (!Make_Global<void*>(0xB1D2EC) && theater != 0)
		VeinholeMonsterClass::LoadVeinholeArt(0);

	return 0;
}

// Applies damage to the veinhole monster
DEFINE_HOOK(0x489671, Damage_at_Cell_Update_Veinhole, 0x6)
{
	GET(OverlayTypeClass*, pOverlay, EAX);
	GET(WarheadTypeClass*, pWH, ESI);
	GET_STACK(CellStruct, pCell, STACK_OFFSET(0xE0, -0x4C));
	GET_STACK(int, damage, STACK_OFFSET(0xE0, -0xBC));
	GET_STACK(ObjectClass*, pAttacker, STACK_OFFSET(0xE0, 0x8));
	GET_STACK(HouseClass*, pAttackingHouse, STACK_OFFSET(0xE0, 0x14));

	if (pOverlay->IsVeinholeMonster)
	{
		if (VeinholeMonsterClass* pVeinhole = VeinholeMonsterClass::GetVeinholeMonsterFrom(&pCell))
			pVeinhole->ReceiveDamage(&damage, 0, pWH, pAttacker, false, false, pAttackingHouse);
	}

	return 0;
}

// Adjusts Veinhole Monster draw elevation when placed on flat ground (e.g. Level 0) without a sunken 3x3 pit
// so the 3x3 monster graphic aligns with its cell footprint instead of shifting one level up.
DEFINE_HOOK(0x74D523, VeinholeMonsterClass_Draw_LevelOffset, 0x7)
{
	GET(CellClass* const, pCenterCell, EAX);

	int effectiveLevel = pCenterCell ? pCenterCell->Level : 0;
	if (pCenterCell)
	{
		bool hasPitSlopes = false;
		for (int dir = 0; dir < 8; dir += 2)
		{
			if (const CellClass* pAdj = pCenterCell->GetNeighbourCell(static_cast<FacingType>(dir)))
			{
				if (pAdj->SlopeIndex != 0)
				{
					hasPitSlopes = true;
					break;
				}
			}
		}

		if (!hasPitSlopes)
			--effectiveLevel;
	}

	R->EAX(effectiveLevel);
	return 0x74D52A;
}

DEFINE_HOOK(0x6D4656, TacticalClass_Draw_Veinhole, 0x5)
{
	enum { ContinueDraw = 0x6D465B };

	VeinholeMonsterClass::DrawAll();
	IonBlastClass::DrawAll();

	return ContinueDraw;
}

DEFINE_HOOK(0x5349A5, Map_ClearVectors_Veinhole, 0x5)
{
	VeinholeMonsterClass::DeleteAll();
	VeinholeMonsterClass::DeleteVeinholeGrowthData();
	return 0;
}

// DEFINE_HOOK(0x55B4E1, LogicClass_Update_Veinhole, 0x5) // Goto ScenarioExt

// Handles the veins' attack animation
DEFINE_HOOK(0x4243BC, AnimClass_Update_VeinholeAttack, 0x6)
{
	GET(AnimClass*, pAnim, ESI);

	if (pAnim->Type->IsVeins)
		AnimExt::VeinAttackAI(pAnim);

	return 0;
}

///
/// Weeder
///

// These 2 I am not sure, maybe they have smth to do with AI, maybe they are for the unit queue at the refinery
DEFINE_HOOK(0x736823, UnitClass_Update_WeederMissionMove, 0x6)
{
	enum
	{
		Continue = 0x736831,
		Skip = 0x736981
	};

	GET(UnitTypeClass*, pUnitType, EAX);

	if (pUnitType->Harvester || pUnitType->Weeder)
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x7368C6, UnitClass_Update_WeederMissionMove2, 0x6)
{
	enum
	{
		Continue = 0x7368D4,
		Skip = 0x736981
	};

	GET(BuildingTypeClass*, pBuildingType, EDX);

	if (pBuildingType->Refinery || pBuildingType->Weeder)
		return Continue;

	return Skip;
}

// Not sure if necessary
/*
// These 2 have something to do with ZAdjustment when unloading
DEFINE_HOOK(0x7043E7, TechnoClass_Get_ZAdjustment_Weeder, 0x6)
{
	enum
	{
		Continue = 0x7043F1,
		Skip = 0x704421
	};

	GET(UnitTypeClass*, pUnitType, ECX);

	if (pUnitType->Harvester || pUnitType->Weeder)
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x70440C, TechnoClass_Get_ZAdjustment_Weeder2, 0x6)
{
	enum
	{
		Continue = 0x704416,
		Skip = 0x704421
	};

	GET(BuildingTypeClass*, pBuildingType, EAX);

	if (pBuildingType->Refinery || pBuildingType->Weeder)
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x741C32, UnitClass_SetDestination_SpecialAnim_Weeder, 0x6)
{
	enum
	{
		CheckSpecialAnimExists = 0x741C3C,
		Skip = 0x741C4F
	};

	GET(UnitTypeClass*, pUnitType, ECX);

	if (pUnitType->Harvester || pUnitType->Weeder)
		return CheckSpecialAnimExists;

	return Skip;
}
*/

DEFINE_HOOK(0x73D0DB, UnitClass_DrawAt_Weeder_Oregath, 0x6)
{
	enum
	{
		DrawOregath = 0x73D0E9,
		Skip = 0x73D298
	};

	GET(UnitClass*, pUnit, ESI);

	if (pUnit->IsHarvesting)
		return DrawOregath;

	const auto pType = pUnit->Type;

	if (pType->Harvester || pType->Weeder)
		return DrawOregath;

	return Skip;
}
/*
DEFINE_HOOK(0x73D2A6, UnitClass_DrawAt_Weeder_UnloadingClass, 0x6)
{
	enum
	{
		ShowUnloadingClass = 0x73D2B0,
		Skip = 0x73D2CA
	};

	GET(UnitTypeClass*, pUnitType, EAX);

	if (pUnitType->Harvester || pUnitType->Weeder)
		return ShowUnloadingClass;

	return Skip;
}
*/

// Enables the weeder to harvest veins
DEFINE_HOOK(0x73D49E, UnitClass_Harvesting_Weeder, 0x0)
{
	enum
	{
		Harvest = 0x73D4DA,
		Skip = 0x73D5FE
	};

	GET(UnitClass*, pUnit, ESI);
	GET(CellClass*, pCell, EBP);
	constexpr unsigned char weedOverlayData = 0x30;

	const auto pType = pUnit->Type;
	const auto landType = pCell->LandType;
	const bool harvesterCanHarvest = pType->Harvester && landType == LandType::Tiberium;
	const bool weederCanWeed = pType->Weeder && landType == LandType::Weeds && pCell->OverlayData >= weedOverlayData;

	if ((harvesterCanHarvest || weederCanWeed) && pUnit->GetStoragePercentage() < 1.0)
		return Harvest;

	return Skip;
}

// Not sure if necessary
/*
DEFINE_HOOK(0x73E005, UnitClass_Unload_WeederAnim, 0x6)
{
	enum
	{
		ProceedWithAnim = 0x73E013,
		Skip = 0x73E093
	};

	GET(UnitTypeClass*, pUnitType, ECX);

	if (pUnitType->Harvester || pUnitType->Weeder)
		return ProceedWithAnim;

	return Skip;
}
*/

// This lets the weeder actually enter the waste facility and unload
// WW removed weeders from this check in YR
DEFINE_HOOK(0x43C788, BuildingClass_ReceivedRadioCommand_Weeder_CompleteEnter, 0x6)
{
	enum
	{
		CompleteEnter = 0x43C796,
		Skip = 0x43CE43
	};

	GET(BuildingTypeClass*, pBuildingType, EAX);

	if (pBuildingType->DockUnload || pBuildingType->Weeder)
		return CompleteEnter;

	return Skip;
}

// This assigns the weeder to the "Harvest" mission when it is granted as a free unit
// Ares made the weeder receive the "Guard" command instead
DEFINE_HOOK(0x446EAD, BuildingClass_GrandOpening_FreeWeeder_Mission, 0x0)
{
	GET(UnitClass*, pUnit, EDI);

	if (pUnit->Type->Weeder)
		pUnit->ForceMission(Mission::Harvest);

	pUnit->NextMission();

	return 0x446EB7;
}

// Teleport cooldown for weeders
// DEFINE_HOOK(0x719580, TeleportLocomotion_Weeder, 0x6) //Goto Hooks.Teleport.cpp

// DockUnload bypass for Weeders when teleporting
DEFINE_HOOK(0x7424BD, UnitClass_AssignDestination_Weeder_Teleport, 0x6)
{
	GET(BuildingTypeClass*, pDestination, ECX);

	return pDestination->DockUnload || pDestination->Weeder ? 0x7424CB : 0x7425DB;
}

//// Skip check for Weeder so that weeders go through teleport stuff
//DEFINE_JUMP(LJMP, 0x73E844, 0x73E793)
//
//
//DEFINE_HOOK(0x73E84A, UnitClass_Mission_Harvest, 0x6)
//{
//	GET(UnitClass*, pUnit, EBP);
//
//	bool isOnTiberium;
//	if (pUnit->Type->Weeder)
//		isOnTiberium = pUnit->MoveToWeed(RulesClass::Instance->TiberiumLongScan / Unsorted::LeptonsPerCell);
//	else
//		isOnTiberium = pUnit->MoveToTiberium(RulesClass::Instance->TiberiumLongScan / Unsorted::LeptonsPerCell);
//
//	R->EBX(isOnTiberium);
//	return 0x73E86B;
//}

DEFINE_HOOK(0x73E9A0, UnitClass_Weeder_StopHarvesting, 0x6)
{
	enum
	{
		StopHarvesting = 0x73E9CA,
		Skip = 0x73EA8D
	};

	GET(UnitClass*, pUnit, EBP);

	const auto pType = pUnit->Type;

	if ((pType->Harvester || pType->Weeder) && pUnit->GetStoragePercentage() == 1.0)
	{
		return StopHarvesting;
	}

	return Skip;
}
