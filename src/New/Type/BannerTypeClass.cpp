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

	this->Content_CSF.Read(exINI, section, "Content.CSF");
	this->Content_PCX.Read(pINI, section, "Content.PCX");
	this->Content_SHP.Read(pINI, section, "Content.SHP");
	this->Content_SHP_Palette.Read(pINI, section, "Content.SHP.Palette");
	this->Content_CSF_Color.Read(exINI, section, "Content.CSF.Color");
	this->Content_CSF_DrawBackground.Read(exINI, section, "Content.CSF.DrawBackground");
	
	if (this->Content_PCX)
	{
		this->Type = BannerType::PCX;
	}
	else if (this->Content_SHP)
	{
		this->Type = BannerType::SHP;
		char filename[0x20];
		strcpy(filename, this->Content_SHP);
		_strlwr_s(filename);
		this->ImageSHP = FileSystem::LoadSHPFile(filename);
		if (this->Content_SHP_Palette)
		{
			strcpy(filename, this->Content_SHP_Palette);
			_strlwr_s(filename);
			this->Palette = FileSystem::LoadPALFile(filename, DSurface::Composite);
		}
		else
		{
			this->Palette = FileSystem::PALETTE_PAL;
		}
	}
	else if (this->Content_CSF.Get())
	{
		this->Type = BannerType::CSF;
		wcscpy(this->Text, this->Content_CSF.Get().Text);
	}
}

template <typename T>
void BannerTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->Content_CSF)
		.Process(this->Content_CSF_Color)
		.Process(this->Content_CSF_DrawBackground)
		.Process(this->Content_PCX)
		.Process(this->Content_SHP)
		.Process(this->Content_SHP_Palette)
		.Process(this->Text)
		.Process(this->ImageSHP)
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
