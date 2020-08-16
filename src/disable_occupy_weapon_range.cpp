#include <YRPP.h>
#include <Helpers/Macro.h>

// void(_cdecl* DebugLog)(const char* pFormat, ...) =
// (void(__cdecl*)(const char*, ...))0x4068E0;

DEFINE_HOOK(0x6F7288, OccupyWeaponRange_SkipRange, 5)
{
	return 0x6F72C8;
}

DEFINE_HOOK(0x6F9187, OccupyWeaponRange_SearchTarget, 5)
{
	return 0x6F91A7;
}

DEFINE_HOOK(0x6FF065, OccupyWeaponRange_NextIndex, 5)
{
	GET_BASE(AbstractClass*, tgt, 0x08);
	GET(BuildingClass*, bld, ECX);

	const int cnt = bld->GetOccupantCount();
	const int prev = bld->FiringOccupantIndex % cnt;
	int next = (prev + 1) % cnt;

	while (next != prev) {
		InfantryClass* inf = bld->Occupants[next];
		int range;

		if (inf->Veterancy.IsElite())
			range = inf->Type->EliteOccupyWeapon.WeaponType->Range;
		else
			range = inf->Type->OccupyWeapon.WeaponType->Range;

		if (range >= bld->DistanceFrom(tgt)) break;

		next = (next + 1) % cnt;
	}

	bld->FiringOccupantIndex = next;

	return 0x6FF08B;
}