#include <JumpjetLocomotionClass.h>
#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x54AEC0, JumpjetLocomotion_Process_TurnToTarget, 0x8)
{
	GET_STACK(ILocomotion*, iLoco, 0x4);
	const auto pLoco = static_cast<JumpjetLocomotionClass*>(iLoco);
	const auto pThis = pLoco->Owner;
	const auto pType = pThis->GetTechnoType();
	const auto pTarget = pThis->Target;

	if (pTarget && pThis->IsInAir() && !pType->TurretSpins && pThis->WhatAmI() == AbstractType::Unit)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		if (pTypeExt->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget))
		{
			CoordStruct& source = pThis->Location;
			CoordStruct target = pTarget->GetCoords();
			DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));

			if (pThis->GetRealFacing().value32() != tgtDir.value32())
				pLoco->LocomotionFacing.turn(tgtDir);
		}
	}

	return 0;
}

DEFINE_HOOK(0x736BF3, UnitClass_UpdateRotation_TurretFacing, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->IsInAir() && !pThis->Type->TurretSpins && !pThis->Target)
	{
		pThis->SecondaryFacing.turn(pThis->PrimaryFacing.current());
		pThis->unknown_49C = pThis->PrimaryFacing.in_motion();
		return 0x736C09;
	}

	return 0;
}
