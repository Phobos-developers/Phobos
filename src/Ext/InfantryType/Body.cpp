#include "Body.h"

DEFINE_HOOK(0x5236B3, InfantryTypeClass_CTOR, 0xA)
{
	GET(InfantryTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.Adopt(new InfantryTypeExt(pItem));

	return 0;
}
