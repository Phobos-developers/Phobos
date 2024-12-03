#include <JumpjetLocomotionClass.h>
#include <UnitClass.h>
#include <BuildingClass.h>
#include <Utilities/Macro.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

// Misc jumpjet facing, turning, drawing fix -- Author: Trsdy
// Jumpjets stuck at FireError::FACING because Jumpjet has its own facing just for JumpjetTurnRate
// We should not touch the linked unit's PrimaryFacing when it's moving and just let the loco sync this shit in 54D692
// The body facing never actually turns, it just syncs
// Whatever, now let's totally forget PrimaryFacing and only use that loco facing
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

// Let the jumpjet increase their height earlier or simply skip the stop check
namespace JumpjetRushHelpers
{
	bool Skip = false;
	int GetJumpjetHeightWithOccupyTechno(CellClass* pCell); // Replace sub_485080
	int JumpjetLocomotionPredictHeight(JumpjetLocomotionClass* pThis); // Replace sub_54D820
}

int JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(CellClass* pCell)
{
	if (const auto pBuilding = pCell->GetBuilding())
	{
		auto dim2 = CoordStruct::Empty;
		pBuilding->Type->Dimension2(&dim2);
		return dim2.Z;
	}

	int height = 0;

	if (pCell->FindTechnoNearestTo(Point2D::Empty, false))
		height += 85; // Vanilla

	if (pCell->ContainsBridge())
		height += CellClass::BridgeHeight;

	return height;
}

int JumpjetRushHelpers::JumpjetLocomotionPredictHeight(JumpjetLocomotionClass* pThis)
{
	const auto pFoot = pThis->LinkedTo;
	const auto pLocation = &pFoot->Location;

	// Initial
	auto curCoord = Point2D { pLocation->X, pLocation->Y };
	auto pCurCell = MapClass::Instance->GetCellAt(CellStruct { static_cast<short>(curCoord.X >> 8), static_cast<short>(curCoord.Y >> 8) });
	auto maxHeight = pCurCell->GetFloorHeight(Point2D { curCoord.X & 0xFF, curCoord.Y & 0xFF }) + JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(pCurCell);

	// If is moving
	if (pThis->CurrentSpeed > 0.0)
	{
		constexpr int checkLength = Unsorted::LeptonsPerCell * 5;
		const auto angle = -pThis->LocomotionFacing.Current().GetRadian<65536>();
		const auto checkCoord = Point2D { static_cast<int>(checkLength * cos(angle) + 0.5), static_cast<int>(checkLength * sin(angle) + 0.5) };
		const auto largeStep = Math::max(abs(checkCoord.X), abs(checkCoord.Y));
		const auto checkSteps = (largeStep > Unsorted::LeptonsPerCell) ? (largeStep / Unsorted::LeptonsPerCell) : 1;
		const auto stepCoord = Point2D { (checkCoord.X / checkSteps), (checkCoord.Y / checkSteps) };

		// Check forward
		auto lastCoord = curCoord;
		curCoord += stepCoord;
		pCurCell = MapClass::Instance->GetCellAt(CellStruct { static_cast<short>(curCoord.X >> 8), static_cast<short>(curCoord.Y >> 8) });
		auto newHeight = pCurCell->GetFloorHeight(Point2D { curCoord.X & 0xFF, curCoord.Y & 0xFF }) + JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(pCurCell);

		if (newHeight > maxHeight)
			maxHeight = newHeight;

		// "Anti-Aliasing"
		if ((curCoord.X >> 8) != (lastCoord.X >> 8) && (curCoord.Y >> 8) != (lastCoord.Y >> 8))
		{
			bool lastX = (abs(stepCoord.X) > abs(stepCoord.Y)) ?
				(((curCoord.Y - static_cast<int>((static_cast<double>((stepCoord.X > 0) ?
					(curCoord.X & 0XFF) :
					((curCoord.X & 0XFF) - Unsorted::LeptonsPerCell)) *
				checkCoord.Y) / checkCoord.X + 0.5)) >> 8) == (curCoord.Y >> 8)) :
				(((curCoord.X - static_cast<int>((static_cast<double>((stepCoord.Y > 0) ?
					(curCoord.Y & 0XFF) :
					((curCoord.Y & 0XFF) - Unsorted::LeptonsPerCell)) *
				checkCoord.X) / checkCoord.Y + 0.5)) >> 8) != (curCoord.X >> 8));

			if (const auto pCheckCell = MapClass::Instance->TryGetCellAt(lastX ?
				CellStruct { static_cast<short>(lastCoord.X >> 8), static_cast<short>(curCoord.Y >> 8) } :
				CellStruct { static_cast<short>(curCoord.X >> 8), static_cast<short>(lastCoord.Y >> 8) }))
			{
				const auto checkHeight = (pCheckCell->Level * Unsorted::LevelHeight) + JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(pCheckCell);

				if (checkHeight > maxHeight)
					maxHeight = checkHeight;
			}
		}

		// The forward cell is not so high, keep moving
		if ((pLocation->Z - maxHeight) >= pFoot->GetTechnoType()->JumpjetHeight)
			JumpjetRushHelpers::Skip = true;

		// Check further
		for (int i = 1; i < checkSteps; ++i)
		{
			lastCoord = curCoord;
			curCoord += stepCoord;
			pCurCell = MapClass::Instance->TryGetCellAt(CellStruct { static_cast<short>(curCoord.X >> 8), static_cast<short>(curCoord.Y >> 8) });

			if (!pCurCell)
				return maxHeight;

			newHeight = pCurCell->GetFloorHeight(Point2D { curCoord.X & 0xFF, curCoord.Y & 0xFF }) + JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(pCurCell);

			if (newHeight > maxHeight)
				maxHeight = newHeight;

			// "Anti-Aliasing"
			if ((curCoord.X >> 8) != (lastCoord.X >> 8) && (curCoord.Y >> 8) != (lastCoord.Y >> 8))
			{
				bool lastX = (abs(stepCoord.X) > abs(stepCoord.Y)) ?
					(((curCoord.Y - static_cast<int>((static_cast<double>((stepCoord.X > 0) ?
						(curCoord.X & 0XFF) :
						((curCoord.X & 0XFF) - Unsorted::LeptonsPerCell)) *
					checkCoord.Y) / checkCoord.X + 0.5)) >> 8) == (curCoord.Y >> 8)) :
					(((curCoord.X - static_cast<int>((static_cast<double>((stepCoord.Y > 0) ?
						(curCoord.Y & 0XFF) :
						((curCoord.Y & 0XFF) - Unsorted::LeptonsPerCell)) *
					checkCoord.X) / checkCoord.Y + 0.5)) >> 8) != (curCoord.X >> 8));

				if (const auto pCheckCell = MapClass::Instance->TryGetCellAt(lastX ?
					CellStruct { static_cast<short>(lastCoord.X >> 8), static_cast<short>(curCoord.Y >> 8) } :
					CellStruct { static_cast<short>(curCoord.X >> 8), static_cast<short>(lastCoord.Y >> 8) }))
				{
					const auto checkHeight = (pCheckCell->Level * Unsorted::LevelHeight) + JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(pCheckCell);

					if (checkHeight > maxHeight)
						maxHeight = checkHeight;
				}
			}
		}
	}

	return maxHeight;
}

DEFINE_HOOK(0x54D827, JumpjetLocomotionClass_sub_54D820_PredictHeight, 0x8)
{
	enum { SkipVanillaCalculate = 0x54D928 };

	GET(JumpjetLocomotionClass*, pThis, ESI);

	if (!RulesExt::Global()->JumpjetClimbPredictHeight)
		return 0;

	R->EAX(JumpjetRushHelpers::JumpjetLocomotionPredictHeight(pThis));
	return SkipVanillaCalculate;
}

DEFINE_HOOK(0x54D4C0, JumpjetLocomotionClass_sub_54D0F0_NoStuck, 0x6)
{
	enum { SkipCheckStop = 0x54D52F };

	if (JumpjetRushHelpers::Skip)
		JumpjetRushHelpers::Skip = false;
	else if (!RulesExt::Global()->JumpjetClimbWithoutCutOut)
		return 0;

	return SkipCheckStop;
}
