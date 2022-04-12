#include "ShieldTypeClass.h"

Enumerable<ShieldTypeClass>::container_t Enumerable<ShieldTypeClass>::Array;

const char* Enumerable<ShieldTypeClass>::GetMainSection()
{
	return "ShieldTypes";
}

void ShieldTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);

	this->Strength.Read(exINI, pSection, "Strength");
	this->InitialStrength.Read(exINI, pSection, "InitialStrength");
	this->Armor.Read(exINI, pSection, "Armor");
	this->Powered.Read(exINI, pSection, "Powered");

	this->Respawn.Read(exINI, pSection, "Respawn");
	this->Respawn_Rate__InMinutes.Read(exINI, pSection, "Respawn.Rate");
	this->Respawn_Rate = (int)(this->Respawn_Rate__InMinutes * 900);

	this->SelfHealing.Read(exINI, pSection, "SelfHealing");
	this->SelfHealing_Rate__InMinutes.Read(exINI, pSection, "SelfHealing.Rate");
	this->SelfHealing_Rate = (int)(this->SelfHealing_Rate__InMinutes * 900);

	this->AbsorbOverDamage.Read(exINI, pSection, "AbsorbOverDamage");
	this->BracketDelta.Read(exINI, pSection, "BracketDelta");

	this->IdleAnim_OfflineAction.Read(exINI, pSection, "IdleAnim.OfflineAction");
	this->IdleAnim_TemporalAction.Read(exINI, pSection, "IdleAnim.TemporalAction");

	this->IdleAnim.Read(exINI, pSection, "IdleAnim");
	if (this->IdleAnim.Get() && this->IdleAnim->Bouncer)
	{
		Debug::Log("[Developer Warning]ShieldTypes don't support Bouncer=yes anims: [%s]IdleAnim=%s\r\n", pSection, this->IdleAnim->get_ID());
		this->IdleAnim.Reset();
	}

	this->BreakAnim.Read(exINI, pSection, "BreakAnim");
	this->HitAnim.Read(exINI, pSection, "HitAnim");
	this->BreakWeapon.Read(exINI, pSection, "BreakWeapon", true);

	this->AbsorbPercent.Read(exINI, pSection, "AbsorbPercent");
	this->PassPercent.Read(exINI, pSection, "PassPercent");

	this->AllowTransfer.Read(exINI, pSection, "AllowTransfer");

	this->ShieldValue_Show.Read(exINI, pSection, "ShieldValue.Show");
	this->ShieldValue_ShowColorHigh.Read(exINI, pSection, "ShieldValue.ShowColorHigh");
	this->ShieldValue_ShowColorMid.Read(exINI, pSection, "ShieldValue.ShowColorMid");
	this->ShieldValue_ShowColorLow.Read(exINI, pSection, "ShieldValue.ShowColorLow");
	this->ShieldValue_ShowOffset.Read(exINI, pSection, "ShieldValue.ShowOffset");
	this->ShieldValue_ShowBackground.Read(exINI, pSection, "ShieldValue_ShowBackground");
	this->ShieldValue_UseSHPShow.Read(exINI, pSection, "ShieldValue_UseSHPShow");
	this->ShieldValue_ShowSHP.Read(pINI, pSection, "ShieldValue_ShowSHP");
	this->ShieldValue_ShowPAL.Read(pINI, pSection, "ShieldValue_ShowPAL");
	this->ShieldValue_ShowInterval.Read(exINI, pSection, "ShieldValue_ShowInterval");
}

template <typename T>
void ShieldTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Strength)
		.Process(this->InitialStrength)
		.Process(this->Armor)
		.Process(this->Powered)
		.Process(this->Respawn)
		.Process(this->Respawn_Rate)
		.Process(this->SelfHealing)
		.Process(this->SelfHealing_Rate)
		.Process(this->AbsorbOverDamage)
		.Process(this->BracketDelta)
		.Process(this->IdleAnim_OfflineAction)
		.Process(this->IdleAnim_TemporalAction)
		.Process(this->IdleAnim)
		.Process(this->BreakAnim)
		.Process(this->HitAnim)
		.Process(this->BreakWeapon)
		.Process(this->AbsorbPercent)
		.Process(this->PassPercent)
		.Process(this->AllowTransfer)
		.Process(this->ShieldValue_ShowColorHigh)
		.Process(this->ShieldValue_ShowColorMid)
		.Process(this->ShieldValue_ShowColorLow)
		.Process(this->ShieldValue_ShowOffset)
		.Process(this->ShieldValue_ShowBackground)
		.Process(this->ShieldValue_UseSHPShow)
		.Process(this->ShieldValue_ShowSHP)
		.Process(this->ShieldValue_ShowPAL)
		.Process(this->ShieldValue_ShowInterval)
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
