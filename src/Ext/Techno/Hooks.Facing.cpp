#include "Body.h"

#include <JumpjetLocomotionClass.h>

#pragma region UnitsFacing

// Would it be better to rewrite the entire UpdateRotation() ?
DEFINE_HOOK(0x7369A5, UnitClass_UpdateRotation_CheckTurnToTarget, 0x6)
{
	enum { SkipGameCode = 0x736A8E, ContinueGameCode = 0x7369B3 };

	GET(UnitClass* const, pThis, ESI);

	if (!pThis->unknown_bool_6AF)
		return ContinueGameCode;

	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
	{
		if (pExt->UnitIdleActionTimer.IsTicking() || pExt->UnitIdleActionGapTimer.IsTicking() || pExt->UnitIdleIsSelected)
			return ContinueGameCode;
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x7369D6, UnitClass_UpdateRotation_StopUnitIdleAction, 0xA)
{
	enum { SkipGameCode = 0x736A8E };

	GET(UnitClass* const, pThis, ESI);
	GET_STACK(DirStruct, dir, STACK_OFFSET(0x10, -0x8));

	if (const auto pWeaponStruct = pThis->GetTurretWeapon())
	{
		if (const auto pWeapon = pWeaponStruct->WeaponType)
		{
			if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
				pExt->StopIdleAction();

			if (!pWeapon->OmniFire)
			{
				if (pWeaponStruct->TurretLocked)
					pThis->SecondaryFacing.SetDesired(pThis->PrimaryFacing.Current());
				else
					pThis->SecondaryFacing.SetDesired(dir);
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x736AFB, UnitClass_UpdateRotation_CheckTurnToForward, 0x6)
{
	enum { SkipGameCode = 0x736BE2, ContinueGameCode = 0x736B21 };

	GET(UnitClass* const, pThis, ESI);

	// Repeatedly check TurretSpins and IsRotating() seems unnecessary
	pThis->unknown_bool_6AF = true;

	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
	{
		if (pExt->UnitIdleActionTimer.IsTicking() || pExt->UnitIdleActionGapTimer.IsTicking() || pExt->UnitIdleIsSelected)
			return ContinueGameCode;
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x736B7E, UnitClass_UpdateRotation_ApplyUnitIdleAction, 0xA)
{
	enum { SkipGameCode = 0x736BE2 };

	GET(UnitClass* const, pThis, ESI);

	const auto pWeaponStruct = pThis->GetTurretWeapon();
	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto currentMission = pThis->CurrentMission;

	if ((pWeaponStruct && pWeaponStruct->WeaponType && pWeaponStruct->TurretLocked) || (currentMission == Mission::Harmless && pThis->Owner == HouseClass::FindSpecial()))
	{
		// Vanilla TurretLocked state and driver been killed state
		if (pExt)
			pExt->StopIdleAction();

		pThis->SecondaryFacing.SetDesired(pThis->PrimaryFacing.Current());
	}
	else
	{
		// Point to mouse
		if (pExt && pExt->UnitIdleActionSelected && pThis->Owner->IsControlledByCurrentPlayer())
			pExt->ManualIdleAction();

		if (!pExt->UnitIdleIsSelected)
		{
			// Bugfix: Align jumpjet turret's facing with body's
			// When jumpjets arrived at their FootClass::Destination, they seems stuck at the Move mission
			// and therefore the turret facing was set to DirStruct{atan2(0,0)}==DirType::East at 0x736BBB
			// that's why they will come back to normal when giving stop command explicitly
			// so the best way is to fix the Mission if necessary, but I don't know how to do it
			// so I skipped jumpjets check temporarily
			if (!pThis->Destination || locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
			{
				// Idle main
				if (pExt && pExt->UnitIdleAction && (currentMission == Mission::Guard || currentMission == Mission::Sticky))
					pExt->ApplyIdleAction();
				else if (pThis->Type->Speed) // What DisallowMoving used to skip
					pThis->SecondaryFacing.SetDesired(pThis->PrimaryFacing.Current());
			}
			else if (pThis->Type->Speed) // What DisallowMoving used to skip
			{
				// Turn to destination
				if (pExt)
					pExt->StopIdleAction();

				pThis->SecondaryFacing.SetDesired(pThis->GetTargetDirection(pThis->Destination));
			}
		}
	}

	return SkipGameCode;
}

#pragma endregion
