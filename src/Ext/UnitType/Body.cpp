#include "Body.h"

UnitTypeExt::ExtContainer UnitTypeExt::ExtMap;

UnitTypeExt::ExtContainer::ExtContainer() : Container("UnitTypeClass") { }
UnitTypeExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x7470E3, UnitTypeClass_CTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ESI);

	UnitTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

// Hooked after the base destructor call in both destructor bodies; the second site
// is the tail of the standalone body (pop/pop/retn, safe to steal - the bytes after
// it are alignment padding that is never executed).
DEFINE_HOOK_AGAIN(0x747366, UnitTypeClass_DTOR, 0x3)
DEFINE_HOOK(0x748206, UnitTypeClass_DTOR, 0x5)
{
	GET(UnitTypeClass*, pItem, ESI);

	UnitTypeExt::ExtMap.Remove(pItem);

	return 0;
}
