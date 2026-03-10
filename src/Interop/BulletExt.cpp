
#include "BulletExt.h"

DEFINE_EXPORT(bool, Bullet_SetFirerOwner, BulletClass* pBullet, HouseClass* pHouse)
{
	if (!pBullet)
		return false;

	const auto pBulletExt = BulletExt::ExtMap.TryFind(pBullet);

	if (!pBulletExt)
		return false;

	pBulletExt->FirerHouse = pHouse;
	return true;
}
