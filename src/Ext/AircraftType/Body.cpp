#include "Body.h"

AircraftTypeExt::ExtContainer AircraftTypeExt::ExtMap;

AircraftTypeExt::ExtContainer::ExtContainer() : Container("AircraftTypeClass") { }
AircraftTypeExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x41C8C0, AircraftTypeClass_CTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

// the second address is the twin destructor body the vtable actually points at
DEFINE_HOOK_AGAIN(0x41CFE0, AircraftTypeClass_DTOR, 0x6)
DEFINE_HOOK(0x41CA20, AircraftTypeClass_DTOR, 0x6)
{
	GET(AircraftTypeClass*, pItem, ECX);

	AircraftTypeExt::ExtMap.Remove(pItem);

	return 0;
}
