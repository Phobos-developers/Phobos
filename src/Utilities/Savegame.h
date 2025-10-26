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

#include "Stream.h"

#include <memory>
#include <type_traits>

namespace Savegame
{
	template <typename T>
	bool ReadPhobosStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange = true);

	template <typename T>
	bool WritePhobosStream(PhobosStreamWriter& Stm, const T& Value);

	template <typename T>
	T* RestoreObject(PhobosStreamReader& Stm, bool RegisterForChange = true);

	template <typename T>
	bool PersistObject(PhobosStreamWriter& Stm, const T* pValue);

	template <typename T>
	struct PhobosStreamObject
	{
		bool ReadFromStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange) const;
		bool WriteToStream(PhobosStreamWriter& Stm, const T& Value) const;
	};

	template <typename T>
	struct ObjectFactory
	{
		std::unique_ptr<T> operator() (PhobosStreamReader& Stm) const
		{
			return std::make_unique<T>();
		}
	};
}
