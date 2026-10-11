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

	this->Shape.Read(exINI, section, "Insignia.%s");
	this->Palette.LoadFromINI(pINI, section, "InsigniaPalette");
	this->Palette_Rookie.LoadFromINI(pINI, section, "InsigniaPalette.Rookie");
	this->Palette_Veteran.LoadFromINI(pINI, section, "InsigniaPalette.Veteran");
	this->Palette_Elite.LoadFromINI(pINI, section, "InsigniaPalette.Elite");
	this->Frame.Read(exINI, section, "InsigniaFrame.%s");
}

template <typename T>
void InsigniaTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Palette_Rookie)
		.Process(this->Palette_Veteran)
		.Process(this->Palette_Elite)
		.Process(this->Frame)
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
