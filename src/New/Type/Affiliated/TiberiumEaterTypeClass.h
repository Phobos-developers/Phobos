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

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class TiberiumEaterTypeClass
{
public:
	Valueable<int> TransDelay { -1 };
	Valueable<float>  CashMultiplier { 1.0 };
	Valueable<int> AmountPerCell { 0 };
	std::vector<Vector2D<int>> Cells { std::vector<Vector2D<int>>(1) };
	Valueable<bool> Display { true };
	Valueable<AffectedHouse> DisplayToHouse { AffectedHouse::All };
	Valueable<Point2D> DisplayOffset { Point2D::Empty };
	ValueableVector<AnimTypeClass*> Anims {};
	NullableVector<AnimTypeClass*> Anims_Tiberiums[4] {};
	Valueable<bool> AnimMove { true };

	TiberiumEaterTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};
