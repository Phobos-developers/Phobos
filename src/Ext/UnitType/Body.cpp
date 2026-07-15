#include "Body.h"

DEFINE_HOOK(0x7470D4, UnitTypeClass_CTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Adopt(new UnitTypeClassExtension(pItem));

	return 0;
}
