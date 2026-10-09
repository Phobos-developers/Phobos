#include "InsigniaTypeClass.h"

template<>
const char* Enumerable<InsigniaTypeClass>::GetMainSection()
{
	return "InsigniaTypes";
}

void InsigniaTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	this->Insignia.Read(exINI, section, "Insignia.%s");
	this->InsigniaFrame.Read(exINI, section, "InsigniaFrame.%s");
	this->InsigniaPalette.LoadFromINI(pINI, section, "InsigniaPalette");
	this->InsigniaPalette_Rookie.LoadFromINI(pINI, section, "InsigniaPalette.Rookie");
	this->InsigniaPalette_Veteran.LoadFromINI(pINI, section, "InsigniaPalette.Veteran");
	this->InsigniaPalette_Elite.LoadFromINI(pINI, section, "InsigniaPalette.Elite");
}

template <typename T>
void InsigniaTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Insignia)
		.Process(this->InsigniaFrame)
		.Process(this->InsigniaPalette)
		.Process(this->InsigniaPalette_Rookie)
		.Process(this->InsigniaPalette_Veteran)
		.Process(this->InsigniaPalette_Elite)
		;
};

void InsigniaTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void InsigniaTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
