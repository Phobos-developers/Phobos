#include "CrateTypeClass.h"

Enumerable<CrateTypeClass>::container_t Enumerable<CrateTypeClass>::Array;

const char * Enumerable<CrateTypeClass>::GetMainSection()
{
	return "CrateTypes";
}

void CrateTypeClass::LoadListSection(CCINIClass *pINI)
{
	for (int i = 0; i < pINI->GetKeyCount(GetMainSection()); ++i)
	{
		if (pINI->ReadString(GetMainSection(), pINI->GetKeyName(GetMainSection(), i), "", Phobos::readBuffer))
		{
			FindOrAllocate(Phobos::readBuffer);
			Debug::Log("CrateType :: LoadListSection check [%s] \n", Phobos::readBuffer);
		}
	}

	for (auto &CrateType : Array)
        CrateType->LoadFromINI(pINI);

}

void CrateTypeClass::LoadFromINI(CCINIClass *pINI)
{
	const char *section = this->Name;

	INI_EX exINI(pINI);

	this->Super.Read(exINI, section, "Crate.SuperWeapon");
	this->SuperGrant.Read(exINI, section, "Crate.SuperWeaponGrant");
	this->WeaponType.Read(exINI, section, "Crate.Weapon", true);
	this->Chance.Read(exINI, section, "Crate.Chance");
	this->Anim.Read(exINI, section, "Crate.Anim");
	this->Type.Read(exINI, section, "Crate.Type");
	this->AllowWater.Read(exINI, section, "Crate.AllowWater");
	this->Sound.Read(exINI, section, "Crate.Sound");
	this->Eva.Read(exINI, section, "Crate.EVA");
	this->Unit.Read(exINI, section, "Crate.Units");
    this->MoneyMin.Read(exINI, section, "Crate.MoneyMin");
    this->MoneyMax.Read(exINI, section, "Crate.MoneyMax");

}

template <typename T>
void CrateTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Super)
		.Process(this->WeaponType)
		.Process(this->Chance)
		.Process(this->Anim)
		.Process(this->Type)
		.Process(this->SuperGrant)
		.Process(this->Sound)
		.Process(this->Eva)
		.Process(this->Unit)
        .Process(this->MoneyMin)
        .Process(this->MoneyMax)
        .Process(this->AllowWater)
		;
}

void CrateTypeClass::LoadFromStream(PhobosStreamReader &Stm)
{
	this->Serialize(Stm);
}

void CrateTypeClass::SaveToStream(PhobosStreamWriter &Stm)
{
	this->Serialize(Stm);
}
