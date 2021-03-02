#pragma once

#include <HouseClass.h>

#include "Enum.h"

class EnumFunctions
{
public:
	static bool CanTargetHouse(CanTargetFlags flags, HouseClass* ownerHouse, HouseClass* targetHouse);
};