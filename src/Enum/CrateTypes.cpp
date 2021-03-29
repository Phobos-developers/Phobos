#include "CrateTypes.h"

Enumerable<CrateType>::container_t Enumerable<CrateType>::Array;

const char * Enumerable<CrateType>::GetMainSection()
{
	return "CrateTypes";
}

CrateType::~CrateType() = default;

CrateType::CrateType(const char* const pTitle)
	: Enumerable<CrateType>(pTitle),
	SWs(),
	WeaponType(),
	Chance(),
	Anim(),
	Tp(),
	SWGrant(),
	Water(),
	Sound()
{ }

void CrateType::LoadListSection(CCINIClass *pINI)
{
	const char *section = GetMainSection();
	int len = pINI->GetKeyCount(section);
	for (int i = 0; i < len; ++i) {
		const char *key = pINI->GetKeyName(section, i);
		if (pINI->ReadString(section, key, "", Phobos::readBuffer)) {
			FindOrAllocate(Phobos::readBuffer);
		}
	}
	Debug::Log("CrateType :: LoadListSection check,list length = %d \n", len);
	for (size_t i = 0; i < Array.size(); ++i) {
		Array[i]->LoadFromINI(pINI);
	}
}
void CrateType::LoadFromINI(CCINIClass *pINI)
{
	const char *section = this->Name;

	INI_EX exINI(pINI);

	this->SWs.Read(exINI, section, "Crate.SW");
	this->SWGrant.Read(exINI, section, "Crate.SWGrant");
	this->WeaponType.Read(exINI, section, "Crate.WP");
	this->Chance.Read(exINI, section, "Crate.Chance");
	this->Anim.Read(exINI, section, "Crate.Anim");
	this->Tp.Read(exINI, section, "Crate.Type");
	this->Water.Read(exINI, section, "Crate.AllowWater");
	this->Sound.Read(exINI, section, "Crate.Sound");

	Debug::Log("CrateType :: LoadFromIni check,Name = %s \n", this->Name);

}
template <typename T>
void CrateType::Serialize(T& Stm) {
	Stm
		.Process(this->SWs)
		.Process(this->WeaponType)
		.Process(this->Chance)
		.Process(this->Anim)
		.Process(this->Tp)
		.Process(this->SWGrant)
		.Process(this->Water)
		.Process(this->Sound)
		;
}

void CrateType::LoadFromStream(PhobosStreamReader &Stm)
{
	this->Serialize(Stm);

}

void CrateType::SaveToStream(PhobosStreamWriter &Stm)
{
	this->Serialize(Stm);

}
