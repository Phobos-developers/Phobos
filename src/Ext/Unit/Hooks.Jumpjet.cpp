#include <JumpjetLocomotionClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

// Misc jumpjet facing, turning, drawing fix -- Author: Trsdy
// Jumpjets stuck at FireError::FACING because Jumpjet has its own facing just for JumpjetTurnRate
// We should not touch the linked unit's PrimaryFacing when it's moving and just let the loco sync this shit in 54D692
// The body facing never actually turns, it just syncs
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
				if (jjLoco->State == JumpjetLocomotionClass::State::Grounded)
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

void __stdcall JumpjetLocomotionClass_DoTurn(ILocomotion* iloco, DirStruct dir)
{
	__assume(iloco != nullptr);
	// This seems to be used only when unloading shit on the ground
	// Rewrite just in case
	auto pThis = static_cast<JumpjetLocomotionClass*>(iloco);
	pThis->LocomotionFacing.SetDesired(dir);
	pThis->LinkedTo->PrimaryFacing.SetDesired(dir);
}
DEFINE_JUMP(VTABLE, 0x7ECDB4, GET_OFFSET(JumpjetLocomotionClass_DoTurn))

DEFINE_HOOK(0x54D326, JumpjetLocomotionClass_MovementAI_CrashSpeedFix, 0x6)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);
	return pThis->LinkedTo->IsCrashing ? 0x54D350 : 0;
}

DEFINE_HOOK(0x54D208, JumpjetLocomotionClass_MovementAI_EMPWobble, 0x5)
{
	GET(JumpjetLocomotionClass* const, pThis, ESI);
	enum { ZeroWobble = 0x54D22C };

	if (pThis->LinkedTo->IsUnderEMP())
		return ZeroWobble;

	return 0;
}

DEFINE_HOOK(0x736990, UnitClass_UpdateRotation_TurretFacing_EMP, 0x6)
{
	GET(UnitClass* const, pThis, ECX);
	enum { SkipAll = 0x736C0E };

	if (pThis->Deactivated || pThis->IsUnderEMP())
		return SkipAll;

	return 0;
}

// Bugfix: Align jumpjet turret's facing with body's
DEFINE_HOOK(0x736BA3, UnitClass_UpdateRotation_TurretFacing_Jumpjet, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	enum { SkipCheckDestination = 0x736BCA, GetDirectionTowardsDestination = 0x736BBB };
	// When jumpjets arrived at their FootClass::Destination, they seems stuck at the Move mission
	// and therefore the turret facing was set to DirStruct{atan2(0,0)}==DirType::East at 0x736BBB
	// that's why they will come back to normal when giving stop command explicitly
	// so the best way is to fix the Mission if necessary, but I don't know how to do it
	// so I skipped jumpjets check temporarily
	if (!pThis->Type->TurretSpins && locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
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

// We no longer explicitly check TiltCrashJumpjet when drawing, do it when crashing
DEFINE_HOOK(0x70B649, TechnoClass_RigidBodyDynamics_NoTiltCrashBlyat, 0x6)
{
	GET(FootClass*, pThis, ESI);

	if (locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor) && !pThis->GetTechnoType()->TiltCrashJumpjet)
		return 0x70BCA4;

	return 0;
}

// Just rewrite this completely to avoid headache
Matrix3D* __stdcall JumpjetLocomotionClass_Draw_Matrix(ILocomotion* iloco, Matrix3D* ret, VoxelIndexKey* pIndex)
{
	__assume(iloco != nullptr);
	auto const pThis = static_cast<JumpjetLocomotionClass*>(iloco);
	auto linked = pThis->LinkedTo;
	// no more TiltCrashJumpjet, do that above svp
	bool const onGround = pThis->State == JumpjetLocomotionClass::State::Grounded;
	// Man, what can I say, you don't want to stick your rotor into the ground
	auto slope_idx = MapClass::Instance->GetCellAt(linked->Location)->SlopeIndex;

	if (onGround && pIndex && pIndex->Is_Valid_Key())
		*(int*)(pIndex) = slope_idx + (*(int*)(pIndex) << 6);

	*ret = Matrix3D::VoxelRampMatrix[onGround ? slope_idx : 0] * pThis->LocomotionClass::Draw_Matrix(pIndex);

	float arf = linked->AngleRotatedForwards;
	float ars = linked->AngleRotatedSideways;

	if (std::abs(ars) >= 0.005 || std::abs(arf) >= 0.005)
	{
		if (pIndex)
			pIndex->Invalidate();

		if (onGround)
		{
			double scalex = linked->GetTechnoType()->VoxelScaleX;
			double scaley = linked->GetTechnoType()->VoxelScaleY;
			Matrix3D pre = Matrix3D::GetIdentity();
			pre.TranslateZ(float(std::abs(Math::sin(ars)) * scalex + std::abs(Math::sin(arf)) * scaley));
			ret->TranslateX(float(Math::sgn(arf) * (scaley * (1 - Math::cos(arf)))));
			ret->TranslateY(float(Math::sgn(-ars) * (scalex * (1 - Math::cos(ars)))));
			ret->RotateX(ars);
			ret->RotateY(arf);
			*ret = pre * *ret;
		}
		else
		{
			// No more translation because I don't like it
			ret->RotateX(ars);
			ret->RotateY(arf);
		}
	}
	return ret;
}

DEFINE_JUMP(VTABLE, 0x7ECD8C, GET_OFFSET(JumpjetLocomotionClass_Draw_Matrix));

FireError __stdcall JumpjetLocomotionClass_Can_Fire(ILocomotion* pThis)
{
	__assume(pThis != nullptr);
	// do not use explicit toggle for this
	if (static_cast<JumpjetLocomotionClass*>(pThis)->State == JumpjetLocomotionClass::State::Crashing)
		return FireError::CANT;
	return FireError::OK;
}

DEFINE_JUMP(VTABLE, 0x7ECDF4, GET_OFFSET(JumpjetLocomotionClass_Can_Fire));

// Fix initial facing when jumpjet locomotor is being attached
DEFINE_HOOK(0x54AE44, JumpjetLocomotionClass_LinkToObject_FixFacing, 0x7)
{
	GET(ILocomotion*, iLoco, EBP);
	__assume(iLoco != nullptr);
	auto const pThis = static_cast<JumpjetLocomotionClass*>(iLoco);

	pThis->LocomotionFacing.SetCurrent(pThis->LinkedTo->PrimaryFacing.Current());
	pThis->LocomotionFacing.SetDesired(pThis->LinkedTo->PrimaryFacing.Desired());
	pThis->LinkedTo->PrimaryFacing.SetROT(pThis->TurnRate);

	return 0;
}

// Fix initial facing when jumpjet locomotor on unlimbo
void __stdcall JumpjetLocomotionClass_Unlimbo(ILocomotion* pThis)
{
	__assume(pThis != nullptr);
	auto const pThisLoco = static_cast<JumpjetLocomotionClass*>(pThis);

	pThisLoco->LocomotionFacing.SetCurrent(pThisLoco->LinkedTo->PrimaryFacing.Current());
	pThisLoco->LocomotionFacing.SetDesired(pThisLoco->LinkedTo->PrimaryFacing.Desired());
}

DEFINE_JUMP(VTABLE, 0x7ECDB8, GET_OFFSET(JumpjetLocomotionClass_Unlimbo))

DEFINE_HOOK(0x54DAC4, JumpjetLocomotionClass_EndPiggyback_Blyat, 0x6)
{
	GET(FootClass*, pLinked, EAX);
	auto const* pType = pLinked->GetTechnoType();

	pLinked->PrimaryFacing.SetROT(pType->ROT);

	if (pType->SensorsSight)
	{
		pLinked->RemoveSensorsAt(pLinked->LastFlightMapCoords);
		pLinked->RemoveSensorsAt(pLinked->GetMapCoords());
		pLinked->AddSensorsAt(pLinked->GetMapCoords());
	}

	return 0;
}
