#include <DriveLocomotionClass.h>
#include <ShipLocomotionClass.h>
#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/TemplateDef.h>

DEFINE_HOOK(0x073B05B, UnitClass_PerCellProcess_TiltWhenCrushes, 0x6)
{
	enum { SkipGameCode = 0x73B074 };

	GET(UnitClass*, pThis, EBP);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->TiltsWhenCrushes_Overlays.Get(pThis->Type->TiltsWhenCrushes))
		return SkipGameCode;

	pThis->RockingForwardsPerFrame += static_cast<float>(pTypeExt->CrushOverlayExtraForwardTilt);

	return SkipGameCode;
}

DEFINE_HOOK(0x0741941, UnitClass_OverrunSquare_TiltWhenCrushes, 0x6)
{
	enum { SkipGameCode = 0x74195E };

	GET(UnitClass*, pThis, EDI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->TiltsWhenCrushes_Vehicles.Get(pThis->Type->TiltsWhenCrushes))
		return SkipGameCode;

	pThis->RockingForwardsPerFrame = static_cast<float>(pTypeExt->CrushForwardTiltPerFrame.Get(-0.050000001));

	return SkipGameCode;
}

DEFINE_HOOK(0x4B1150, DriveLocomotionClass_WhileMoving_CrushSlowdown, 0x9)
{
	enum { SkipGameCode = 0x4B116B };

	GET(DriveLocomotionClass*, pThis, EBP);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->LinkedTo->GetTechnoType());
	auto slowdownCoefficient = pThis->movementspeed_50;

	if (slowdownCoefficient > pTypeExt->CrushSlowdownMultiplier)
		slowdownCoefficient = pTypeExt->CrushSlowdownMultiplier;

	__asm { fld slowdownCoefficient };

	return SkipGameCode;

}

DEFINE_HOOK(0x7418AA, UnitClass_CrushCell_WhenCrushed, 6)
{
	GET(UnitClass* const, pThis, EDI);
	GET(ObjectClass* const, pVictim, ESI);

	if(auto const pTechno = abstract_cast<TechnoClass*>(pVictim)) {
		auto pExt = TechnoTypeExt::ExtMap.Find(pVictim->GetTechnoType());

		const auto pWeapon = pExt->WhenCrushed_Weapon;

		auto const pWarhead = pExt->WhenCrushed_Warhead.Get(RulesClass::Instance->C4Warhead);

		if (pWeapon)
			WeaponTypeExt::DetonateAt(pWeapon, pThis->GetCoords(), nullptr, pExt->WhenCrushed_Damage.Get(pWeapon->Damage), pVictim->GetOwningHouse());
		else
		{
			if (pExt->WhenCrushed_Warhead_Full)
				WarheadTypeExt::DetonateAt(pWarhead, pThis->GetCoords(), nullptr, pExt->WhenCrushed_Damage.Get(0),  pVictim->GetOwningHouse());
			else
				MapClass::DamageArea(pThis->GetCoords(), pExt->WhenCrushed_Damage.Get(0), nullptr, pWarhead, true,  pVictim->GetOwningHouse());
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x4B1A4B, DriveLocomotionClass_WhileMoving_CrushTilt, 0xD)
DEFINE_HOOK(0x4B19F7, DriveLocomotionClass_WhileMoving_CrushTilt, 0xD)
{
	enum { SkipGameCode1 = 0x4B1A04, SkipGameCode2 = 0x4B1A58 };

	GET(DriveLocomotionClass*, pThis, EBP);

	auto const pLinkedTo = pThis->LinkedTo;
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());
	pLinkedTo->RockingForwardsPerFrame = static_cast<float>(pTypeExt->CrushForwardTiltPerFrame.Get(-0.050000001));

	return R->Origin() == 0x4B19F7 ? SkipGameCode1 : SkipGameCode2;
}

DEFINE_HOOK(0x6A0813, ShipLocomotionClass_WhileMoving_CrushSlowdown, 0xB)
{
	enum { SkipGameCode = 0x6A082E };

	GET(ShipLocomotionClass*, pThis, EBP);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->LinkedTo->GetTechnoType());
	auto slowdownCoefficient = pThis->movementspeed_50;

	if (slowdownCoefficient > pTypeExt->CrushSlowdownMultiplier)
		slowdownCoefficient = pTypeExt->CrushSlowdownMultiplier;

	__asm { fld slowdownCoefficient };

	return SkipGameCode;
}

DEFINE_HOOK(0x6A108D, ShipLocomotionClass_WhileMoving_CrushTilt, 0xD)
{
	enum { SkipGameCode = 0x6A109A };

	GET(DriveLocomotionClass*, pThis, EBP);

	auto const pLinkedTo = pThis->LinkedTo;
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());
	pLinkedTo->RockingForwardsPerFrame = static_cast<float>(pTypeExt->CrushForwardTiltPerFrame.Get(-0.02));

	return SkipGameCode;
}
