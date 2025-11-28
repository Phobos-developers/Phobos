#include "SWSignalTypeClass.h"

const char* Enumerable<SWSignalTypeClass>::GetMainSection()
{
	return "SuperWeaponSignalTypes";
}

void SWSignalTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	//char tempBuffer[0x20];

	this->Range.Read(exINI, pSection, "Range");
	this->Affects.Read(exINI, pSection, "Affects");
	this->Powered.Read(exINI, pSection, "Powered");
	this->StopInTemporal.Read(exINI, pSection, "StopInTemporal");
}

// =============================
// load / save

template <typename T>
void SWSignalTypeClass::Serialize(T& stm)
{
	stm
		.Process(this->Range)
		.Process(this->Affects)
		.Process(this->Powered)
		.Process(this->StopInTemporal)
		;
}

void SWSignalTypeClass::LoadFromStream(PhobosStreamReader& stm)
{
	this->Serialize(stm);
}

void SWSignalTypeClass::SaveToStream(PhobosStreamWriter& stm)
{
	this->Serialize(stm);
}
