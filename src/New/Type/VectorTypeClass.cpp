#include "VectorTypeClass.h"

template<>
const char* Enumerable<VectorTypeClass>::GetMainSection()
{
	return "VectorTypes";
}

void VectorTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	this->Duration.Read(exINI, section, "Vector.Duration");
	this->Next.Read(exINI, section, "Vector.Next");

	this->Data.Read(exINI, section);
}

template <typename T>
void VectorTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Duration)
		.Process(this->Next)
		.Process(this->Data)
		;
}

void VectorTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void VectorTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
