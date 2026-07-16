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

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK_AGAIN(0x7359DA, UnitClass_DTOR, 0x9)
DEFINE_HOOK(0x735967, UnitClass_DTOR, 0x9)
{
	GET(UnitClass*, pItem, ESI);

	UnitExt::ExtMap.Remove(pItem);

	return 0;
}
