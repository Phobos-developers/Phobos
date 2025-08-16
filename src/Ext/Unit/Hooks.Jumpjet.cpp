#include <JumpjetLocomotionClass.h>
#include <UnitClass.h>
#include <BuildingClass.h>

#include <Utilities/Macro.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

// Misc jumpjet facing, turning, drawing fix -- Author: Trsdy
// Jumpjets stuck at FireError::FACING because Jumpjet has its own facing just for JumpjetTurnRate
// We should not touch the linked unit's PrimaryFacing when it's moving and just let the loco sync this shit in 54D692
// The body facing never actually turns, it just syncs
// Whatever, now let's totally forget PrimaryFacing and only use that loco facing
DEFINE_HOOK(0x736F78, UnitClass_UpdateFiring_FireErrorIsFACING, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

	const auto pType = pThis->Type;
	CoordStruct& source = pThis->Location;
	const CoordStruct target = pThis->Target->GetCoords(); // Target checked so it's not null here
	const DirStruct tgtDir { Math::atan2(source.Y - target.Y, target.X - source.X) };

	if (pType->Turret && !pType->HasTurret) // 0x736F92
	{
		pThis->SecondaryFacing.SetDesired(tgtDir);
	}
	else // 0x736FB6
	{
		if (const auto jjLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
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
DEFINE_HOOK(0x736E6E, UnitClass_UpdateFiring_OmniFireTurnToTarget, 0x9)
{
	GET(FireError, err, EBP);

	if (err != FireError::OK && err != FireError::REARM)
		return 0;

	GET(UnitClass* const, pThis, ESI);

	if (pThis->IsWarpingIn())
		return 0;

	auto const pType = pThis->Type;

	if ((pType->Turret && !pType->HasTurret) || pType->TurretSpins)
		return 0;

	GET(int const, wpIdx, EDI);

	if ((pType->DeployFire || pType->DeployFireWeapon == wpIdx) && pThis->CurrentMission == Mission::Unload)
		return 0;

	if (err == FireError::REARM && !TechnoTypeExt::ExtMap.Find(pType)->NoTurret_TrackTarget.Get(RulesExt::Global()->NoTurret_TrackTarget))
		return 0;

	auto const pWpn = pThis->GetWeapon(wpIdx)->WeaponType;

	if (pWpn->OmniFire)
	{
		if (WeaponTypeExt::ExtMap.Find(pWpn)->OmniFire_TurnToTarget.Get() && !pThis->Locomotor->Is_Moving_Now())
		{
			CoordStruct& source = pThis->Location;
			const CoordStruct target = pThis->Target->GetCoords();
			const DirStruct tgtDir { Math::atan2(source.Y - target.Y, target.X - source.X) };

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
	const auto pThis = static_cast<JumpjetLocomotionClass*>(iloco);
	pThis->LocomotionFacing.SetDesired(dir);
	pThis->LinkedTo->PrimaryFacing.SetDesired(dir);
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7ECDB4, JumpjetLocomotionClass_DoTurn)

DEFINE_HOOK(0x54D326, JumpjetLocomotionClass_MovementAI_CrashSpeedFix, 0x6)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);
	return pThis->LinkedTo->IsCrashing ? 0x54D350 : 0;
}

DEFINE_HOOK(0x54D208, JumpjetLocomotionClass_MovementAI_EMPWobble, 0x5)
{
	GET(JumpjetLocomotionClass* const, pThis, ESI);
	enum { ZeroWobble = 0x54D22C };

	if (pThis->LinkedTo->Deactivated || pThis->LinkedTo->IsUnderEMP())
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

DEFINE_HOOK(0x54CB0E, JumpjetLocomotionClass_State5_CrashSpin, 0x7)
{
	GET(JumpjetLocomotionClass*, pThis, EDI);
	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis->LinkedTo)->TypeExtData;
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

DEFINE_FUNCTION_JUMP(VTABLE, 0x7ECDF4, JumpjetLocomotionClass_Can_Fire);

DEFINE_HOOK(0x54DAC4, JumpjetLocomotionClass_EndPiggyback_Blyat, 0x6)
{
	GET(FootClass*, pLinkedTo, EAX);
	auto const* pType = pLinkedTo->GetTechnoType();

	pLinkedTo->PrimaryFacing.SetROT(pType->ROT);

	if (pType->SensorsSight)
	{
		const auto pExt = TechnoExt::ExtMap.Find(pLinkedTo);
		pLinkedTo->RemoveSensorsAt(pExt->LastSensorsMapCoords);
		pLinkedTo->AddSensorsAt(CellStruct::Empty);
	}

	return 0;
}

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

DEFINE_FUNCTION_JUMP(VTABLE, 0x7ECDB8, JumpjetLocomotionClass_Unlimbo)

// Let the jumpjet increase their height earlier or simply skip the stop check
namespace JumpjetRushHelpers
{
	bool Skip = false;
	int GetJumpjetHeightWithOccupyTechno(const CellClass* pCell); // Replace sub_485080
	int JumpjetLocomotionPredictHeight(JumpjetLocomotionClass* pThis); // Replace sub_54D820
}

int JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(const CellClass* pCell)
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

	constexpr int shift = 8; // >> shift -> / Unsorted::LeptonsPerCell
	constexpr auto point2Cell = [](const Point2D& point) -> CellStruct
	{
		return CellStruct { static_cast<short>(point.X >> shift), static_cast<short>(point.Y >> shift) };
	};
	auto getJumpjetHeight = [](const CellClass* const pCell, const Point2D& point) -> int
	{
		return pCell->GetFloorHeight(Point2D { point.X, point.Y }) + JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(pCell);
	};

	// Initialize
	auto curCoord = Point2D { pLocation->X, pLocation->Y };
	const CellClass* pCurCell = MapClass::Instance.GetCellAt(point2Cell(curCoord));
	int maxHeight = getJumpjetHeight(pCurCell, curCoord);

	// If is moving
	if (pThis->CurrentSpeed > 0.0)
	{
		// Prepare for prediction
		auto lastCoord = Point2D::Empty;
		const int checkLength = (pThis->LocomotionFacing.IsRotating() || !pFoot->Destination)
			? Unsorted::LeptonsPerCell
			: Math::min((Unsorted::LeptonsPerCell * 5), pFoot->DistanceFrom(pFoot->Destination)); // Predict the distance of 5 cells ahead
		const double angle = -pThis->LocomotionFacing.Current().GetRadian<65536>();
		const auto checkCoord = Point2D { static_cast<int>(checkLength * Math::cos(angle) + 0.5), static_cast<int>(checkLength * Math::sin(angle) + 0.5) };
		const int largeStep = Math::max(std::abs(checkCoord.X), std::abs(checkCoord.Y));
		const int checkSteps = (largeStep > Unsorted::LeptonsPerCell) ? (largeStep / Unsorted::LeptonsPerCell + 1) : 1;
		const auto stepCoord = Point2D { (checkCoord.X / checkSteps), (checkCoord.Y / checkSteps) };

		auto getSideHeight = [](const CellClass* const pCell) -> int
		{
			return (pCell->Level * Unsorted::LevelHeight) + JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(pCell);
		};
		auto getAntiAliasingCell = [&stepCoord, &checkCoord](const Point2D& curCoord, const Point2D& lastCoord) -> CellClass*
		{
			// Check if it is a diagonal relationship
			if ((curCoord.X >> shift) == (lastCoord.X >> shift) || (curCoord.Y >> shift) == (lastCoord.Y >> shift))
				return nullptr;

			constexpr int mask = 0xFF; // & mask -> % Unsorted::LeptonsPerCell
			bool lastX = false;

			// Calculate the bias of the previous cell
			if (std::abs(stepCoord.X) > std::abs(stepCoord.Y))
			{
				const int offsetX = curCoord.X & mask;
				const int deltaX = (stepCoord.X > 0) ? offsetX : (offsetX - Unsorted::LeptonsPerCell);
				const int projectedY = curCoord.Y - deltaX * checkCoord.Y / checkCoord.X;
				lastX = (projectedY ^ curCoord.Y) >> shift == 0;
			}
			else
			{
				const int offsetY = curCoord.Y & mask;
				const int deltaY = (stepCoord.Y > 0) ? offsetY : (offsetY - Unsorted::LeptonsPerCell);
				const int projectedX = curCoord.X - deltaY * checkCoord.X / checkCoord.Y;
				lastX = (projectedX ^ curCoord.X) >> shift != 0;
			}

			// Get cell
			return MapClass::Instance.TryGetCellAt(lastX
				? CellStruct { static_cast<short>(lastCoord.X >> shift), static_cast<short>(curCoord.Y >> shift) }
				: CellStruct { static_cast<short>(curCoord.X >> shift), static_cast<short>(lastCoord.Y >> shift) });
		};
		auto checkStepHeight = [&maxHeight, &curCoord, &lastCoord, &pCurCell, &stepCoord,
			&getJumpjetHeight, &getAntiAliasingCell, &getSideHeight]() -> bool
		{
			// Check forward
			lastCoord = curCoord;
			curCoord += stepCoord;
			pCurCell = MapClass::Instance.TryGetCellAt(point2Cell(curCoord));

			if (!pCurCell)
				return false;

			maxHeight = Math::max(maxHeight, getJumpjetHeight(pCurCell, curCoord));

			// "Anti-Aliasing"
			if (const auto pCheckCell = getAntiAliasingCell(curCoord, lastCoord))
				maxHeight = Math::max(maxHeight, getSideHeight(pCheckCell));

			return true;
		};

		// Predict height
		if (checkStepHeight())
		{
			// The forward cell is not so high, keep moving
			if ((pLocation->Z - maxHeight) >= pFoot->GetTechnoType()->JumpjetHeight)
				JumpjetRushHelpers::Skip = true;

			// Check further
			for (int i = 1; i < checkSteps && checkStepHeight(); ++i);
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
