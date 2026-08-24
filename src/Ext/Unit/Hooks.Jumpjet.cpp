#pragma warning(disable: 4996)
#include <JumpjetLocomotionClass.h>
#include <AircraftTrackerClass.h>
#include <CellSpread.h>
#include <ScenarioClass.h>
#include <FootClass.h>
#include <InfantryClass.h>
#include <MapClass.h>
#include <Unsorted.h>
#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Cell/Body.h>
#include <Utilities/GeneralUtils.h>

#include <Ext/Foot/Body.h>
#include <Ext/UnitType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

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
	GET(const FireError, err, EBP);

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

	if (err == FireError::REARM && !UnitTypeExt::Fetch(pType)->NoTurret_TrackTarget.Get(RulesExt::Global()->NoTurret_TrackTarget))
		return 0;

	auto const pWpn = pThis->GetWeapon(wpIdx)->WeaponType;

	if (pWpn->OmniFire)
	{
		if (WeaponTypeExt::Fetch(pWpn)->OmniFire_TurnToTarget.Get(RulesExt::Global()->OmniFire_TurnToTarget) && !pThis->Locomotor->Is_Moving_Now())
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

static void __stdcall JumpjetLocomotionClass_DoTurn(ILocomotion* iloco, DirStruct dir)
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

	if (pThis->IsUnderEMP())
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
	auto const pTypeExt = TechnoExt::Fetch(pThis->LinkedTo)->TypeExtData;
	return pTypeExt->JumpjetRotateOnCrash.Get(RulesExt::Global()->JumpjetRotateOnCrash) ? 0 : 0x54CB3E;
}

// We no longer explicitly check TiltCrashJumpjet when drawing, do it when crashing
DEFINE_HOOK(0x70B649, TechnoClass_RigidBodyDynamics_NoTiltCrashBlyat, 0x6)
{
	GET(FootClass*, pThis, ESI);

	if (locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor) && !pThis->GetTechnoType()->TiltCrashJumpjet)
		return 0x70BCA4;

	return 0;
}

static FireError __stdcall JumpjetLocomotionClass_Can_Fire(ILocomotion* pThis)
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
	const auto pType = pLinkedTo->GetTechnoType();
	const auto pExt = FootExt::Fetch(pLinkedTo);

	pExt->JumpjetSpeed = pType->JumpjetSpeed;
	pLinkedTo->PrimaryFacing.SetROT(pType->ROT);

	if (pType->SensorsSight)
	{
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
static void __stdcall JumpjetLocomotionClass_Unlimbo(ILocomotion* pThis)
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

inline int JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(const CellClass* pCell)
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
	const bool ignoreOccupy = TechnoExt::Fetch(pFoot)->TypeExtData->JumpjetClimbIgnoreBuilding.Get(RulesExt::Global()->JumpjetClimbIgnoreBuilding);

	constexpr int shift = 8; // >> shift -> / Unsorted::LeptonsPerCell
	constexpr auto point2Cell = [](const Point2D& point) -> CellStruct
	{
		return CellStruct { static_cast<short>(point.X >> shift), static_cast<short>(point.Y >> shift) };
	};
	auto getJumpjetHeight = [ignoreOccupy](const CellClass* const pCell, const Point2D& point) -> int
	{
		return pCell->GetFloorHeight(Point2D { point.X, point.Y }) + (ignoreOccupy ? 0 : JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(pCell));
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

		auto getSideHeight = [ignoreOccupy](const CellClass* const pCell) -> int
		{
			return (pCell->Level * Unsorted::LevelHeight) + (ignoreOccupy ? 0 : JumpjetRushHelpers::GetJumpjetHeightWithOccupyTechno(pCell));
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

#pragma region JumpjetStraightAscend

// Skip adjusting max speed and rotation while ascending if flag is set.
DEFINE_HOOK(0x54BBD0, JumpjetLocomotionClass_Ascending_JumpjetStraightAscend, 0x6)
{
	enum { SkipGameCode = 0x54BC59 };

	GET(JumpjetLocomotionClass*, pThis, ESI);

	auto const pTechnoExt = FootExt::Fetch(pThis->LinkedTo);

	if (pTechnoExt->JumpjetStraightAscend)
		return SkipGameCode;

	return 0;
}

// Skip adjusting coords if flag is set, unit is alive, not crashing and is in JJ loco states 0-1.
// Unset flag in any other state.
DEFINE_HOOK(0x54D600, JumpjetLocomotionClass_MovementAI_JumpjetStraightAscend, 0x6)
{
	enum { SkipGameCode = 0x54D697 };

	GET(JumpjetLocomotionClass*, pThis, ESI);

	auto const pLinkedTo = pThis->LinkedTo;
	auto const pTechnoExt = FootExt::Fetch(pLinkedTo);

	if (pTechnoExt->JumpjetStraightAscend)
	{
		if (pLinkedTo->IsCrashing || pLinkedTo->Health < 1)
		{
			pTechnoExt->JumpjetStraightAscend = false;
			return 0;
		}

		if (pThis->State <= JumpjetLocomotionClass::State::Ascending)
			return SkipGameCode;
		else
			pTechnoExt->JumpjetStraightAscend = false;
	}

	return 0;
}

#pragma endregion


#pragma region JumpjetClimbIgnoreBuilding

namespace JumpjetClimbIgnoreBuilding
{
	bool Ignore = false;
	int Z = 0;
}

DEFINE_HOOK(0x54D820, JumpjetLocomotionClass_GetFloorZ_SetContext, 0x6)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);
	JumpjetClimbIgnoreBuilding::Ignore = TechnoExt::Fetch(pThis->LinkedTo)->TypeExtData->JumpjetClimbIgnoreBuilding.Get(RulesExt::Global()->JumpjetClimbIgnoreBuilding);

	if (JumpjetClimbIgnoreBuilding::Ignore)
		JumpjetClimbIgnoreBuilding::Z = MapClass::Instance.GetCellFloorHeight(pThis->LinkedTo->Location);

	return 0;
}

DEFINE_HOOK_AGAIN(0x54D8EA, JumpjetLocomotionClass_GetFloorZ_IgnoreBuilding, 0x6);
DEFINE_HOOK(0x54D859, JumpjetLocomotionClass_GetFloorZ_IgnoreBuilding, 0x9)
{
	if (JumpjetClimbIgnoreBuilding::Ignore)
		R->EAX(JumpjetClimbIgnoreBuilding::Z);

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x54AD41, JumpjetLocomotionClass_Link_To_Object_LocomotorWarhead, 0x8)
{
	enum { SkipGameCode = 0x54ADF8 };

	GET(ILocomotion*, pThis, EBP);
	GET(FootClass*, pLinkedTo, EBX);
	const auto pLoco = static_cast<JumpjetLocomotionClass*>(pThis);
	const auto pLinkedToExt = FootExt::Fetch(pLinkedTo);
	const auto pType = pLinkedTo->GetTechnoType();

	if (const auto pLocomotorWarhead = WarheadTypeExt::LocomotorWarhead)
	{
		const auto pWHExt = WarheadTypeExt::Fetch(pLocomotorWarhead);
		pLoco->TurnRate = pWHExt->JumpjetTurnRate.Get(pType->JumpjetTurnRate);
		pLoco->Speed = pLinkedToExt->JumpjetSpeed = pWHExt->JumpjetSpeed.Get(pType->JumpjetSpeed);
		pLoco->Climb = pWHExt->JumpjetClimb.Get(pType->JumpjetClimb);
		pLoco->Crash = pWHExt->JumpjetCrash.Get(pType->JumpjetCrash);
		pLoco->Height = std::max(pWHExt->JumpjetHeight.Get(pType->JumpjetHeight), Unsorted::CellHeight);
		pLoco->Accel = pWHExt->JumpjetAccel.Get(pType->JumpjetAccel);
		pLoco->Wobbles = pWHExt->JumpjetWobbles.Get(pType->JumpjetWobbles);
		pLoco->Deviation = pWHExt->JumpjetDeviation.Get(pType->JumpjetDeviation);
		pLoco->NoWobbles = pWHExt->JumpjetNoWobbles.Get(pType->JumpjetNoWobbles);
	}
	else
	{
		pLoco->TurnRate = pType->JumpjetTurnRate;
		pLoco->Speed = pLinkedToExt->JumpjetSpeed = pType->JumpjetSpeed;
		pLoco->Climb = pType->JumpjetClimb;
		pLoco->Crash = pType->JumpjetCrash;
		pLoco->Height = std::max(pType->JumpjetHeight, Unsorted::CellHeight);
		pLoco->Accel = pType->JumpjetAccel;
		pLoco->Wobbles = pType->JumpjetWobbles;
		pLoco->Deviation = pType->JumpjetDeviation;
		pLoco->NoWobbles = pType->JumpjetNoWobbles;
	}

	return SkipGameCode;
}

// ============================================================================
// ExtendedJumpjetHovering
// ============================================================================

static void ProcessJumpjetScatter(JumpjetLocomotionClass* pLoco, int dir)
{
	if (dir < 0 || dir > 7)
		return;

	double dist = static_cast<double>(pLoco->Speed);
	FootClass* pFoot = pLoco->LinkedTo;
	CoordStruct oldPos = pFoot->GetCoords();
	auto pTechnoExt = TechnoExt::Fetch(pFoot);
	CellStruct off = CellSpread::GetNeighbourOffset(dir);
	if (off.X && off.Y)
		dist /= Math::Sqrt2;

	CoordStruct newPos { oldPos.X + static_cast<int>(static_cast<double>(off.X) * dist),
	                     oldPos.Y + static_cast<int>(static_cast<double>(off.Y) * dist),
	                     oldPos.Z };

	AircraftTrackerClass::Instance.Update(pFoot, CellClass::Coord2Cell(oldPos), CellClass::Coord2Cell(newPos));
	bool onMap = pFoot->IsOnMap;
	pFoot->IsOnMap = false;
	pFoot->SetLocation(newPos);
	pFoot->IsOnMap = onMap;
	pLoco->DestinationCoords.X = newPos.X;
	pLoco->DestinationCoords.Y = newPos.Y;

	if (pTechnoExt->Jumpjet_ScatterFinishFrame - Unsorted::CurrentFrame <= 0)
		pTechnoExt->Jumpjet_ScatterFinishFrame = Unsorted::CurrentFrame + static_cast<int>(std::ceil(128.0 / dist));
}

static bool ShouldScatter(FootClass* pHov, int* outDir)
{
	bool should = false;
	int scatterDir = -1;
	std::vector<std::pair<int,int>> vec;

	CellClass* pCell = pHov->GetCell();
	int cellCount = 0;
	if (auto pCellExt = CellExt::TryFetch(pCell))
	{
		if (Unsorted::CurrentFrame - pCellExt->Jumpjet_LastScatterAffectedFrame > 2)
			return false;
		if (pCellExt->InAirJumpjets.size())
			cellCount += pCellExt->GetWeightedJumpjetCount(pHov, &should);
	}

	CellStruct cellPos = pCell->MapCoords;

	if (auto pTarget = pHov->Target)
	{
		int targetDir = pHov->GetTargetDirection(pTarget).GetFacing<8>();
		int d1 = (targetDir + 2) % 8;
		int d2 = (targetDir - 2 + 8) % 8;
		int c1 = 0, c2 = 0;
		CellStruct off1 = CellSpread::GetNeighbourOffset(d1);
		if (auto pC1 = MapClass::Instance.GetCellAt(cellPos + off1))
			if (auto pE1 = CellExt::TryFetch(pC1))
				if (pE1->InAirJumpjets.size())
					c1 += pE1->GetWeightedJumpjetCount(pHov, &should);
		CellStruct off2 = CellSpread::GetNeighbourOffset(d2);
		if (auto pC2 = MapClass::Instance.GetCellAt(cellPos + off2))
			if (auto pE2 = CellExt::TryFetch(pC2))
				if (pE2->InAirJumpjets.size())
					c2 += pE2->GetWeightedJumpjetCount(pHov, &should);

		int c3 = 27;
		int c4 = 27;
		int d3 = (targetDir + 1) % 8;
		int d4 = (targetDir - 1 + 8) % 8;
		vec = { {cellCount, -1}, {c1, d1}, {c2, d2}, {c3, d3}, {c4, d4} };
		const int maxPlus = 30;
		for (auto& pr : vec)
			pr.first = maxPlus - pr.first;
	}
	else
	{
		int counts[8] = {0};
		for (int i = 0; i < 8; ++i)
		{
			CellStruct off = CellSpread::GetNeighbourOffset(i);
			if (auto pC = MapClass::Instance.GetCellAt(cellPos + off))
				if (auto pE = CellExt::TryFetch(pC))
					if (pE->InAirJumpjets.size())
						counts[i] += pE->GetWeightedJumpjetCount(pHov, &should);
		}
		vec = { {cellCount, -1},
		        {counts[0],0},{counts[1],1},{counts[2],2},{counts[3],3},
		        {counts[4],4},{counts[5],5},{counts[6],6},{counts[7],7} };
		int maxVal = cellCount;
		for (int i = 0; i < 8; ++i) maxVal = std::max(maxVal, counts[i]);
		int maxPlus = 3 + maxVal;
		for (auto& pr : vec)
			pr.first = maxPlus - pr.first;
	}

	auto pExt = TechnoExt::Fetch(pHov);
	if (pExt->Jumpjet_ScatterFinishFrame - Unsorted::CurrentFrame <= 0)
	{
		// build weight list for GeneralUtils::ChooseOneWeighted
		std::vector<int> weights;
		weights.reserve(vec.size());
		for (auto& pr : vec) weights.push_back(pr.first);
		double dice = ScenarioClass::Instance->Random.RandomDouble();
		int idx = GeneralUtils::ChooseOneWeighted(dice, &weights);
		if (idx >= 0 && idx < static_cast<int>(vec.size()))
			scatterDir = vec[idx].second;
		else
			scatterDir = vec[0].second;
		pExt->Jumpjet_ScatterDir = scatterDir;
		*outDir = scatterDir;
	}
	else
	{
		*outDir = pExt->Jumpjet_ScatterDir;
	}
	return should;
}

DEFINE_HOOK(0x4135A0, AircraftTracker_JumpjetShouldScatter, 0x5)
{
	if (!RulesExt::Global()->ExtendedJumpjetHovering)
		return 0;
	R->EAX(false);
	return 0x4135C6;
}

DEFINE_HOOK(0x54BD93, JumpjetLocomotion_ProcessHovering_Scatter, 0x6)
{
	if (!RulesExt::Global()->ExtendedJumpjetHovering)
		return 0;
	GET(JumpjetLocomotionClass*, pLoco, ESI);
	int dir = -1;
	if (ShouldScatter(pLoco->LinkedTo, &dir))
		ProcessJumpjetScatter(pLoco, dir);
	else
	{
		FootClass* pFoot = pLoco->LinkedTo;
		auto pExt = TechnoExt::Fetch(pFoot);
		if (pExt->Jumpjet_LastCell && pExt->Jumpjet_LastCell != pFoot->Destination)
			pFoot->Destination = pExt->Jumpjet_LastCell;
	}
	return 0;
}

// Foot tracking for ExtendedJumpjetHovering
DEFINE_HOOK(0x4DB83F, FootClass_SetLocation_ExtendedJumpjet, 0x5)
{
	GET(bool, changed, EBX);
	if (!changed)
		return 0;
	GET(FootClass*, pFoot, ESI);
	GET_STACK(CoordStruct*, pTargetPos, STACK_OFFSET(0xC, 0x4));

	auto pTechnoExt = TechnoExt::Fetch(pFoot);
	CellClass* pOldCell = pTechnoExt->Jumpjet_LastCell;
	CellClass* pNewCell = MapClass::Instance.GetCellAt(*pTargetPos);
	auto pNewExt = CellExt::TryFetch(pNewCell);

	int bridgeAdj = pFoot->OnBridge ? -CellClass::BridgeHeight : 0;
	int oldHeight = pTechnoExt->Jumpjet_LastHeight;
	int newHeight = pTargetPos->Z - MapClass::Instance.GetCellFloorHeight(*pTargetPos) + bridgeAdj;
	pTechnoExt->Jumpjet_LastHeight = newHeight;

	if (pNewCell == pOldCell)
	{
		if (pNewExt)
		{
			pNewExt->UpdateJumpjet(pFoot, newHeight, oldHeight);
			if (pFoot->Location.X != pTargetPos->X || pFoot->Location.Y != pTargetPos->Y)
				pNewExt->MarkJumpjetScatterCell();
		}
	}
	else
	{
		pTechnoExt->Jumpjet_LastCell = pNewCell;
		if (auto pOldExt = CellExt::TryFetch(pOldCell))
			pOldExt->RemoveJumpjet(pFoot, oldHeight);
		if (pNewExt)
			pNewExt->AddJumpjet(pFoot, newHeight);
	}
	return 0;
}

DEFINE_HOOK(0x5F5FBB, ObjectClass_SetHeight_ExtendedJumpjet, 0x5)
{
	GET(FootClass*, pFoot, ESI);
	if ((pFoot->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
		return 0;
	GET(int, curHeight, EDI);
	auto pExt = TechnoExt::Fetch(pFoot);
	int oldHeight = pExt->Jumpjet_LastHeight;
	pExt->Jumpjet_LastHeight = curHeight;
	if (auto pCellExt = CellExt::TryFetch(pExt->Jumpjet_LastCell))
		pCellExt->UpdateJumpjet(pFoot, curHeight, oldHeight);
	return 0;
}

// ExtendedJumpjetHovering - Skip Jumpjet StopMoving when hovering
DEFINE_HOOK(0x54B4F2, JumpjetLocomotion_StopMoving_Skip, 0x9)
{
    GET(ILocomotion*, pILoco, EDI);

    if (!RulesExt::Global()->ExtendedJumpjetHovering)
        return 0;
	
    auto pLoco = static_cast<JumpjetLocomotionClass*>(pILoco);
    auto pFoot = pLoco->LinkedTo;
    auto crd = pFoot->GetCoords();
    pLoco->DestinationCoords.X = crd.X;
    pLoco->DestinationCoords.Y = crd.Y;
    pLoco->CurrentSpeed = 0;
    pLoco->MaxSpeed = 0;
    pLoco->State = JumpjetLocomotionClass::State::Hovering;
    pFoot->AbortMotion();
    return 0x54B6D6;
}


