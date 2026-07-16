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

DEFINE_HOOK(0x7472F0, UnitTypeClass_DTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ECX);

	UnitTypeExt::ExtMap.Remove(pItem);

	return 0;
}
