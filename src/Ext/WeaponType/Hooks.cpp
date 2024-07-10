#include "Body.h"
#include <UnitClass.h>
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


// An example for quick jumpjet tilting test
DEFINE_HOOK(0x7413DD, UnitClass_Fire_RecoilForce, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	if (pThis->Transporter || !pThis->IsVoxel())
		return 0;

	GET(BulletClass* const, pBullet, EDI);

	auto force = WeaponTypeExt::ExtMap.Find(pBullet->WeaponType)->RecoilForce.Get();

	if (std::abs(force) < 0.002)
		return 0;

	const double theta = pThis->GetRealFacing().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>();

	pThis->RockingForwardsPerFrame += (float)(-force * Math::cos(theta) * Math::cos(pThis->AngleRotatedForwards) / pThis->Type->Weight);
	pThis->RockingSidewaysPerFrame += (float)(force * Math::sin(theta) * Math::cos(pThis->AngleRotatedSideways) / pThis->Type->Weight);

	return 0;
}
