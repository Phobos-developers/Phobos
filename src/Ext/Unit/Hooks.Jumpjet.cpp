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

// Bugfix: Jumpjet turn to target when attacking
// The way vanilla game handles facing turning is a total mess, so even though this is not the most correct place to do it, given that 0x54BF5B has something similar, I just do it here too
// TODO : The correct fix : 0x736FC4 for stucking at FireError::FACING, 0x736EE9 for something else like OmniFire etc.
DEFINE_HOOK(0x54BD93, JumpjetLocomotionClass_State2_TurnToTarget, 0x6)
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
			DirStruct tgtDir { Math::atan2(source.Y - target.Y, target.X - source.X) };

			if (pThis->GetRealFacing() != tgtDir)
				pLoco->LocomotionFacing.SetDesired(tgtDir);
		}
	}

	R->EAX(pTarget);
	return EndFunction;
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
//TODO : Issue #690 #655
