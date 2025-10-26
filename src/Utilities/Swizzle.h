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

// Ares has hooked the SwizzleManagerClass,
// so what we need to do is just call the original functions.

#include <type_traits>

#include <SwizzleManagerClass.h>

class PhobosSwizzle
{
public:
	/**
	* pass in the *address* of the pointer you want to have changed
	* caution, after the call *p will be NULL
	*/
	inline static HRESULT RegisterForChange(void** p)
	{
		return SwizzleManagerClass::Instance.Swizzle(p);
	}

	/**
	* the original game objects all save their `this` pointer to the save stream
	* that way they know what ptr they used and call this function with that old ptr and `this` as the new ptr
	*/
	inline static HRESULT RegisterChange(void* was, void* is)
	{
		return SwizzleManagerClass::Instance.Here_I_Am((long)was, is);
	}

	template<typename T>
	inline static void RegisterPointerForChange(T*& ptr)
	{
		auto pptr = const_cast<std::remove_cv_t<T>**>(&ptr);
		RegisterForChange(reinterpret_cast<void**>(pptr));
	}
};

struct Swizzle
{
	template <typename T>
	Swizzle(T& object)
	{
		if constexpr (std::is_pointer_v<T>)
			PhobosSwizzle::RegisterPointerForChange(object);
	}
};
