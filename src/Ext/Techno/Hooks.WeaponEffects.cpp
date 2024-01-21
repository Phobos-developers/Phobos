#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <ParticleSystemClass.h>
#include <FootClass.h>

#include <Ext/ParticleSystemType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/Macro.h>

// Contains hooks that fix weapon graphical effects like lasers, railguns, electric bolts, beams and waves not interacting
// correctly with obstacles between firer and target, as well as railgun / railgun particles being cut off by elevation.

namespace FireAtTemp
{
	CoordStruct originalTargetCoords;
	CellClass* pObstacleCell = nullptr;
	AbstractClass* pOriginalTarget = nullptr;
}

// Set obstacle cell.
DEFINE_HOOK(0x6FF15F, TechnoClass_FireAt_ObstacleCellSet, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET_BASE(AbstractClass*, pTarget, 0x8);
	LEA_STACK(CoordStruct*, pSourceCoords, STACK_OFFSET(0xB0, -0x6C));

	auto coords = pTarget->GetCenterCoords();

	if (const auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
		coords = pBuilding->GetTargetCoords();

	FireAtTemp::pObstacleCell = TrajectoryHelper::FindFirstObstacle(*pSourceCoords, coords, pWeapon->Projectile, pThis->Owner);

	return 0;
}

// Apply obstacle logic to fire & spark particle system targets.
DEFINE_HOOK_AGAIN(0x6FF1D7, TechnoClass_FireAt_SparkFireTargetSet, 0x5)
DEFINE_HOOK(0x6FF189, TechnoClass_FireAt_SparkFireTargetSet, 0x5)
{
	if (FireAtTemp::pObstacleCell)
	{
		if (R->Origin() == 0x6FF189)
			R->ECX(FireAtTemp::pObstacleCell);
		else
			R->EDX(FireAtTemp::pObstacleCell);
	}

	return 0;
}

// Fix fire particle target coordinates potentially differing from actual target coords.
DEFINE_HOOK(0x62FA20, ParticleSystemClass_FireAI_TargetCoords, 0x6)
{
	enum { SkipGameCode = 0x62FA51, Continue = 0x62FBAF };

	GET(ParticleSystemClass*, pThis, ESI);
	GET(TechnoClass*, pOwner, EBX);

	if (pOwner->PrimaryFacing.IsRotating())
	{
		auto const pTypeExt = ParticleSystemTypeExt::ExtMap.Find(pThis->Type);

		if (!pTypeExt->AdjustTargetCoordsOnRotation)
			return Continue;

		auto coords = pThis->TargetCoords;
		R->EAX(&coords);
		return SkipGameCode;
	}

	return Continue;
}

// Fix fire particles being disallowed from going upwards.
DEFINE_HOOK(0x62D685, ParticleSystemClass_Fire_Coords, 0x5)
{
	enum { SkipGameCode = 0x62D6B7 };

	// Game checks if MapClass::GetCellFloorHeight() for currentCoords is larger than for previousCoords and sets the flags on ParticleClass to
	// remove it if so. Below is an attempt to create a smarter check that allows upwards movement and does not needlessly collide with elevation
	// but removes particles when colliding with flat ground. It doesn't work perfectly and covering all edge-cases is difficult or impossible so
	// preference was to disable it. Keeping the code here commented out, however.

	/*
	GET(ParticleClass*, pThis, ESI);
	REF_STACK(CoordStruct, currentCoords, STACK_OFFSET(0x24, -0x18));
	REF_STACK(CoordStruct, previousCoords, STACK_OFFSET(0x24, -0xC));

	auto const sourceLocation = pThis->ParticleSystem ? pThis->ParticleSystem->Location : CoordStruct { INT_MAX, INT_MAX, INT_MAX };
	auto const pCell = MapClass::Instance->TryGetCellAt(currentCoords);
	int cellFloor = MapClass::Instance->GetCellFloorHeight(currentCoords);
	bool downwardTrajectory = currentCoords.Z < previousCoords.Z;
	bool isBelowSource = cellFloor < sourceLocation.Z - Unsorted::LevelHeight * 2;
	bool isRamp = pCell ? pCell->SlopeIndex : false;

	if (!isRamp && isBelowSource && downwardTrajectory && currentCoords.Z < cellFloor)
	{
		pThis->unknown_12D = 1;
		pThis->unknown_131 = 1;
	}
	*/

	return SkipGameCode;
}

// Fix railgun target coordinates potentially differing from actual target coords.
DEFINE_HOOK(0x70C6B5, TechnoClass_Railgun_TargetCoords, 0x5)
{
	GET(AbstractClass*, pTarget, EBX);

	auto coords = pTarget->GetCenterCoords();

	if (const auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
		coords = pBuilding->GetTargetCoords();
	else if (const auto pCell = abstract_cast<CellClass*>(pTarget))
		coords = pCell->GetCoordsWithBridge();

	R->EAX(&coords);
	return 0;
}

// Cut railgun logic off at obstacle coordinates.
DEFINE_HOOK(0x70CA64, TechnoClass_Railgun_Obstacles, 0x5)
{
	enum { Continue = 0x70CA79, Stop = 0x70CAD8 };

	REF_STACK(CoordStruct const, coords, STACK_OFFSET(0xC0, -0x80));

	auto pCell = MapClass::Instance->GetCellAt(coords);

	if (pCell == FireAtTemp::pObstacleCell)
		return Stop;

	return Continue;
}

DEFINE_HOOK(0x70C862, TechnoClass_Railgun_AmbientDamageIgnoreTarget1, 0x5)
{
	enum { IgnoreTarget = 0x70CA59 };

	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);

	if (WeaponTypeExt::ExtMap.Find(pWeapon)->AmbientDamage_IgnoreTarget)
		return IgnoreTarget;

	return 0;
}

DEFINE_HOOK(0x70CA8B, TechnoClass_Railgun_AmbientDamageIgnoreTarget2, 0x6)
{
	enum { IgnoreTarget = 0x70CBB0 };

	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);
	REF_STACK(DynamicVectorClass<ObjectClass*>, objects, STACK_OFFSET(0xC0, -0xAC));

	if (WeaponTypeExt::ExtMap.Find(pWeapon)->AmbientDamage_IgnoreTarget)
	{
		R->EAX(objects.Count);
		return IgnoreTarget;
	}

	return 0;
}

DEFINE_HOOK(0x70CBE0, TechnoClass_Railgun_AmbientDamageWarhead, 0x5)
{
	GET(WeaponTypeClass*, pWeapon, EDI);

	R->EDX(WeaponTypeExt::ExtMap.Find(pWeapon)->AmbientDamage_Warhead.Get(pWeapon->Warhead));

	return 0;
}

// Do not adjust map coordinates for railgun or fire stream particles that are below cell coordinates.
DEFINE_HOOK(0x62B8BC, ParticleClass_CTOR_CoordAdjust, 0x6)
{
	enum { SkipCoordAdjust = 0x62B8CB };

	GET(ParticleClass*, pThis, ESI);

	if (pThis->ParticleSystem && (pThis->ParticleSystem->Type->BehavesLike == BehavesLike::Railgun || pThis->ParticleSystem->Type->BehavesLike == BehavesLike::Fire))
		return SkipCoordAdjust;

	return 0;
}

// Adjust target coordinates for laser drawing.
DEFINE_HOOK(0x6FD38D, TechnoClass_LaserZap_Obstacles, 0x7)
{
	GET(CoordStruct*, pTargetCoords, EAX);

	auto coords = *pTargetCoords;
	auto pObstacleCell = FireAtTemp::pObstacleCell;

	if (pObstacleCell)
		coords = pObstacleCell->GetCoordsWithBridge();

	R->EAX(&coords);
	return 0;
}

// Adjust target for bolt / beam / wave drawing.
DEFINE_HOOK(0x6FF43F, TechnoClass_FireAt_TargetSet, 0x6)
{
	LEA_STACK(CoordStruct*, pTargetCoords, STACK_OFFSET(0xB0, -0x28));
	GET(AbstractClass*, pTarget, EDI);

	FireAtTemp::originalTargetCoords = *pTargetCoords;
	FireAtTemp::pOriginalTarget = pTarget;

	if (FireAtTemp::pObstacleCell)
	{
		auto coords = FireAtTemp::pObstacleCell->GetCoordsWithBridge();
		pTargetCoords->X = coords.X;
		pTargetCoords->Y = coords.Y;
		pTargetCoords->Z = coords.Z;
		R->EDI(FireAtTemp::pObstacleCell);
	}

	return 0;
}

// Restore original target values and unset obstacle cell.
DEFINE_HOOK(0x6FF660, TechnoClass_FireAt_ObstacleCellUnset, 0x6)
{
	LEA_STACK(CoordStruct*, pTargetCoords, STACK_OFFSET(0xB0, -0x28));

	auto coords = FireAtTemp::originalTargetCoords;
	pTargetCoords->X = coords.X;
	pTargetCoords->Y = coords.Y;
	pTargetCoords->Z = coords.Z;
	auto target = FireAtTemp::pOriginalTarget;

	FireAtTemp::originalTargetCoords = CoordStruct::Empty;
	FireAtTemp::pOriginalTarget = nullptr;
	FireAtTemp::pObstacleCell = nullptr;

	R->EDI(target);

	return 0;
}

// Allow drawing single color lasers with thickness.
DEFINE_HOOK(0x6FD446, TechnoClass_LaserZap_IsSingleColor, 0x7)
{
	GET(WeaponTypeClass* const, pWeapon, ECX);
	GET(LaserDrawClass* const, pLaser, EAX);

	if (auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		if (!pLaser->IsHouseColor && pWeaponExt->Laser_IsSingleColor)
			pLaser->IsHouseColor = true;
	}

	// Fixes drawing thick lasers for non-PrismSupport building-fired lasers.
	pLaser->IsSupported = pLaser->Thickness > 3;

	return 0;
}
