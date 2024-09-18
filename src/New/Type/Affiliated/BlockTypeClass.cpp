#include "BlockTypeClass.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

void BlockTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->Block_Chances.Read(exINI, pSection, "Block.Chances");
	this->Block_DamageMultipliers.Read(exINI, pSection, "Block.DamageMultipliers");
	this->Block_AffectBelowPercents.Read(exINI, pSection, "Block.AffectBelowPercents");
	this->Block_AffectsHouses.Read(exINI, pSection, "Block.AffectsHouses");
	this->Block_CanActive_NoFirer.Read(exINI, pSection, "Block.CanActive.NoFirer");
	this->Block_CanActive_Powered.Read(exINI, pSection, "Block.CanActive.Powered");
	this->Block_CanActive_ShieldActive.Read(exINI, pSection, "Block.CanActive.ShieldActive");
	this->Block_CanActive_ShieldInactive.Read(exINI, pSection, "Block.CanActive.ShieldInactive");
	this->Block_CanActive_ZeroDamage.Read(exINI, pSection, "Block.CanActive.ZeroDamage");
	this->Block_CanActive_NegativeDamage.Read(exINI, pSection, "Block.CanActive.NegativeDamage");
	this->Block_CanActive_Move.Read(exINI, pSection, "Block.CanActive.Move");
	this->Block_CanActive_Stationary.Read(exINI, pSection, "Block.CanActive.Stationary");
	this->Block_Flash.Read(exINI, pSection, "Block.Flash");
	this->Block_Flash_FixedSize.Read(exINI, pSection, "Block.Flash.FixedSize");
	this->Block_Flash_Red.Read(exINI, pSection, "Block.Flash.Red");
	this->Block_Flash_Green.Read(exINI, pSection, "Block.Flash.Green");
	this->Block_Flash_Blue.Read(exINI, pSection, "Block.Flash.Blue");
	this->Block_Flash_Black.Read(exINI, pSection, "Block.Flash.Black");
	this->Block_Anims.Read(exINI, pSection, "Block.Anims");
	this->Block_Weapon.Read(exINI, pSection, "Block.Weapon");
	this->Block_ReflectDamage.Read(exINI, pSection, "Block.ReflectDamage");
	this->Block_ReflectDamage_Chance.Read(exINI, pSection, "Block.ReflectDamage.Chance");
	this->Block_ReflectDamage_Warhead.Read(exINI, pSection, "Block.ReflectDamage.Warhead");
	this->Block_ReflectDamage_Warhead_Detonate.Read(exINI, pSection, "Block.ReflectDamage.Warhead.Detonate");
	this->Block_ReflectDamage_Multiplier.Read(exINI, pSection, "Block.ReflectDamage.Multiplier");
	this->Block_ReflectDamage_Override.Read(exINI, pSection, "Block.ReflectDamage.Override");
	this->Block_ReflectDamage_AffectsHouses.Read(exINI, pSection, "Block.ReflectDamage.AffectsHouses");
}

#pragma region(save/load)

template <class T>
bool BlockTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->Block_Chances)
		.Process(this->Block_DamageMultipliers)
		.Process(this->Block_AffectBelowPercents)
		.Process(this->Block_AffectsHouses)
		.Process(this->Block_CanActive_NoFirer)
		.Process(this->Block_CanActive_Powered)
		.Process(this->Block_CanActive_ShieldActive)
		.Process(this->Block_CanActive_ShieldInactive)
		.Process(this->Block_CanActive_ZeroDamage)
		.Process(this->Block_CanActive_NegativeDamage)
		.Process(this->Block_CanActive_Move)
		.Process(this->Block_CanActive_Stationary)
		.Process(this->Block_Flash)
		.Process(this->Block_Flash_FixedSize)
		.Process(this->Block_Flash_Red)
		.Process(this->Block_Flash_Green)
		.Process(this->Block_Flash_Blue)
		.Process(this->Block_Flash_Black)
		.Process(this->Block_Anims)
		.Process(this->Block_Weapon)
		.Process(this->Block_ReflectDamage)
		.Process(this->Block_ReflectDamage_Chance)
		.Process(this->Block_ReflectDamage_Warhead)
		.Process(this->Block_ReflectDamage_Warhead_Detonate)
		.Process(this->Block_ReflectDamage_Multiplier)
		.Process(this->Block_ReflectDamage_AffectsHouses)
		.Process(this->Block_ReflectDamage_Override)
		.Success();
}

bool BlockTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool BlockTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<BlockTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)
