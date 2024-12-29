#include "Body.h"

#include <JumpjetLocomotionClass.h>
#include <BulletTypeClass.h>

#include <Ext/WeaponType/Body.h>

#pragma region UnitsFacing

// Would it be better to rewrite the entire UpdateRotation() ?
DEFINE_HOOK(0x7369D6, UnitClass_UpdateRotation_StopUnitIdleAction, 0xA)
{
	enum { SkipGameCode = 0x736A8E };

	GET(UnitClass* const, pThis, ESI);

	if (!RulesExt::Global()->ExpandTurretRotation)
		return 0;

	if (const auto pWeaponStruct = pThis->GetTurretWeapon())
	{
		const auto pWeapon = pWeaponStruct->WeaponType;
		const auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeapon);

		if (pWeapon && (!pWeapon->OmniFire || (pWeaponTypeExt && pWeaponTypeExt->OmniFire_TurnToTarget)))
		{
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

			if (pWeaponStruct->TurretLocked)
			{
				pTypeExt->SetTurretLimitedDir(pThis, pThis->PrimaryFacing.Current());
			}
			else
			{
				const auto targetDir = pThis->GetTargetDirection(pThis->Target);
				pTypeExt->SetTurretLimitedDir(pThis, targetDir);

				if (pTypeExt->Turret_BodyOrientation && !pThis->Locomotor->Is_Moving_Now())
				{
					const auto curDir = pThis->PrimaryFacing.Current();
					const auto tgtDir = pTypeExt->GetBodyDesiredDir(curDir, targetDir);

					if (abs(static_cast<short>(static_cast<short>(tgtDir.Raw) - static_cast<short>(curDir.Raw))) >= 8192)
						pThis->PrimaryFacing.SetDesired(tgtDir);
				}
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x736AEA, UnitClass_UpdateRotation_ApplyUnitIdleAction, 0x6)
{
	enum { SkipGameCode = 0x736BE2 };

	GET(UnitClass* const, pThis, ESI);

	if (!RulesExt::Global()->ExpandTurretRotation)
		return 0;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	// Turning to target?
	if (pThis->SecondaryFacing.IsRotating())
	{
		// Repeatedly check TurretSpins and IsRotating() seems unnecessary
		pThis->unknown_bool_6AF = true;

		if (!pExt->UnitIdleActionTimer.IsTicking() && !pExt->UnitIdleActionGapTimer.IsTicking() && !pExt->UnitIdleIsSelected)
			return SkipGameCode;
	}

	const auto pTypeExt = pExt->TypeExtData;
	const auto currentMission = pThis->CurrentMission;

	// Busy in attacking or driver dead?
	if (pThis->Target || (Unsorted::CurrentFrame - pThis->LastFireBulletFrame) < (RulesClass::Instance->GuardAreaTargetingDelay + 5) || (currentMission == Mission::Harmless && pThis->Owner == HouseClass::FindSpecial()))
	{
		if (pTypeExt->Turret_IdleRotate.Get(RulesExt::Global()->Turret_IdleRotate))
			pExt->StopIdleAction();

		return SkipGameCode;
	}

	const auto pWeaponStruct = pThis->GetTurretWeapon();

	// Turret locked?
	if (pWeaponStruct && pWeaponStruct->WeaponType && pWeaponStruct->TurretLocked)
	{
		if (pTypeExt->Turret_IdleRotate.Get(RulesExt::Global()->Turret_IdleRotate))
			pExt->StopIdleAction();

		if (!pThis->BunkerLinkedItem && pThis->Type->Speed && (!pThis->Type->IsSimpleDeployer || !pThis->Deployed))
			pTypeExt->SetTurretLimitedDir(pThis, pThis->PrimaryFacing.Current());

		return SkipGameCode;
	}

	// Point to mouse
	if (SessionClass::IsSingleplayer() && pThis->Owner->IsControlledByCurrentPlayer())
	{
		if (pTypeExt->Turret_PointToMouse.Get(RulesExt::Global()->Turret_PointToMouse))
			pExt->ManualIdleAction();

		if (pExt->UnitIdleIsSelected)
			return SkipGameCode;
	}

	const auto pDestination = pThis->Destination;
	const auto pJumpjetLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor);

	if (pDestination && !pJumpjetLoco)
	{
		if (pTypeExt->Turret_IdleRotate.Get(RulesExt::Global()->Turret_IdleRotate))
			pExt->StopIdleAction();

		if (!pThis->BunkerLinkedItem && pThis->Type->Speed && (!pThis->Type->IsSimpleDeployer || !pThis->Deployed))
			pTypeExt->SetTurretLimitedDir(pThis, pThis->GetTargetDirection(pDestination));

		return SkipGameCode;
	}

	// Idle main
	if (pTypeExt->Turret_IdleRotate.Get(RulesExt::Global()->Turret_IdleRotate))
	{
		if (currentMission == Mission::Guard || (pJumpjetLoco && pJumpjetLoco->State == JumpjetLocomotionClass::State::Hovering))
		{
			pExt->ApplyIdleAction();
			return SkipGameCode;
		}

		pExt->StopIdleAction();
	}

	if (!pThis->BunkerLinkedItem && pThis->Type->Speed && (!pThis->Type->IsSimpleDeployer || !pThis->Deployed))
		pTypeExt->SetTurretLimitedDir(pThis, pThis->PrimaryFacing.Current());

	return SkipGameCode;
}

#pragma endregion

#pragma region CheckFacing

DEFINE_HOOK(0x7412BB, UnitClass_GetFireError_CheckFacingDeviation, 0x7)
{
	enum { SkipGameCode = 0x7412D4 };

	GET(UnitClass* const, pThis, ESI);
	GET(AbstractClass* const, pTarget, EBP);
	GET(BulletTypeClass* const, pBulletType, EDX);
	GET(DirStruct* const, pTgtDir, EAX);

	*pTgtDir = pThis->GetTargetDirection(pTarget);

	if (RulesExt::Global()->ExpandTurretRotation && pThis->Type->Turret)
		*pTgtDir = TechnoTypeExt::ExtMap.Find(pThis->Type)->GetTurretDesiredDir(*pTgtDir);

	R->EBX(pBulletType->ROT ? 16 : 8);
	return SkipGameCode;
}

#pragma endregion
