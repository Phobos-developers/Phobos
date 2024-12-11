#pragma once

#include <AnimTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
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
		Valueable<TechnoTypeClass*> CreateUnit;
		Valueable<DirType> CreateUnit_Facing;
		Valueable<bool> CreateUnit_InheritDeathFacings;
		Valueable<bool> CreateUnit_InheritTurretFacings;
		Valueable<bool> CreateUnit_RemapAnim;
		Valueable<bool> CreateUnit_RandomFacing;
		Valueable<Mission> CreateUnit_Mission;
		Nullable<Mission> CreateUnit_AIMission;
		Valueable<OwnerHouseKind> CreateUnit_Owner;
		Valueable<bool> CreateUnit_AlwaysSpawnOnGround;
		Valueable<bool> CreateUnit_SpawnParachutedInAir;
		Valueable<bool> CreateUnit_ConsiderPathfinding;
		Valueable<AnimTypeClass*> CreateUnit_SpawnAnim;
		Valueable<int> CreateUnit_SpawnHeight;
		Valueable<int> XDrawOffset;
		Valueable<int> HideIfNoOre_Threshold;
		Nullable<bool> Layer_UseObjectLayer;
		Valueable<bool> UseCenterCoordsIfAttached;
		Valueable<WeaponTypeClass*> Weapon;
		Valueable<int> Damage_Delay;
		Valueable<bool> Damage_DealtByInvoker;
		Valueable<bool> Damage_ApplyOncePerLoop;
		Valueable<bool> ExplodeOnWater;
		Valueable<bool> Warhead_Detonate;
		Valueable<AnimTypeClass*> WakeAnim;
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

		ExtData(AnimTypeClass* OwnerObject) : Extension<AnimTypeClass>(OwnerObject)
			, Palette { CustomPalette::PaletteMode::Temperate }
			, CreateUnit_Facing { DirType::North }
			, CreateUnit_RandomFacing { true }
			, CreateUnit_InheritDeathFacings { false }
			, CreateUnit_InheritTurretFacings { false }
			, CreateUnit_RemapAnim { false }
			, CreateUnit_Mission { Mission::Guard }
			, CreateUnit_AIMission {}
			, CreateUnit_Owner { OwnerHouseKind::Victim }
			, CreateUnit_AlwaysSpawnOnGround { false }
			, CreateUnit_SpawnParachutedInAir { false }
			, CreateUnit_ConsiderPathfinding { false }
			, CreateUnit_SpawnAnim {}
			, CreateUnit_SpawnHeight { -1 }
			, XDrawOffset { 0 }
			, HideIfNoOre_Threshold { 0 }
			, Layer_UseObjectLayer {}
			, UseCenterCoordsIfAttached { false }
			, Weapon {}
			, Damage_Delay { 0 }
			, Damage_DealtByInvoker { false }
			, Damage_ApplyOncePerLoop { false }
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
