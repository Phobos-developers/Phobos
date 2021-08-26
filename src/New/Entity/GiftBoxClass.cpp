#include "GiftBoxClass.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <ScenarioClass.h>

const bool GiftBoxClass::AllowDestroy(TechnoClass* pTechno)
{
	if (pTechno)
	{
		auto pCell = pTechno->GetCell();
		auto pCoords = pTechno->GetCoords();

		if (auto pDestCell = pCell ? pCell : MapClass::Instance->TryGetCellAt(pCoords))
		{
			if (auto const pBuildingBelow = pDestCell->GetBuilding())
			{
				if (pTechno->WhatAmI() == AbstractType::Unit)
				{
					if (auto pLink = *pTechno->RadioLinks.Items)
					{
						if (auto pBuilding = specific_cast<BuildingClass*>(pLink))
						{
							if (pBuilding->Type->WeaponsFactory)
							{
								if (pBuildingBelow == pBuilding)
								{
									return false;
								}
							}
						}
					}
				}
			}
		}
	}

	return true;
}

const bool GiftBoxClass::CreateType(int nIndex, GiftBoxData& nGboxData, HouseClass* pOwner, CoordStruct nCoord, CoordStruct nDestCoord)
{
	auto pItem = nGboxData.TechnoList.at(nIndex);

	if (!pItem || !nGboxData.Count.at(nIndex))
		return false;

	bool bSuccess = false;

	for (int b = 0; b < nGboxData.Count.at(nIndex); ++b)
	{
		if (auto pObject = pItem->CreateObject(pOwner))
		{
			if (auto pCell = MapClass::Instance->TryGetCellAt(nCoord))
				pObject->OnBridge = pCell->ContainsBridge();

			if (pObject->WhatAmI() == AbstractType::Building)
			{
				bSuccess = pObject->Unlimbo(nCoord, Direction::E);
				pObject->Location = nCoord;
			}
			else
			{
				auto nRandFacing = static_cast<unsigned int>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
				bSuccess = pObject->Unlimbo(CoordStruct{ 0,0,100000 }, nRandFacing);
				pObject->SetLocation(nCoord);

				auto pDest = MapClass::Instance->TryGetCellAt(nDestCoord);
				abstract_cast<FootClass*>(pObject)->SetDestination(pDest, true);
				abstract_cast<FootClass*>(pObject)->QueueMission(Mission::Move, false);

			}

			if (bSuccess)
			{
				if (!pOwner->Type->MultiplayPassive)
				{
					pOwner->RecheckTechTree = true;
					pObject->DiscoveredBy(pOwner);

				}
			}

			if (!bSuccess && pObject)
				pObject->UnInit();
		}
	}

	return bSuccess;
}

const void GiftBoxClass::AI(TechnoClass* pTechno)
{
	if (!pTechno)
		return;

	if (auto const pGiftBox = TechnoExt::ExtMap.Find(pTechno)->AttachedGiftBox.get())
	{
		if (GiftBoxClass::AllowDestroy(pTechno))
		{
			if (pGiftBox->Open())
			{
				auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

				//cache everything that needed
				auto pOwner = pTechno->Owner;
				auto nCoord = pTechno->GetCoords();
				auto pFocus = pTechno->Focus;

				if (pTypeExt->GboxData.RandomRange.Get() > 0)
				{
					if (auto nCellLoc = MapClass::Instance->TryGetCellAt(nCoord))
					{
						for (CellSpreadEnumerator it((size_t)abs(pTypeExt->GboxData.RandomRange.Get())); it; ++it)
						{
							auto const& offset = *it;
							if (offset == CellStruct::Empty)
								continue;

							if (auto pCell = MapClass::Instance->TryGetCellAt(CellClass::Cell2Coord(nCellLoc->MapCoords + offset)))
							{
								if (pTypeExt->GboxData.EmptyCell.Get() && (pCell->GetBuilding() || pCell->GetUnit(false) || pCell->GetInfantry(false)))
								{
									continue;
								}

								nCoord = CellClass::Cell2Coord(pCell->MapCoords);
								break;
							}
						}
					}
				}

				auto nDestination = nCoord;
				if (pTechno->What_Am_I() != AbstractType::Building)
				{
					if (pFocus)
						nDestination = pFocus->GetCoords();
					else if (auto pDest = abstract_cast<FootClass*>(pTechno)->Destination)
						nDestination = pDest->GetCoords();
				}

				if (!pTypeExt->GboxData.RandomType.Get())
				{
					for (size_t nIndex = 0; nIndex < pTypeExt->GboxData.TechnoList.size(); ++nIndex)
					{
						if (!GiftBoxClass::CreateType(nIndex, pTypeExt->GboxData, pOwner, nCoord, nDestination))
							continue;
					}
				}
				else
				{
					auto nRandIdx = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(pTypeExt->GboxData.TechnoList.size()) - 1);
					GiftBoxClass::CreateType(nRandIdx, pTypeExt->GboxData, pOwner, nCoord, nDestination);
				}

				if (pTypeExt->GboxData.Remove.Get())
				{
					if (pTypeExt->GboxData.Destroy.Get())
						pTechno->Destroy();

					pTechno->Undiscover();
					pTechno->Limbo();
					pTechno->UnInit();

				}
				else
				{
					pGiftBox->Reset(pTypeExt->GboxData.DelayMinMax.Get().Y == 0 ? abs(pTypeExt->GboxData.Delay.Get()) : abs(ScenarioClass::Instance->Random.RandomRanged(pTypeExt->GboxData.DelayMinMax.Get().X, pTypeExt->GboxData.DelayMinMax.Get().Y)));
				}
			}
		}
	}
}

const void GiftBoxClass::Construct(TechnoClass* pTechno)
{
	if (!pTechno)
		return;

	if (auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno))
	{
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

		if (!pTechnoExt->AttachedGiftBox.get() && pTypeExt->GboxData)
		{
			//this only called once then the parent removed after 
			//smart pointer is best to use , so we dont need to GameDelet<> it manually!
			pTechnoExt->AttachedGiftBox = std::make_unique<GiftBoxClass>(pTypeExt->GboxData);
		}
	}
}

DEFINE_HOOK(0x6F9E50, TechnoClass_AI_Gbox, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	GiftBoxClass::AI(pThis);

	return 0;
}

DEFINE_HOOK(0x6F6CA0, TechnoClass_Put_Gbox, 0x7)
{
	GET(TechnoClass* const, pThis, ECX);
	//	GET_STACK(CoordStruct*, pCoord, 0x4);
	//	GET_STACK(int, Direction, 0x8);

	GiftBoxClass::Construct(pThis);

	return 0;
}