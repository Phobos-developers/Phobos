#include "Body.h"

#include <BitFont.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/AresHelper.h>
#include <Ext/House/Body.h>
#include <New/Type/ResourceTypeClass.h>

BuildingExt::ExtContainer BuildingExt::ExtMap;

BuildingExt::ExtContainer::ExtContainer() : Container("BuildingClass") { }
BuildingExt::ExtContainer::~ExtContainer() = default;

void BuildingExt::DisplayIncomeString()
{
	bool hasResourceIncome = false;
	for (float val : this->AccumulatedResources)
	{
		if (val >= 1.0f || val <= -1.0f)
		{
			hasResourceIncome = true;
			break;
		}
	}

	if (this->AccumulatedIncome != 0 || hasResourceIncome)
	{
		int const delay = this->GetTypeExtData()->DisplayIncome_Delay.Get(RulesExt::Global()->DisplayIncome_Delay.Get());
		if (Unsorted::CurrentFrame % delay == 0)
		{
			auto const pThis = this->OwnerObject();
			auto const pTypeExt = this->GetTypeExtData();

			if ((RulesExt::Global()->DisplayIncome_AllowAI || pThis->Owner->IsControlledByHuman())
				&& pTypeExt->DisplayIncome.Get(RulesExt::Global()->DisplayIncome))
			{
				std::wstring combinedStr;
				ColorStruct displayColor = ColorStruct { 0, 255, 0 };
				bool hasCustomColor = false;

				if (this->AccumulatedIncome != 0)
				{
					const bool isPositive = this->AccumulatedIncome > 0;
					wchar_t moneyBuf[32];
					swprintf_s(moneyBuf, L"%ls%ls%d", isPositive ? L"+" : L"-", Phobos::UI::CostLabel, std::abs(this->AccumulatedIncome));
					combinedStr += moneyBuf;
					if (!isPositive)
						displayColor = ColorStruct { 255, 0, 0 };
				}

				for (size_t i = 0; i < this->AccumulatedResources.size(); ++i)
				{
					const int amount = static_cast<int>(this->AccumulatedResources[i]);
					if (amount != 0 && i < ResourceTypeClass::Array.size())
					{
						if (const auto pResource = ResourceTypeClass::Array[i].get())
						{
							if (!combinedStr.empty())
								combinedStr += L" ";

							const bool isPositive = amount > 0;
							wchar_t resBuf[32];
							const wchar_t* label = pResource->Display_Label.Get();
							const bool useSpace = pResource->Display_Label_UseSpace.Get();
							if (label && *label)
							{
								if (pResource->Display_Label_InvertPosition.Get())
									swprintf_s(resBuf, useSpace ? L"%ls%d %ls" : L"%ls%d%ls", isPositive ? L"+" : L"-", std::abs(amount), label);
								else
									swprintf_s(resBuf, useSpace ? L"%ls%ls %d" : L"%ls%ls%d", isPositive ? L"+" : L"-", label, std::abs(amount));
							}
							else
							{
								swprintf_s(resBuf, L"%ls%d", isPositive ? L"+" : L"-", std::abs(amount));
							}
							combinedStr += resBuf;

							if (this->AccumulatedIncome == 0 && !hasCustomColor)
							{
								const ColorStruct resColor = pResource->Display_Color.Get();
								if (resColor != ColorStruct { 0, 0, 0 })
								{
									displayColor = resColor;
									hasCustomColor = true;
								}
								else if (!isPositive)
								{
									displayColor = ColorStruct { 255, 0, 0 };
								}
							}
						}
					}
				}

				if (!combinedStr.empty() && !MapClass::Instance.IsLocationShrouded(pThis->GetCoords()))
				{
					const auto displayHouses = pTypeExt->DisplayIncome_Houses.Get(RulesExt::Global()->DisplayIncome_Houses.Get());
					if (displayHouses == AffectedHouse::All || EnumFunctions::CanTargetHouse(displayHouses, pThis->Owner, HouseClass::CurrentPlayer))
					{
						if (pThis->VisualCharacter(false, nullptr) != VisualType::Hidden)
						{
							Point2D offset = pTypeExt->DisplayIncome_Offset;
							int width = 0, height = 0;
							if (BitFont::Instance)
							{
								BitFont::Instance->GetTextDimension(combinedStr.c_str(), &width, &height, 120);
								offset.X -= (width / 2);
							}
							FlyingStrings::Add(combinedStr.c_str(), pThis->GetCoords(), displayColor, offset);
						}
					}
				}
			}

			for (size_t i = 0; i < this->AccumulatedResources.size(); ++i)
			{
				const int amount = static_cast<int>(this->AccumulatedResources[i]);
				this->AccumulatedResources[i] -= static_cast<float>(amount);
			}

			this->AccumulatedIncome = 0;
		}
	}
}

bool BuildingExt::HasSuperWeapon(const int index) const
{
	const auto pThis = this->OwnerObject();
	const auto pExt = BuildingTypeExt::Fetch(pThis->Type);
	const auto pOwner = pThis->Owner;

	const int count = pExt->GetSuperWeaponCount();

	for (int i = 0; i < count; ++i)
	{
		const int idxSW = pExt->GetSuperWeaponIndex(i, pOwner);

		if (idxSW == index)
			return true;
	}

	if (pThis->UpgradeLevel)
	{
		for (auto const& pUpgrade : pThis->Upgrades)
		{
			if (const auto pUpgradeExt = BuildingTypeExt::TryFetch(pUpgrade))
			{
				const int countUpgrade = pUpgradeExt->GetSuperWeaponCount();

				for (int i = 0; i < countUpgrade; ++i)
				{
					const int idxSW = pUpgradeExt->GetSuperWeaponIndex(i, pOwner);

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

	if (amount > 0.0f)
	{
		auto const pExt = BuildingTypeExt::Fetch(pThis->Type);

		if (pExt->Refinery_UseStorage)
		{
			// Store Tiberium in structures
			depositableTiberiumAmount = (amount * pTiberium->Value) / pDepositableTiberium->Value;
			pThis->Owner->GiveTiberium(depositableTiberiumAmount, idxStorageTiberiumType);
		}
	}
}

void BuildingExt::UpdatePrimaryFactoryAI()
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
		for (int i = 0; i < pBuilding->RadioLinks.Capacity; ++i)
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

	auto const pExt = BuildingTypeExt::Fetch(pBldType);

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
	if (auto const pExt = BuildingExt::TryFetch(pBuilding))
	{
		auto const pTypeExt = pExt->GetTypeExtData();

		pExt->AccumulatedIncome += refund;
		pExt->GrindingWeapon_AccumulatedCredits += refund;

		if (const auto pHouseExt = HouseExt::TryFetch(pBuilding->Owner))
		{
			const size_t resCount = ResourceTypeClass::Array.size();
			for (size_t i = 0; i < resCount; ++i)
			{
				const int resRefund = TechnoExt::GetResourceRefund(pTechno, static_cast<int>(i), true);
				if (resRefund > 0)
				{
					if (i >= pExt->AccumulatedResources.size())
						pExt->AccumulatedResources.resize(i + 1, 0.0f);

					pExt->AccumulatedResources[i] += static_cast<float>(resRefund);
					pHouseExt->UpdateResourceAmount(static_cast<int>(i), resRefund);
				}
			}
		}

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
void BuildingExt::ApplyPoweredKillSpawns()
{
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->GetTypeExtData();

	if (pTypeExt->Powered_KillSpawns && !pThis->IsPowerOnline())
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

bool BuildingExt::HandleInfiltrate(HouseClass* pInfiltratorHouse, int moneybefore)
{
	const auto pThis = this->OwnerObject();
	const auto pVictimHouse = pThis->Owner;
	const auto pTypeExt = this->GetTypeExtData();
	this->AccumulatedIncome += pVictimHouse->Available_Money() - moneybefore;

	if (!pVictimHouse->IsControlledByHuman() && !RulesExt::Global()->DisplayIncome_AllowAI)
	{
		if (AresHelper::CanUseAres)
			*reinterpret_cast<int*>(reinterpret_cast<char*>(this->OwnerObject()->align_154) + 168) += pVictimHouse->Available_Money() - moneybefore;
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

WeaponStruct* BuildingExt::GetLaserWeapon(BuildingClass* pThis)
{
	auto const pExt = BuildingExt::Fetch(pThis);

	if (pExt->CurrentLaserWeaponIndex.has_value())
		return pThis->GetWeapon(pExt->CurrentLaserWeaponIndex.value());

	return pThis->GetPrimaryWeapon();
}

void BuildingExt::UpdateFactoryQueues(BuildingClass* pThis)
{
	const auto pType = pThis->Type;
	const auto factory = pType->Factory;

	if (factory == AbstractType::None)
		return;

	if (const auto pFactory = pThis->Factory)
	{
		if (pFactory->Object)
		{
			if (pThis->Deactivated || !pThis->HasPower)
				pFactory->Suspend(false);
			else if (pFactory->IsSuspended && !pFactory->IsManual)
				pFactory->Unsuspend(false);
		}
	}

	pThis->Owner->Update_FactoriesQueues(factory, pType->Naval, BuildCat::DontCare);
}

void BuildingExt::KickOutClone(std::pair<TechnoTypeClass*, HouseClass*>& info, void*, BuildingClass* pFactory)
{
	if (!pFactory->IsAlive || pFactory->InLimbo || (BuildingTypeExt::Fetch(pFactory->Type)->Cloning_Powered && !pFactory->IsPowerOnline()) || pFactory->IsBeingWarpedOut())
		return;

	const auto pClone = static_cast<TechnoClass*>(info.first->CreateObject(info.second));

	if (pFactory->KickOutUnit(pClone, CellStruct::Empty) != KickOutResult::Succeeded)
		pClone->UnInit();
}

int BuildingExt::GetTurretFrame(BuildingClass* pThis)
{
	auto const pExt = BuildingExt::Fetch(pThis);
	auto const pTypeExt = pExt->GetTypeExtData();
	const int facing = pThis->PrimaryFacing.Current().GetValue<5>();
	const int shapeFacing = ObjectClass::BodyShape[facing];

	const bool isLowPower = !pThis->StuffEnabled || !pThis->IsPowerOnline();
	const bool isFiring = pExt->TurretAnimFiringFrame != -1;

	const int idleBlockSize = 32 * pTypeExt->TurretAnim_IdleFrames;
	const int lowPowerIdleBlockSize = 32 * pTypeExt->TurretAnim_LowPowerIdleFrames;
	const int firingBlockSize = 32 * pTypeExt->TurretAnim_FiringFrames;

	int framesPerFacing = pTypeExt->TurretAnim_IdleFrames;
	int baseOffset = 0;
	bool hasFiringFrames = false;

	if (isLowPower)
	{
		if (isFiring && pTypeExt->TurretAnim_LowPowerFiringFrames > 0)
		{
			framesPerFacing = pTypeExt->TurretAnim_LowPowerFiringFrames;
			baseOffset = idleBlockSize + lowPowerIdleBlockSize + firingBlockSize;
			hasFiringFrames = true;
		}
		else if (pTypeExt->TurretAnim_LowPowerIdleFrames > 0)
		{
			framesPerFacing = pTypeExt->TurretAnim_LowPowerIdleFrames;
			baseOffset = idleBlockSize;
		}
	}
	else
	{
		if (isFiring && pTypeExt->TurretAnim_FiringFrames > 0)
		{
			framesPerFacing = pTypeExt->TurretAnim_FiringFrames;
			baseOffset = idleBlockSize + lowPowerIdleBlockSize;
			hasFiringFrames = true;
		}
	}

	int animFrame = 0;

	if (isFiring && hasFiringFrames)
	{
		animFrame = pExt->TurretAnimFiringFrame;
		pExt->TurretAnimRateTick++;

		if (pExt->TurretAnimRateTick >= pTypeExt->TurretAnim_FiringRate)
		{
			pExt->TurretAnimRateTick = 0;
			pExt->TurretAnimFiringFrame++;
		}

		if (pExt->TurretAnimFiringFrame >= framesPerFacing)
		{
			pExt->TurretAnimFiringFrame = -1;
			pExt->TurretAnimIdleFrame = 0; // Reset idle anim frame.
			pExt->TurretAnimRateTick = 0;
		}
	}
	else if (framesPerFacing > 1)
	{
		animFrame = pExt->TurretAnimIdleFrame;
		pExt->TurretAnimRateTick++;

		if (pExt->TurretAnimRateTick >= pTypeExt->TurretAnim_IdleRate)
		{
			pExt->TurretAnimRateTick = 0;
			pExt->TurretAnimIdleFrame++;
		}

		if (pExt->TurretAnimIdleFrame >= framesPerFacing)
		{
			pExt->TurretAnimIdleFrame = 0;
			pExt->TurretAnimRateTick = 0;
		}
	}

	return baseOffset + (shapeFacing * framesPerFacing) + animFrame;
}

bool BuildingExt::BuildingOnline(BuildingClass* pThis)
{
	const Mission currentMission = pThis->CurrentMission;

	if (currentMission == Mission::Construction || currentMission == Mission::Selling
		|| pThis->EMPLockRemaining > 0 || !pThis->WasOnline || pThis->BunkerLinkedItem)
	{
		return false;
	}

	return true;
}

// =============================
// load / save

template <typename T>
void BuildingExt::Serialize(T& Stm)
{
	Stm
		.Process(this->DeployedTechno)
		.Process(this->IsCreatedFromMapFile)
		//.Process(this->HasPowerFromMapFile)
		.Process(this->LimboID)
		.Process(this->GrindingWeapon_LastFiredFrame)
		.Process(this->GrindingWeapon_AccumulatedCredits)
		.Process(this->CurrentAirFactory)
		.Process(this->AccumulatedIncome)
		.Process(this->AccumulatedResources)
		.Process(this->CurrentLaserWeaponIndex)
		.Process(this->PoweredUpToLevel)
		.Process(this->CurrentEMPulseSW)
		//.Process(this->IsFiringNow) It is set and reset within a same function.
		.Process(this->TurretAnimIdleFrame)
		.Process(this->TurretAnimFiringFrame)
		.Process(this->TurretAnimRateTick)
		.Process(this->ConstructionStartFacing)
		;
}

void BuildingExt::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingExt::SaveToStream(PhobosStreamWriter& Stm)
{
	TechnoExt::SaveToStream(Stm);
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
// container facade defined at the top of this file

// =============================
// container hooks

// Right after the TechnoClass base constructor returns: the extension must exist
// before the Init call further down this constructor (0x43BB29), which fills in
// TypeExtData and the rest of the per-object state.
DEFINE_HOOK(0x43B750, BuildingClass_CTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x454174, BuildingClass_Load_LightSource, 0xA)
{
	GET(BuildingClass*, pThis, EDI);

	SWIZZLE(pThis->LightSource);

	return 0x45417E;
}

static void __fastcall BuildingClass_InfiltratedBy_Wrapper(BuildingClass* pThis, void*, HouseClass* pInfiltratorHouse)
{
	const int oldBalance = pThis->Owner->Available_Money();
	// explicitly call because Ares rewrote it
	reinterpret_cast<void(__thiscall*)(BuildingClass*, HouseClass*)>(0x4571E0)(pThis, pInfiltratorHouse);

	BuildingExt::Fetch(pThis)->HandleInfiltrate(pInfiltratorHouse, oldBalance);
}

DEFINE_FUNCTION_JUMP(CALL, 0x51A00B, BuildingClass_InfiltratedBy_Wrapper);

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK(0x43C0B6, BuildingClass_DTOR, 0x9)
{
	GET(BuildingClass*, pItem, ESI);

	if (const auto pExt = BuildingExt::TryFetch(pItem))
	{
		pExt->ApplyCollectorRegistration(false);
	}

	BuildingExt::ExtMap.Remove(pItem);

	return 0;
}
