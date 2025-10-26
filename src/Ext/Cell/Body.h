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
#include <CellClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>

class CellExt
{
public:
	using base_type = CellClass;

	static constexpr DWORD Canary = 0x13371337;
	static constexpr size_t ExtPointerOffset = 0x144;

	struct RadLevel
	{
		RadSiteClass* Rad { nullptr };
		int Level { 0 };

		RadLevel() = default;
		RadLevel(RadSiteClass* pRad, int level) : Rad(pRad), Level(level)
		{ }

		bool Load(PhobosStreamReader& stm, bool registerForChange);
		bool Save(PhobosStreamWriter& stm) const;

	private:
		template <typename T>
		bool Serialize(T& stm);
	};

	class ExtData final : public Extension<CellClass>
	{
	public:
		std::vector<RadSiteClass*> RadSites {};
		std::vector<RadLevel> RadLevels { };
		int InfantryCount{ 0 };

		ExtData(CellClass* OwnerObject) : Extension<CellClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool removed) override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<CellExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override;
	};

	static ExtContainer ExtMap;
};
