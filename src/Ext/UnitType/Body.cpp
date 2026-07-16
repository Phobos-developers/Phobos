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

// Hooked after the base destructor call in the twin destructor body the vtable
// points at (the standalone destructor body has no callers and no hookable window).
DEFINE_HOOK(0x748206, UnitTypeClass_DTOR, 0x5)
{
	GET(UnitTypeClass*, pItem, ESI);

	UnitTypeExt::ExtMap.Remove(pItem);

	return 0;
}
