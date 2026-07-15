#include "Body.h"

// A unit's extension is a concrete UnitClassExtension leaf, owned by the TechnoClass container.
DEFINE_HOOK(0x7353C4, UnitClass_CTOR, 0x5)
{
	GET(UnitClass*, pItem, ECX);

	TechnoExt::ExtMap.Adopt(new UnitClassExtension(pItem));

	return 0;
}
