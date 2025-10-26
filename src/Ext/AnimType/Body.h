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

#include <AnimTypeClass.h>

#include <New/Type/Affiliated/CreateUnitTypeClass.h>
#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>

enum class AttachedAnimPosition : BYTE
{
	Default = 0,
	Center = 1,
	Ground = 2
};

class AnimTypeExt
{
public:
	using base_type = AnimTypeClass;

	static constexpr DWORD Canary = 0xEEEEEEEE;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<AnimTypeClass>
	{
	public:
		CustomPalette Palette;
		std::unique_ptr<CreateUnitTypeClass> CreateUnitType;
		Valueable<int> XDrawOffset;
		Valueable<int> HideIfNoOre_Threshold;
		Nullable<bool> Layer_UseObjectLayer;
		Valueable<AttachedAnimPosition> AttachedAnimPosition;
		Valueable<WeaponTypeClass*> Weapon;
		Valueable<int> Damage_Delay;
		Valueable<bool> Damage_DealtByInvoker;
		Valueable<bool> Damage_ApplyOncePerLoop;
		Valueable<bool> Damage_ApplyFirepowerMult;
		Valueable<bool> ExplodeOnWater;
		Valueable<bool> Warhead_Detonate;
		ValueableVector<AnimTypeClass*> WakeAnim;
		NullableVector<AnimTypeClass*> SplashAnims;
		Valueable<bool> SplashAnims_PickRandom;
		Valueable<ParticleSystemTypeClass*> AttachedSystem;
		Valueable<bool> AltPalette_ApplyLighting;
		Valueable<OwnerHouseKind> MakeInfantryOwner;
		Valueable<bool> ExtraShadow;
		ValueableIdx<VocClass> DetachedReport;
		Valueable<AffectedHouse> VisibleTo;
		Valueable<bool> VisibleTo_ConsiderInvokerAsOwner;
		Valueable<bool> RestrictVisibilityIfCloaked;
		Valueable<bool> DetachOnCloak;
		Valueable<bool> ConstrainFireAnimsToCellSpots;
		Nullable<LandTypeFlags> FireAnimDisallowedLandTypes;
		Nullable<bool> AttachFireAnimsToParent;
		Nullable<int> SmallFireCount;
		ValueableVector<AnimTypeClass*> SmallFireAnims;
		ValueableVector<double> SmallFireChances;
		ValueableVector<double> SmallFireDistances;
		Valueable<int> LargeFireCount;
		ValueableVector<AnimTypeClass*> LargeFireAnims;
		ValueableVector<double> LargeFireChances;
		ValueableVector<double> LargeFireDistances;
		Nullable<bool> Crater_DestroyTiberium;

		ExtData(AnimTypeClass* OwnerObject) : Extension<AnimTypeClass>(OwnerObject)
			, Palette { CustomPalette::PaletteMode::Temperate }
			, CreateUnitType { nullptr }
			, XDrawOffset { 0 }
			, HideIfNoOre_Threshold { 0 }
			, Layer_UseObjectLayer {}
			, AttachedAnimPosition { AttachedAnimPosition::Default }
			, Weapon {}
			, Damage_Delay { 0 }
			, Damage_DealtByInvoker { false }
			, Damage_ApplyOncePerLoop { false }
			, Damage_ApplyFirepowerMult { false }
			, ExplodeOnWater { false }
			, Warhead_Detonate { false }
			, WakeAnim {}
			, SplashAnims {}
			, SplashAnims_PickRandom { false }
			, AttachedSystem {}
			, AltPalette_ApplyLighting { false }
			, MakeInfantryOwner { OwnerHouseKind::Victim }
			, ExtraShadow { true }
			, DetachedReport {}
			, VisibleTo { AffectedHouse::All }
			, VisibleTo_ConsiderInvokerAsOwner { false }
			, RestrictVisibilityIfCloaked { false }
			, DetachOnCloak { true }
			, ConstrainFireAnimsToCellSpots { true }
			, FireAnimDisallowedLandTypes {}
			, AttachFireAnimsToParent { false }
			, SmallFireCount {}
			, SmallFireAnims {}
			, SmallFireChances {}
			, SmallFireDistances {}
			, LargeFireCount { 1 }
			, LargeFireAnims {}
			, LargeFireChances {}
			, LargeFireDistances {}
			, Crater_DestroyTiberium {}
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AnimTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void ProcessDestroyAnims(UnitClass* pThis, TechnoClass* pKiller = nullptr);
};
