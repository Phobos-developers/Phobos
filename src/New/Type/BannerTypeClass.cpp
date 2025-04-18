#include "BannerTypeClass.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>

const char* Enumerable<BannerTypeClass>::GetMainSection()
{
	return "BannerTypes";
}

inline void BannerTypeClass::DetermineType()
{
	if (PCX)
		BannerType = BannerType::PCX;
	else if (Shape)
		BannerType = BannerType::SHP;
	else if (VariableFormat != BannerNumberType::None)
		BannerType = BannerType::VariableFormat;
	else
		BannerType = BannerType::CSF;
}

void BannerTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->PCX.Read(exINI, section, "Content.PCX");
	this->Shape.Read(exINI, section, "Content.SHP");
	this->Palette.LoadFromINI(pINI, section, "Content.SHP.Palette");
	this->CSF.Read(exINI, section, "Content.CSF");
	this->CSF_Color.Read(exINI, section, "Content.CSF.Color");
	this->CSF_Background.Read(exINI, section, "Content.CSF.DrawBackground");
	this->VariableFormat.Read(exINI, section, "Content.VariableFormat");
	this->VariableFormat_Label.Read(exINI, section, "Content.VariableFormat.Label");

	DetermineType();
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
		.Process(this->VariableFormat)
		.Process(this->VariableFormat_Label)
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
