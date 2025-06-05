#include "SelectBoxTypeClass.h"

template<>
const char* Enumerable<SelectBoxTypeClass>::GetMainSection()
{
	return "SelectBoxTypes";
}

void SelectBoxTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (!_stricmp(pSection, NONE_STR))
		return;

	INI_EX exINI(pINI);

	this->Shape.Read(exINI, pSection, "Shape");
	this->Palette.LoadFromINI(pINI, pSection, "Palette");
	this->Frames.Read(exINI, pSection, "Frames");
	this->Grounded.Read(exINI, pSection, "Grounded");
	this->Offset.Read(exINI, pSection, "Offset");
	this->Translucency.Read(exINI, pSection, "Translucency");
	this->VisibleToHouses.Read(exINI, pSection, "VisibleToHouses");
	this->VisibleToHouses_Observer.Read(exINI, pSection, "VisibleToHouses.Observer");
	this->DrawAboveTechno.Read(exINI, pSection, "DrawAboveTechno");
}

template <typename T>
void SelectBoxTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Frames)
		.Process(this->Grounded)
		.Process(this->Offset)
		.Process(this->Translucency)
		.Process(this->VisibleToHouses)
		.Process(this->VisibleToHouses_Observer)
		.Process(this->DrawAboveTechno)
		;
}

void SelectBoxTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void SelectBoxTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
