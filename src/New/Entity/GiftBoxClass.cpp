#include "GiftBoxClass.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <ScenarioClass.h>
#include <SlaveManagerClass.h>
#include <SpawnManagerClass.h>
#include <InfantryClass.h>

const bool GiftBoxClass::OpenDisallowed()
{
	if (auto pTechno = this->Techno)
	{
		bool bIsOnWarfactory = false;
		if (pTechno->WhatAmI() == AbstractType::Unit)
		{
			if (auto pCell = pTechno->GetCell())
			{
				if (auto pBuildingBelow = pCell->GetBuilding())
				{
					if (auto pLinkedBuilding = specific_cast<BuildingClass*>(*pTechno->RadioLinks.Items))
					{
						bIsOnWarfactory = pLinkedBuilding->Type->WeaponsFactory && !pLinkedBuilding->Type->Naval && pBuildingBelow == pLinkedBuilding;
					}
				}
			}
		}

		bool bIsGarrisoned = false;
		if (auto pInfantry = specific_cast<InfantryClass*>(pTechno))
		{
			for (auto const& pBuildingGlobal : *BuildingClass::Array)
			{
				bIsGarrisoned = pBuildingGlobal->Occupants.Count > 0 && pBuildingGlobal->Occupants.FindItemIndex(pInfantry) != -1;
			}
		}

		return pTechno->Absorbed ||
			pTechno->InOpenToppedTransport ||
			pTechno->InLimbo ||
			bIsGarrisoned ||
			bIsOnWarfactory ||
			pTechno->TemporalTargetingMe;

	}

	return false;
}

const bool GiftBoxClass::CreateType(int nIndex, GiftBoxData& nGboxData, CoordStruct nCoord, CoordStruct nDestCoord)
{
	auto pItem = nGboxData.TechnoList.at(nIndex);

	if (!pItem || !nGboxData.Count.at(nIndex))
		return false;

	bool bSuccess = false;

	for (int i = 0; i < nGboxData.Count.at(nIndex); ++i)
	{
		if (auto pObject = pItem->CreateObject(this->Techno->Owner))
		{
			auto pCell = MapClass::Instance->TryGetCellAt(nCoord);
			if (!pCell)
				continue;

			pObject->OnBridge = pCell->ContainsBridge();

			if (pObject->WhatAmI() == AbstractType::Building)
			{
				++Unsorted::IKnowWhatImDoing();
				bSuccess = pObject->Unlimbo(nCoord, Direction::E);
				--Unsorted::IKnowWhatImDoing();
				pObject->Location = nCoord;
			}
			else
			{
				auto pFoot = abstract_cast<FootClass*>(pObject);
				auto nRandFacing = static_cast<unsigned int>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
				bSuccess = pObject->Unlimbo(CoordStruct{ 0,0,100000 }, nRandFacing);
				pObject->SetLocation(nCoord);

				auto pCellDest = MapClass::Instance->TryGetCellAt(nDestCoord);

				if (pCellDest)
				{
					if (pObject->IsCellOccupied(pCellDest, -1, -1, nullptr, false) == Move::OK)
					{
						pCellDest->ScatterContent(CoordStruct::Empty, true, true, pObject->OnBridge);
					}
					else
					{
						pCellDest = nullptr;
					}
				}

				pFoot->SlaveOwner = nullptr;
				pFoot->Transporter = nullptr;
				pFoot->Absorbed = false;
				pFoot->LastMapCoords = pCell->MapCoords;
				pFoot->SetDestination(pCellDest, true);
				pFoot->QueueMission(Mission::Move, false);
				pFoot->ShouldEnterOccupiable = false;
				pFoot->ShouldGarrisonStructure = false;

			}

			if (bSuccess)
			{
				if (this->Techno->IsSelected)
					pObject->Select();

				pObject->DiscoveredBy(this->Techno->Owner);
			}
			else
			{
				if (pObject)
					pObject->UnInit();
			}

		}
	}
	return bSuccess;
}

CoordStruct GiftBoxClass::GetRandomCoordsNear(GiftBoxData& nGiftBox, CoordStruct nCoord)
{
	if (nGiftBox.RandomRange.Get() > 0)
	{
		if (auto nCellLoc = MapClass::Instance->TryGetCellAt(nCoord))
		{
			for (CellSpreadEnumerator it((size_t)abs(nGiftBox.RandomRange.Get())); it; ++it)
			{
				auto const& offset = *it;
				if (offset == CellStruct::Empty)
					continue;

				if (auto pCell = MapClass::Instance->TryGetCellAt(CellClass::Cell2Coord(nCellLoc->MapCoords + offset)))
				{
					if (nGiftBox.EmptyCell.Get() && (pCell->GetBuilding() || pCell->GetUnit(false) || pCell->GetInfantry(false)))
					{
						continue;
					}

					nCoord = CellClass::Cell2Coord(pCell->MapCoords);
					break;
				}
			}
		}
	}

	return nCoord;
}

void GiftBoxClass::SyncToAnotherTechno(TechnoClass* pFrom, TechnoClass* pTo)
{
	const auto pFromExt = TechnoExt::ExtMap.Find(pFrom);
	const auto pToExt = TechnoExt::ExtMap.Find(pTo);

	if (pFromExt->AttachedGiftBox)
	{
		pToExt->AttachedGiftBox = std::make_unique<GiftBoxClass>(pTo);
		strcpy_s(pToExt->AttachedGiftBox->TechnoID, pFromExt->AttachedGiftBox->TechnoID);
		pToExt->AttachedGiftBox->Delay = pFromExt->AttachedGiftBox->Delay;
		pToExt->AttachedGiftBox->IsOpen = pFromExt->AttachedGiftBox->IsOpen;
		pToExt->AttachedGiftBox->IsEnabled = pFromExt->AttachedGiftBox->IsEnabled;
		pToExt->AttachedGiftBox->IsTechnoChange = true;
		pFromExt->AttachedGiftBox = nullptr;
	}
}

const void GiftBoxClass::AI()
{
	auto pTechno = this->Techno;
	if (!pTechno)
		return;

	if (auto const pGiftBox = TechnoExt::ExtMap.Find(pTechno)->AttachedGiftBox.get())
	{
		auto pType = pTechno->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		const auto newID = pTechno->get_ID();

		if (!IsTechnoChange && (strcmp(this->TechnoID, newID) != 0))
		{
			strcpy_s(this->TechnoID, newID);
		}

		if (!pTypeExt->GiftBoxData)
			return;

		if (!pGiftBox->Delay)
		{
			pGiftBox->Delay = pTypeExt->GiftBoxData.DelayMinMax.Get().Y == 0 ?
				abs(pTypeExt->GiftBoxData.Delay.Get()) : abs(ScenarioClass::Instance->Random.RandomRanged(pTypeExt->GiftBoxData.DelayMinMax.Get().X, pTypeExt->GiftBoxData.DelayMinMax.Get().Y));
		}

		pGiftBox->IsEnabled = !pGiftBox->OpenDisallowed();

		if (pGiftBox->Open())
		{
			auto nCoord = GiftBoxClass::GetRandomCoordsNear(pTypeExt->GiftBoxData, pTechno->GetCoords());
			auto nDestination = nCoord;

			if (pTechno->What_Am_I() != AbstractType::Building)
			{
				if (auto pFocus = pTechno->Focus)
					nDestination = pFocus->GetCoords();
				else if (auto pDest = abstract_cast<FootClass*>(pTechno)->Destination)
					nDestination = pDest->GetCoords();
			}

			if (!pTypeExt->GiftBoxData.RandomType.Get())
			{
				for (size_t nIndex = 0; nIndex < pTypeExt->GiftBoxData.TechnoList.size(); ++nIndex)
				{
					if (!pGiftBox->CreateType(nIndex, pTypeExt->GiftBoxData, nCoord, nDestination))
						continue;
				}
			}
			else
			{
				auto nRandIdx = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(pTypeExt->GiftBoxData.TechnoList.size()) - 1);
				pGiftBox->CreateType(nRandIdx, pTypeExt->GiftBoxData, nCoord, nDestination);
			}

			if (pTypeExt->GiftBoxData.Remove.Get())
			{
				// Limboing stuffs is not safe method depend on case
				// maybe need to check if anything else need to be handle 
				pTechno->Undiscover();
				pTechno->Limbo();
				pTechno->UnInit();

			}
			else if (pTypeExt->GiftBoxData.Destroy.Get())
			{
				auto nDamage = pType->Strength;
				pTechno->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			}
			else
			{
				auto nDelay = pTypeExt->GiftBoxData.DelayMinMax.Get().Y == 0 ? abs(pTypeExt->GiftBoxData.Delay.Get()) : abs(ScenarioClass::Instance->Random.RandomRanged(pTypeExt->GiftBoxData.DelayMinMax.Get().X, pTypeExt->GiftBoxData.DelayMinMax.Get().Y));
				pGiftBox->Reset(nDelay);
			}
		}
	}
}

DEFINE_HOOK(0x6F6CA0, TechnoClass_Put_Gbox, 0x7)
{
	GET(TechnoClass* const, pThis, ECX);
	//	GET_STACK(CoordStruct*, pCoord, 0x4);
	//	GET_STACK(int, Direction, 0x8);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pTechnoExt->AttachedGiftBox.get() && pTypeExt->GiftBoxData)
		pTechnoExt->AttachedGiftBox = std::make_unique<GiftBoxClass>(pThis);

	return 0;
}