#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <ParticleSystemClass.h>
#include <FootClass.h>

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

// Do not adjust map coordinates for railgun particles that are below cell coordinates.
DEFINE_HOOK(0x62B8BC, ParticleClass_CTOR_RailgunCoordAdjust, 0x6)
{
	enum { SkipCoordAdjust = 0x62B8CB };

	GET(ParticleClass*, pThis, ESI);

	if (pThis->ParticleSystem && pThis->ParticleSystem->Type->BehavesLike == BehavesLike::Railgun)
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
DEFINE_HOOK(0x6FF57D, TechnoClass_FireAt_TargetSet, 0x6)
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
	FireAtTemp::pObstacleCell = nullptr;
	FireAtTemp::pOriginalTarget = nullptr;

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
