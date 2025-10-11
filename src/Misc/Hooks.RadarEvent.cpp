#include <RadarEventClass.h>
#include <Ext/Rules/Body.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x65FC6E, RadarEventClass_CTOR_SkipSetRadarEventCell, 0x6)
{
	if (!RulesExt::Global()->IgnoreCenterMinorRadarEvent)
		return 0;

	GET(RadarEventClass*, pThis, ESI);

	switch (pThis->Type)
	{
	case RadarEventType::UnitProduced:
	case RadarEventType::UnitRepaired:
	case RadarEventType::BuildingInfiltrated:
	case RadarEventType::BuildingCaptured:
	case RadarEventType::BridgeRepaired:
	case RadarEventType::GarrisonAbandoned:
		break;
	default:
		return 0;
	}

	R->ECX(Make_Global<int>(0xB04DB8)); // RadarEventClass::Array.Count
	return 0x65FC9E;
}
