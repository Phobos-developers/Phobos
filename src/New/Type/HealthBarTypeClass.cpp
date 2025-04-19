#include "HealthBarTypeClass.h"

template<>
const char* Enumerable<HealthBarTypeClass>::GetMainSection()
{
	return "HealthBarTypes";
}

void HealthBarTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (!_stricmp(pSection, NONE_STR))
		return;

	INI_EX exINI(pINI);

	this->Pips.Read(exINI, pSection, "Pips");
	this->Pips_Building.Read(exINI, pSection, "Pips.Building");
	this->PipsEmpty.Read(exINI, pSection, "PipsEmpty");
	this->PipsInterval.Read(exINI, pSection, "PipsInterval");
	this->PipsInterval_Building.Read(exINI, pSection, "PipsInterval.Building");
	this->PipsLength.Read(exINI, pSection, "PipsLength");
	this->PipsShape.Read(exINI, pSection, "PipsShape");
	this->PipsPalette.LoadFromINI(pINI, pSection, "PipsPalette");

	this->PipBrd.Read(exINI, pSection, "PipBrd");
	this->PipBrdShape.Read(exINI, pSection, "PipBrdShape");
	this->PipBrdPalette.LoadFromINI(pINI, pSection, "PipBrdPalette");
	this->PipBrdXOffset.Read(exINI, pSection, "PipBrdXOffset");

	this->XOffset.Read(exINI, pSection, "XOffset");
}

template <typename T>
void HealthBarTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Pips)
		.Process(this->Pips_Building)
		.Process(this->PipsEmpty)
		.Process(this->PipsInterval)
		.Process(this->PipsInterval_Building)
		.Process(this->PipsLength)
		.Process(this->PipsShape)
		.Process(this->PipsPalette)
		.Process(this->PipBrd)
		.Process(this->PipBrdShape)
		.Process(this->PipBrdPalette)
		.Process(this->PipBrdXOffset)
		.Process(this->XOffset)
		;
}

void HealthBarTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void HealthBarTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
