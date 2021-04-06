#include "FogOfWar.h"

// Loading

//DEFINE_HOOK(6B8DA9, ScenarioClass_LoadSpecialFlags, 6)
//{
//	// Original game won't read these SFs unless in debug 
//	// mode if not in a campaign. I read them all.
//	// Affected labels:
//	// TiberiumGrows, TiberiumSpreads, DestroyableBridges,
//	// FixedAlliance, FogOfWar, Inert, HarvesterImmune.
//
//	// GET(ScenarioFlags*, pSFs, ESI);
//	// pSFs->FogOfWar = false;
//	return 0x6B8DC1;
//}

//DEFINE_HOOK(686C03, SetScenarioFlags_FogOfWar, 5)
//{
//	// Just make it reads first, will look into it later.
//
//	GET(DWORD, dwSFs, EAX);
//	return 0x686C0E;
//}