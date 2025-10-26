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


#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Enum.h>

class SelectBoxTypeClass final : public Enumerable<SelectBoxTypeClass>
{
public:
	Valueable<SHPStruct*> Shape;
	CustomPalette Palette;
	Nullable<Vector3D<int>> Frames;
	Valueable<Point2D> Offset;
	TranslucencyLevel Translucency;
	Valueable<AffectedHouse> VisibleToHouses;
	Valueable<bool> VisibleToHouses_Observer;
	Valueable<bool> DrawAboveTechno;
	Valueable<SHPStruct*> GroundShape;
	CustomPalette GroundPalette;
	Nullable<Vector3D<int>> GroundFrames;
	Valueable<Point2D> GroundOffset;
	Valueable<bool> Ground_AlwaysDraw;
	Valueable<bool> GroundLine;
	Damageable<ColorStruct> GroundLineColor;
	Valueable<bool> GroundLine_Dashed;

	SelectBoxTypeClass(const char* pTitle = NONE_STR) : Enumerable<SelectBoxTypeClass>(pTitle)
		, Shape { FileSystem::LoadSHPFile("select.shp") }
		, Palette {}
		, Frames {}
		, Offset { Point2D::Empty }
		, Translucency { 0 }
		, VisibleToHouses { AffectedHouse::All }
		, VisibleToHouses_Observer { true }
		, DrawAboveTechno { true }
		, GroundShape { nullptr }
		, GroundPalette {}
		, GroundFrames {}
		, GroundOffset { Point2D::Empty }
		, Ground_AlwaysDraw { true }
		, GroundLine { false }
		, GroundLineColor { { 0,255,0 } }
		, GroundLine_Dashed { false}
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
