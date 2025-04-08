#include <DriveLocomotionClass.h>
#include <ShipLocomotionClass.h>
#include <UnitClass.h>
#include <OverlayTypeClass.h>

#include <Ext/Techno/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/TemplateDef.h>

#define Hook_IsCrusher(addr, name, mode, size, reg, retn, retn2) \
DEFINE_HOOK(addr, ##name##_IsCrusher##mode##, size) \
{ \
	GET(FootClass*, pThis, reg); \
	return TechnoExt::GetCrushLevel(pThis) > 0 ? retn : retn2; \
}

Hook_IsCrusher(0x4B19B8, DriveLocomotionClass_ProcessMoving, , 0x8, ECX, 0x4B19D8, 0x4B1A04)
Hook_IsCrusher(0x5B1034, MechLocomotionClass_ProcessMoving, , 0x8, ECX, 0x5B1054, 0x5B108A)
Hook_IsCrusher(0x5B1418, MechLocomotionClass_ProcessMoving, 2, 0x8, ECX, 0x5B1438, 0x5B146E)
Hook_IsCrusher(0x6A1044, ShipLocomotionClass_ProcessMoving, , 0x8, ECX, 0x6A1064, 0x6A10B9)
Hook_IsCrusher(0x73AFEB, UnitClass_PerCellProcess, , 0x6, EBP, 0x73B002, 0x73B074)
Hook_IsCrusher(0x73FC6C, UnitClass_IsCellOccupied, , 0x6, EBX, 0x73FD37, 0x73FC91)
Hook_IsCrusher(0x741733, UnitClass_CrushCell, , 0x6, EDI, 0x741754, 0x74195E)

#undef Hook_IsCrusher

DEFINE_JUMP(LJMP, 0x73FB2A, 0x73FB47)// Skip Crusher check before call ObjectClass::IsCrushable()

DEFINE_JUMP(LJMP, 0x73FE5F, 0x73FE7C)// Skip Crusher check before call ObjectClass::IsCrushable()
DEFINE_JUMP(LJMP, 0x741529, 0x741546)// Skip Crusher check before call ObjectClass::IsCrushable()

DEFINE_HOOK(0x741603, UnitClass_ApproachTarget_OmniCrusher, 0x6)
{
	enum { IsOmniCrusher = 0x741613, NotOmniCrusher = 0x741685 };

	GET(UnitClass*, pThis, ESI);

	return TechnoExt::GetCrushLevel(pThis) >= RulesExt::Global()->OmniCrusherLevel ? IsOmniCrusher : NotOmniCrusher;
}

DEFINE_JUMP(LJMP, 0x7438F7, 0x743918)// Skip Crusher check before call ObjectClass::IsCrushable()

DEFINE_HOOK(0x73B013, UnitClass_PerCellProcess_CrusherWall, 0x6)
{
	enum { CanCrush = 0x73B036, CannotCrush = 0x73B074 };

	GET(OverlayTypeClass*, pOverlay, ESI);

	if (pOverlay->Crushable)
		return CanCrush;

	if (!pOverlay->Wall)
		return CannotCrush;

	GET(UnitClass*, pThis, EBP);

	return pThis->Type->MovementZone == MovementZone::CrusherAll || TechnoExt::GetCrushLevel(pThis) > RulesExt::Global()->WallCrushableLevel ? CanCrush : CannotCrush;
}

DEFINE_HOOK(0x73F42E, UnitClass_IsCellOccupied_CrushWall, 0x6)
{
	enum { CanCrush = 0x73F46E, CannotCrush = 0x73F483 };

	GET(UnitClass*, pThis, EBX);

	return pThis->Type->MovementZone == MovementZone::CrusherAll || TechnoExt::GetCrushLevel(pThis) > RulesExt::Global()->WallCrushableLevel ? CanCrush : CannotCrush;
}

DEFINE_HOOK(0x4B1A1B, DriveLocomotionClass_4B0F20_CrusherAll, 0x8)
{
	enum { CanCrush = 0x4B1A2C, CannotCrush = 0x4B1A77 };

	GET(FootClass*, pLinkedTo, ECX);

	return pLinkedTo->GetTechnoType()->MovementZone == MovementZone::CrusherAll || TechnoExt::GetCrushLevel(pLinkedTo) > RulesExt::Global()->OmniCrusherLevel ? CanCrush : CannotCrush;
}

DEFINE_HOOK(0x5F6CD0, ObjectClass_IsCrushable, 0x6)
{
	enum { SkipGameCode = 0x5F6D90 };

	GET(ObjectClass*, pThis, ECX);
	GET_STACK(FootClass*, pCrusher, 0x4);
	bool result = false;

	if (pThis && pCrusher && pThis != pCrusher)
	{
		if (pThis->AbstractFlags & AbstractFlags::Techno)
		{
			const auto pFoot = abstract_cast<FootClass*>(pThis);

			if (pFoot && !pCrusher->Owner->IsAlliedWith(pFoot) && !pFoot->IsIronCurtained())
				result = TechnoExt::GetCrushLevel(pCrusher) > TechnoExt::GetCrushableLevel(pFoot);
		}
		else
		{
			result = pThis->GetType()->Crushable;
		}
	}

	R->AL(result);
	return SkipGameCode;
}

DEFINE_HOOK(0x73B05B, UnitClass_PerCellProcess_TiltWhenCrushes, 0x6)
{
	enum { SkipGameCode = 0x73B074 };

	GET(UnitClass*, pThis, EBP);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->TiltsWhenCrushes_Overlays.Get(pThis->Type->TiltsWhenCrushes))
		return SkipGameCode;

	pThis->RockingForwardsPerFrame += static_cast<float>(pTypeExt->CrushOverlayExtraForwardTilt);

	return SkipGameCode;
}

DEFINE_HOOK(0x741941, UnitClass_OverrunSquare_TiltWhenCrushes, 0x6)
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
