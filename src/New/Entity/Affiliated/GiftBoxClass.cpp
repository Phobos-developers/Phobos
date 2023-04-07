#include <New/Entity/Affiliated/GiftBoxClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <ScenarioClass.h>
#include <BuildingClass.h>
#include <SlaveManagerClass.h>
#include <SpawnManagerClass.h>
#include <InfantryClass.h>

bool GiftBoxClass::Open()
{
	return this->IsOpen ? false : CheckDelay();
}

bool GiftBoxClass::CheckDelay()
{
	//dont execute everytime if not enabled
	if (this->IsEnabled)
	{
		if (--this->Delay <= 0)
		{
			this->IsOpen = true;
			return true;
		}
	}

	return false;
}

void GiftBoxClass::Reset(int delay)
{
	this->Delay = delay;
	this->IsOpen = false;
	this->IsTechnoChange = false;
}

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

const bool GiftBoxClass::CreateType(int nIndex, GiftBoxTypeClass* nGboxData, CoordStruct nCoord, CoordStruct nDestCoord)
{
	auto pItem = nGboxData->TechnoList.at(nIndex);

	if (!pItem || !nGboxData->Count.at(nIndex))
		return false;

	bool bSuccess = false;

	for (int i = 0; i < nGboxData->Count.at(nIndex); ++i)
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
				bSuccess = pObject->Unlimbo(nCoord, DirType::East);
				--Unsorted::IKnowWhatImDoing();
				pObject->Location = nCoord;
			}
			else
			{
				auto pFoot = abstract_cast<FootClass*>(pObject);
				auto nRandFacing = static_cast<unsigned int>(ScenarioClass::Instance->Random.RandomRanged(0, 65535));
				bSuccess = pObject->Unlimbo(CoordStruct { 0,0,100000 }, DirType(nRandFacing));
				pObject->SetLocation(nCoord);

				auto pCurrentCell = MapClass::Instance->TryGetCellAt(nCoord);
				auto pCellDest = MapClass::Instance->TryGetCellAt(nDestCoord);

				if (pCellDest)
					pCellDest->ScatterContent(CoordStruct::Empty, true, true, pObject->OnBridge);
				else if (pCurrentCell)
					pCurrentCell->ScatterContent(CoordStruct::Empty, true, true, pObject->OnBridge);

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

CoordStruct GiftBoxClass::GetRandomCoordsNear(GiftBoxTypeClass* nGiftBox, CoordStruct nCoord)
{
	if (nGiftBox->RandomRange.Get() > 0)
	{
		if (auto nCellLoc = MapClass::Instance->TryGetCellAt(nCoord))
		{
			for (CellSpreadEnumerator it((size_t)abs(nGiftBox->RandomRange.Get())); it; ++it)
			{
				auto const& offset = *it;
				if (offset == CellStruct::Empty)
					continue;

				if (auto pCell = MapClass::Instance->TryGetCellAt(CellClass::Cell2Coord(nCellLoc->MapCoords + offset)))
				{
					if (nGiftBox->EmptyCell.Get() && (pCell->GetBuilding() || pCell->GetUnit(false) || pCell->GetInfantry(false)))
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
		int nDelay;

		if (!IsTechnoChange && (strcmp(this->TechnoID, newID) != 0))
			strcpy_s(this->TechnoID, newID);

		if (!pTypeExt->GiftBoxType)
			return;

		if (!pGiftBox->Delay)
		{
			nDelay = pTypeExt->GiftBoxType->Delay.Get();

			// Use RandomDelay Instead
			if (pTypeExt->GiftBoxType->DelayMinMax.Get().Y > 0)
				nDelay = ScenarioClass::Instance->Random.RandomRanged(pTypeExt->GiftBoxType->DelayMinMax.Get().X, pTypeExt->GiftBoxType->DelayMinMax.Get().Y);

			pGiftBox->Delay = abs(nDelay);
		}

		pGiftBox->IsEnabled = !pGiftBox->OpenDisallowed();

		if (pGiftBox->Open())
		{
			auto nCoord = GiftBoxClass::GetRandomCoordsNear(pTypeExt->GiftBoxType.get(), pTechno->GetCoords());
			auto nDestination = nCoord;

			if (pTechno->What_Am_I() != AbstractType::Building)
			{
				if (auto pFocus = pTechno->Focus)
					nDestination = pFocus->GetCoords();
				else if (auto pDest = abstract_cast<FootClass*>(pTechno)->Destination)
					nDestination = pDest->GetCoords();
			}

			if (!pTypeExt->GiftBoxType->RandomType.Get())
			{
				for (size_t nIndex = 0; nIndex < pTypeExt->GiftBoxType->TechnoList.size(); ++nIndex)
				{
					if (!pGiftBox->CreateType(nIndex, pTypeExt->GiftBoxType.get(), nCoord, nDestination))
						continue;
				}
			}
			else
			{
				auto nRandIdx = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(pTypeExt->GiftBoxType->TechnoList.size()) - 1);
				pGiftBox->CreateType(nRandIdx, pTypeExt->GiftBoxType.get(), nCoord, nDestination);
			}

			if (pTypeExt->GiftBoxType->Remove.Get())
			{
				// Limboing stuffs is not safe method depend on case
				// maybe need to check if anything else need to be handle
				pTechno->Undiscover();
				pTechno->Limbo();
				pTechno->UnInit();
			}
			else if (pTypeExt->GiftBoxType->Destroy.Get())
			{
				auto nDamage = pType->Strength;
				pTechno->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			}
			else
			{
				nDelay = pTypeExt->GiftBoxType->Delay.Get();

				// Use RandomDelay Instead
				if (pTypeExt->GiftBoxType->DelayMinMax.Get().Y > 0)
					nDelay = ScenarioClass::Instance->Random.RandomRanged(pTypeExt->GiftBoxType->DelayMinMax.Get().X, pTypeExt->GiftBoxType->DelayMinMax.Get().Y);

				pGiftBox->Reset(abs(nDelay));
			}
		}
	}
}

#pragma region Save/Load

template <typename T>
bool GiftBoxClass::Serialize(T& Stm)
{
	return Stm
		.Process(Techno)
		.Process(IsEnabled)
		.Process(IsTechnoChange)
		.Process(IsOpen)
		.Process(Delay)
		.Process(TechnoID)
		.Success();
}

bool GiftBoxClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool GiftBoxClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<GiftBoxClass*>(this)->Serialize(Stm);
}

#pragma endregion
