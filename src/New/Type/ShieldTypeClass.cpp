#include "ShieldTypeClass.h"

template<>
const char* Enumerable<ShieldTypeClass>::GetMainSection()
{
	return "ShieldTypes";
}

AnimTypeClass* ShieldTypeClass::GetIdleAnimType(bool isDamaged, double healthRatio) const
{
	auto damagedAnim = this->IdleAnimDamaged.Get(healthRatio);

	if (isDamaged && damagedAnim)
		return damagedAnim;
	else
		return this->IdleAnim.Get(healthRatio, this->GetConditionYellow(), this->GetConditionRed());
}

double ShieldTypeClass::GetConditionYellow() const
{
	return this->ConditionYellow.Get(RulesExt::Global()->Shield_ConditionYellow.Get(RulesClass::Instance->ConditionYellow));
}

double ShieldTypeClass::GetConditionRed() const
{
	return this->ConditionRed.Get(RulesExt::Global()->Shield_ConditionRed.Get(RulesClass::Instance->ConditionRed));
}

void ShieldTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);

	this->Strength.Read(exINI, pSection, "Strength");
	this->InitialStrength.Read(exINI, pSection, "InitialStrength");
	this->ConditionYellow.Read(exINI, pSection, "ConditionYellow");
	this->ConditionRed.Read(exINI, pSection, "ConditionRed");
	this->Armor.Read(exINI, pSection, "Armor");
	this->InheritArmorFromTechno.Read(exINI, pSection, "InheritArmorFromTechno");
	this->Powered.Read(exINI, pSection, "Powered");

	this->Respawn.Read(exINI, pSection, "Respawn");
	Nullable<double> Respawn_Rate__InMinutes;
	Respawn_Rate__InMinutes.Read(exINI, pSection, "Respawn.Rate");
	if (Respawn_Rate__InMinutes.isset())
		this->Respawn_Rate = (int)(Respawn_Rate__InMinutes.Get() * 900);

	this->SelfHealing.Read(exINI, pSection, "SelfHealing");
	Nullable<double> SelfHealing_Rate__InMinutes;
	SelfHealing_Rate__InMinutes.Read(exINI, pSection, "SelfHealing.Rate");
	if (SelfHealing_Rate__InMinutes.isset())
		this->SelfHealing_Rate = (int)(SelfHealing_Rate__InMinutes.Get() * 900);

	this->SelfHealing_RestartInCombat.Read(exINI, pSection, "SelfHealing.RestartInCombat");
	this->SelfHealing_RestartInCombatDelay.Read(exINI, pSection, "SelfHealing.RestartInCombatDelay");

	this->AbsorbOverDamage.Read(exINI, pSection, "AbsorbOverDamage");
	this->BracketDelta.Read(exINI, pSection, "BracketDelta");
	this->ReceivedDamage_Minimum.Read(exINI, pSection, "ReceivedDamage.Minimum");
	this->ReceivedDamage_Maximum.Read(exINI, pSection, "ReceivedDamage.Maximum");

	this->IdleAnim_OfflineAction.Read(exINI, pSection, "IdleAnim.OfflineAction");
	this->IdleAnim_TemporalAction.Read(exINI, pSection, "IdleAnim.TemporalAction");

	this->IdleAnim.Read(exINI, pSection, "IdleAnim.%s");
	this->IdleAnimDamaged.Read(exINI, pSection, "IdleAnimDamaged.%s");

	this->BreakAnim.Read(exINI, pSection, "BreakAnim");
	this->HitAnim.Read(exINI, pSection, "HitAnim");
	this->HitFlash.Read(exINI, pSection, "HitFlash");
	this->HitFlash_FixedSize.Read(exINI, pSection, "HitFlash.FixedSize");
	this->HitFlash_Red.Read(exINI, pSection, "HitFlash.Red");
	this->HitFlash_Green.Read(exINI, pSection, "HitFlash.Green");
	this->HitFlash_Blue.Read(exINI, pSection, "HitFlash.Blue");
	this->HitFlash_Black.Read(exINI, pSection, "HitFlash.Black");
	this->BreakWeapon.Read<true>(exINI, pSection, "BreakWeapon");

	this->AbsorbPercent.Read(exINI, pSection, "AbsorbPercent");
	this->PassPercent.Read(exINI, pSection, "PassPercent");

	this->AllowTransfer.Read(exINI, pSection, "AllowTransfer");

	this->Pips.Read(exINI, pSection, "Pips");
	this->Pips_Background.Read(exINI, pSection, "Pips.Background");
	this->Pips_Building.Read(exINI, pSection, "Pips.Building");
	this->Pips_Building_Empty.Read(exINI, pSection, "Pips.Building.Empty");

	this->ImmuneToBerserk.Read(exINI, pSection, "ImmuneToBerserk");
	this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");

	this->Tint_Color.Read(exINI, pSection, "Tint.Color");
	this->Tint_Intensity.Read(exINI, pSection, "Tint.Intensity");
	this->Tint_VisibleToHouses.Read(exINI, pSection, "Tint.VisibleToHouses");
}

template <typename T>
void ShieldTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Strength)
		.Process(this->InitialStrength)
		.Process(this->ConditionYellow)
		.Process(this->ConditionRed)
		.Process(this->Armor)
		.Process(this->InheritArmorFromTechno)
		.Process(this->Powered)
		.Process(this->Respawn)
		.Process(this->Respawn_Rate)
		.Process(this->SelfHealing)
		.Process(this->SelfHealing_Rate)
		.Process(this->SelfHealing_RestartInCombat)
		.Process(this->SelfHealing_RestartInCombatDelay)
		.Process(this->AbsorbOverDamage)
		.Process(this->BracketDelta)
		.Process(this->ReceivedDamage_Minimum)
		.Process(this->ReceivedDamage_Maximum)
		.Process(this->IdleAnim_OfflineAction)
		.Process(this->IdleAnim_TemporalAction)
		.Process(this->IdleAnim)
		.Process(this->BreakAnim)
		.Process(this->HitAnim)
		.Process(this->HitFlash)
		.Process(this->HitFlash_FixedSize)
		.Process(this->HitFlash_Red)
		.Process(this->HitFlash_Green)
		.Process(this->HitFlash_Blue)
		.Process(this->HitFlash_Black)
		.Process(this->BreakWeapon)
		.Process(this->AbsorbPercent)
		.Process(this->PassPercent)
		.Process(this->AllowTransfer)
		.Process(this->Pips)
		.Process(this->Pips_Background)
		.Process(this->Pips_Building)
		.Process(this->Pips_Building_Empty)
		.Process(this->ImmuneToBerserk)
		.Process(this->ImmuneToCrit)
		.Process(this->Tint_Color)
		.Process(this->Tint_Intensity)
		.Process(this->Tint_VisibleToHouses)
		;
}

void ShieldTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void ShieldTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
