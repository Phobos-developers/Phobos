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
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/LaserTrailTypeClass.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

class BulletTypeExt
{
public:
	using base_type = BulletTypeClass;

	static constexpr DWORD Canary = 0xF00DF00D;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<BulletTypeClass>
	{
	public:
		// Valueable<int> Strength; //Use OwnerObject()->ObjectTypeClass::Strength
		Nullable<ArmorType> Armor;
		Valueable<bool> Interceptable;
		Valueable<bool> Interceptable_DeleteOnIntercept;
		Valueable<WeaponTypeClass*> Interceptable_WeaponOverride;
		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
		Nullable<double> Gravity;
		Valueable<bool> Vertical_AircraftFix;
		Nullable<bool> VerticalInitialFacing;

		TrajectoryTypePointer TrajectoryType;

		Valueable<bool> Shrapnel_AffectsGround;
		Valueable<bool> Shrapnel_AffectsBuildings;
		Valueable<bool> Shrapnel_UseWeaponTargeting;
		Nullable<bool> SubjectToLand;
		Valueable<bool> SubjectToLand_Detonate;
		Nullable<bool> SubjectToWater;
		Valueable<bool> SubjectToWater_Detonate;

		Valueable<Leptons> ClusterScatter_Min;
		Valueable<Leptons> ClusterScatter_Max;

		Valueable<bool> AAOnly;
		Valueable<bool> Arcing_AllowElevationInaccuracy;
		Valueable<WeaponTypeClass*> ReturnWeapon;
		Valueable<bool> ReturnWeapon_ApplyFirepowerMult;

		Valueable<bool> SubjectToGround;

		Valueable<bool> Splits;
		Valueable<double> AirburstSpread;
		Valueable<double> RetargetAccuracy;
		Valueable<bool> RetargetSelf;
		Valueable<double> RetargetSelf_Probability;
		Nullable<bool> AroundTarget;
		Valueable<bool> Airburst_UseCluster;
		Valueable<bool> Airburst_RandomClusters;
		Valueable<bool> Airburst_TargetAsSource;
		Valueable<bool> Airburst_TargetAsSource_SkipHeight;
		Valueable<Leptons> Splits_TargetingDistance;
		Valueable<int> Splits_TargetCellRange;
		Valueable<bool> Splits_UseWeaponTargeting;
		Valueable<bool> AirburstWeapon_ApplyFirepowerMult;
		Valueable<Leptons> AirburstWeapon_SourceScatterMin;
		Valueable<Leptons> AirburstWeapon_SourceScatterMax;

		Valueable<bool> Parachuted;
		Valueable<int> Parachuted_FallRate;
		Nullable<int> Parachuted_MaxFallRate;
		Nullable<AnimTypeClass*> BombParachute;

		Valueable<bool> AU;

		// Ares 0.7
		Nullable<Leptons> BallisticScatter_Min;
		Nullable<Leptons> BallisticScatter_Max;

		ExtData(BulletTypeClass* OwnerObject) : Extension<BulletTypeClass>(OwnerObject)
			, Armor {}
			, Interceptable { false }
			, Interceptable_DeleteOnIntercept { false }
			, Interceptable_WeaponOverride {}
			, LaserTrail_Types {}
			, Gravity {}
			, Vertical_AircraftFix { true }
			, VerticalInitialFacing {}
			, TrajectoryType { }
			, Shrapnel_AffectsGround { false }
			, Shrapnel_AffectsBuildings { false }
			, Shrapnel_UseWeaponTargeting { false }
			, ClusterScatter_Min { Leptons(256) }
			, ClusterScatter_Max { Leptons(512) }
			, BallisticScatter_Min {}
			, BallisticScatter_Max {}
			, SubjectToLand {}
			, SubjectToLand_Detonate { true }
			, SubjectToWater {}
			, SubjectToWater_Detonate { true }
			, AAOnly { false }
			, Arcing_AllowElevationInaccuracy { true }
			, ReturnWeapon {}
			, ReturnWeapon_ApplyFirepowerMult { false }
			, SubjectToGround { false }
			, Splits { false }
			, AirburstSpread { 1.5 }
			, RetargetAccuracy { 0.0 }
			, RetargetSelf { true }
			, RetargetSelf_Probability { 0.5 }
			, AroundTarget {}
			, Airburst_UseCluster { false }
			, Airburst_RandomClusters { false }
			, Airburst_TargetAsSource { false }
			, Airburst_TargetAsSource_SkipHeight { false }
			, Splits_TargetingDistance{ Leptons(1280) }
			, Splits_TargetCellRange { 3 }
			, Splits_UseWeaponTargeting { false }
			, AirburstWeapon_ApplyFirepowerMult { false }
			, AirburstWeapon_SourceScatterMin { Leptons(0) }
			, AirburstWeapon_SourceScatterMax { Leptons(0) }
			, Parachuted { false }
			, Parachuted_FallRate { 1 }
			, Parachuted_MaxFallRate {}
			, BombParachute {}
			, AU { false }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		// virtual void Initialize() override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);

		void TrajectoryValidation() const;
	};

	class ExtContainer final : public Container<BulletTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static double GetAdjustedGravity(BulletTypeClass* pType);
	static BulletTypeClass* GetDefaultBulletType();
};
