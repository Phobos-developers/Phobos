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
	this->Frame.Read(exINI, pSection, "Frame");
	this->Grounded.Read(exINI, pSection, "Grounded");
	this->Offset.Read(exINI, pSection, "Offset");
	this->Translucency.Read(exINI, pSection, "Translucency");
	this->VisibleToHouses.Read(exINI, pSection, "VisibleToHouses");
	this->VisibleToObserver.Read(exINI, pSection, "VisibleToObserver");
	this->OverTechno.Read(exINI, pSection, "OverTechno");
}

template <typename T>
void SelectBoxTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Frame)
		.Process(this->Grounded)
		.Process(this->Offset)
		.Process(this->Translucency)
		.Process(this->VisibleToHouses)
		.Process(this->VisibleToObserver)
		.Process(this->OverTechno)
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
