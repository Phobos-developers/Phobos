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
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TiberiumClass.h>
#include <FactoryClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Misc/FlyingStrings.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

class BuildingExt
{
public:
	using base_type = BuildingClass;

	static constexpr DWORD Canary = 0x87654321;
	static constexpr size_t ExtPointerOffset = 0x6FC;
	static constexpr bool ShouldConsiderInvalidatePointer = true;

	class ExtData final : public Extension<BuildingClass>
	{
	public:
		BuildingTypeExt::ExtData* TypeExtData;
		TechnoExt::ExtData* TechnoExtData;
		bool DeployedTechno;
		bool IsCreatedFromMapFile;
		int LimboID;
		int GrindingWeapon_LastFiredFrame;
		int GrindingWeapon_AccumulatedCredits;
		BuildingClass* CurrentAirFactory;
		int AccumulatedIncome;
		std::optional<int> CurrentLaserWeaponIndex;
		int PoweredUpToLevel; // Distinct from UpgradeLevel, and set to highest PowersUpToLevel out of applied upgrades regardless of how many are currently applied to this building.
		SuperClass* CurrentEMPulseSW;

		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject)
			, TypeExtData { nullptr }
			, TechnoExtData { nullptr }
			, DeployedTechno { false }
			, IsCreatedFromMapFile { false }
			, LimboID { -1 }
			, GrindingWeapon_LastFiredFrame { 0 }
			, GrindingWeapon_AccumulatedCredits { 0 }
			, CurrentAirFactory { nullptr }
			, AccumulatedIncome { 0 }
			, CurrentLaserWeaponIndex {}
			, PoweredUpToLevel { 0 }
			, CurrentEMPulseSW {}
		{ }

		void DisplayIncomeString();
		void ApplyPoweredKillSpawns();
		bool HasSuperWeapon(int index, bool withUpgrades) const;
		bool HandleInfiltrate(HouseClass* pInfiltratorHouse, int moneybefore);
		void UpdatePrimaryFactoryAI();
		virtual ~ExtData() = default;

		// virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(CurrentAirFactory, ptr);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();

			switch (abs)
			{
			case AbstractType::Building:
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType);

	static int CountOccupiedDocks(BuildingClass* pBuilding);
	static bool HasFreeDocks(BuildingClass* pBuilding);
	static bool CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno);
	static bool DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int refund);
	static bool CanUndeployOnSell(BuildingClass* pThis);
	static void KickOutStuckUnits(BuildingClass* pThis);
	static const std::vector<CellStruct> GetFoundationCells(BuildingClass* pThis, CellStruct baseCoords, bool includeOccupyHeight = false);
};
