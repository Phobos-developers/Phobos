#include "Body.h"

DEFINE_HOOK(0x65DBD0, TeamTypeClass_CreateInstance_Plane, 0x6)
{
	GET(TeamTypeClass*, pThis, EDI);

	const auto pTeamTypeExt = TeamTypeExt::Fetch(pThis);

	if (AircraftTypeClass* const pAircraftType = pTeamTypeExt->ParaDropAircraft)
		R->ECX(pAircraftType);

	return 0;
}
