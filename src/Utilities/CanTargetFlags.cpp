#include "CanTargetFlags.h"
#include "../Phobos.h"
#include "trim.h"

bool CanTargetHouse(CanTargetFlags flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	return (flags & CanTargetFlags::Self) && ownerHouse == targetHouse ||
		(flags & CanTargetFlags::Ally) && ownerHouse != targetHouse && ownerHouse->IsAlliedWith(targetHouse) ||
		(flags & CanTargetFlags::Enemy) && ownerHouse != targetHouse && !ownerHouse->IsAlliedWith(targetHouse);
}

CanTargetFlags ParseCanTargetFlags(char* value, CanTargetFlags defValue)
{
	CanTargetFlags retval = CanTargetFlags::None;
	
	// TODO rewrite with generics and RTTI

	char* token = strtok(value, Phobos::readDelims);

	while (token != 0)
	{
		token = Trim::FullTrim(token);
		if (!_stricmp(token, "Self"))
			retval |= CanTargetFlags::Self;
		if (!_stricmp(token, "Ally"))
			retval |= CanTargetFlags::Ally;
		if (!_stricmp(token, "Enemy"))
			retval |= CanTargetFlags::Enemy;

		token = strtok(0, Phobos::readDelims);
	}

	if (retval == CanTargetFlags::None) {
		retval = defValue;
	}

	return retval;
}