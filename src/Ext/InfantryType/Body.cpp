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
