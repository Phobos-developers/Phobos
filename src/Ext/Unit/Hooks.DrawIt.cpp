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
#include <Helpers/Macro.h>
#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x73C7AC, UnitClass_DrawAsSHP_DrawTurret_TintFix, 0x6)
{
	enum { SkipDrawCode = 0x73CE00 };

	GET(UnitClass*, pThis, EBP);

	const auto pThisType = pThis->Type;
	const VoxelStruct barrelVoxel = pThisType->BarrelVoxel;

	if (barrelVoxel.VXL && barrelVoxel.HVA)
		return 0;

	GET(UnitTypeClass*, pType, ECX);
	GET(SHPStruct*, pShape, EDI);
	GET(const int, bodyFrameIdx, EBX);
	REF_STACK(Point2D, location, STACK_OFFSET(0x128, 0x4));
	REF_STACK(RectangleStruct, bounds, STACK_OFFSET(0x128, 0xC));
	GET_STACK(const int, extraLight, STACK_OFFSET(0x128, 0x1C));

	const bool tooBigToFitUnderBridge = pType->TooBigToFitUnderBridge
		&& reinterpret_cast<bool(__thiscall*)(TechnoClass*)>(0x703B10)(pThis) && !reinterpret_cast<int(__thiscall*)(TechnoClass*)>(0x703E70)(pThis);
	const int zAdjust = tooBigToFitUnderBridge ? -16 : 0;
	const ZGradient zGradient = tooBigToFitUnderBridge ? ZGradient::Ground : pThis->GetZGradient();

	pThis->Draw_A_SHP(pShape, bodyFrameIdx, &location, &bounds, 0, 256, zAdjust, zGradient, 0, extraLight, 0, 0, 0, 0, 0, 0);

	const auto secondaryDir = pThis->SecondaryFacing.Current();
	const int frameIdx = secondaryDir.GetFacing<32>(4) + pType->WalkFrames * pType->Facings;

	const auto primaryDir = pThis->PrimaryFacing.Current();
	const double bodyRad = primaryDir.GetRadian<32>();
	Matrix3D mtx = Matrix3D::GetIdentity();
	mtx.RotateZ(static_cast<float>(bodyRad));

	TechnoTypeExt::ApplyTurretOffset(pThisType, &mtx);
	const double turretRad = pType->Turret ? secondaryDir.GetRadian<32>() : bodyRad;
	mtx.RotateZ(static_cast<float>(turretRad - bodyRad));

	const auto res = mtx.GetTranslation();
	const auto offset = CoordStruct { static_cast<int>(res.X), static_cast<int>(-res.Y), static_cast<int>(res.Z) };
	Point2D drawPoint = location + TacticalClass::Instance->CoordsToScreen(offset);

	const bool originalDrawShadow = std::exchange(Game::bDrawShadow, false);
	pThis->Draw_A_SHP(pShape, frameIdx, &drawPoint, &bounds, 0, 256, static_cast<DWORD>(-32), zGradient, 0, extraLight, 0, 0, 0, 0, 0, 0);
	Game::bDrawShadow = originalDrawShadow;
	return SkipDrawCode;
}
