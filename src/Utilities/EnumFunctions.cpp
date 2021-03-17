#include "EnumFunctions.h"

bool EnumFunctions::CanTargetHouse(SuperWeaponAffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	return (flags & SuperWeaponAffectedHouse::Owner) && ownerHouse == targetHouse ||
		(flags & SuperWeaponAffectedHouse::Allies) && ownerHouse != targetHouse && ownerHouse->IsAlliedWith(targetHouse) ||
		(flags & SuperWeaponAffectedHouse::Enemies) && ownerHouse != targetHouse && !ownerHouse->IsAlliedWith(targetHouse);
};