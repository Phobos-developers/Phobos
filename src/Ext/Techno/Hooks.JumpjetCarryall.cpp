#include <JumpjetLocomotionClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/UnitType/Body.h>

// Jumpjet Carryall System for Vehicles
// Allows jumpjet vehicles (especially BalloonHover) to pick up and carry other units
// Author: Phobos Development Team

// Helper: Check if a unit can act as a jumpjet carryall
bool IsJumpjetCarryall(UnitClass* pUnit)
{
	if (!pUnit)
		return false;

	auto const pType = pUnit->Type;
	if (!pType->JumpJet)
		return false;

	auto const pExt = UnitTypeExt::Fetch(pType);
	return pExt->JumpjetCarryall;
}

// Helper: Check if a techno can be carried by a jumpjet carryall
bool CanBeCarriedByJumpjetVehicle(UnitClass* pCarrier, TechnoClass* pTarget)
{
	if (!pCarrier || !pTarget)
		return false;

	auto const pCarrierType = pCarrier->Type;
	auto const pCarrierExt = UnitTypeExt::Fetch(pCarrierType);

	// Must have JumpjetCarryall enabled
	if (!pCarrierExt->JumpjetCarryall)
		return false;

	// Must have jumpjet locomotor
	if (!pCarrierType->JumpJet)
		return false;

	// Check if target is infantry or vehicle
	auto const rtti = pTarget->WhatAmI();
	bool isInfantry = (rtti == AbstractType::Infantry);
	bool isVehicle = (rtti == AbstractType::Unit);

	if (!isInfantry && !isVehicle)
		return false;

	// Check allowed types
	if (isInfantry && !pCarrierExt->JumpjetCarryall_AllowInfantry)
		return false;

	if (isVehicle && !pCarrierExt->JumpjetCarryall_AllowVehicles)
		return false;

	// Target must be a foot class (infantry or unit)
	auto const pTargetFoot = abstract_cast<FootClass*>(pTarget);
	if (!pTargetFoot)
		return false;

	// Target must be on the ground
	if (pTargetFoot->GetHeight() > 0)
		return false;

	// Target cannot be in water (for now)
	auto const pTargetCell = pTarget->GetCell();
	if (pTargetCell && pTargetCell->LandType == LandType::Water)
		return false;

	// Check Size limit (Ares-compatible)
	auto const pTargetType = pTarget->GetTechnoType();
	if (pCarrierExt->JumpjetCarryall_SizeLimit >= 0)
	{
		if (pTargetType->Size > pCarrierExt->JumpjetCarryall_SizeLimit)
			return false;
	}

	// Check if target type is in the allowed list (if specified)
	if (!pCarrierExt->JumpjetCarryall_CanLift.empty())
	{
		bool found = false;

		for (auto const& allowedType : pCarrierExt->JumpjetCarryall_CanLift)
		{
			if (pTargetType == allowedType)
			{
				found = true;
				break;
			}
		}

		if (!found)
			return false;
	}

	// Check if carrier already has cargo at capacity
	if (pCarrier->HasAnyLink())
	{
		// TODO: Implement multi-cargo support
		// For now, only allow one unit
		return false;
	}

	// Check if target is already being carried
	if (pTargetFoot->BunkerLinkedItem)
		return false;

	// Target cannot be mind-controlled if carrier doesn't own it
	if (pTarget->MindControlledBy && pTarget->Owner != pCarrier->Owner)
		return false;

	return true;
}

// Helper: Get cargo count for a jumpjet carryall
int GetJumpjetCargoCount(UnitClass* pUnit)
{
	int count = 0;
	if (pUnit->HasAnyLink())
	{
		auto pCargo = abstract_cast<FootClass*>(pUnit->AttachTrigger);
		while (pCargo)
		{
			count++;
			pCargo = abstract_cast<FootClass*>(pCargo->NextObject);
		}
	}
	return count;
}

// Hook: Allow jumpjet vehicles to execute Enter mission (pickup)
DEFINE_HOOK(0x7393D0, UnitClass_Mission_Enter_JumpjetCarryall, 0x6)
{
	GET(UnitClass*, pThis, ECX);

	// Check if this is a jumpjet carryall
	if (!IsJumpjetCarryall(pThis))
		return 0;

	// Get the target
	auto const pDest = abstract_cast<FootClass*>(pThis->Destination);
	if (!pDest)
		return 0;

	// Check if we can carry this target
	if (!CanBeCarriedByJumpjetVehicle(pThis, pDest))
		return 0;

	// Continue with mission
	return 0;
}

// Hook: Handle jumpjet carryall pickup when reaching target
DEFINE_HOOK(0x4CE8CF, FlyLocomotionClass_ILocomotion_MoveTo_JumpjetPickup, 0x6)
{
	GET(ILocomotion*, pLoco, ESI);

	auto const jjLoco = static_cast<JumpjetLocomotionClass*>(pLoco);
	auto const pUnit = static_cast<UnitClass*>(jjLoco->LinkedTo);

	if (!pUnit)
		return 0;

	// Only process if this is a jumpjet carryall
	if (!IsJumpjetCarryall(pUnit))
		return 0;

	auto const pType = pUnit->Type;
	auto const pExt = UnitTypeExt::Fetch(pType);

	// Check if we're hovering near the target
	if (pType->BalloonHover && pUnit->IsInAir())
	{
		auto const pTarget = abstract_cast<FootClass*>(pUnit->Destination);
		if (pTarget && CanBeCarriedByJumpjetVehicle(pUnit, pTarget))
		{
			// Check distance to target
			auto const dist = pUnit->DistanceFrom(pTarget);
			if (dist < pExt->JumpjetCarryall_PickupRange)
			{
				// Attach the target as cargo
				pUnit->AttachTrigger = pTarget;
				pTarget->BunkerLinkedItem = pUnit;

				// Hide the carried unit
				pTarget->Remove();
				pTarget->Limbo();

				// Play pickup voice if set
				if (pExt->JumpjetCarryall_VoicePickup.isset())
				{
					pUnit->QueueVoice(pExt->JumpjetCarryall_VoicePickup.Get());
				}
				else
				{
					pUnit->QueueVoice(pType->VoiceMove);
				}

				// Clear destination so carrier doesn't keep trying to move
				pUnit->SetDestination(nullptr, false);
			}
		}
	}

	return 0;
}

// Hook: Handle jumpjet carryall unload mission
DEFINE_HOOK(0x739B10, UnitClass_Mission_Unload_JumpjetCarryall, 0x6)
{
	GET(UnitClass*, pThis, ECX);

	// Only process if this is a jumpjet carryall
	if (!IsJumpjetCarryall(pThis))
		return 0;

	auto const pType = pThis->Type;
	auto const pExt = UnitTypeExt::Fetch(pType);

	// Check if we have cargo
	auto const pCargo = abstract_cast<FootClass*>(pThis->AttachTrigger);
	if (!pCargo)
		return 0;

	// If we're hovering and ready to drop
	if (pType->BalloonHover && pThis->IsInAir())
	{
		// Get drop location (carrier's current position)
		CoordStruct dropCoord = pThis->Location;
		auto const pCell = pThis->GetCell();

		if (pCell)
		{
			// Place cargo at cell center, ground level
			dropCoord = pCell->GetCoordsWithBridge();
		}
		else
		{
			dropCoord.Z = 0; // Fallback: drop to ground level
		}

		// Detach cargo
		pThis->AttachTrigger = nullptr;
		pCargo->BunkerLinkedItem = nullptr;

		// Place cargo on map
		++Unsorted::IKnowWhatImDoing;
		pCargo->Unlimbo(dropCoord, DirType::North);
		--Unsorted::IKnowWhatImDoing;

		// Make cargo visible again
		pCargo->Transporter = nullptr;

		// Play dropoff voice if set
		if (pExt->JumpjetCarryall_VoiceDropoff.isset())
		{
			pThis->QueueVoice(pExt->JumpjetCarryall_VoiceDropoff.Get());
		}
	}

	return 0;
}

// Hook: Update cargo position while being carried
DEFINE_HOOK(0x4D9FED, FootClass_Update_JumpjetCarryallCargo, 0x6)
{
	GET(FootClass*, pThis, ESI);

	// Check if this unit is being carried by a jumpjet
	auto const pCarrier = abstract_cast<UnitClass*>(pThis->BunkerLinkedItem);
	if (!pCarrier)
		return 0;

	// Only process if carrier is a jumpjet carryall
	if (!IsJumpjetCarryall(pCarrier))
		return 0;

	// Keep cargo hidden and synchronized with carrier position
	if (!pThis->InLimbo)
	{
		pThis->Remove();
		pThis->Limbo();
	}

	return 0;
}

// Hook: Render cargo position beneath jumpjet carrier (visual)
DEFINE_HOOK(0x73C4F5, UnitClass_Draw_It_JumpjetCarryallCargo, 0x6)
{
	GET(UnitClass*, pThis, EBP);

	// Only process if this is a jumpjet carryall
	if (!IsJumpjetCarryall(pThis))
		return 0;

	auto const pExt = UnitTypeExt::Fetch(pThis->Type);

	// Check if we have cargo and should draw it
	if (!pExt->JumpjetCarryall_DrawCargo)
		return 0;

	auto const pCargo = abstract_cast<TechnoClass*>(pThis->AttachTrigger);
	if (!pCargo)
		return 0;

	// TODO: Implement cargo rendering
	// This requires:
	// 1. Getting the carrier's draw parameters
	// 2. Calculating offset position using JumpjetCarryall_CargoOffset
	// 3. Drawing the cargo sprite at that offset
	// 4. Handling shadows and transparency

	return 0;
}

// Hook: Modify speed when carrying cargo
DEFINE_HOOK(0x4CE5B8, FlyLocomotionClass_GetCurrentSpeed_JumpjetCarryall, 0x6)
{
	GET(JumpjetLocomotionClass*, pLoco, ESI);

	auto const pUnit = static_cast<UnitClass*>(pLoco->LinkedTo);
	if (!pUnit)
		return 0;

	// Only process if this is a jumpjet carryall with cargo
	if (!IsJumpjetCarryall(pUnit) || !pUnit->HasAnyLink())
		return 0;

	auto const pExt = UnitTypeExt::Fetch(pUnit->Type);

	// Apply speed multiplier when carrying cargo
	if (pExt->JumpjetCarryall_SpeedMultiplier != 1.0)
	{
		GET(int, speed, EAX);
		R->EAX(static_cast<int>(speed * pExt->JumpjetCarryall_SpeedMultiplier));
	}

	return 0;
}

// Hook: Release cargo if carrier dies
DEFINE_HOOK(0x4D97CD, FootClass_ReceiveDamage_JumpjetCarryallDeath, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(int*, pDamage, EBX);

	auto const pUnit = abstract_cast<UnitClass*>(pThis);
	if (!pUnit || !IsJumpjetCarryall(pUnit))
		return 0;

	// Check if carrier will die from this damage
	if (pThis->Health - *pDamage <= 0 && pUnit->HasAnyLink())
	{
		auto const pCargo = abstract_cast<FootClass*>(pUnit->AttachTrigger);
		if (pCargo)
		{
			// Get drop location
			CoordStruct dropCoord = pUnit->Location;
			dropCoord.Z = 0; // Drop to ground

			// Detach cargo
			pUnit->AttachTrigger = nullptr;
			pCargo->BunkerLinkedItem = nullptr;

			// Place cargo on map (might take falling damage)
			++Unsorted::IKnowWhatImDoing;
			pCargo->Unlimbo(dropCoord, DirType::North);
			--Unsorted::IKnowWhatImDoing;

			pCargo->Transporter = nullptr;
		}
	}

	return 0;
}
