#include <JumpjetLocomotionClass.h>
#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x54B8E9, JumpjetLocomotionClass_In_Which_Layer_Deviation, 0x6)
{
	GET(FootClass* const, pThis, EAX);

	if (pThis->IsInAir())
	{
		if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (!pExt->JumpjetAllowLayerDeviation.Get(RulesExt::Global()->JumpjetAllowLayerDeviation.Get()))
			{
				R->EDX(INT32_MAX); // Override JumpjetHeight / CruiseHeight check so it always results in 3 / Layer::Air.
				return 0x54B96B;
			}
		}
	}

	return 0;
}

// I think JumpjetLocomotionClass::State is probably an enum, where
// 0 - On ground
// 1 - Taking off from ground
// 2 - Hovering in air
// 3 - Moving in air
// 4 - Deploying to land
// 5 - Crashing
// 6 - Invalid?

// Bugfix: Jumpjet turn to target when attacking
// The way vanilla game handles facing turning is a total mess, so even though this is not the most correct place to do it, given that 0x54BF5B has something similar, I just do it here too
// TODO : The correct fix : 0x736FC4 for stucking at FireError::FACING, 0x736EE9 for something else like OmniFire etc.
DEFINE_HOOK(0x54BD93, JumpjetLocomotionClass_State2_54BD30_TurnToTarget, 0x6)
{
	enum { ContinueNoTarget = 0x54BDA1, EndFunction = 0x54BFDE };
	GET(JumpjetLocomotionClass* const, pLoco, ESI);
	GET(FootClass* const, pLinkedTo, EDI);

	const auto pTarget = pLinkedTo->Target;
	if (!pTarget)
		return ContinueNoTarget;

	if (const auto pThis = abstract_cast<UnitClass*>(pLinkedTo))
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
		if (pTypeExt->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget))
		{
			CoordStruct& source = pThis->Location;
			CoordStruct target = pTarget->GetCoords();
			DirStruct tgtDir = DirStruct { Math::atan2(source.Y - target.Y, target.X - source.X) };

			if (pThis->GetRealFacing().GetFacing<32>() != tgtDir.GetFacing<32>())
				pLoco->LocomotionFacing.SetDesired(tgtDir);
		}
	}

	R->EAX(pTarget);
	return EndFunction;
}
#ifdef REMOVE_THIS_AFTER_V03_RELEASE
// Bugfix: Align jumpjet turret's facing with body's
DEFINE_HOOK(0x736BF3, UnitClass_UpdateRotation_TurretFacing, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	// I still don't know why jumpjet loco behaves differently for the moment
	// so I don't check jumpjet loco or InAir here, feel free to change if it doesn't break performance.
	if (!pThis->Target && !pThis->Type->TurretSpins && (pThis->Type->JumpJet || pThis->Type->BalloonHover))
	{
		pThis->SecondaryFacing.SetDesired(pThis->PrimaryFacing.Current());
		pThis->TurretIsRotating = pThis->SecondaryFacing.IsRotating();
		return 0x736C09;
	}

	return 0;
}
#else

DEFINE_HOOK(0x736BA3, UnitClass_UpdateRotation_TurretFacing_TemporaryFix, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	// When jumpjets arrived at their FootClass::Destination, they seems stuck at the Move mission
	// and therefore the turret facing was set to DirStruct{atan2(0,0)}==DirType::East at 0x736BBB
	// that's why they will come back to normal when giving stop command explicitly
	// so the best way is to fix the Mission if necessary, but I don't know how to do it
	// so I skipped jumpjets check temporarily, and in most cases Jumpjet/BallonHover should cover most of it
	if (pThis->Type->JumpJet || pThis->Type->BalloonHover)
		return 0x736BCA;
	return 0;
}

#endif
// Bugfix: Jumpjet detect cloaked objects beneath
DEFINE_HOOK(0x54C036, JumpjetLocomotionClass_State3_54BFF0_UpdateSensors, 0x7)
{
	GET(FootClass* const, pLinkedTo, ECX);
	GET(CellStruct const, currentCell, EAX);

	// Copied from FootClass::UpdatePosition
	if (pLinkedTo->GetTechnoType()->SensorsSight)
	{
		CellStruct const lastCell = pLinkedTo->LastJumpjetMapCoords;
		if (lastCell != currentCell)
		{
			pLinkedTo->RemoveSensorsAt(lastCell);
			pLinkedTo->AddSensorsAt(currentCell);
		}
	}
	// Something more may be missing

	return 0;
}

//TODO : Issue #690 #655
