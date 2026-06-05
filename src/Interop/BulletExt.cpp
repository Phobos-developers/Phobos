
#include "BulletExt.h"

DEFINE_EXPORT(HRESULT, Bullet_SetFirerOwner, BulletClass* pBullet, HouseClass* pHouse)
{
	if (!pBullet)
		return E_POINTER;

	const auto pBulletExt = BulletExt::ExtMap.TryFind(pBullet);

	if (!pBulletExt)
		return E_UNEXPECTED;

	pBulletExt->FirerHouse = pHouse;
	return S_OK;
}
