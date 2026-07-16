#include "Body.h"

DEFINE_HOOK(0x41C8C0, AircraftTypeClass_CTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.Adopt(new AircraftTypeClassExtension(pItem));

	return 0;
}
