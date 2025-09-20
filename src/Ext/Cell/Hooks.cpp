#include "Body.h"

#include <Ext/Rules/Body.h>

DEFINE_HOOK(0x480EA8, CellClass_DamageWall_AdjacentWallDamage, 0x5)
{
	enum{ SkipGameCode = 0x480EB4 };

	GET(CellClass*, pThis, EAX);
	pThis->DamageWall(RulesExt::Global()->AdjacentWallDamage);

	if (pThis->OverlayTypeIndex == -1)
		reinterpret_cast<void(__thiscall*)(AbstractClass*)>(0x70D4A0)(pThis);// pThis->BecomeUntargetable();

	return SkipGameCode;
}
