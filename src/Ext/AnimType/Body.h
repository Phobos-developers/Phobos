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
		Nullable<int> Translucency_Cloaked;
		Valueable<double> Translucent_Stage1_Percent;
		Nullable<int> Translucent_Stage1_Frame;
		Valueable<TranslucencyLevel> Translucent_Stage1_Translucency;
		Valueable<double> Translucent_Stage2_Percent;
		Nullable<int> Translucent_Stage2_Frame;
		Valueable<TranslucencyLevel> Translucent_Stage2_Translucency;
		Valueable<double> Translucent_Stage3_Percent;
		Nullable<int> Translucent_Stage3_Frame;
		Valueable<TranslucencyLevel> Translucent_Stage3_Translucency;
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
			, Translucency_Cloaked {}
			, Translucent_Stage1_Percent { 0.2 }
			, Translucent_Stage1_Frame {}
			, Translucent_Stage1_Translucency { 25 }
			, Translucent_Stage2_Percent { 0.4 }
			, Translucent_Stage2_Frame {}
			, Translucent_Stage2_Translucency { 50 }
			, Translucent_Stage3_Percent { 0.6 }
			, Translucent_Stage3_Frame {}
			, Translucent_Stage3_Translucency { 75 }
			, ConstrainFireAnimsToCellSpots { true }
			, FireAnimDisallowedLandTypes {}
			, AttachFireAnimsToParent {}
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
