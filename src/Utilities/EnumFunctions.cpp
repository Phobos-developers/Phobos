#include "EnumFunctions.h"

bool EnumFunctions::CanTargetHouse(CanTargetFlags flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	return (flags & CanTargetFlags::Self) && ownerHouse == targetHouse ||
		(flags & CanTargetFlags::Ally) && ownerHouse != targetHouse && ownerHouse->IsAlliedWith(targetHouse) ||
		(flags & CanTargetFlags::Enemy) && ownerHouse != targetHouse && !ownerHouse->IsAlliedWith(targetHouse);
};