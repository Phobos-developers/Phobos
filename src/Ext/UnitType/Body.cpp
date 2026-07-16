#include "Body.h"

DEFINE_HOOK(0x7470E3, UnitTypeClass_CTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.Adopt(new UnitTypeExt(pItem));

	return 0;
}
