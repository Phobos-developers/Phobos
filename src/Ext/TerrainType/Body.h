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
#include <TerrainTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>

class TerrainTypeExt
{
public:
	using base_type = TerrainTypeClass;

	static constexpr DWORD Canary = 0xBEE78007;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<TerrainTypeClass>
	{
	public:
		Valueable<int> SpawnsTiberium_Type;
		Valueable<int> SpawnsTiberium_Range;
		Valueable<PartialVector2D<int>> SpawnsTiberium_GrowthStage;
		Valueable<PartialVector2D<int>> SpawnsTiberium_CellsPerAnim;
		ValueableVector<AnimTypeClass*> DestroyAnim;
		ValueableIdx<VocClass> DestroySound;
		Nullable<ColorStruct> MinimapColor;
		Valueable<bool> IsPassable;
		Valueable<bool> CanBeBuiltOn;
		Valueable<bool> HasDamagedFrames;
		Valueable<bool> HasCrumblingFrames;
		ValueableIdx<VocClass> CrumblingSound;
		Nullable<int> AnimationLength;

		PhobosFixedString<32u> PaletteFile;
		DynamicVectorClass<ColorScheme*>* Palette; // Intentionally not serialized - rebuilt from the palette file on load.

		ExtData(TerrainTypeClass* OwnerObject) : Extension<TerrainTypeClass>(OwnerObject)
			, SpawnsTiberium_Type { 0 }
			, SpawnsTiberium_Range { 1 }
			, SpawnsTiberium_GrowthStage { { 3, 0 } }
			, SpawnsTiberium_CellsPerAnim { { 1, 0 } }
			, DestroyAnim {}
			, DestroySound {}
			, MinimapColor {}
			, IsPassable { false }
			, CanBeBuiltOn { false }
			, HasDamagedFrames { false }
			, HasCrumblingFrames { false }
			, CrumblingSound {}
			, AnimationLength {}
			, PaletteFile {}
			, Palette {}
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		int GetTiberiumGrowthStage();
		int GetCellsPerAnim();
		void PlayDestroyEffects(const CoordStruct& coords);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TerrainTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void Remove(TerrainClass* pTerrain);
};
