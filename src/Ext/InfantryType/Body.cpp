#include "Body.h"

InfantryTypeExt::ExtContainer InfantryTypeExt::ExtMap;

InfantryTypeExt::ExtContainer::ExtContainer() : Container("InfantryTypeClass") { }
InfantryTypeExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x5236B3, InfantryTypeClass_CTOR, 0xA)
{
	GET(InfantryTypeClass*, pItem, ESI);

	InfantryTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x5239D0, InfantryTypeClass_DTOR, 0x5)
{
	GET(InfantryTypeClass*, pItem, ECX);

	InfantryTypeExt::ExtMap.Remove(pItem);

	return 0;
}
