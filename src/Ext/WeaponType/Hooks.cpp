#include "Body.h"

#include <Ext/BulletType/Body.h>

DEFINE_HOOK(0x772A0A, WeaponTypeClass_SetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x772A29;
}

DEFINE_HOOK(0x773087, WeaponTypeClass_GetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x7730A3;
}

DEFINE_HOOK(0x772AA2, WeaponTypeClass_AllowedThreats_AAOnly, 0x5)
{
	GET(BulletTypeClass* const, pType, ECX);

	if (BulletTypeExt::ExtMap.Find(pType)->AAOnly)
	{
		R->EAX(4);
		return 0x772AB3;
	}

	return 0;
}
