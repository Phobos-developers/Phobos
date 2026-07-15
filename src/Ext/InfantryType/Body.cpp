#include "Body.h"

DEFINE_HOOK(0x5236A4, InfantryTypeClass_CTOR, 0x5)
{
	GET(InfantryTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Adopt(new InfantryTypeClassExtension(pItem));

	return 0;
}
