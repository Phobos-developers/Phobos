#include "TriggerMPOwner.h"

#include <Utilities/SavegameDef.h>

std::map<int, int> TriggerMPOwner::TriggerType_Owner;

bool TriggerMPOwner::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(TriggerType_Owner)
		.Success();
}

bool TriggerMPOwner::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(TriggerType_Owner)
		.Success();
}
