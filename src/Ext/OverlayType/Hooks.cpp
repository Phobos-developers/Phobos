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

#include <HouseClass.h>

DEFINE_HOOK(0x47F71D, CellClass_DrawOverlay_ZAdjust, 0x5)
{
	GET(const int, zAdjust, EDI);
	GET_STACK(OverlayTypeClass*, pOverlayType, STACK_OFFSET(0x24, -0x14));

	auto const pTypeExt = OverlayTypeExt::ExtMap.Find(pOverlayType);

	if (pTypeExt->ZAdjust != 0)
		R->EDI(zAdjust - pTypeExt->ZAdjust);

	return 0;
}

// Replaces an Ares hook at 0x47F9A4
DEFINE_HOOK(0x47F974, CellClass_DrawOverlay_Walls, 0x5)
{
	enum { SkipGameCode = 0x47FB86 };

	GET(CellClass*, pThis, ESI);
	GET(SHPStruct*, pShape, EAX);
	GET(RectangleStruct*, pBounds, EBP);
	GET(const int, zAdjust, EDI);
	GET_STACK(OverlayTypeClass*, pOverlayType, STACK_OFFSET(0x24, -0x14));
	REF_STACK(Point2D, pLocation, STACK_OFFSET(0x24, -0x10));

	const int wallOwnerIndex = pThis->WallOwnerIndex;
	int colorSchemeIndex = HouseClass::CurrentPlayer->ColorSchemeIndex;

	if (wallOwnerIndex >= 0)
		colorSchemeIndex = HouseClass::Array[wallOwnerIndex]->ColorSchemeIndex;

	LightConvertClass* pConvert = nullptr;
	auto const pTypeExt = OverlayTypeExt::ExtMap.Find(pOverlayType);

	if (pTypeExt->Palette)
		pConvert = pTypeExt->Palette->Items[colorSchemeIndex]->LightConvert;
	else
		pConvert = ColorScheme::Array[colorSchemeIndex]->LightConvert;

	DSurface::Temp->DrawSHP(pConvert, pShape, pThis->OverlayData, &pLocation, pBounds,
		BlitterFlags(0x4E00), 0, -2 - zAdjust, ZGradient::Deg90, pThis->Intensity_Normal, 0, 0, 0, 0, 0);

	return SkipGameCode;
}
