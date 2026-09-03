#include "Body.h"

#include <JumpjetLocomotionClass.h>

#include <Ext/Scenario/Body.h>
#include <Ext/UnitType/Body.h>
#include <Utilities/AresFunctions.h>

UnitExt::ExtContainer UnitExt::ExtMap;

UnitClass* UnitExt::Deployer = nullptr;

UnitExt::~UnitExt()
{
	if (this->UndergroundTracked)
		ScenarioExt::Global()->UndergroundTracker.Remove(this->OwnerObject());
}

// =============================
// load / save

template <typename T>
void UnitExt::Serialize(T& Stm)
{
	Stm
		.Process(this->SubterraneanHarvStatus)
		.Process(this->SubterraneanHarvRallyPoint)
		.Process(this->ReceiveDamage)
		.Process(this->DeployFireTimer)
		.Process(this->KeepTargetOnMove)
		.Process(this->SimpleDeployerAnimationTimer)
		.Process(this->IsBurrowed)
		.Process(this->UndergroundTracked)
		.Process(this->ExtraTurretRecoil)
		.Process(this->ExtraBarrelRecoil)
		;
}

// Subterranean harvester factory exit state machine.
void UnitExt::UpdateSubterraneanHarvester()
{
	auto const pThis = this->OwnerObject();

	// Unnecessary for AI players.
	if (!pThis->Owner->IsControlledByHuman())
		return;

	switch (this->SubterraneanHarvStatus)
	{
	case 0: // No state to handle.
		break;
	case 1: // Unit has been created.
		// If we're still in the factory do not advance.
		if (pThis->HasAnyLink())
			break;

		pThis->ClearNavigationList();

		// If we have rally point available, move to it and advance to next state, otherwise end here.
		if (this->SubterraneanHarvRallyPoint)
		{
			pThis->SetDestination(this->SubterraneanHarvRallyPoint, false);
			pThis->QueueMission(Mission::Move, true);
			this->SubterraneanHarvRallyPoint = nullptr;
			this->SubterraneanHarvStatus = 2;
			break;
		}
		else
		{
			this->SubterraneanHarvStatus = 0;
		}

		break;
	case 2: // Out of factory and on move.
		// If we're still moving don't start harvesting.
		if (pThis->Destination || pThis->CurrentMission == Mission::Move)
			break;

		// If harvester stops moving and becomes anything except idle, reset the state machine.
		if (pThis->CurrentMission != Mission::Guard)
		{
			this->SubterraneanHarvStatus = 0;
			break;
		}

		// Go harvest ore.
		pThis->ClearNavigationList();
		pThis->QueueMission(Mission::Harvest, true);
		this->SubterraneanHarvStatus = 0;
		break;
	default:
		break;
	}
}

// Resets target if KeepTargetOnMove unit moves beyond weapon range.
void UnitExt::UpdateKeepTargetOnMove()
{
	if (!this->KeepTargetOnMove)
		return;

	auto const pThis = this->OwnerObject();

	if (!pThis->Target)
	{
		this->KeepTargetOnMove = false;
		return;
	}

	const auto pTypeExt = this->TypeExtData;

	if (!pTypeExt->KeepTargetOnMove)
	{
		pThis->SetTarget(nullptr);
		this->KeepTargetOnMove = false;
		return;
	}

	if (pThis->CurrentMission == Mission::Guard)
	{
		if (!pTypeExt->KeepTargetOnMove_NoMorePursuit)
		{
			pThis->QueueMission(Mission::Attack, false);
			this->KeepTargetOnMove = false;
			return;
		}
	}
	else if (pThis->CurrentMission != Mission::Move)
	{
		return;
	}

	const int weaponIndex = pTypeExt->KeepTargetOnMove_Weapon >= 0 ? pTypeExt->KeepTargetOnMove_Weapon : pThis->SelectWeapon(pThis->Target);

	if (auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		const int extraDistance = static_cast<int>(pTypeExt->KeepTargetOnMove_ExtraDistance.Get());
		const int range = pWeapon->Range;
		pWeapon->Range += extraDistance; // Temporarily adjust weapon range based on the extra distance.

		if (!pThis->IsCloseEnough(pThis->Target, weaponIndex))
		{
			pThis->SetTarget(nullptr);
			this->KeepTargetOnMove = false;
		}

		pWeapon->Range = range;
	}
}

// Queues or cancels auto-deploy depending on the unit's remaining ammo.
void UnitExt::DepletedAmmoActions()
{
	auto const pTypeExt = this->GetTypeExtData();
	const int min = pTypeExt->Ammo_AutoDeployMinimumAmount;
	const int max = pTypeExt->Ammo_AutoDeployMaximumAmount;

	if (min < 0 && max < 0)
		return;

	auto const pType = pTypeExt->OwnerObject();

	if (pType->Ammo <= 0)
		return;

	auto const pThis = this->OwnerObject();
	auto const pUnitType = pThis->Type;

	if (!pUnitType->IsSimpleDeployer && !pUnitType->DeploysInto && !pUnitType->DeployFire
		&& pUnitType->Passengers < 1 && pThis->Passengers.NumPassengers < 1)
	{
		return;
	}

	const int ammo = pThis->Ammo;
	const bool canDeploy = UnitExt::HasAmmoToDeploy(pThis) && (min < 0 || ammo >= min) && (max < 0 || ammo <= max);
	const bool isDeploying = pThis->CurrentMission == Mission::Unload || pThis->QueuedMission == Mission::Unload;

	if (canDeploy && !isDeploying)
	{
		pThis->QueueMission(Mission::Unload, true);
	}
	else if (!canDeploy && isDeploying)
	{
		pThis->QueueMission(Mission::Guard, true);

		if (pUnitType->IsSimpleDeployer && pThis->InAir)
		{
			if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
				pJJLoco->State = JumpjetLocomotionClass::State::Ascending;
		}
	}
}

bool UnitExt::CannotMove(UnitClass* pThis)
{
	if (pThis->LocomotorSource)
		return false;

	const auto pType = pThis->Type;

	if (pType->Speed == 0)
		return true;

	const auto movementRestrictedTo = pType->MovementRestrictedTo;

	if (movementRestrictedTo == LandType::None)
		return false;

	auto landType = pThis->GetCell()->LandType;

	if (landType == LandType::Tunnel)
		return false;

	if (pThis->OnBridge && (landType == LandType::Water || landType == LandType::Beach))
		landType = LandType::Road;

	if (movementRestrictedTo != landType)
		return true;

	return false;
}

bool UnitExt::HasAmmoToDeploy(UnitClass* pThis)
{
	const auto pTypeExt = UnitTypeExt::Fetch(pThis->Type);

	const int min = pTypeExt->Ammo_DeployUnlockMinimumAmount;
	const int max = pTypeExt->Ammo_DeployUnlockMaximumAmount;

	if (min < 0 && max < 0)
		return true;

	const int ammo = pThis->Ammo;

	if ((min < 0 || ammo >= min) && (max < 0 || ammo <= max))
		return true;

	return false;
}

void UnitExt::HandleOnDeployAmmoChange(UnitClass* pThis, int maxAmmoOverride)
{
	const auto pTypeExt = UnitTypeExt::Fetch(pThis->Type);

	if (const int add = pTypeExt->Ammo_AddOnDeploy)
	{
		const int maxAmmo = maxAmmoOverride >= 0 ? maxAmmoOverride : pTypeExt->OwnerObject()->Ammo;
		const int originalAmmo = pThis->Ammo;
		pThis->Ammo = std::clamp(originalAmmo + add, 0, maxAmmo);

		if (originalAmmo != pThis->Ammo)
		{
			pThis->StartReloading();
			pThis->Mark(MarkType::Change);
		}
	}
}

bool UnitExt::SimpleDeployerAllowedToDeploy(UnitClass* pThis, bool defaultValue, bool alwaysCheckLandTypes)
{
	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return defaultValue;

	auto const pTypeExt = UnitTypeExt::Fetch(pType);

	if (alwaysCheckLandTypes || pTypeExt->IsSimpleDeployer_ConsiderPathfinding)
	{
		LandTypeFlags disallowedLandTypes;

		if (pTypeExt->IsSimpleDeployer_DisallowedLandTypes.isset())
		{
			disallowedLandTypes = pTypeExt->IsSimpleDeployer_DisallowedLandTypes.Get();
		}
		else
		{
			const bool isHover = pType->Locomotor == LocomotionClass::CLSIDs::Hover;
			const bool isJumpjet = pType->Locomotor == LocomotionClass::CLSIDs::Jumpjet;
			const bool isLander = pType->DeployToLand && (isJumpjet || isHover);
			disallowedLandTypes = isLander ? (LandTypeFlags)(LandTypeFlags::Water | LandTypeFlags::Beach) : LandTypeFlags::None;
		}

		if (IsLandTypeInFlags(disallowedLandTypes, pThis->GetCell()->LandType))
			return false;

		if (alwaysCheckLandTypes && !pTypeExt->IsSimpleDeployer_ConsiderPathfinding)
			return true;
	}
	else
	{
		return defaultValue;
	}

	auto const pTypeConvert = pTypeExt->Convert_Deploy;
	SpeedType speed = SpeedType::None;
	MovementZone mZone = MovementZone::None;

	if (AresFunctions::ConvertTypeTo && pTypeConvert)
	{
		speed = pTypeConvert->SpeedType;
		mZone = pTypeConvert->MovementZone;
	}
	else
	{
		speed = pType->SpeedType;
		mZone = pType->MovementZone;
	}

	if (speed != SpeedType::None && mZone != MovementZone::None)
	{
		auto const pCell = pThis->GetCell();
		return pCell->IsClearToMove(speed, true, true, -1, mZone, -1, pCell->ContainsBridge());
	}

	return true;
}

// Checks if vehicle can deploy into a building at its current location. If unit has no DeploysInto set returns noDeploysIntoDefaultValue (def = false) instead.
bool UnitExt::CanDeployIntoBuilding(UnitClass* pThis, bool noDeploysIntoDefaultValue)
{
	if (!pThis)
		return false;

	auto const pDeployType = pThis->Type->DeploysInto;

	if (!pDeployType)
		return noDeploysIntoDefaultValue;

	auto mapCoords = CellClass::Coord2Cell(pThis->GetCoords());

	if (pDeployType->GetFoundationWidth() > 2 || pDeployType->GetFoundationHeight(false) > 2)
		mapCoords += CellStruct { -1, -1 };

	// The vanilla game used an inappropriate approach here, resulting in potential risk of desync.
	// Now, through additional checks, we can directly exclude the unit who want to deploy.
	UnitExt::Deployer = pThis;
	const bool canDeploy = pDeployType->CanCreateHere(mapCoords, pThis->Owner);
	UnitExt::Deployer = nullptr;

	return canDeploy;
}

UnitTypeClass* UnitExt::GetUnitTypeExtra(UnitClass* pUnit, UnitTypeExt* pData)
{
	if (pUnit->IsGreenHP())
	{
		return nullptr;
	}
	else if (pUnit->IsYellowHP())
	{
		if (pUnit->GetCell()->LandType == LandType::Water && !pUnit->OnBridge)
		{
			if (auto const imageYellow = pData->WaterImage_ConditionYellow)
				return imageYellow;
		}
		else if (auto const imageYellow = pData->Image_ConditionYellow)
		{
			return abstract_cast<UnitTypeClass*, true>(imageYellow);
		}
	}
	else
	{
		if (pUnit->GetCell()->LandType == LandType::Water && !pUnit->OnBridge)
		{
			if (auto const imageRed = pData->WaterImage_ConditionRed)
				return imageRed;
			else if (auto const imageYellow = pData->WaterImage_ConditionYellow)
				return imageYellow;
		}
		else if (auto const imageRed = pData->Image_ConditionRed)
		{
			return abstract_cast<UnitTypeClass*, true>(imageRed);
		}
		else if (auto const imageYellow = pData->Image_ConditionYellow)
		{
			return abstract_cast<UnitTypeClass*, true>(imageYellow);
		}
	}

	return nullptr;
}

void UnitExt::InitializeRecoilData()
{
	const auto pTypeExt = static_cast<UnitTypeExt*>(this->TypeExtData);
	const auto pType = pTypeExt->OwnerObject();

	if (!pType->TurretRecoil)
		return;

	// Always resize to match the current type's count so that type conversions
	// (e.g. 9 turrets -> 2) do not leave stale elements that waste memory and
	// inflate the save file.
	this->ExtraTurretRecoil.resize(pTypeExt->ExtraTurretCount);

	if (pTypeExt->ExtraTurretCount)
	{
		const auto& refData = pType->TurretAnimData;

		for (auto& data : this->ExtraTurretRecoil)
		{
			data.Turret.Travel = refData.Travel;
			data.Turret.CompressFrames = refData.CompressFrames;
			data.Turret.RecoverFrames = refData.RecoverFrames;
			data.Turret.HoldFrames = refData.HoldFrames;
			data.TravelPerFrame = 0.0;
			data.TravelSoFar = 0.0;
			data.State = RecoilData::RecoilState::Inactive;
			data.TravelFramesLeft = 0;
		}
	}

	const auto dataCount = (pTypeExt->ExtraBarrelCount + 1) * (pTypeExt->ExtraTurretCount + 1) - 1;
	this->ExtraBarrelRecoil.resize(dataCount);

	if (dataCount)
	{
		const auto& refData = pType->BarrelAnimData;

		for (auto& data : this->ExtraBarrelRecoil)
		{
			data.Turret.Travel = refData.Travel;
			data.Turret.CompressFrames = refData.CompressFrames;
			data.Turret.RecoverFrames = refData.RecoverFrames;
			data.Turret.HoldFrames = refData.HoldFrames;
			data.TravelPerFrame = 0.0;
			data.TravelSoFar = 0.0;
			data.State = RecoilData::RecoilState::Inactive;
			data.TravelFramesLeft = 0;
		}
	}
}

void UnitExt::RecordRecoilData()
{
	const auto pThis = this->OwnerObject();
	const auto pTypeExt = static_cast<UnitTypeExt*>(this->TypeExtData);

	if (auto turretIndex = pTypeExt->BurstPerTurret
		? ((pThis->CurrentBurstIndex / pTypeExt->BurstPerTurret) % (pTypeExt->ExtraTurretCount + 1))
		: 0)
	{
		turretIndex -= 1;
		this->ExtraTurretRecoil[turretIndex].TravelSoFar = 0.0;
		this->ExtraTurretRecoil[turretIndex].Fire();
	}
	else
	{
		pThis->TurretRecoil.TravelSoFar = 0.0;
		pThis->TurretRecoil.Fire();
	}

	if (auto barrelIndex = (pTypeExt->ExtraTurretCount || pTypeExt->ExtraBarrelCount)
		? (pThis->CurrentBurstIndex % ((pTypeExt->ExtraBarrelCount + 1) * (pTypeExt->ExtraTurretCount + 1)))
		: 0)
	{
		barrelIndex -= 1;
		this->ExtraBarrelRecoil[barrelIndex].TravelSoFar = 0.0;
		this->ExtraBarrelRecoil[barrelIndex].Fire();
	}
	else
	{
		pThis->BarrelRecoil.TravelSoFar = 0.0;
		pThis->BarrelRecoil.Fire();
	}
}

// Skip vanilla recoil update
DEFINE_JUMP(LJMP, 0x6FA4D1, 0x6FA4FB);

void UnitExt::UpdateRecoilData()
{
	if (!this->TypeExtData->OwnerObject()->TurretRecoil)
		return;

	const auto pThis = this->OwnerObject();

	pThis->TurretRecoil.Update();
	pThis->BarrelRecoil.Update();

	for (auto& data : this->ExtraTurretRecoil)
		data.Update();

	for (auto& data : this->ExtraBarrelRecoil)
		data.Update();
}

void UnitExt::LoadFromStream(PhobosStreamReader& Stm)
{
	FootExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void UnitExt::SaveToStream(PhobosStreamWriter& Stm)
{
	FootExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

UnitExt::ExtContainer::ExtContainer() : Container("UnitClass") { }
UnitExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x7353D3, UnitClass_CTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);

	UnitExt::ExtMap.Allocate(pItem);

	return 0;
}

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK_AGAIN(0x7359DA, UnitClass_DTOR, 0x9)
DEFINE_HOOK(0x735967, UnitClass_DTOR, 0x9)
{
	GET(UnitClass*, pItem, ESI);

	UnitExt::ExtMap.Remove(pItem);

	return 0;
}
