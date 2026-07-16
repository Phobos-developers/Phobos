#include "Body.h"

DEFINE_HOOK(0x517A60, InfantryClass_CTOR, 0xE)
{
	GET(InfantryClass*, pItem, ESI);

	TechnoExt::ExtMap.Adopt(new InfantryClassExtension(pItem));

	return 0;
}
