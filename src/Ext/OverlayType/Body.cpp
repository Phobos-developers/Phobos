#include "Body.h"

#include <OverlayTypeClass.h>
#include <TacticalClass.h>

#include <Utilities/GeneralUtils.h>

OverlayTypeExt::ExtContainer OverlayTypeExt::ExtMap;

// =============================
// load / save

template <typename T>
void OverlayTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->PaletteFile)
		;
}

void OverlayTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();

	//const char* pSection = pThis->ID;
	//
	//if (!pINI->GetSection(pSection))
	//	return;
	//
	//INI_EX exINI(pINI);

	auto pArtSection = pThis->ImageFile;
	auto const pArtINI = &CCINIClass::INI_Art();
	INI_EX exArtINI(pArtINI);

	this->ZAdjust.Read(exArtINI, pArtSection, "ZAdjust");
	this->PaletteFile.Read(pArtINI, pArtSection, "Palette");
	this->Palette = GeneralUtils::BuildPalette(this->PaletteFile);

	if (GeneralUtils::IsValidString(this->PaletteFile) && !this->Palette)
		Debug::Log("[Developer warning] [%s] has Palette=%s set but no palette file was loaded (missing file or wrong filename). Missing palettes cause issues with lighting recalculations.\n", pArtSection, this->PaletteFile.data());
}

void OverlayTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<OverlayTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
	this->Palette = GeneralUtils::BuildPalette(this->PaletteFile);
}

void OverlayTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<OverlayTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool OverlayTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool OverlayTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

OverlayTypeExt::ExtContainer::ExtContainer() : Container("OverlayTypeClass") { }
OverlayTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK_AGAIN(0x5FE3AF, OverlayTypeClass_CTOR, 0x5)
DEFINE_HOOK(0x5FE3A2, OverlayTypeClass_CTOR, 0x5)
{
	GET(OverlayTypeClass*, pItem, EAX);

	OverlayTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x5FEF61, OverlayTypeClass_SDDTOR, 0x5)
{
	GET(OverlayTypeClass*, pItem, ESI);

	OverlayTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x5FEAF0, OverlayTypeClass_SaveLoad_Prefix, 0xA)
DEFINE_HOOK(0x5FEC10, OverlayTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(OverlayTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	OverlayTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x5FEBFA, OverlayTypeClass_Load_Suffix, 0x6)
{
	OverlayTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x5FEC2A, OverlayTypeClass_Save_Suffix, 0x6)
{
	OverlayTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x5FEA11, OverlayTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x5FEA1E, OverlayTypeClass_LoadFromINI, 0xA)
{
	GET(OverlayTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0x28C, 0x4));

	OverlayTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
