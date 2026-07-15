#include "Body.h"

DEFINE_HOOK(0x517A54, InfantryClass_CTOR, 0x6)
{
	GET(InfantryClass*, pItem, ECX);

	TechnoExt::ExtMap.Adopt(new InfantryClassExtension(pItem));

	return 0;
}
