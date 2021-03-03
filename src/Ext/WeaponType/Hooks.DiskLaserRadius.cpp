#include "Body.h"

int WeaponTypeExt::nOldCircumference = 240;
DEFINE_HOOK(4A757B, DiskLaser_Circle, 6)
{
	GET(WeaponTypeClass*, pWeapon, EDX);
	int new_Circumference = WeaponTypeExt::ExtMap.Find(pWeapon)->DiskLaser_Circumference;
	
	if (WeaponTypeExt::nOldCircumference != new_Circumference) {
		WeaponTypeExt::nOldCircumference = new_Circumference;

		Point2D* DiscLaserCoords = reinterpret_cast<Point2D*>(0x8A0180);
		DiscLaserCoords[0].X = 0;
		DiscLaserCoords[0].Y = -1 * new_Circumference;
		DiscLaserCoords[1].X = (int)(0.3746065934159128 * new_Circumference);
		DiscLaserCoords[1].Y = (int)(-0.9271838545667871 * new_Circumference);
		DiscLaserCoords[2].X = (int)(0.707106781186548 * new_Circumference);
		DiscLaserCoords[2].Y = -1 * DiscLaserCoords[2].X;
		DiscLaserCoords[3].X = (int)(0.9205048534524406 * new_Circumference);
		DiscLaserCoords[3].Y = (int)(-0.39073112848927305 * new_Circumference);
		DiscLaserCoords[4].X = new_Circumference;
		DiscLaserCoords[4].Y = 0;
		DiscLaserCoords[5].X = -1 * DiscLaserCoords[1].Y;
		DiscLaserCoords[5].Y = DiscLaserCoords[1].X;
		DiscLaserCoords[6].X = DiscLaserCoords[2].X;
		DiscLaserCoords[6].Y = DiscLaserCoords[2].X;
		DiscLaserCoords[7].X = -1 * DiscLaserCoords[3].Y;
		DiscLaserCoords[7].Y = DiscLaserCoords[3].X;
		DiscLaserCoords[8].X = 0;
		DiscLaserCoords[8].Y = DiscLaserCoords[4].X;
		DiscLaserCoords[9].X = -1 * DiscLaserCoords[1].X;
		DiscLaserCoords[9].Y = DiscLaserCoords[5].X;
		DiscLaserCoords[10].X = DiscLaserCoords[2].Y;
		DiscLaserCoords[10].Y = DiscLaserCoords[6].Y;
		DiscLaserCoords[11].X = -1 * DiscLaserCoords[3].X;
		DiscLaserCoords[11].Y = DiscLaserCoords[7].X;
		DiscLaserCoords[12].X = DiscLaserCoords[0].Y;
		DiscLaserCoords[12].Y = 0;
		DiscLaserCoords[13].X = DiscLaserCoords[1].Y;
		DiscLaserCoords[13].Y = DiscLaserCoords[9].X;
		DiscLaserCoords[14].X = (int)(-0.5735764363510456 * new_Circumference);
		DiscLaserCoords[14].Y = (int)(-0.8191520442889921 * new_Circumference);
		DiscLaserCoords[15].Y = (int)(-0.9743700647852354 * new_Circumference);
		DiscLaserCoords[15].X = (int)(-0.2249510543438644 * new_Circumference);
	}
	return 0;
}
