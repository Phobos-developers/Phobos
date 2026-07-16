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

// the second address is the twin destructor body the vtable actually points at
DEFINE_HOOK_AGAIN(0x524D70, InfantryTypeClass_DTOR, 0x6)
DEFINE_HOOK(0x5239D0, InfantryTypeClass_DTOR, 0x5)
{
	GET(InfantryTypeClass*, pItem, ECX);

	InfantryTypeExt::ExtMap.Remove(pItem);

	return 0;
}
