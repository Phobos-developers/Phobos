#pragma once

#include <map>

#include <Utilities/Savegame.h>

class TriggerMPOwner
{
public:
	static std::map<int, int> TriggerType_Owner;

	static bool LoadGlobals(PhobosStreamReader& stm)
	{
		return stm
			.Process(TriggerType_Owner)
			.Success();
	}

	static bool SaveGlobals(PhobosStreamWriter& stm)
	{
		return stm
			.Process(TriggerType_Owner)
			.Success();
	}
};

std::map<int, int> TriggerMPOwner::TriggerType_Owner;
