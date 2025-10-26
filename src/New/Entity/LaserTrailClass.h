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

#include <GeneralStructures.h>
#include <HouseClass.h>

#include <New/Type/LaserTrailTypeClass.h>


class LaserTrailClass
{
public:
	LaserTrailTypeClass* Type;
	bool Visible;
	bool Cloaked;
	CoordStruct FLH;
	bool IsOnTurret;
	ColorStruct CurrentColor;
	Nullable<CoordStruct> LastLocation;
	bool Intrinsic;

	LaserTrailClass(LaserTrailTypeClass* pTrailType, HouseClass* pHouse = nullptr,
		CoordStruct flh = { 0, 0, 0 }, bool isOnTurret = false) :
		Type { pTrailType }
		, Visible { true }
		, Cloaked { false }
		, FLH { flh }
		, IsOnTurret { isOnTurret }
		, CurrentColor { pTrailType->Color }
		, LastLocation {}
		, Intrinsic { true }
	{
		if (this->Type->IsHouseColor && pHouse)
			this->CurrentColor = pHouse->LaserColor;
	}

	LaserTrailClass() :
		Type {},
		Visible {},
		Cloaked {},
		FLH {},
		IsOnTurret {},
		CurrentColor {},
		LastLocation {},
		Intrinsic {}
	{ }

	bool Update(CoordStruct location);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};
