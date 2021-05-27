#include "ShieldType.h"

Enumerable<ShieldTypeClass>::container_t Enumerable<ShieldTypeClass>::Array;

const char* Enumerable<ShieldTypeClass>::GetMainSection()
{
	return "ShieldTypes";
}

void ShieldTypeClass::LoadListSection(CCINIClass* pINI)
{
	for (int i = 0; i < pINI->GetKeyCount(GetMainSection()); ++i)
	{
		if (pINI->ReadString(GetMainSection(), pINI->GetKeyName(GetMainSection(), i), "", Phobos::readBuffer))
		{
			FindOrAllocate(Phobos::readBuffer);
			Debug::Log("ShieldTypeClass :: LoadListSection check [%s] \n", Phobos::readBuffer);
		}
	}

	for (auto& pType : Array)
		pType->LoadFromINI(pINI);
}

void ShieldTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);

	this->Strength.Read(exINI, pSection, "Strength");
	this->Armor.Read(exINI, pSection, "Armor");
	this->Powered.Read(exINI, pSection, "Powered");

	this->Respawn.Read(exINI, pSection, "Respawn");
	this->Respawn_Rate__InMinutes.Read(exINI, pSection, "Respawn.Rate");
	this->Respawn_Rate = (int)this->Respawn_Rate__InMinutes * 900;

	this->SelfHealing.Read(exINI, pSection, "SelfHealing");
	this->SelfHealing_Rate__InMinutes.Read(exINI, pSection, "SelfHealing.Rate");
	this->SelfHealing_Rate = (int)this->SelfHealing_Rate__InMinutes * 900;

	this->AbsorbOverDamage.Read(exINI, pSection, "AbsorbOverDamage");
	this->BracketDelta.Read(exINI, pSection, "BracketDelta");

	this->IdleAnim_OfflineAction.Read(exINI, pSection, "IdleAnim.OfflineAction");
	this->IdleAnim_TemporalAction.Read(exINI, pSection, "IdleAnim.TemporalAction");

	this->IdleAnim.Read(exINI, pSection, "IdleAnim");
	this->BreakAnim.Read(exINI, pSection, "BreakAnim");
	this->RespawnAnim.Read(exINI, pSection, "RespawnAnim");
	this->HitAnim.Read(exINI, pSection, "HitAnim");
}

template <typename T>
void ShieldTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Strength)
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
		.Process(this->RespawnAnim)
		.Process(this->HitAnim)
		.Success()
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
