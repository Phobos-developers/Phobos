#include "BannerTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

Enumerable<BannerTypeClass>::container_t Enumerable<BannerTypeClass>::Array;

const char* Enumerable<BannerTypeClass>::GetMainSection()
{
	return "BannerTypes";
}

void BannerTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->Banner_CSF.Read(exINI, section, "Banner.CSF");

	pINI->ReadString(section, "Banner.PCX", NONE_STR, Phobos::readBuffer);
	// ???????//
	//this->Banner_PCX = Valueable<PhobosFixedString<32>>(PhobosFixedString<32>(GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"$")));
	this->Banner_PCX = Valueable<PhobosFixedString<32>>(PhobosFixedString<32>());

	if (this->Banner_PCX.Get())
	{
		this->Type = BannerType::PCX;
		PCX::Instance->LoadFile(this->Banner_PCX.Get().data());
	}
	else if (this->Banner_CSF.Get())
	{
		this->Type = BannerType::CSF;
	}
}

template <typename T>
void BannerTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->Banner_CSF)
		.Process(this->Banner_PCX)
		;
}

void BannerTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void BannerTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
