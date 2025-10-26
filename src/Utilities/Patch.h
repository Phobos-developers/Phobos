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
#include <windows.h>
#include <initializer_list>

// no more than 8 characters
#define PATCH_SECTION_NAME ".patch"
#pragma section(PATCH_SECTION_NAME, read)

#pragma pack(push, 1)
#pragma warning(push)
#pragma warning(disable : 4324)
struct __declspec(novtable) Patch
{
	DWORD offset;
	DWORD size;
	byte* pData;

	void Apply();

	// static
	static void ApplyStatic();

	template <typename T>
	static void Apply_TYPED(DWORD offset, std::initializer_list<T> data)
	{
		Patch patch = { offset, data.size() * sizeof(T), const_cast<byte*>(reinterpret_cast<const byte*>(data.begin())) };
		patch.Apply();
	};

	template <size_t Size>
	static inline void Apply_RAW(DWORD offset, const char(&str)[Size])
	{
		Patch patch = { offset, Size, (byte*)str };
		patch.Apply();
	};

	static inline void Apply_RAW(DWORD offset, std::initializer_list<byte> data)
	{
		Apply_TYPED<byte>(offset, data);
	};

	static void Apply_LJMP(DWORD offset, DWORD pointer);
	static inline void Apply_LJMP(DWORD offset, void* pointer)
	{
		Apply_LJMP(offset, reinterpret_cast<DWORD>(pointer));
	};

	static void Apply_CALL(DWORD offset, DWORD pointer);
	static inline void Apply_CALL(DWORD offset, void* pointer)
	{
		Apply_CALL(offset, reinterpret_cast<DWORD>(pointer));
	};

	static void Apply_CALL6(DWORD offset, DWORD pointer);
	static inline void Apply_CALL6(DWORD offset, void* pointer)
	{
		Apply_CALL6(offset, reinterpret_cast<DWORD>(pointer));
	};

	static inline void Apply_VTABLE(DWORD offset, DWORD pointer)
	{
		Patch::Apply_TYPED<DWORD>(offset, { pointer });
	};

	static inline void Apply_VTABLE(DWORD offset, void* pointer)
	{
		Apply_OFFSET(offset, reinterpret_cast<DWORD>(pointer));
	};

	static inline void Apply_OFFSET(DWORD offset, DWORD pointer)
	{
		Apply_VTABLE(offset, pointer);
	};

	static inline void Apply_OFFSET(DWORD offset, void* pointer)
	{
		Apply_VTABLE(offset, pointer);
	};
};
#pragma warning(pop)
#pragma pack(pop)
