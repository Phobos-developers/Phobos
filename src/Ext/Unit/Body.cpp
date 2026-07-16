#include "Body.h"

// A unit's extension is a concrete UnitExt leaf, owned by the TechnoClass container.
DEFINE_HOOK(0x7353D3, UnitClass_CTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);

	TechnoExt::ExtMap.Adopt(new UnitExt(pItem));

	return 0;
}
