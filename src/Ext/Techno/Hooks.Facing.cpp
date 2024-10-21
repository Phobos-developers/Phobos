#include "Body.h"

#pragma region UnitsFacing

// Would it be better to rewrite the entire UpdateRotation() ?
DEFINE_HOOK(0x7369A5, UnitClass_UpdateRotation_CheckTurnToTarget, 0x6)
{
	enum { SkipGameCode = 0x736A8E, ContinueGameCode = 0x7369B3 };

	GET(UnitClass* const, pThis, ESI);

	if (!pThis->unknown_bool_6AF)
		return ContinueGameCode;

	if (TechnoExt::ExtData* const pExt = TechnoExt::ExtMap.Find(pThis))
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

	if (WeaponStruct* const pWeaponStruct = pThis->GetTurretWeapon())
	{
		if (WeaponTypeClass* const pWeapon = pWeaponStruct->WeaponType)
		{
			if (TechnoExt::ExtData* const pExt = TechnoExt::ExtMap.Find(pThis))
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

	// Repeatedly judging TurretSpins and IsRotating() is unnecessary
	pThis->unknown_bool_6AF = true;

	if (TechnoExt::ExtData* const pExt = TechnoExt::ExtMap.Find(pThis))
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

	WeaponStruct* const pWeaponStruct = pThis->GetTurretWeapon();
	TechnoExt::ExtData* const pExt = TechnoExt::ExtMap.Find(pThis);
	const Mission currentMission = pThis->CurrentMission;

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
			if (!pThis->Destination)
			{
				// Idle main
				if (pExt && pExt->UnitIdleAction && (currentMission == Mission::Guard || currentMission == Mission::Sticky))
					pExt->ApplyIdleAction();
				else
					pThis->SecondaryFacing.SetDesired(pThis->PrimaryFacing.Current());
			}
			else
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
