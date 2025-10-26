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
#include <EBolt.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class EBoltExt
{
public:
	using base_type = EBolt;

	static constexpr DWORD Canary = 0x06C28114;

	class ExtData final : public Extension<EBolt>
	{
	public:
		ColorStruct Color[3] {};
		bool Disable[3] { false };
		int Arcs { 8 };
		int BurstIndex { 0 };

		ExtData(EBolt* OwnerObject) : Extension<EBolt>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void Initialize() override { };

		virtual void InvalidatePointer(void* ptr, bool removed) override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<EBoltExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int __forceinline GetDefaultColor_Int(ConvertClass* pConvert, int idx)
	{
		if (pConvert->BytesPerPixel == 1)
			return reinterpret_cast<uint8_t*>(pConvert->PaletteData)[idx];
		else
			return reinterpret_cast<uint16_t*>(pConvert->PaletteData)[idx];
	}

	static EBolt* CreateEBolt(WeaponTypeClass* pWeapon);
	static DWORD _cdecl _EBolt_Draw_Colors(REGISTERS* R);
};
