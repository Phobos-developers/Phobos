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

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK_AGAIN(0x524E90, InfantryTypeClass_DTOR, 0xE)
DEFINE_HOOK(0x523AF0, InfantryTypeClass_DTOR, 0xE)
{
	GET(InfantryTypeClass*, pItem, ESI);

	InfantryTypeExt::ExtMap.Remove(pItem);

	return 0;
}
