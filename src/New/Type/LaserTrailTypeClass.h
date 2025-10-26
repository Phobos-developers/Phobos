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
#include <Utilities/Template.h>

class LaserTrailTypeClass final : public Enumerable<LaserTrailTypeClass>
{
public:
	Valueable<LaserTrailDrawType> DrawType;
	Valueable<bool> IsHouseColor;
	Valueable<ColorStruct> Color;
	Valueable<int> Thickness;
	Valueable<bool> IsAlternateColor;
	Nullable<ColorStruct> Bolt_Color[3];
	Valueable<bool> Bolt_Disable[3];
	Valueable<int> Bolt_Arcs;
	Nullable<ColorStruct> Beam_Color;
	Valueable<double> Beam_Amplitude;
	Nullable<int> FadeDuration;
	Valueable<int> SegmentLength;
	Valueable<bool> IgnoreVertical;
	Valueable<bool> IsIntense;
	Valueable<bool> CloakVisible;
	Valueable<bool> CloakVisible_DetectedOnly;
	Valueable<bool> DroppodOnly;
	Valueable<bool> IsHideable;

	LaserTrailTypeClass(const char* pTitle = NONE_STR) : Enumerable<LaserTrailTypeClass>(pTitle)
		, DrawType { LaserTrailDrawType::Laser }
		, IsHouseColor { false }
		, Color { { 255, 0, 0 } }
		, Thickness { 4 }
		, IsAlternateColor { false }
		, Bolt_Color {}
		, Bolt_Disable { Valueable<bool>(false) }
		, Bolt_Arcs { 8 }
		, Beam_Color {}
		, Beam_Amplitude { 40.0 }
		, FadeDuration {}
		, SegmentLength { 128 }
		, IgnoreVertical { false }
		, IsIntense { false }
		, CloakVisible { false }
		, CloakVisible_DetectedOnly { false }
		, DroppodOnly { false }
		, IsHideable { true }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
