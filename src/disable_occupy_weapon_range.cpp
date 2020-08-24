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
	GET_BASE(AbstractClass*, target, 0x08);
	GET(BuildingClass*, building, ECX);

	const int count = building->GetOccupantCount();
	const int oldIndex = building->FiringOccupantIndex % count;
	int newIndex = (oldIndex + 1) % count;

	while (newIndex != oldIndex) {
		InfantryClass* infantry = building->Occupants[newIndex];
		int range, minRange;
		int distance = building->DistanceFrom(target);

		if (infantry->Veterancy.IsElite())
		{
			range = infantry->Type->EliteOccupyWeapon.WeaponType->Range;
			minRange = infantry->Type->EliteOccupyWeapon.WeaponType->MinimumRange;
		}
		else
		{
			range = infantry->Type->OccupyWeapon.WeaponType->Range;
			minRange = infantry->Type->OccupyWeapon.WeaponType->MinimumRange;
		}

		if (distance <= range && distance >= minRange)
			break;

		newIndex = (newIndex + 1) % count;
	}

	building->FiringOccupantIndex = newIndex;

	return 0x6FF08B;
}