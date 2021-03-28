#include "Body.h"

DEFINE_HOOK(679A15, RulesData_LoadBeforeTypeData, 6)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	//	RulesClass::Initialized = true;
	RulesExt::LoadBeforeTypeData(pItem, pINI);
	return 0;
}
