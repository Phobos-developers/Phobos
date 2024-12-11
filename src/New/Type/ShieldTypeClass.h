#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>
#include <Utilities/TemplateDef.h>

class ShieldTypeClass final : public Enumerable<ShieldTypeClass>
{
public:
	Valueable<int> Strength;
	Nullable<int> InitialStrength;
	Nullable<double> ConditionYellow;
	Nullable<double> ConditionRed;
	Valueable<ArmorType> Armor;
	Valueable<bool> InheritArmorFromTechno;
	Valueable<bool> Powered;
	Valueable<double> Respawn;
	Valueable<int> Respawn_Rate;
	Valueable<double> SelfHealing;
	Valueable<int> SelfHealing_Rate;
	Valueable<bool> SelfHealing_RestartInCombat;
	Valueable<int> SelfHealing_RestartInCombatDelay;

	Valueable<bool> AbsorbOverDamage;
	Valueable<int> BracketDelta;
	Valueable<AttachedAnimFlag> IdleAnim_OfflineAction;
	Valueable<AttachedAnimFlag> IdleAnim_TemporalAction;
	Damageable<AnimTypeClass*> IdleAnim;
	Damageable<AnimTypeClass*> IdleAnimDamaged;
	Valueable<AnimTypeClass*> BreakAnim;
	Valueable<AnimTypeClass*> HitAnim;
	Valueable<bool> HitFlash;
	Nullable<int> HitFlash_FixedSize;
	Valueable<bool> HitFlash_Red;
	Valueable<bool> HitFlash_Green;
	Valueable<bool> HitFlash_Blue;
	Valueable<bool> HitFlash_Black;
	Valueable<WeaponTypeClass*> BreakWeapon;
	Valueable<double> AbsorbPercent;
	Valueable<double> PassPercent;
	Valueable<int> ReceivedDamage_Minimum;
	Valueable<int> ReceivedDamage_Maximum;

	Nullable<bool> AllowTransfer;

	Valueable<Vector3D<int>> Pips;
	Nullable<SHPStruct*> Pips_Background;
	Valueable<Vector3D<int>> Pips_Building;
	Nullable<int> Pips_Building_Empty;
	Valueable<bool> ImmuneToCrit;
	Valueable<bool> ImmuneToBerserk;

	Nullable<ColorStruct> Tint_Color;
	Valueable<double> Tint_Intensity;
	Valueable<AffectedHouse> Tint_VisibleToHouses;

public:
	ShieldTypeClass(const char* const pTitle) : Enumerable<ShieldTypeClass>(pTitle)
		, Strength { 0 }
		, InitialStrength { }
		, ConditionYellow { }
		, ConditionRed { }
		, Armor { Armor::None }
		, InheritArmorFromTechno { false }
		, Powered { false }
		, Respawn { 0.0 }
		, Respawn_Rate { 0 }
		, SelfHealing { 0.0 }
		, SelfHealing_Rate { 0 }
		, SelfHealing_RestartInCombat { true }
		, SelfHealing_RestartInCombatDelay { 0 }
		, AbsorbOverDamage { false }
		, BracketDelta { 0 }
		, IdleAnim_OfflineAction { AttachedAnimFlag::Hides }
		, IdleAnim_TemporalAction { AttachedAnimFlag::Hides }
		, IdleAnim { }
		, IdleAnimDamaged { }
		, BreakAnim { }
		, HitAnim { }
		, HitFlash { false }
		, HitFlash_FixedSize {}
		, HitFlash_Red { true }
		, HitFlash_Green { true }
		, HitFlash_Blue { true }
		, HitFlash_Black { false }
		, BreakWeapon { }
		, AbsorbPercent { 1.0 }
		, PassPercent { 0.0 }
		, ReceivedDamage_Minimum { INT32_MIN }
		, ReceivedDamage_Maximum { INT32_MAX }
		, AllowTransfer { }
		, Pips { { -1,-1,-1 } }
		, Pips_Background { }
		, Pips_Building { { -1,-1,-1 } }
		, Pips_Building_Empty { }
		, ImmuneToBerserk { false }
		, ImmuneToCrit { false }
		, Tint_Color {}
		, Tint_Intensity { 0.0 }
		, Tint_VisibleToHouses { AffectedHouse::All }
	{ };

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	bool HasTint() const
	{
		return this->Tint_Color.isset() || this->Tint_Intensity != 0.0;
	}

	AnimTypeClass* GetIdleAnimType(bool isDamaged, double healthRatio) const;
	double GetConditionYellow() const;
	double GetConditionRed() const;

private:
	template <typename T>
	void Serialize(T& Stm);
};
