#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class BlockTypeClass
{
public:

	BlockTypeClass() = default;
	BlockTypeClass(TechnoTypeClass* pOwnerType);
	BlockTypeClass(WarheadTypeClass* pOwnerType);

	ValueableVector<double> Block_Chances;
	ValueableVector<double> Block_DamageMultipliers;
	ValueableVector<double> Block_AffectBelowPercents;
	Nullable<AffectedHouse> Block_AffectsHouses;
	Nullable<bool> Block_CanActive_NoFirer;
	Nullable<bool> Block_CanActive_Powered;
	Nullable<bool> Block_CanActive_ShieldActive;
	Nullable<bool> Block_CanActive_ShieldInactive;
	Nullable<bool> Block_CanActive_ZeroDamage;
	Nullable<bool> Block_CanActive_NegativeDamage;
	Nullable<bool> Block_Flash;
	Nullable<int> Block_Flash_FixedSize;
	Nullable<bool> Block_Flash_Red;
	Nullable<bool> Block_Flash_Green;
	Nullable<bool> Block_Flash_Blue;
	Nullable<bool> Block_Flash_Black;
	ValueableVector<AnimTypeClass*> Block_Anims;
	Nullable<WeaponTypeClass*> Block_Weapon;
	Nullable<bool> Block_ReflectDamage;
	Nullable<double> Block_ReflectDamage_Chance;
	Nullable<WarheadTypeClass*> Block_ReflectDamage_Warhead;
	Nullable<bool> Block_ReflectDamage_Warhead_Detonate;
	Nullable<double> Block_ReflectDamage_Multiplier;
	Nullable<int> Block_ReflectDamage_Override;
	Nullable<AffectedHouse> Block_ReflectDamage_AffectsHouses;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};
