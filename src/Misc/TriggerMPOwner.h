#pragma once

#include <map>

#include <Utilities/Savegame.h>

class TriggerMPOwner
{
public:
	static std::map<int, int> TriggerType_Owner;

	static bool LoadGlobals(PhobosStreamReader& stm);

	static bool SaveGlobals(PhobosStreamWriter& stm);
};
