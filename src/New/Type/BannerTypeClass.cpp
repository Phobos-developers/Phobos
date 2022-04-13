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

	this->Content.PCX.Read(pINI, section, "Content.PCX");
	this->Content.SHP._.Read(pINI, section, "Content.SHP");
	this->Content.CSF._.Read(exINI, section, "Content.CSF");
	this->Content.VariableFormat._.Read(exINI, section, "Content.VariableFormat");
	
	if (this->Content.PCX)
	{
		this->BannerType = BannerType::PCX;
	}
	else if (this->Content.SHP._)
	{
		this->BannerType = BannerType::SHP;
		this->Content.SHP.Palette.Read(pINI, section, "Content.SHP.Palette");
	}
	else if (this->Content.CSF._.Get())
	{
		this->BannerType = BannerType::CSF;
		this->Content.CSF.Color.Read(exINI, section, "Content.CSF.Color");
		this->Content.CSF.DrawBackground.Read(exINI, section, "Content.CSF.DrawBackground");
	}
	else if (this->Content.VariableFormat._.Get() != BannerNumberType::None)
	{
		this->BannerType = BannerType::VariableFormat;
		this->Content.VariableFormat.Label.Read(exINI, section, "Content.VariableFormat.Label");
	}
}

void BannerTypeClass::LoadImage()
{
	switch (this->BannerType)
	{
	case BannerType::PCX:
		PCX::Instance->LoadFile(this->Content.PCX.data());
		char filename[0x20];
		strcpy(filename, this->Content.PCX.data());
		_strlwr_s(filename);
		this->ImagePCX = PCX::Instance->GetSurface(filename);
		break;
	case BannerType::SHP:
		char filename[0x20];
		strcpy(filename, this->Content.SHP._);
		_strlwr_s(filename);
		this->ImageSHP = FileSystem::LoadSHPFile(filename);
		if (this->Content.SHP.Palette)
		{
			strcpy(filename, this->Content.SHP.Palette);
			_strlwr_s(filename);
			this->Palette = FileSystem::LoadPALFile(filename, DSurface::Composite);
		}
		else
		{
			this->Palette = FileSystem::PALETTE_PAL;
		}
		break;
	case BannerType::CSF:
		break;
	case BannerType::VariableFormat:
		break;
	default:
		break;
	}
};

template <typename T>
void BannerTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->Content.PCX)
		.Process(this->Content.SHP._)
		.Process(this->Content.SHP.Palette)
		.Process(this->Content.CSF._)
		.Process(this->Content.CSF.Color)
		.Process(this->Content.CSF.DrawBackground)
		.Process(this->Content.VariableFormat._)
		.Process(this->Content.VariableFormat.Label)
		.Process(this->ImageSHP)
		.Process(this->ImagePCX)
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
