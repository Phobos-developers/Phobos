#include "CanTargetFlags.h"

bool CanTargetHouse(CanTargetFlags flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	return (flags & CanTargetFlags::Own) && ownerHouse == targetHouse ||
		(flags & CanTargetFlags::Ally) && ownerHouse != targetHouse && ownerHouse->IsAlliedWith(targetHouse) ||
		(flags & CanTargetFlags::Enemy) && ownerHouse != targetHouse && !ownerHouse->IsAlliedWith(targetHouse);
}

CanTargetFlags ParseCanTargetFlags(char* value)
{
	CanTargetFlags retval = CanTargetFlags::None;
	
	// TODO rewrite with generics and RTTI

	char* token = strtok(value, ",");

	while (token != 0)
	{
		if (!_stricmp(token, "Own"))
			retval |= CanTargetFlags::Own;
		if (!_stricmp(token, "Ally"))
			retval |= CanTargetFlags::Ally;
		if (!_stricmp(token, "Enemy"))
			retval |= CanTargetFlags::Enemy;

		token = strtok(0, ",");
	}

	return retval;
}