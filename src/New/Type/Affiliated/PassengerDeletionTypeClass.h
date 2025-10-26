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

class PassengerDeletionTypeClass
{
public:

	PassengerDeletionTypeClass() = default;

	PassengerDeletionTypeClass(TechnoTypeClass* pOwnerType);

	TechnoTypeClass* OwnerType;

	Valueable<int> Rate;
	Valueable<bool> Rate_SizeMultiply;
	Valueable<bool> UseCostAsRate;
	Valueable<double> CostMultiplier;
	Nullable<int> CostRateCap;
	Valueable<AffectedHouse> AllowedHouses;
	Valueable<bool> DontScore;
	Valueable<bool> Soylent;
	Valueable<double> SoylentMultiplier;
	Valueable<AffectedHouse> SoylentAllowedHouses;
	Valueable<bool> DisplaySoylent;
	Valueable<AffectedHouse> DisplaySoylentToHouses;
	Valueable<Point2D> DisplaySoylentOffset;
	ValueableIdx<VocClass> ReportSound;
	ValueableVector<AnimTypeClass*> Anim;
	Valueable<bool> UnderEMP;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static std::pair<bool, bool> CanParse(INI_EX exINI, const char* pSection);

private:

	template <typename T>
	bool Serialize(T& stm);
};
