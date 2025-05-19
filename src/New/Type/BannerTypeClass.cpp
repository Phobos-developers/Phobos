#include "BannerTypeClass.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>

template<>
const char* Enumerable<BannerTypeClass>::GetMainSection()
{
	return "BannerTypes";
}

inline void BannerTypeClass::DetermineType()
{
	if (this->PCX.GetSurface())
		this->BannerType = BannerType::PCX;
	else if (this->Shape)
		this->BannerType = BannerType::SHP;
	else if (!this->CSF.Get().empty() || this->CSF_VariableFormat != BannerNumberType::None)
		this->BannerType = BannerType::CSF;
	else
		Debug::Log("[Developer warning] BannerType [%s] doesn't have an available content.", this->Name);
}

void BannerTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->PCX.Read(pINI, section, "PCX");
	this->Shape.Read(exINI, section, "SHP");
	this->Palette.LoadFromINI(pINI, section, "SHP.Palette");
	this->CSF.Read(exINI, section, "CSF");
	this->CSF_Color.Read(exINI, section, "CSF.Color");
	this->CSF_Background.Read(exINI, section, "CSF.Background");
	this->CSF_VariableFormat.Read(exINI, section, "CSF.VariableFormat");

	this->DetermineType();
}

template <typename T>
void BannerTypeClass::Serialize(T& stm)
{
	stm
		.Process(this->PCX)
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->CSF)
		.Process(this->CSF_Color)
		.Process(this->CSF_Background)
		.Process(this->CSF_VariableFormat)
		.Process(this->BannerType)
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
