// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#include "Body.h"

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
