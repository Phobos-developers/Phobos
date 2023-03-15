#include "AttachEffectTypeClass.h"

Enumerable<AttachEffectTypeClass>::container_t Enumerable<AttachEffectTypeClass>::Array;

bool AttachEffectTypeClass::HasTint()
{
	return this->Tint_Color.isset() || this->Tint_Intensity != 0.0;
}

const char* Enumerable<AttachEffectTypeClass>::GetMainSection()
{
	return "AttachEffectTypes";
}

void AttachEffectTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;

	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);

	this->Duration.Read(exINI, pSection, "Duration");
	this->Cumulative.Read(exINI, pSection, "Cumulative");
	this->Cumulative_MaxCount.Read(exINI, pSection, "Cumulative.MaxCount");
	this->Powered.Read(exINI, pSection, "Powered");
	this->DiscardOnEntry.Read(exINI, pSection, "DiscardOnEntry");
	this->PenetratesIronCurtain.Read(exINI, pSection, "PenetratesIronCurtain");

	this->Animation.Read(exINI, pSection, "Animation");
	this->Animation_ResetOnReapply.Read(exINI, pSection, "Animation.ResetOnReapply");
	this->Animation_OfflineAction.Read(exINI, pSection, "Animation.OfflineAction");
	this->Animation_TemporalAction.Read(exINI, pSection, "Animation.TemporalAction");
	this->Animation_UseInvokerAsOwner.Read(exINI, pSection, "Animation.UseInvokerAsOwner");

	this->Tint_Color.Read(exINI, pSection, "Tint.Color");
	this->Tint_Intensity.Read(exINI, pSection, "Tint.Intensity");
	this->Tint_VisibleToHouses.Read(exINI, pSection, "Tint.VisibleToHouses");

	this->FirepowerMultiplier.Read(exINI, pSection, "FirepowerMultiplier");
	this->ArmorMultiplier.Read(exINI, pSection, "ArmorMultiplier");
	this->SpeedMultiplier.Read(exINI, pSection, "SpeedMultiplier");
	this->ROFMultiplier.Read(exINI, pSection, "ROFMultiplier");
	this->ROFMultiplier_ApplyOnCurrentTimer.Read(exINI, pSection, "ROFMultiplier.ApplyOnCurrentTimer");

	this->Cloakable.Read(exINI, pSection, "Cloakable");
	this->ForceDecloak.Read(exINI, pSection, "ForceDecloak");

	this->WeaponRange_Multiplier.Read(exINI, pSection, "WeaponRange.Multiplier");
	this->WeaponRange_ExtraRange.Read(exINI, pSection, "WeaponRange.ExtraRange");
	this->WeaponRange_AllowWeapons.Read(exINI, pSection, "WeaponRange.AllowWeapons");
	this->WeaponRange_DisallowWeapons.Read(exINI, pSection, "WeaponRange.DisallowWeapons");

	this->Crit_Multiplier.Read(exINI, pSection, "Crit.Multiplier");
	this->Crit_ExtraChance.Read(exINI, pSection, "Crit.ExtraChance");
	this->Crit_AllowWarheads.Read(exINI, pSection, "Crit.AllowWarheads");
	this->Crit_DisallowWarheads.Read(exINI, pSection, "Crit.DisallowWarheads");

	this->RevengeWeapon.Read(exINI, pSection, "RevengeWeapon");
	this->RevengeWeapon_AffectsHouses.Read(exINI, pSection, "RevengeWeapon.AffectsHouses");
}

template <typename T>
void AttachEffectTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Duration)
		.Process(this->Cumulative)
		.Process(this->Cumulative_MaxCount)
		.Process(this->Powered)
		.Process(this->DiscardOnEntry)
		.Process(this->PenetratesIronCurtain)
		.Process(this->Animation)
		.Process(this->Animation_ResetOnReapply)
		.Process(this->Animation_OfflineAction)
		.Process(this->Animation_TemporalAction)
		.Process(this->Animation_UseInvokerAsOwner)
		.Process(this->Tint_Color)
		.Process(this->Tint_Intensity)
		.Process(this->Tint_VisibleToHouses)
		.Process(this->FirepowerMultiplier)
		.Process(this->ArmorMultiplier)
		.Process(this->SpeedMultiplier)
		.Process(this->ROFMultiplier)
		.Process(this->ROFMultiplier_ApplyOnCurrentTimer)
		.Process(this->Cloakable)
		.Process(this->ForceDecloak)
		.Process(this->WeaponRange_Multiplier)
		.Process(this->WeaponRange_ExtraRange)
		.Process(this->WeaponRange_AllowWeapons)
		.Process(this->WeaponRange_DisallowWeapons)
		.Process(this->Crit_Multiplier)
		.Process(this->Crit_ExtraChance)
		.Process(this->Crit_AllowWarheads)
		.Process(this->Crit_DisallowWarheads)
		.Process(this->RevengeWeapon)
		.Process(this->RevengeWeapon_AffectsHouses)
		;
}

void AttachEffectTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void AttachEffectTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
