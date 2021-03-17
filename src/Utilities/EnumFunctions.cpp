#include "EnumFunctions.h"

bool EnumFunctions::CanTargetHouse(AffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	return (flags & AffectedHouse::Owner) && ownerHouse == targetHouse ||
		(flags & AffectedHouse::Allies) && ownerHouse != targetHouse && ownerHouse->IsAlliedWith(targetHouse) ||
		(flags & AffectedHouse::Enemies) && ownerHouse != targetHouse && !ownerHouse->IsAlliedWith(targetHouse);
};