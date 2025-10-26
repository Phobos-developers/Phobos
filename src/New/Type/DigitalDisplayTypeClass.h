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
#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Anchor.h>

class DigitalDisplayTypeClass final : public Enumerable<DigitalDisplayTypeClass>
{
public:
	Damageable<ColorStruct> Text_Color;
	Valueable<bool> Text_Background;
	Valueable<Vector2D<int>> Offset;
	Nullable<Vector2D<int>> Offset_ShieldDelta;
	Valueable<TextAlign> Align;
	Anchor AnchorType;
	Valueable<BuildingSelectBracketPosition> AnchorType_Building;
	Valueable<SHPStruct*> Shape;
	CustomPalette Palette;
	Nullable<Vector2D<int>> Shape_Spacing;
	Valueable<bool> Shape_PercentageFrame;
	Valueable<bool> Percentage;
	Nullable<bool> HideMaxValue;
	Valueable<AffectedHouse> VisibleToHouses;
	Valueable<bool> VisibleToHouses_Observer;
	Valueable<bool> VisibleInSpecialState;
	Valueable<DisplayInfoType> InfoType;
	Valueable<int> InfoIndex;
	Nullable<int> ValueScaleDivisor;
	Valueable<bool> ValueAsTimer;
	Valueable<DisplayShowType> ShowType;

	DigitalDisplayTypeClass(const char* pTitle = NONE_STR) : Enumerable<DigitalDisplayTypeClass>(pTitle)
		, Text_Color { { 0, 255, 0 }, { 255, 255, 0 }, { 255, 0, 0 } }
		, Text_Background { false }
		, Offset { Point2D::Empty }
		, Offset_ShieldDelta {}
		, Align { TextAlign::Right }
		, AnchorType { HorizontalPosition::Right, VerticalPosition::Top }
		, AnchorType_Building { BuildingSelectBracketPosition::Top }
		, Shape { nullptr }
		, Palette {}
		, Shape_Spacing {}
		, Shape_PercentageFrame { false }
		, Percentage { false }
		, HideMaxValue {}
		, VisibleToHouses { AffectedHouse::All }
		, VisibleToHouses_Observer { true }
		, VisibleInSpecialState { true }
		, InfoType { DisplayInfoType::Health }
		, InfoIndex { 0 }
		, ValueScaleDivisor {}
		, ValueAsTimer { false }
		, ShowType { DisplayShowType::Select }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	bool CanShow(TechnoClass* pThis);
	void Draw(Point2D position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);

private:

	void DisplayText(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);
	void DisplayShape(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);

	template <typename T>
	void Serialize(T& Stm);
};
