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

// Hooked after the base destructor call in the twin destructor body the vtable
// points at (the standalone destructor body has no callers and no hookable window).
DEFINE_HOOK(0x41D056, AircraftTypeClass_DTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExt::ExtMap.Remove(pItem);

	return 0;
}
