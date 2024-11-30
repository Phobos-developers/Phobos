#include "Body.h"

#include <JumpjetLocomotionClass.h>

#pragma region UnitsFacing

// Would it be better to rewrite the entire UpdateRotation() ?
DEFINE_HOOK(0x736AEA, UnitClass_UpdateRotation_ApplyUnitIdleAction, 0x6)
{
	enum { SkipGameCode = 0x736BE2 };

	GET(UnitClass* const, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	// Turning to target?
	if (pThis->SecondaryFacing.IsRotating())
	{
		// Repeatedly check TurretSpins and IsRotating() seems unnecessary
		pThis->unknown_bool_6AF = true;

		if (!pExt || (!pExt->UnitIdleActionTimer.IsTicking() && !pExt->UnitIdleActionGapTimer.IsTicking() && !pExt->UnitIdleIsSelected))
			return SkipGameCode;
	}

	const bool canCheck = pExt && pExt->TypeExtData;
	const auto currentMission = pThis->CurrentMission;

	// Busy in attacking or driver dead?
	if (pThis->Target || (Unsorted::CurrentFrame - pThis->unknown_int_120) < (RulesClass::Instance->GuardAreaTargetingDelay + 5) || (currentMission == Mission::Harmless && pThis->Owner == HouseClass::FindSpecial()))
	{
		if (canCheck && pExt->TypeExtData->UnitIdleRotateTurret.Get(RulesExt::Global()->UnitIdleRotateTurret))
			pExt->StopIdleAction();

		return SkipGameCode;
	}

	const auto pWeaponStruct = pThis->GetTurretWeapon();

	// Turret locked?
	if (pWeaponStruct && pWeaponStruct->WeaponType && pWeaponStruct->TurretLocked)
	{
		if (canCheck && pExt->TypeExtData->UnitIdleRotateTurret.Get(RulesExt::Global()->UnitIdleRotateTurret))
			pExt->StopIdleAction();

		if (!pThis->BunkerLinkedItem && pThis->Type->Speed && (!pThis->Type->IsSimpleDeployer || !pThis->Deployed))
			pThis->SecondaryFacing.SetDesired(pThis->PrimaryFacing.Current());

		return SkipGameCode;
	}

	// Point to mouse
	if (canCheck && SessionClass::IsSingleplayer() && pThis->Owner->IsControlledByCurrentPlayer())
	{
		if (pExt->TypeExtData->UnitIdlePointToMouse.Get(RulesExt::Global()->UnitIdlePointToMouse))
			pExt->ManualIdleAction();

		if (pExt->UnitIdleIsSelected)
			return SkipGameCode;
	}

	const auto pDestination = pThis->Destination;
	// Bugfix: Align jumpjet turret's facing with body's
	// When jumpjets arrived at their FootClass::Destination, they seems stuck at the Move mission
	// and therefore the turret facing was set to DirStruct{atan2(0,0)}==DirType::East at 0x736BBB
	// that's why they will come back to normal when giving stop command explicitly
	// so the best way is to fix the Mission if necessary, but I don't know how to do it
	// so I skipped jumpjets check temporarily
	if (pDestination && !locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
	{
		if (canCheck && pExt->TypeExtData->UnitIdleRotateTurret.Get(RulesExt::Global()->UnitIdleRotateTurret))
			pExt->StopIdleAction();

		if (!pThis->BunkerLinkedItem && pThis->Type->Speed && (!pThis->Type->IsSimpleDeployer || !pThis->Deployed))
			pThis->SecondaryFacing.SetDesired(pThis->GetTargetDirection(pDestination));

		return SkipGameCode;
	}

	// Idle main
	if (canCheck && pExt->TypeExtData->UnitIdleRotateTurret.Get(RulesExt::Global()->UnitIdleRotateTurret))
	{
		if (currentMission == Mission::Guard || currentMission == Mission::Sticky)
		{
			pExt->ApplyIdleAction();
			return SkipGameCode;
		}

		pExt->StopIdleAction();
	}

	if (!pThis->BunkerLinkedItem && pThis->Type->Speed && (!pThis->Type->IsSimpleDeployer || !pThis->Deployed))
		pThis->SecondaryFacing.SetDesired(pThis->PrimaryFacing.Current());

	return SkipGameCode;
}

#pragma endregion
