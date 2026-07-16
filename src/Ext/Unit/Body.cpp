#include "Body.h"

UnitExt::ExtContainer UnitExt::ExtMap;

UnitExt::ExtContainer::ExtContainer() : Container("UnitClass") { }
UnitExt::ExtContainer::~ExtContainer() = default;

// A unit's extension is a concrete UnitExt leaf, owned by the TechnoClass container.
DEFINE_HOOK(0x7353D3, UnitClass_CTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);

	UnitExt::ExtMap.Allocate(pItem);

	return 0;
}
