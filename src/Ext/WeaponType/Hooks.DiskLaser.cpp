#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Body.h>

double WeaponTypeExt::OldRadius = DiskLaserClass::Radius;
// 0x4A6CF0 :
// Angles = [int((i*360/16+270)%360) for i in range(0,16)] = [270, 292, 315, 337, 0, 22, 45, 67, 90, 112, 135, 157, 180, 202, 235, 257] (except the last 2)
// DrawCoords = [( int(np.cos(np.pi/180*deg) * 240), int(np.sin(np.pi/180*deg) * 240) )  for deg in Angles]
constexpr double Cos16Sects[16]
{
	0.0, 0.37460659341591196, 0.7071067811865474, 0.9205048534524403,
	1.0, 0.9271838545667874, 0.7071067811865476, 0.3907311284892737,
	0.0, -0.37460659341591207, -0.7071067811865475, -0.9205048534524404,
	-1.0, -0.9271838545667874, -0.5735764363510464, -0.22495105434386525
};

constexpr double Sin16Sects[16]
{
	-1.0, -0.9271838545667874, -0.7071067811865477, -0.3907311284892739,
	0.0, 0.374606593415912, 0.7071067811865476, 0.9205048534524404,
	1.0, 0.9271838545667874, 0.7071067811865476, 0.39073112848927377,
	0.0, -0.374606593415912, -0.8191520442889916, -0.9743700647852351
};

DEFINE_HOOK(0x4A757B, DiskLaser_Circle, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EDX);

	auto const pTypeData = WeaponTypeExt::ExtMap.TryFind(pWeapon);

	if (pTypeData && WeaponTypeExt::OldRadius != pTypeData->DiskLaser_Radius)
	{
		const double newRadius = pTypeData->DiskLaser_Radius;
		WeaponTypeExt::OldRadius = newRadius;

		for (int i = 0; i < 16; i++)
		{
			DiskLaserClass::DrawCoords[i].X = (int)(Cos16Sects[i] * newRadius);
			DiskLaserClass::DrawCoords[i].Y = (int)(Sin16Sects[i] * newRadius);
		}
	}
	return 0;
}

namespace DiskLaserTemp
{
	struct DiskLaserData
	{
		double FirepowerMultiplier { 1.0 };
	};

	PhobosMap<DiskLaserClass*, DiskLaserData*> DataMap;
}

DEFINE_HOOK(0x4A7A6A, DiskLaserClass_CTOR, 0x6)
{
	GET(DiskLaserClass*, pDiskLaser, ESI);

	DiskLaserTemp::DataMap[pDiskLaser] = DLLCreate<DiskLaserTemp::DiskLaserData>();

	return 0;
}

DEFINE_HOOK(0x4A7CBE, DiskLaserClass_DTOR, 0x5)
{
	GET(DiskLaserClass*, pDiskLaser, ESI);

	DiskLaserTemp::DataMap.erase(pDiskLaser);

	return 0;
}

DEFINE_HOOK(0x6FE489, TechnoClass_Fire_SetDiskLaserFirepower, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);
	GET(DiskLaserClass*, pDiskLaser, EBX);
	const auto pData = DiskLaserTemp::DataMap.get_or_default(pDiskLaser);

	pData->FirepowerMultiplier = TechnoExt::GetCurrentFirepowerMultiplier(pTechno);

	return 0;
}

DEFINE_HOOK(0x4A7755, DiskLaserClass_AI_ChargeUpSound, 0x5)
{
	enum { SkipGameCode = 0x4A7760 };

	GET(DiskLaserClass*, pDiskLaser, ESI);
	const int sound = WeaponTypeExt::ExtMap.Find(pDiskLaser->Weapon)->DiskLaser_ChargeUp;

	if (sound == -1)
		return 0;

	R->ECX(sound);
	return SkipGameCode;
}

DEFINE_HOOK(0x4A7696, DiskLaserClass_AI_SimulateFiring, 0x6)
{
	enum { ReturnFromFunction = 0x4A76F4 };

	GET(DiskLaserClass*, pDiskLaser, ESI);
	const auto pWeapon = pDiskLaser->Weapon;
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (!pWeaponExt->DiskLaser_SimulateFire)
		return 0;

	REF_STACK(CoordStruct, targetCoords, STACK_OFFSET(0x50, -0x18));
	REF_STACK(CoordStruct, fireCoords, STACK_OFFSET(0x50, -0x24));
	const CoordStruct targetCoords2D { targetCoords.X, targetCoords.Y, 0 };
	const CoordStruct fireCoords2D { fireCoords.X, fireCoords.Y, 0 };

	auto GetSpeed = reinterpret_cast<int(__thiscall*)(WeaponTypeClass*, int)>(0x773070);
	int bulletSpeed = GetSpeed(pWeapon, static_cast<int>(fireCoords2D.DistanceFrom(targetCoords2D)));

	const auto pOwner = pDiskLaser->Owner;
	const auto pData = DiskLaserTemp::DataMap.get_or_default(pDiskLaser);
	const int damage = static_cast<int>(pDiskLaser->Damage * pData->FirepowerMultiplier);

	if (const auto pBullet = pWeapon->Projectile->CreateBullet(pDiskLaser->Target, pOwner, damage, pWeapon->Warhead, bulletSpeed, pWeapon->Bright))
	{
		BulletExt::ExtMap.Find(pBullet)->FirepowerMult = pData->FirepowerMultiplier;
		BulletExt::SimulatedFiringUnlimbo(pBullet, pOwner->Owner, pWeapon, fireCoords, false);
		BulletExt::SimulatedFiringEffects(pBullet, pOwner->Owner, pOwner, true, false);
	}

	pDiskLaser->unknown_30 = static_cast<DWORD>(-1); // Restrore overriden instruction and skip Ares's hook here
	return ReturnFromFunction;
}
