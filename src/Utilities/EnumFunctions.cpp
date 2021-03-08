#include "EnumFunctions.h"

bool EnumFunctions::CanTargetHouse(AffectsHouses flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	return (flags & AffectsHouses::Owner) && ownerHouse == targetHouse ||
		(flags & AffectsHouses::Allies) && ownerHouse != targetHouse && ownerHouse->IsAlliedWith(targetHouse) ||
		(flags & AffectsHouses::Enemies) && ownerHouse != targetHouse && !ownerHouse->IsAlliedWith(targetHouse);
};