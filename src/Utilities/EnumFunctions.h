#pragma once

#include <HouseClass.h>

#include "Enum.h"

class EnumFunctions
{
public:
	static bool CanTargetHouse(AffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse);
};