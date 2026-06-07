#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>

BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;

// Assuming SuperWeapon & SuperWeapon2 are used (for the moment)
int BuildingTypeExt::ExtData::GetSuperWeaponCount() const
{
	// The user should only use SuperWeapon and SuperWeapon2 if the attached sw count isn't bigger than 2
	const auto pThis = this->OwnerObject();
	int count = pThis->SuperWeapon >= 0 ? 1 : 0;
	count += pThis->SuperWeapon2 >= 0 ? 1 : 0;
	return count + this->SuperWeapons.size();
}

int BuildingTypeExt::ExtData::GetSuperWeaponIndex(const int index, HouseClass* pHouse) const
{
	const int idxSW = this->GetSuperWeaponIndex(index);

	if (const auto pSuper = pHouse->Supers.GetItemOrDefault(idxSW))
	{
		const auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);

		if (!pExt->IsAvailable(pHouse))
			return -1;
	}

	return idxSW;
}

int BuildingTypeExt::ExtData::GetSuperWeaponIndex(const int index) const
{
	const auto pThis = this->OwnerObject();

	// 2 = SuperWeapon & SuperWeapon2
	if (index < 2)
		return !index ? pThis->SuperWeapon : pThis->SuperWeapon2;
	else if (index - 2 < (int)this->SuperWeapons.size())
		return this->SuperWeapons[index - 2];

	return -1;
}

std::pair<int, int> BuildingTypeExt::GetEnhancedPower(BuildingTypeClass* pBuilding, int output, HouseClass* pHouse, BuildingClass* pPowerPlant)
{
	const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
	int amount = 0;
	float factor = 1.0f;
	std::map<int, int> applied; // index, count

	for (const auto pEnhancer : pHouseExt->PowerPlantEnhancers)
	{
		if (!TechnoExt::IsActive(pEnhancer) || !pEnhancer->HasPower)
			continue;

		const auto pEnhancerType = pEnhancer->Type;
		const auto pEnhancerTypeExt = BuildingTypeExt::ExtMap.Find(pEnhancerType);

		if (!pEnhancerTypeExt->PowerPlantEnhancer_Buildings.Contains(pBuilding))
			continue;

		const int range = pEnhancerTypeExt->PowerPlantEnhancer_Range.Get();

		if (range > 0 && (!pPowerPlant || pEnhancer->DistanceFrom(pPowerPlant) > range))
			continue;

		const int max = pEnhancerTypeExt->PowerPlantEnhancer_MaxCount;

		if (max > 0)
		{
			const auto it = applied.find(pEnhancerType->ArrayIndex);

			if (it != applied.cend() && it->second >= max)
				continue;
		}

		factor *= pEnhancerTypeExt->PowerPlantEnhancer_Factor;
		amount += pEnhancerTypeExt->PowerPlantEnhancer_Amount;
		++applied[pEnhancerType->ArrayIndex];
	}

	return std::make_pair(static_cast<int>(std::round(output * factor)), amount);
}

void BuildingTypeExt::PlayBunkerSound(BuildingClass const* pThis, bool buildUp)
{
	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	auto const nSound = buildUp
		? pTypeExt->BunkerWallsUpSound.Get(RulesClass::Instance->BunkerWallsUpSound)
		: pTypeExt->BunkerWallsDownSound.Get(RulesClass::Instance->BunkerWallsDownSound);

	if (nSound != -1)
		VocClass::PlayAt(nSound, pThis->Location);
}

int BuildingTypeExt::CountOwnedNowWithDeployOrUpgrade(BuildingTypeClass* pType, HouseClass* pHouse)
{
	const auto upgrades = BuildingTypeExt::GetUpgradesAmount(pType, pHouse);

	if (upgrades != -1)
		return upgrades;

	if (const auto pUndeploy = pType->UndeploysInto)
		return pHouse->CountOwnedNow(pType) + pHouse->CountOwnedNow(pUndeploy);

	return pHouse->CountOwnedNow(pType);
}

int BuildingTypeExt::GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse) // not including producing upgrades
{
	int result = 0;
	bool isUpgrade = false;

	auto checkUpgrade = [pHouse, pBuilding, &result, &isUpgrade](BuildingTypeClass* pTPowersUp)
	{
		isUpgrade = true;

		for (auto const& pBld : pHouse->Buildings)
		{
			if (pBld->UpgradeLevel && pBld->Type == pTPowersUp)
			{
				for (auto const& pUpgrade : pBld->Upgrades)
				{
					if (pUpgrade == pBuilding)
						++result;
				}
			}
		}
	};

	// June 7, 2026 - Starkku: PowersUpBuilding is now put in PowersUp_Buildings
	/*
	auto const pPowersUp = pBuilding->PowersUpBuilding;

	if (pPowersUp[0])
	{
		if (auto const pTPowersUp = BuildingTypeClass::Find(pPowersUp))
			checkUpgrade(pTPowersUp);
	}*/

	for (auto const pTPowersUp : BuildingTypeExt::ExtMap.Find(pBuilding)->PowersUp_Buildings)
		checkUpgrade(pTPowersUp);

	return isUpgrade ? result : -1;
}

void BuildingTypeExt::ExtData::Initialize()
{ }

// =============================
// load / save

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;
	auto pArtINI = &CCINIClass::INI_Art;
	INI_EX exINI(pINI);
	INI_EX exArtINI(pArtINI);

	this->PowersUp_Owner.Read(exINI, pSection, "PowersUp.Owner");
	this->PowersUp_Buildings.Read(exINI, pSection, "PowersUp.Buildings");
	this->PowerPlant_DamageFactor.Read(exINI, pSection, "PowerPlant.DamageFactor");
	this->PowerPlantEnhancer_Buildings.Read(exINI, pSection, "PowerPlantEnhancer.PowerPlants");
	this->PowerPlantEnhancer_Range.Read(exINI, pSection, "PowerPlantEnhancer.Range");
	this->PowerPlantEnhancer_Amount.Read(exINI, pSection, "PowerPlantEnhancer.Amount");
	this->PowerPlantEnhancer_Factor.Read(exINI, pSection, "PowerPlantEnhancer.Factor");
	this->PowerPlantEnhancer_MaxCount.Read(exINI, pSection, "PowerPlantEnhancer.MaxCount");
	this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");
	this->CanC4_AllowZeroDamage.Read(exINI, pSection, "CanC4.AllowZeroDamage");

	this->InitialStrength_Cloning.Read(exINI, pSection, "InitialStrength.Cloning");
	this->Cloning_Powered.Read(exINI, pSection, "Cloning.Powered");
	this->ExcludeFromMultipleFactoryBonus.Read(exINI, pSection, "ExcludeFromMultipleFactoryBonus");

	this->Grinding_AllowAllies.Read(exINI, pSection, "Grinding.AllowAllies");
	this->Grinding_AllowOwner.Read(exINI, pSection, "Grinding.AllowOwner");
	this->Grinding_AllowTypes.Read(exINI, pSection, "Grinding.AllowTypes");
	this->Grinding_DisallowTypes.Read(exINI, pSection, "Grinding.DisallowTypes");
	this->Grinding_Sound.Read(exINI, pSection, "Grinding.Sound");
	this->Grinding_PlayDieSound.Read(exINI, pSection, "Grinding.PlayDieSound");
	this->Grinding_Weapon.Read<true>(exINI, pSection, "Grinding.Weapon");
	this->Grinding_Weapon_RequiredCredits.Read(exINI, pSection, "Grinding.Weapon.RequiredCredits");

	this->DisplayIncome.Read(exINI, pSection, "DisplayIncome");
	this->DisplayIncome_Delay.Read(exINI, pSection, "DisplayIncome.Delay");
	if (this->DisplayIncome_Delay.isset() && this->DisplayIncome_Delay == 0)
	{
		Debug::Log("[Developer warning] [%s] DisplayIncome.Delay is set to 0, forcing to 1.\n", pSection);
		this->DisplayIncome_Delay = 1;
	}
	this->DisplayIncome_Houses.Read(exINI, pSection, "DisplayIncome.Houses");
	this->DisplayIncome_Offset.Read(exINI, pSection, "DisplayIncome.Offset");

	this->ConsideredVehicle.Read(exINI, pSection, "ConsideredVehicle");
	this->SellBuildupLength.Read(exINI, pSection, "SellBuildupLength");
	this->IsDestroyableObstacle.Read(exINI, pSection, "IsDestroyableObstacle");

	this->FactoryPlant_AllowTypes.Read(exINI, pSection, "FactoryPlant.AllowTypes");
	this->FactoryPlant_DisallowTypes.Read(exINI, pSection, "FactoryPlant.DisallowTypes");
	this->FactoryPlant_MaxCount.Read(exINI, pSection, "FactoryPlant.MaxCount");

	this->Units_RepairRate.Read(exINI, pSection, "Units.RepairRate");
	this->Units_RepairStep.Read(exINI, pSection, "Units.RepairStep");
	this->Units_RepairPercent.Read(exINI, pSection, "Units.RepairPercent");
	this->Units_UseRepairCost.Read(exINI, pSection, "Units.UseRepairCost");

	this->NoBuildAreaOnBuildup.Read(exINI, pSection, "NoBuildAreaOnBuildup");
	this->Adjacent_Allowed.Read(exINI, pSection, "Adjacent.Allowed");
	this->Adjacent_Disallowed.Read(exINI, pSection, "Adjacent.Disallowed");
	this->Adjacent_Disallowed_Prohibit.Read(exINI, pSection, "Adjacent.Disallowed.Prohibit");
	this->Adjacent_Disallowed_ProhibitDistance.Read(exINI, pSection, "Adjacent.Disallowed.ProhibitDistance");

	this->BarracksExitCell.Read(exINI, pSection, "BarracksExitCell");

	this->Overpower_KeepOnline.Read(exINI, pSection, "Overpower.KeepOnline");
	this->Overpower_ChargeWeapon.Read(exINI, pSection, "Overpower.ChargeWeapon");

	this->DisableDamageSound.Read(exINI, pSection, "DisableDamageSound");

	this->BuildingOccupyDamageMult.Read(exINI, pSection, "OccupyDamageMultiplier");
	this->BuildingOccupyROFMult.Read(exINI, pSection, "OccupyROFMultiplier");
	this->BuildingBunkerDamageMult.Read(exINI, pSection, "BunkerDamageMultiplier");
	this->BuildingBunkerROFMult.Read(exINI, pSection, "BunkerROFMultMultiplier");
	this->BunkerWallsUpSound.Read(exINI, pSection, "BunkerWallsUpSound");
	this->BunkerWallsDownSound.Read(exINI, pSection, "BunkerWallsDownSound");
	this->BuildingRepairedSound.Read(exINI, pSection, "BuildingRepairedSound");
	this->Refinery_UseStorage.Read(exINI, pSection, "Refinery.UseStorage");
	this->UndeploysInto_Sellable.Read(exINI, pSection, "UndeploysInto.Sellable");
	this->BuildingRadioLink_SyncOwner.Read(exINI, pSection, "BuildingRadioLink.SyncOwner");
	this->GuardRetryDelay.Read(exINI, pSection, "GuardRetryDelay");

	if (pThis->PowersUpBuilding[0] == NULL && this->PowersUp_Buildings.size() > 0)
	{
		strcpy_s(pThis->PowersUpBuilding, this->PowersUp_Buildings[0]->ID);
	}
	else if (pThis->PowersUpBuilding[0])
	{
		auto pPowerUpType = BuildingTypeClass::Find(pThis->PowersUpBuilding);

		if (pPowerUpType && !this->PowersUp_Buildings.Contains(pPowerUpType))
			this->PowersUp_Buildings.emplace_back(pPowerUpType);
	}

	if (pThis->NumberOfDocks > 0)
	{
		std::optional<DirType> empty;
		this->AircraftDockingDirs.resize(pThis->NumberOfDocks, empty);

		Nullable<DirType> nLandingDir;
		nLandingDir.Read(exINI, pSection, "AircraftDockingDir");

		if (nLandingDir.isset())
			this->AircraftDockingDirs[0] = nLandingDir.Get();

		for (int i = 0; i < pThis->NumberOfDocks; ++i)
		{
			char tempBuffer[32];
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "AircraftDockingDir%d", i);
			nLandingDir.Read(exINI, pSection, tempBuffer);

			if (nLandingDir.isset())
				this->AircraftDockingDirs[i] = nLandingDir.Get();
		}
	}

	this->Refinery_UseNormalActiveAnim.Read(exArtINI, pArtSection, "Refinery.UseNormalActiveAnim");

	// Ares tag
	this->SpyEffect_Custom.Read(exINI, pSection, "SpyEffect.Custom");
	if (SuperWeaponTypeClass::Array.Count > 0)
	{
		this->SuperWeapons.Read(exINI, pSection, "SuperWeapons");

		this->SpyEffect_VictimSuperWeapon.Read(exINI, pSection, "SpyEffect.VictimSuperWeapon");
		this->SpyEffect_InfiltratorSuperWeapon.Read(exINI, pSection, "SpyEffect.InfiltratorSuperWeapon");
	}

	if (pThis->MaxNumberOccupants > 10)
	{
		char tempBuffer[32];
		this->OccupierMuzzleFlashes.clear();
		this->OccupierMuzzleFlashes.resize(pThis->MaxNumberOccupants);

		for (int i = 0; i < pThis->MaxNumberOccupants; ++i)
		{
			Nullable<Point2D> nMuzzleLocation;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "MuzzleFlash%d", i);
			nMuzzleLocation.Read(exArtINI, pArtSection, tempBuffer);
			this->OccupierMuzzleFlashes[i] = nMuzzleLocation.Get(Point2D::Empty);
		}
	}

	// PlacementPreview
	{
		this->PlacementPreview.Read(exINI, pSection, "PlacementPreview");
		this->PlacementPreview_Shape.Read(exINI, pSection, "PlacementPreview.Shape");
		this->PlacementPreview_ShapeFrame.Read(exINI, pSection, "PlacementPreview.ShapeFrame");
		this->PlacementPreview_Offset.Read(exINI, pSection, "PlacementPreview.Offset");
		this->PlacementPreview_Remap.Read(exINI, pSection, "PlacementPreview.Remap");
		this->PlacementPreview_Palette.LoadFromINI(pINI, pSection, "PlacementPreview.Palette");
		this->PlacementPreview_Translucency.Read(exINI, pSection, "PlacementPreview.Translucency");
	}

	// Art
	this->IsAnimDelayedBurst.Read(exArtINI, pArtSection, "IsAnimDelayedBurst");
	this->ZShapePointMove_OnBuildup.Read(exArtINI, pArtSection, "ZShapePointMove.OnBuildup");

	// Ares 0.2
	this->CloningFacility.Read(exINI, pSection, "CloningFacility");

	// Ares 0.A
	this->RubbleIntact.Read(exINI, pSection, "Rubble.Intact");
	this->RubbleIntactRemove.Read(exINI, pSection, "Rubble.Intact.Remove");

	// Ares 3.0
	this->UnitSell.Read(exINI, pSection, "UnitSell");
}

void BuildingTypeExt::ExtData::CompleteInitialization()
{
	auto const pThis = this->OwnerObject();
	UNREFERENCED_PARAMETER(pThis);
}

template <typename T>
void BuildingTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->PowersUp_Owner)
		.Process(this->PowersUp_Buildings)
		.Process(this->PowerPlant_DamageFactor)
		.Process(this->PowerPlantEnhancer_Buildings)
		.Process(this->PowerPlantEnhancer_Range)
		.Process(this->PowerPlantEnhancer_Amount)
		.Process(this->PowerPlantEnhancer_Factor)
		.Process(this->PowerPlantEnhancer_MaxCount)
		.Process(this->SuperWeapons)
		.Process(this->OccupierMuzzleFlashes)
		.Process(this->Powered_KillSpawns)
		.Process(this->CanC4_AllowZeroDamage)
		.Process(this->InitialStrength_Cloning)
		.Process(this->Cloning_Powered)
		.Process(this->ExcludeFromMultipleFactoryBonus)
		.Process(this->Refinery_UseStorage)
		.Process(this->Grinding_AllowAllies)
		.Process(this->Grinding_AllowOwner)
		.Process(this->Grinding_AllowTypes)
		.Process(this->Grinding_DisallowTypes)
		.Process(this->Grinding_Sound)
		.Process(this->Grinding_PlayDieSound)
		.Process(this->Grinding_Weapon)
		.Process(this->Grinding_Weapon_RequiredCredits)
		.Process(this->DisplayIncome)
		.Process(this->DisplayIncome_Delay)
		.Process(this->DisplayIncome_Houses)
		.Process(this->DisplayIncome_Offset)
		.Process(this->PlacementPreview)
		.Process(this->PlacementPreview_Shape)
		.Process(this->PlacementPreview_ShapeFrame)
		.Process(this->PlacementPreview_Offset)
		.Process(this->PlacementPreview_Remap)
		.Process(this->PlacementPreview_Palette)
		.Process(this->PlacementPreview_Translucency)
		.Process(this->SpyEffect_Custom)
		.Process(this->SpyEffect_VictimSuperWeapon)
		.Process(this->SpyEffect_InfiltratorSuperWeapon)
		.Process(this->ConsideredVehicle)
		.Process(this->ZShapePointMove_OnBuildup)
		.Process(this->SellBuildupLength)
		.Process(this->AircraftDockingDirs)
		.Process(this->FactoryPlant_AllowTypes)
		.Process(this->FactoryPlant_DisallowTypes)
		.Process(this->FactoryPlant_MaxCount)
		.Process(this->IsAnimDelayedBurst)
		.Process(this->IsDestroyableObstacle)
		.Process(this->Units_RepairRate)
		.Process(this->Units_RepairStep)
		.Process(this->Units_RepairPercent)
		.Process(this->Units_UseRepairCost)
		.Process(this->NoBuildAreaOnBuildup)
		.Process(this->Adjacent_Allowed)
		.Process(this->Adjacent_Disallowed)
		.Process(this->Adjacent_Disallowed_Prohibit)
		.Process(this->Adjacent_Disallowed_ProhibitDistance)
		.Process(this->BarracksExitCell)
		.Process(this->Overpower_KeepOnline)
		.Process(this->Overpower_ChargeWeapon)
		.Process(this->DisableDamageSound)
		.Process(this->BuildingOccupyDamageMult)
		.Process(this->BuildingOccupyROFMult)
		.Process(this->BuildingBunkerDamageMult)
		.Process(this->BuildingBunkerROFMult)
		.Process(this->BunkerWallsUpSound)
		.Process(this->BunkerWallsDownSound)
		.Process(this->BuildingRepairedSound)
		.Process(this->Refinery_UseNormalActiveAnim)
		.Process(this->HasPowerUpAnim)
		.Process(this->UndeploysInto_Sellable)
		.Process(this->BuildingRadioLink_SyncOwner)
		.Process(this->GuardRetryDelay)

		// Ares 0.2
		.Process(this->CloningFacility)

		// Ares 0.A
		.Process(this->RubbleIntact)
		.Process(this->RubbleIntactRemove)

		// Ares 3.0
		.Process(this->UnitSell)
		;
}

void BuildingTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BuildingTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BuildingTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingTypeExt::ExtContainer::Load(BuildingTypeClass* pThis, IStream* pStm)
{
	BuildingTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	return pData != nullptr;
};

bool BuildingTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{

	return Stm.Success();
}

bool BuildingTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{


	return Stm.Success();
}
// =============================
// container

BuildingTypeExt::ExtContainer::ExtContainer() : Container("BuildingTypeClass") { }

BuildingTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x45E50C, BuildingTypeClass_CTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x45E707, BuildingTypeClass_DTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, ESI);

	BuildingTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x465300, BuildingTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x465010, BuildingTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x4652ED, BuildingTypeClass_Load_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46536A, BuildingTypeClass_Save_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.SaveStatic();
	return 0;
}

//DEFINE_HOOK_AGAIN(0x464A56, BuildingTypeClass_LoadFromINI, 0xA)// Section dont exist!
DEFINE_HOOK(0x464A49, BuildingTypeClass_LoadFromINI, 0xA)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
