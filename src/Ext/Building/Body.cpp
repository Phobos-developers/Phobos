#include "Body.h"

#include <BitFont.h>

#include <Utilities/EnumFunctions.h>

BuildingExt::ExtContainer BuildingExt::ExtMap;

void BuildingExt::ExtData::DisplayIncomeString()
{
	if (this->AccumulatedIncome && Unsorted::CurrentFrame % 15 == 0)
	{
		auto const pOwnerObject = this->OwnerObject();
		auto const pTypeExt = this->TypeExtData;

		if ((RulesExt::Global()->DisplayIncome_AllowAI || pOwnerObject->Owner->IsControlledByHuman())
			&& pTypeExt->DisplayIncome.Get(RulesExt::Global()->DisplayIncome))
		{
			FlyingStrings::AddMoneyString(
				this->AccumulatedIncome,
				pOwnerObject->Owner,
				pTypeExt->DisplayIncome_Houses.Get(RulesExt::Global()->DisplayIncome_Houses.Get()),
				pOwnerObject->GetRenderCoords(),
				pTypeExt->DisplayIncome_Offset
			);
		}
		this->AccumulatedIncome = 0;
	}
}

bool BuildingExt::ExtData::HasSuperWeapon(const int index, const bool withUpgrades) const
{
	const auto pThis = this->OwnerObject();
	const auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	const auto pOwner = pThis->Owner;

	const auto count = pExt->GetSuperWeaponCount();
	for (auto i = 0; i < count; ++i)
	{
		const auto idxSW = pExt->GetSuperWeaponIndex(i, pOwner);

		if (idxSW == index)
			return true;
	}

	if (withUpgrades)
	{
		for (auto const& pUpgrade : pThis->Upgrades)
		{
			if (const auto pUpgradeExt = BuildingTypeExt::ExtMap.TryFind(pUpgrade))
			{
				const auto countUpgrade = pUpgradeExt->GetSuperWeaponCount();
				for (auto i = 0; i < countUpgrade; ++i)
				{
					const auto idxSW = pUpgradeExt->GetSuperWeaponIndex(i, pOwner);

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

	if (amount > 0.0)
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

		if (pExt->Refinery_UseStorage)
		{
			// Store Tiberium in structures
			depositableTiberiumAmount = (amount * pTiberium->Value) / pDepositableTiberium->Value;
			pThis->Owner->GiveTiberium(depositableTiberiumAmount, idxStorageTiberiumType);
		}
	}
}

void BuildingExt::ExtData::UpdatePrimaryFactoryAI()
{
	auto const pOwner = this->OwnerObject()->Owner;

	if (!pOwner || pOwner->ProducingAircraftTypeIndex < 0)
		return;

	auto const pAircraft = AircraftTypeClass::Array.GetItem(pOwner->ProducingAircraftTypeIndex);
	auto currFactory = pOwner->GetFactoryProducing(pAircraft);
	std::vector<BuildingClass*> airFactoryBuilding;
	BuildingClass* newBuilding = nullptr;

	// Update what is the current air factory for future comparisons
	if (this->CurrentAirFactory)
	{
		int nDocks = 0;
		if (const auto pFactoryType = this->CurrentAirFactory->Type)
			nDocks = pFactoryType->NumberOfDocks;

		const int nOccupiedDocks = BuildingExt::CountOccupiedDocks(this->CurrentAirFactory);

		if (nOccupiedDocks < nDocks)
			currFactory = this->CurrentAirFactory->Factory;
		else
			this->CurrentAirFactory = nullptr;
	}

	// Obtain a list of air factories for optimizing the comparisons
	for (auto const pBuilding : pOwner->Buildings)
	{
		if (pBuilding->Type->Factory == AbstractType::AircraftType)
		{
			if (!currFactory && pBuilding->Factory)
				currFactory = pBuilding->Factory;

			airFactoryBuilding.emplace_back(pBuilding);
		}
	}

	if (auto const pCurrent = this->CurrentAirFactory)
	{
		for (auto const pBuilding : airFactoryBuilding)
		{
			if (pBuilding == pCurrent)
			{
				pCurrent->Factory = currFactory;
				pCurrent->IsPrimaryFactory = true;
			}
			else
			{
				pBuilding->IsPrimaryFactory = false;

				if (pBuilding->Factory)
				{
					//auto const* prodType = pBuilding->Factory->Object->GetType();
					pBuilding->Factory->AbandonProduction();
					//Debug::Log("%s is not CurrentAirFactory of %s, production of %s aborted\n", pBuilding->Type->ID, pOwner->PlainName, prodType->ID);
				}
			}
		}

		return;
	}

	if (!currFactory)
		return;

	for (auto const pBuilding : airFactoryBuilding)
	{
		if (!newBuilding)
		{
			if (BuildingExt::CountOccupiedDocks(pBuilding) < pBuilding->Type->NumberOfDocks)
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
			//auto const* prodType = pBuilding->Factory->Object->GetType();
			pBuilding->Factory->AbandonProduction();
			//Debug::Log("%s of %s abandonded production of %s due to redundancies\n", pBuilding->Type->ID, pOwner->PlainName, prodType->ID);
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
	auto const pType = pBuilding->Type;

	if (pType->Factory == AbstractType::AircraftType)
	{
		const int nDocks = pType->NumberOfDocks;
		const int nOccupiedDocks = BuildingExt::CountOccupiedDocks(pBuilding);

		if (nOccupiedDocks < nDocks)
			return true;
		else
			return false;
	}

	return false;
}

bool BuildingExt::CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno)
{
	auto const pBldType = pBuilding->Type;
	auto const whatAmI = pTechno->WhatAmI();

	if (!pBldType->Grinding || (whatAmI != AbstractType::Infantry && whatAmI != AbstractType::Unit))
		return false;

	if ((pBldType->InfantryAbsorb || pBldType->UnitAbsorb)
		&& (whatAmI == AbstractType::Infantry && !pBldType->InfantryAbsorb
			|| whatAmI == AbstractType::Unit && !pBldType->UnitAbsorb))
	{
		return false;
	}

	auto const pExt = BuildingTypeExt::ExtMap.Find(pBldType);

	if (pBuilding->Owner == pTechno->Owner && !pExt->Grinding_AllowOwner)
		return false;

	if (pBuilding->Owner != pTechno->Owner && pBuilding->Owner->IsAlliedWith(pTechno) && !pExt->Grinding_AllowAllies)
		return false;

	auto const pType = pTechno->GetTechnoType();
	auto const& allowTypes = pExt->Grinding_AllowTypes;
	auto const& disallowTypes = pExt->Grinding_DisallowTypes;

	if (allowTypes.size() > 0 && !allowTypes.Contains(pType))
		return false;

	if (disallowTypes.size() > 0 && disallowTypes.Contains(pType))
		return false;


	return true;
}

bool BuildingExt::DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int refund)
{
	if (auto const pExt = BuildingExt::ExtMap.TryFind(pBuilding))
	{
		auto const pTypeExt = pExt->TypeExtData;

		pExt->AccumulatedIncome += refund;
		pExt->GrindingWeapon_AccumulatedCredits += refund;

		if (pTypeExt->Grinding_Weapon
			&& Unsorted::CurrentFrame >= pExt->GrindingWeapon_LastFiredFrame + pTypeExt->Grinding_Weapon->ROF
			&& pExt->GrindingWeapon_AccumulatedCredits >= pTypeExt->Grinding_Weapon_RequiredCredits)
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
		if (auto const pManager = pThis->SpawnManager)
		{
			pManager->ResetTarget();
			for (auto const pItem : pManager->SpawnedNodes)
			{
				auto const status = pItem->Status;
				if (status == SpawnNodeStatus::Attacking || status == SpawnNodeStatus::Returning)
				{
					auto const pUnit = pItem->Unit;
					pUnit->ReceiveDamage(&pUnit->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
				}
			}
		}
	}
}

bool BuildingExt::ExtData::HandleInfiltrate(HouseClass* pInfiltratorHouse, int moneybefore)
{
	const auto pVictimHouse = this->OwnerObject()->Owner;
	const auto pTypeExt = this->TypeExtData;
	this->AccumulatedIncome += pVictimHouse->Available_Money() - moneybefore;

	if (!pVictimHouse->IsControlledByHuman() && !RulesExt::Global()->DisplayIncome_AllowAI)
	{
		// TODO there should be a better way...
		FlyingStrings::AddMoneyString(
				this->AccumulatedIncome,
				pVictimHouse,
				pTypeExt->DisplayIncome_Houses.Get(RulesExt::Global()->DisplayIncome_Houses.Get()),
				this->OwnerObject()->GetRenderCoords(),
				pTypeExt->DisplayIncome_Offset
		);
	}

	if (!pTypeExt->SpyEffect_Custom)
		return false;

	if (pInfiltratorHouse != pVictimHouse)
	{
		// I assume you were not launching for real, Morton

		auto launchTheSWHere = [this](SuperClass* const pSuper, HouseClass* const pHouse)->void
			{
				const int oldstart = pSuper->RechargeTimer.StartTime;
				const int oldleft = pSuper->RechargeTimer.TimeLeft;
				pSuper->SetReadiness(true);
				pSuper->Launch(CellClass::Coord2Cell(this->OwnerObject()->GetCenterCoords()), pHouse->IsCurrentPlayer());
				pSuper->Reset();
				pSuper->RechargeTimer.StartTime = oldstart;
				pSuper->RechargeTimer.TimeLeft = oldleft;
			};

		const int idx1 = pTypeExt->SpyEffect_VictimSuperWeapon;
		if (idx1 >= 0)
			launchTheSWHere(pVictimHouse->Supers.Items[idx1], pVictimHouse);

		const int idx2 = pTypeExt->SpyEffect_InfiltratorSuperWeapon;
		if (idx2 >= 0)
			launchTheSWHere(pInfiltratorHouse->Supers.Items[idx2], pInfiltratorHouse);
	}

	return true;
}

// For unit's weapons factory only
void BuildingExt::KickOutStuckUnits(BuildingClass* pThis)
{
	auto buffer = CoordStruct::Empty;
	pThis->GetExitCoords(&buffer, 0);

	auto cell = CellClass::Coord2Cell(buffer);

	const auto pType = pThis->Type;
	const short start = static_cast<short>(pThis->Location.X / Unsorted::LeptonsPerCell + pType->GetFoundationWidth() - 2); // door
	const short end = cell.X; // exit
	cell.X = start;
	auto pCell = MapClass::Instance.GetCellAt(cell);

	while (true)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (const auto pUnit = abstract_cast<UnitClass*, true>(pObject))
			{
				if (pThis->Owner != pUnit->Owner || pUnit->Locomotor->Destination() != CoordStruct::Empty)
					continue;

				const auto height = pUnit->GetHeight();

				if (height < 0 || height > Unsorted::CellHeight)
					continue;

				pThis->SendCommand(RadioCommand::RequestLink, pUnit);
				pThis->QueueMission(Mission::Unload, false);
				return; // one after another
			}
		}

		if (--cell.X < end)
			return; // no stuck

		pCell = MapClass::Instance.GetCellAt(cell);
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
			foundationCells.emplace_back(actualCell);
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
		.Process(this->CurrentEMPulseSW)
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
	const int oldBalance = pThis->Owner->Available_Money();
	// explicitly call because Ares rewrote it
	reinterpret_cast<void(__thiscall*)(BuildingClass*, HouseClass*)>(0x4571E0)(pThis, pInfiltratorHouse);

	BuildingExt::ExtMap.Find(pThis)->HandleInfiltrate(pInfiltratorHouse, oldBalance);
}

DEFINE_FUNCTION_JUMP(CALL, 0x51A00B, BuildingClass_InfiltratedBy_Wrapper);
