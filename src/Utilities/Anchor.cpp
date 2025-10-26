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
#include "Anchor.h"

#include <GeneralDefinitions.h>

double Anchor::GetRelativeOffsetHorizontal() const
{
	// Enum goes from 0 to 2 from left to right. Cast it and divide it
	// by 2 and you get the percentage. Pretty clever huh? - Kerbiter
	return (static_cast<double>(this->Horizontal.Get()) / 2.0);
}

double Anchor::GetRelativeOffsetVertical() const
{
	// Same deal as with the left-right one - Kerbiter
	return (static_cast<double>(this->Vertical.Get()) / 2.0);
}

Point2D Anchor::OffsetPosition(
	const Point2D& topLeft,
	const Point2D& topRight,
	const Point2D& bottomLeft) const
{
	Point2D result { topLeft };
	Point2D deltaTopRight { topRight - topLeft };
	Point2D deltaBottomLeft { bottomLeft - topLeft };

	result += deltaTopRight * this->GetRelativeOffsetHorizontal();
	result += deltaBottomLeft * this->GetRelativeOffsetVertical();

	return result;
}

Point2D Anchor::OffsetPosition(const RectangleStruct& rect) const
{
	Point2D result { rect.X, rect.Y };

	result.X += static_cast<int>(rect.Width * this->GetRelativeOffsetHorizontal());
	result.Y += static_cast<int>(rect.Height * this->GetRelativeOffsetVertical());

	return result;
}

Point2D Anchor::OffsetPosition(const LTRBStruct& ltrb) const
{
	Point2D result { ltrb.Left, ltrb.Top };
	int deltaX = ltrb.Right - ltrb.Left;
	int deltaY = ltrb.Bottom - ltrb.Top;

	result.X += static_cast<int>(deltaX * this->GetRelativeOffsetHorizontal());
	result.Y += static_cast<int>(deltaY * this->GetRelativeOffsetVertical());

	return result;
}

void Anchor::Read(INI_EX& parser, const char* pSection, const char* pFlagFormat)
{
	char flagName[0x40];

	_snprintf_s(flagName, _TRUNCATE, pFlagFormat, "Horizontal");
	this->Horizontal.Read(parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pFlagFormat, "Vertical");
	this->Vertical.Read(parser, pSection, flagName);
}

bool Anchor::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool Anchor::Save(PhobosStreamWriter& stm) const
{
	return const_cast<Anchor*>(this)->Serialize(stm);
}

template <typename T>
bool Anchor::Serialize(T& stm)
{
	return stm
		.Process(this->Horizontal)
		.Process(this->Vertical)
		.Success();
}
