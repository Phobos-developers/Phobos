#include "Body.h"

InfantryExt::ExtContainer InfantryExt::ExtMap;

InfantryExt::ExtContainer::ExtContainer() : Container("InfantryClass") { }
InfantryExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x517A60, InfantryClass_CTOR, 0xE)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x517D90, InfantryClass_DTOR, 0x5)
{
	GET(InfantryClass*, pItem, ECX);

	InfantryExt::ExtMap.Remove(pItem);

	return 0;
}
