#include <JumpjetLocomotionClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

// Bugfix: Jumpjet turn to target when attacking

// Jumpjets stuck at FireError::FACING because WW didn't use a correct facing
DEFINE_HOOK(0x736F78, UnitClass_UpdateFiring_FireErrorIsFACING, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

	auto pType = pThis->Type;
	CoordStruct& source = pThis->Location;
	CoordStruct target = pThis->Target->GetCoords(); // Target checked so it's not null here
	DirStruct tgtDir { Math::atan2(source.Y - target.Y, target.X - source.X) };

	if (pType->Turret && !pType->HasTurret) // 0x736F92
	{
		pThis->SecondaryFacing.SetDesired(tgtDir);
	}
	else // 0x736FB6
	{
		if (auto jjLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
		{
			//wrong destination check and wrong Is_Moving usage for jumpjets, should have used Is_Moving_Now
			if (jjLoco->State != JumpjetLocomotionClass::State::Cruising)
			{
				jjLoco->LocomotionFacing.SetDesired(tgtDir);
				pThis->PrimaryFacing.SetDesired(tgtDir);
				pThis->SecondaryFacing.SetDesired(tgtDir);
			}
		}
		else if (!pThis->Destination && !pThis->Locomotor->Is_Moving())
		{
			pThis->PrimaryFacing.SetDesired(tgtDir);
			pThis->SecondaryFacing.SetDesired(tgtDir);
		}
	}

	return 0x736FB1;
}

// For compatibility with previous builds
DEFINE_HOOK(0x736EE9, UnitClass_UpdateFiring_FireErrorIsOK, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	GET(int const, wpIdx, EDI);
	auto pType = pThis->Type;

	if ((pType->Turret && !pType->HasTurret) || pType->TurretSpins)
		return 0;

	if ((pType->DeployFire || pType->DeployFireWeapon == wpIdx) && pThis->CurrentMission == Mission::Unload)
		return 0;

	auto const pWpn = pThis->GetWeapon(wpIdx)->WeaponType;
	if (pWpn->OmniFire)
	{
		const auto pTypeExt = WeaponTypeExt::ExtMap.Find(pWpn);
		if (pTypeExt->OmniFire_TurnToTarget.Get() && !pThis->Locomotor->Is_Moving_Now())
		{
			CoordStruct& source = pThis->Location;
			CoordStruct target = pThis->Target->GetCoords();
			DirStruct tgtDir { Math::atan2(source.Y - target.Y, target.X - source.X) };

			if (pThis->GetRealFacing() != tgtDir)
			{
				if (auto const pLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
					pLoco->LocomotionFacing.SetDesired(tgtDir);
				else
					pThis->PrimaryFacing.SetDesired(tgtDir);
			}
		}
	}

	return 0;
}

// Bugfix: Align jumpjet turret's facing with body's
DEFINE_HOOK(0x736BA3, UnitClass_UpdateRotation_TurretFacing_TemporaryFix, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	enum { SkipCheckDestination = 0x736BCA, GetDirectionTowardsDestination = 0x736BBB };
	// When jumpjets arrived at their FootClass::Destination, they seems stuck at the Move mission
	// and therefore the turret facing was set to DirStruct{atan2(0,0)}==DirType::East at 0x736BBB
	// that's why they will come back to normal when giving stop command explicitly
	auto pType = pThis->Type;
	// so the best way is to fix the Mission if necessary, but I don't know how to do it
	// so I skipped jumpjets check temporarily, and in most cases Jumpjet/BallonHover should cover most of it
	if (!pType->TurretSpins && (pType->JumpJet || pType->BalloonHover))
		return SkipCheckDestination;

	return 0;
}

// Bugfix: Jumpjet detect cloaked objects beneath
DEFINE_HOOK(0x54C036, JumpjetLocomotionClass_State3_UpdateSensors, 0x7)
{
	GET(FootClass* const, pLinkedTo, ECX);
	GET(CellStruct const, currentCell, EAX);

	// Copied from FootClass::UpdatePosition
	if (pLinkedTo->GetTechnoType()->SensorsSight)
	{
		CellStruct const lastCell = pLinkedTo->LastFlightMapCoords;

		if (lastCell != currentCell)
		{
			pLinkedTo->RemoveSensorsAt(lastCell);
			pLinkedTo->AddSensorsAt(currentCell);
		}
	}
	// Something more may be missing

	return 0;
}

DEFINE_HOOK(0x54CB0E, JumpjetLocomotionClass_State5_CrashSpin, 0x7)
{
	GET(JumpjetLocomotionClass*, pThis, EDI);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->LinkedTo->GetTechnoType());
	return pTypeExt->JumpjetRotateOnCrash ? 0 : 0x54CB3E;
}


// These are subject to changes if someone wants to properly implement jumpjet tilting
DEFINE_HOOK(0x54DCCF, JumpjetLocomotionClass_DrawMatrix_TiltCrashJumpjet, 0x5)
{
	GET(ILocomotion*, iloco, ESI);
	//if (static_cast<JumpjetLocomotionClass*>(iloco)->State < JumpjetLocomotionClass::State::Crashing)
	if (static_cast<JumpjetLocomotionClass*>(iloco)->State == JumpjetLocomotionClass::State::Grounded)
		return 0x54DCE8;

	return 0;
}

/*
DEFINE_HOOK(0x54DD3D, JumpjetLocomotionClass_DrawMatrix_AxisCenterInAir, 0x5)
{
	GET(ILocomotion*, iloco, ESI);
	auto state = static_cast<JumpjetLocomotionClass*>(iloco)->State;
	if (state && state < JumpjetLocomotionClass::State::Crashing)
		return  0x54DE88;
	return 0;
}
*/

FireError __stdcall JumpjetLocomotionClass_Can_Fire(ILocomotion* pThis)
{
	// do not use explicit toggle for this
	if (static_cast<JumpjetLocomotionClass*>(pThis)->State == JumpjetLocomotionClass::State::Crashing)
		return FireError::CANT;
	return FireError::OK;
}

DEFINE_JUMP(VTABLE, 0x7ECDF4, GET_OFFSET(JumpjetLocomotionClass_Can_Fire));

//TODO : Issue #690 #655

// Fix initial facing when jumpjet locomotor is being attached
DEFINE_HOOK(0x54AE44, JumpjetLocomotionClass_LinkToObject_FixFacing, 0x7)
{
	GET(ILocomotion*, iLoco, EBP);
	auto const pThis = static_cast<JumpjetLocomotionClass*>(iLoco);

	pThis->LocomotionFacing.SetCurrent(pThis->LinkedTo->PrimaryFacing.Current());
	pThis->LocomotionFacing.SetDesired(pThis->LinkedTo->PrimaryFacing.Desired());

	return 0;
}

// Fix initial facing when jumpjet locomotor on unlimbo
void __stdcall JumpjetLocomotionClass_Unlimbo(ILocomotion* pThis)
{
	auto const pThisLoco = static_cast<JumpjetLocomotionClass*>(pThis);

	pThisLoco->LocomotionFacing.SetCurrent(pThisLoco->LinkedTo->PrimaryFacing.Current());
	pThisLoco->LocomotionFacing.SetDesired(pThisLoco->LinkedTo->PrimaryFacing.Desired());
}

DEFINE_JUMP(VTABLE, 0x7ECDB8, GET_OFFSET(JumpjetLocomotionClass_Unlimbo))
