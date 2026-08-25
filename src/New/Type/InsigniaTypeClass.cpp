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
}
