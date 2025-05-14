#include "Body.h"

#include <BitFont.h>

#include <Utilities/EnumFunctions.h>

BuildingExt::ExtContainer BuildingExt::ExtMap;

void BuildingExt::ExtData::DisplayIncomeString()
{
	if (this->AccumulatedIncome && Unsorted::CurrentFrame % 15 == 0)
	{
		auto const ownerObject = this->OwnerObject();
		auto const pRuleExt = RulesExt::Global();

		if ((pRuleExt->DisplayIncome_AllowAI || ownerObject->Owner->IsControlledByHuman())
			&& this->TypeExtData->DisplayIncome.Get(pRuleExt->DisplayIncome))
		{
			FlyingStrings::AddMoneyString(
				this->AccumulatedIncome,
				ownerObject->Owner,
				this->TypeExtData->DisplayIncome_Houses.Get(pRuleExt->DisplayIncome_Houses.Get()),
				ownerObject->GetRenderCoords(),
				this->TypeExtData->DisplayIncome_Offset
			);
		}
		this->AccumulatedIncome = 0;
	}
}

bool BuildingExt::ExtData::HasSuperWeapon(const int index, const bool withUpgrades) const
{
	const auto pThis = this->OwnerObject();
	const auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	const auto count = pExt->GetSuperWeaponCount();
	for (auto i = 0; i < count; ++i)
	{
		const auto idxSW = pExt->GetSuperWeaponIndex(i, pThis->Owner);

		if (idxSW == index)
			return true;
	}

	if (withUpgrades)
	{
		for (auto const& pUpgrade : pThis->Upgrades)
		{
			if (const auto pUpgradeExt = BuildingTypeExt::ExtMap.Find(pUpgrade))
			{
				const auto countUpgrade = pUpgradeExt->GetSuperWeaponCount();

				for (auto i = 0; i < countUpgrade; ++i)
				{
					const auto idxSW = pUpgradeExt->GetSuperWeaponIndex(i, pThis->Owner);

					if (idxSW == index)
						return true;
				}
			}
		}
	}

	return false;
}

void BuildingExt::StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType)
{
	auto const pDepositableTiberium = TiberiumClass::Array.GetItem(idxStorageTiberiumType);
	float depositableTiberiumAmount = 0.0f; // Number of 'bails' that will be stored.
	auto const pTiberium = TiberiumClass::Array.GetItem(idxTiberiumType);

	if (amount > 0.0 && BuildingTypeExt::ExtMap.Find(pThis->Type)->Refinery_UseStorage)
	{
		// Store Tiberium in structures
		depositableTiberiumAmount = (amount * pTiberium->Value) / pDepositableTiberium->Value;
		pThis->Owner->GiveTiberium(depositableTiberiumAmount, idxStorageTiberiumType);
	}
}

void BuildingExt::ExtData::UpdatePrimaryFactoryAI()
{
	auto pOwner = this->OwnerObject()->Owner;

	if (!pOwner || pOwner->ProducingAircraftTypeIndex < 0)
		return;

	AircraftTypeClass* pAircraft = AircraftTypeClass::Array.GetItem(pOwner->ProducingAircraftTypeIndex);
	FactoryClass* currFactory = pOwner->GetFactoryProducing(pAircraft);
	std::vector<BuildingClass*> airFactoryBuilding;
	BuildingClass* newBuilding = nullptr;

	// Update what is the current air factory for future comparisons
	if (this->CurrentAirFactory)
	{
		int nDocks = 0;
		if (this->CurrentAirFactory->Type)
			nDocks = this->CurrentAirFactory->Type->NumberOfDocks;

		int nOccupiedDocks = BuildingExt::CountOccupiedDocks(this->CurrentAirFactory);

		if (nOccupiedDocks < nDocks)
			currFactory = this->CurrentAirFactory->Factory;
		else
			this->CurrentAirFactory = nullptr;
	}

	// Obtain a list of air factories for optimizing the comparisons
	for (auto pBuilding : pOwner->Buildings)
	{
		if (pBuilding->Type->Factory == AbstractType::AircraftType)
		{
			if (!currFactory && pBuilding->Factory)
				currFactory = pBuilding->Factory;

			airFactoryBuilding.emplace_back(pBuilding);
		}
	}

	if (this->CurrentAirFactory)
	{
		for (auto pBuilding : airFactoryBuilding)
		{
			if (pBuilding == this->CurrentAirFactory)
			{
				this->CurrentAirFactory->Factory = currFactory;
				this->CurrentAirFactory->IsPrimaryFactory = true;
			}
			else
			{
				pBuilding->IsPrimaryFactory = false;

				if (pBuilding->Factory)
				{
					auto const* prodType = pBuilding->Factory->Object->GetType();
					pBuilding->Factory->AbandonProduction();
					Debug::Log("%s is not CurrentAirFactory of %s, production of %s aborted\n", pBuilding->Type->ID, pOwner->PlainName, prodType->ID);
				}
			}
		}

		return;
	}

	if (!currFactory)
		return;

	for (auto pBuilding : airFactoryBuilding)
	{
		int nDocks = pBuilding->Type->NumberOfDocks;
		int nOccupiedDocks = BuildingExt::CountOccupiedDocks(pBuilding);

		if (nOccupiedDocks < nDocks)
		{
			if (!newBuilding)
			{
				newBuilding = pBuilding;
				newBuilding->Factory = currFactory;
				newBuilding->IsPrimaryFactory = true;
				this->CurrentAirFactory = newBuilding;

				continue;
			}
		}

		pBuilding->IsPrimaryFactory = false;

		if (pBuilding->Factory)
		{
			auto const* prodType = pBuilding->Factory->Object->GetType();
			pBuilding->Factory->AbandonProduction();
			Debug::Log("%s of %s abandonded production of %s due to redundancies\n", pBuilding->Type->ID, pOwner->PlainName, prodType->ID);
		}
	}

	return;
}

int BuildingExt::CountOccupiedDocks(BuildingClass* pBuilding)
{
	if (!pBuilding)
		return 0;

	int nOccupiedDocks = 0;

	if (pBuilding->RadioLinks.IsAllocated)
	{
		for (auto i = 0; i < pBuilding->RadioLinks.Capacity; ++i)
		{
			if (auto const pLink = pBuilding->GetNthLink(i))
				nOccupiedDocks++;
		}
	}

	return nOccupiedDocks;
}

bool BuildingExt::HasFreeDocks(BuildingClass* pBuilding)
{
	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		int nDocks = pBuilding->Type->NumberOfDocks;
		int nOccupiedDocks = BuildingExt::CountOccupiedDocks(pBuilding);

		if (nOccupiedDocks < nDocks)
			return true;
		else
			return false;
	}

	return false;
}

bool BuildingExt::CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno)
{
	auto const whatAmI = pTechno->WhatAmI();

	if (!pBuilding->Type->Grinding || (whatAmI != AbstractType::Infantry && whatAmI != AbstractType::Unit))
		return false;

	if ((pBuilding->Type->InfantryAbsorb || pBuilding->Type->UnitAbsorb) &&
		(whatAmI == AbstractType::Infantry && !pBuilding->Type->InfantryAbsorb ||
			whatAmI == AbstractType::Unit && !pBuilding->Type->UnitAbsorb))
	{
		return false;
	}

	const auto pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	if (pBuilding->Owner == pTechno->Owner && !pExt->Grinding_AllowOwner)
		return false;

	if (pBuilding->Owner != pTechno->Owner && pBuilding->Owner->IsAlliedWith(pTechno) && !pExt->Grinding_AllowAllies)
		return false;

	if (pExt->Grinding_AllowTypes.size() > 0 && !pExt->Grinding_AllowTypes.Contains(pTechno->GetTechnoType()))
		return false;

	if (pExt->Grinding_DisallowTypes.size() > 0 && pExt->Grinding_DisallowTypes.Contains(pTechno->GetTechnoType()))
		return false;

	return true;
}

bool BuildingExt::DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int refund)
{
	if (auto const pExt = BuildingExt::ExtMap.Find(pBuilding))
	{
		auto const pTypeExt = pExt->TypeExtData;

		pExt->AccumulatedIncome += refund;
		pExt->GrindingWeapon_AccumulatedCredits += refund;

		if (pTypeExt->Grinding_Weapon &&
			Unsorted::CurrentFrame >= pExt->GrindingWeapon_LastFiredFrame + pTypeExt->Grinding_Weapon->ROF &&
			pExt->GrindingWeapon_AccumulatedCredits >= pTypeExt->Grinding_Weapon_RequiredCredits)
		{
			TechnoExt::FireWeaponAtSelf(pBuilding, pTypeExt->Grinding_Weapon);
			pExt->GrindingWeapon_LastFiredFrame = Unsorted::CurrentFrame;
			pExt->GrindingWeapon_AccumulatedCredits = 0;
		}

		if (pTypeExt->Grinding_Sound >= 0)
		{
			VocClass::PlayAt(pTypeExt->Grinding_Sound, pTechno->GetCoords());
			return true;
		}
	}

	return false;
}

// Building only or allow units too?
void BuildingExt::ExtData::ApplyPoweredKillSpawns()
{
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;

	if (pTypeExt->Powered_KillSpawns && pThis->Type->Powered && !pThis->IsPowerOnline())
	{
		if (auto pManager = pThis->SpawnManager)
		{
			pManager->ResetTarget();
			for (auto pItem : pManager->SpawnedNodes)
			{
				if (pItem->Status == SpawnNodeStatus::Attacking || pItem->Status == SpawnNodeStatus::Returning)
				{
					pItem->Unit->ReceiveDamage(&pItem->Unit->Health, 0,
						RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
				}
			}
		}
	}
}

bool BuildingExt::ExtData::HandleInfiltrate(HouseClass* pInfiltratorHouse, int moneybefore)
{
	auto const pTypeExt = this->TypeExtData;
	auto const pThis = this->OwnerObject();
	auto pVictimHouse = pThis->Owner;
	this->AccumulatedIncome += pVictimHouse->Available_Money() - moneybefore;

	if (!pVictimHouse->IsControlledByHuman() && !RulesExt::Global()->DisplayIncome_AllowAI)
	{
		// TODO there should be a better way...
		FlyingStrings::AddMoneyString(
				this->AccumulatedIncome,
				pVictimHouse,
				pTypeExt->DisplayIncome_Houses.Get(RulesExt::Global()->DisplayIncome_Houses.Get()),
				pThis->GetRenderCoords(),
				pTypeExt->DisplayIncome_Offset
		);
	}

	if (!pTypeExt->SpyEffect_Custom)
		return false;

	if (pInfiltratorHouse != pVictimHouse)
	{
		// I assume you were not launching for real, Morton

		auto launchTheSWHere = [pThis](SuperClass* const pSuper, HouseClass* const pHouse)->void
			{
				int oldstart = pSuper->RechargeTimer.StartTime;
				int oldleft = pSuper->RechargeTimer.TimeLeft;
				pSuper->SetReadiness(true);
				pSuper->Launch(CellClass::Coord2Cell(pThis->GetCenterCoords()), pHouse->IsCurrentPlayer());
				pSuper->Reset();
				pSuper->RechargeTimer.StartTime = oldstart;
				pSuper->RechargeTimer.TimeLeft = oldleft;
			};

		int idx = pTypeExt->SpyEffect_VictimSuperWeapon;
		if (idx >= 0)
			launchTheSWHere(pVictimHouse->Supers.Items[idx], pVictimHouse);

		idx = pTypeExt->SpyEffect_InfiltratorSuperWeapon;
		if (idx >= 0)
			launchTheSWHere(pInfiltratorHouse->Supers.Items[idx], pInfiltratorHouse);
	}

	return true;
}

// For unit's weapons factory only
void BuildingExt::KickOutStuckUnits(BuildingClass* pThis)
{
	if (const auto pUnit = abstract_cast<UnitClass*>(pThis->GetNthLink()))
	{
		if (!pUnit->IsTether && pUnit->GetCurrentSpeed() <= 0)
		{
			if (const auto pTeam = pUnit->Team)
				pTeam->LiberateMember(pUnit);

			pThis->SendCommand(RadioCommand::NotifyUnlink, pUnit);
			pUnit->QueueMission(Mission::Guard, false);
			return; // one after another
		}
	}

	auto buffer = CoordStruct::Empty;
	auto pCell = MapClass::Instance.GetCellAt(*pThis->GetExitCoords(&buffer, 0));
	int i = 0;

	while (true)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (pObject->WhatAmI() == AbstractType::Unit)
			{
				const auto pUnit = static_cast<UnitClass*>(pObject);

				if (pThis->Owner != pUnit->Owner || pUnit->IsTether)
					continue;

				const auto height = pUnit->GetHeight();

				if (height < 0 || height > Unsorted::CellHeight)
					continue;

				if (const auto pTeam = pUnit->Team)
					pTeam->LiberateMember(pUnit);

				pThis->SendCommand(RadioCommand::RequestLink, pUnit);
				pThis->QueueMission(Mission::Unload, false);
				return; // one after another
			}
		}

		if (++i >= 2)
			return; // no stuck

		// Continue checking towards the bottom right corner
		pCell = pCell->GetNeighbourCell(FacingType::East);
	}
}

// Get all cells covered by the building, optionally including those covered by OccupyHeight.
const std::vector<CellStruct> BuildingExt::GetFoundationCells(BuildingClass* const pThis, CellStruct const baseCoords, bool includeOccupyHeight)
{
	const CellStruct foundationEnd = { 0x7FFF, 0x7FFF };
	auto const pFoundation = pThis->GetFoundationData(false);

	int occupyHeight = includeOccupyHeight ? pThis->Type->OccupyHeight : 1;

	if (occupyHeight <= 0)
		occupyHeight = 1;

	auto pCellIterator = pFoundation;

	while (*pCellIterator != foundationEnd)
		++pCellIterator;

	std::vector<CellStruct> foundationCells;
	foundationCells.reserve(static_cast<int>(std::distance(pFoundation, pCellIterator + 1)) * occupyHeight);
	pCellIterator = pFoundation;

	while (*pCellIterator != foundationEnd)
	{
		auto actualCell = baseCoords + *pCellIterator;

		for (auto i = occupyHeight; i > 0; --i)
		{
			foundationCells.push_back(actualCell);
			--actualCell.X;
			--actualCell.Y;
		}
		++pCellIterator;
	}

	std::sort(foundationCells.begin(), foundationCells.end(),
		[](const CellStruct& lhs, const CellStruct& rhs) -> bool
	{
		return lhs.X > rhs.X || lhs.X == rhs.X && lhs.Y > rhs.Y;
	});

	auto const it = std::unique(foundationCells.begin(), foundationCells.end());
	foundationCells.erase(it, foundationCells.end());

	return foundationCells;
}

// =============================
// load / save

template <typename T>
void BuildingExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeExtData)
		.Process(this->TechnoExtData)
		.Process(this->DeployedTechno)
		.Process(this->IsCreatedFromMapFile)
		.Process(this->LimboID)
		.Process(this->GrindingWeapon_LastFiredFrame)
		.Process(this->GrindingWeapon_AccumulatedCredits)
		.Process(this->CurrentAirFactory)
		.Process(this->AccumulatedIncome)
		.Process(this->CurrentLaserWeaponIndex)
		.Process(this->PoweredUpToLevel)
		.Process(this->EMPulseSW)
		;
}

void BuildingExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BuildingClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BuildingClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool BuildingExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

BuildingExt::ExtContainer::ExtContainer() : Container("BuildingClass") { }

BuildingExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x43BCBD, BuildingClass_CTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	auto const pExt = BuildingExt::ExtMap.TryAllocate(pItem);

	if (pExt)
	{
		pExt->TypeExtData = BuildingTypeExt::ExtMap.Find(pItem->Type);
		pExt->TechnoExtData = TechnoExt::ExtMap.Find(pItem);
	}

	return 0;
}

DEFINE_HOOK(0x43C022, BuildingClass_DTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x454190, BuildingClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x453E20, BuildingClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x454174, BuildingClass_Load_LightSource, 0xA)
{
	GET(BuildingClass*, pThis, EDI);

	SwizzleManagerClass::Instance.Swizzle((void**)&pThis->LightSource);

	return 0x45417E;
}

DEFINE_HOOK(0x45417E, BuildingClass_Load_Suffix, 0x5)
{
	BuildingExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x454244, BuildingClass_Save_Suffix, 0x7)
{
	BuildingExt::ExtMap.SaveStatic();

	return 0;
}

// Removes setting otherwise unused field (0x6FC) in BuildingClass when building has airstrike applied on it so that it can safely be used to store BuildingExt pointer.
DEFINE_JUMP(LJMP, 0x41D9FB, 0x41DA05);


void __fastcall BuildingClass_InfiltratedBy_Wrapper(BuildingClass* pThis, void*, HouseClass* pInfiltratorHouse)
{
	int oldBalance = pThis->Owner->Available_Money();
	// explicitly call because Ares rewrote it
	reinterpret_cast<void(__thiscall*)(BuildingClass*, HouseClass*)>(0x4571E0)(pThis, pInfiltratorHouse);

	BuildingExt::ExtMap.Find(pThis)->HandleInfiltrate(pInfiltratorHouse, oldBalance);
}

DEFINE_FUNCTION_JUMP(CALL, 0x51A00B, BuildingClass_InfiltratedBy_Wrapper);
