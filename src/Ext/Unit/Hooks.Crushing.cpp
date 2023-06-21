#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/TemplateDef.h>

DEFINE_HOOK(0x073B061, UnitClass_PerCellProcess_TiltWhenCrushes, 0x6)
{
	enum { SkipTilt = 0x73B067 };

	GET(UnitClass*, pThis, EBP);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->TiltsWhenCrushes_Walls.Get(pThis->Type->TiltsWhenCrushes))
		return SkipTilt;

	return 0;
}

DEFINE_HOOK(0x0741941, UnitClass_OverrunSquare_TiltWhenCrushes, 0x6)
{
	enum { SkipTilt = 0x74195E };

	GET(UnitClass*, pThis, EDI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->TiltsWhenCrushes_Vehicles.Get(pThis->Type->TiltsWhenCrushes))
		return SkipTilt;

	return 0;
}

DEFINE_HOOK(0x4B1150, DriveLocomotionClass_WhileMoving_WallCrushSlowdown, 0x9)
{
	enum { SkipSlowdown = 0x4B1182 };

	GET(FootClass*, pLinkedTo, ECX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());

	if (!pTypeExt->WallCrushSlowdown)
		return SkipSlowdown;

	return 0;
}

DEFINE_HOOK(0x6A0813, ShipLocomotionClass_WhileMoving_WallCrushSlowdown, 0x9)
{
	enum { SkipSlowdown = 0x6A0845 };

	GET(FootClass*, pLinkedTo, ECX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());

	if (!pTypeExt->WallCrushSlowdown)
		return SkipSlowdown;

	return 0;
}
