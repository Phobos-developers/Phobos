#include "Body.h"

DEFINE_HOOK(0x41C8B4, AircraftTypeClass_CTOR, 0x6)
{
	GET(AircraftTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Adopt(new AircraftTypeClassExtension(pItem));

	return 0;
}
