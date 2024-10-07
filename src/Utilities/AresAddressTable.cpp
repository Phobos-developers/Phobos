#pragma once
#include "AresFunctions.h"

const AresHelper::AresVersionFunctionMap AresHelper::AresFunctionOffsets =
{
	{
		AresHelper::Version::Ares30,
		{
			{ ARES_FUN(ConvertTypeTo),  0x043650 },
			{ ARES_FUN(SpawnSurvivors), 0x0464C0 },
			{ ARES_FUN(HasFactory),     0x0217C0 },
			{ ARES_FUN(CanBeBuiltAt),   0x03E3B0 },

			{ "IsTargetConstraintsEligible", 0x032110 },

			{ "ExtMap_Find", 0x057C70 },
			{ "SWTypeExt_Map", 0x0C1C54 },
		}
	},
	{
		AresHelper::Version::Ares30p,
		{
			{ ARES_FUN(ConvertTypeTo),  0x044130 },
			{ ARES_FUN(SpawnSurvivors), 0x047030 },
			{ ARES_FUN(HasFactory),     0x0217C0 },
			{ ARES_FUN(CanBeBuiltAt),   0x03E3B0 },

			{ "IsTargetConstraintsEligible", 0x032AF0 },

			{ "ExtMap_Find", 0x058900 },
			{ "SWTypeExt_Map", 0x0C2C50 },
		}
	},
};
