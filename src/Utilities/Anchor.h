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
#pragma once
#include <Utilities/Enum.h>
#include <Utilities/TemplateDef.h>

#include <YRPPCore.h>

// Helper class to get anchor points on an arbitrary rectangle or a parallelogram
class Anchor
{
public:
	Valueable<HorizontalPosition> Horizontal { HorizontalPosition::Left };
	Valueable<VerticalPosition>   Vertical { VerticalPosition::Top };

	Anchor(HorizontalPosition hPos, VerticalPosition vPos)
		: Horizontal { hPos }, Vertical { vPos }
	{ }

	// Maps enum values to offset relative to width
	double GetRelativeOffsetHorizontal() const;
	// Maps enum values to offset relative to height
	double GetRelativeOffsetVertical() const;

	// Get an anchor point for a freeform parallelogram
	Point2D OffsetPosition(
		const Point2D& topLeft,
		const Point2D& topRight,
		const Point2D& bottomLeft
	) const;

	Point2D OffsetPosition(const RectangleStruct& rect) const;
	Point2D OffsetPosition(const LTRBStruct& ltrb) const;

	void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};
