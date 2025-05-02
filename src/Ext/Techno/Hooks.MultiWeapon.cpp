#include "Body.h"

// Do you think the infantry's way of determining that weapons are secondary is stupid ?
// I think it's kind of stupid.
DEFINE_HOOK(0x520888, InfantryClass_UpdateFiring_IsSecondary, 0x8)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, weaponIdx, EDI);
	enum { Primary = 0x5208D6, Secondary = 0x520890 };

	R->AL(pThis->Crawling);
	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->IsSecondary(weaponIdx) ? Secondary : Primary;
}
