#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <HouseClass.h>

enum class CanTargetFlags : unsigned int
{
	None  = 0,
	Own   = 1 << 0,
	Ally  = 1 << 1,
	Enemy = 1 << 2
};

MAKE_ENUM_FLAGS(CanTargetFlags);

bool CanTargetHouse(CanTargetFlags flags, HouseClass* ownerHouse, HouseClass* targetHouse);

CanTargetFlags ParseCanTargetFlags(char* value);