#include "Body.h"

#include <Ext/Rules/Body.h>

OverlayTypeExt::ExtContainer OverlayTypeExt::ExtMap;

bool OverlayTypeExt::CanPlaceBuildingOnOverlay(int overlayTypeIndex, BuildingTypeClass* pBuildingType, bool requireToBeRemovable)
{
	auto const pOverlayType = OverlayTypeClass::Array[overlayTypeIndex];
	auto const pTypeExt = OverlayTypeExt::Fetch(pOverlayType);

	if (!pTypeExt->CanBeBuiltOn.Get(pOverlayType->Tiberium ? RulesExt::Global()->Tiberium_CanBeBuiltOn
		: pOverlayType->Wall ? RulesExt::Global()->Wall_CanBeBuiltOn
		: pOverlayType->IsARock ? RulesExt::Global()->Rock_CanBeBuiltOn
		: false))
	{
		return false;
	}

	const bool remove = pTypeExt->CanBeBuiltOn_Remove.Get(RulesExt::Global()->CanBeBuiltOnOverlay_Remove);

	if (((pBuildingType && pBuildingType->Wall) || pOverlayType->Wall) && !remove)
		return false;

	return requireToBeRemovable ? remove : true;
}

void OverlayTypeExt::RemoveOverlayFromCell(int overlayTypeIndex, CellClass* pCell, HouseClass* pSource)
{
	if (overlayTypeIndex != -1 && OverlayTypeClass::Array[overlayTypeIndex]->Wall)
	{
		if (pSource && pCell->WallOwnerIndex == pSource->ArrayIndex)
			pSource->SellWall(pCell->MapCoords, true);
		else
			pCell->DamageWall(-1);
	}
	else
	{
		pCell->OverlayTypeIndex = -1;
		pCell->OverlayData = 0;
		pCell->RecalcAttributes(-1);
	}
}

// =============================
// load / save

template <typename T>
void OverlayTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->CanBeBuiltOn)
		.Process(this->CanBeBuiltOn_Remove)
		.Process(this->ZAdjust)
		.Process(this->PaletteFile)
		;
}

void OverlayTypeExt::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();

	const char* pSection = pThis->ID;
	
	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->CanBeBuiltOn.Read(exINI, pSection, "CanBeBuiltOn");
	this->CanBeBuiltOn_Remove.Read(exINI, pSection, "CanBeBuiltOn.Remove");

	auto pArtSection = pThis->ImageFile;
	INI_EX exArtINI(&CCINIClass::INI_Art);

	this->ZAdjust.Read(exArtINI, pArtSection, "ZAdjust");
	this->PaletteFile.Read(&CCINIClass::INI_Art, pArtSection, "Palette");
	this->Palette = GeneralUtils::BuildPalette(this->PaletteFile);

	if (GeneralUtils::IsValidString(this->PaletteFile) && !this->Palette)
		Debug::Log("[Developer warning] [%s] has Palette=%s set but no palette file was loaded (missing file or wrong filename). Missing palettes cause issues with lighting recalculations.\n", pArtSection, this->PaletteFile.data());
}

void OverlayTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	ObjectTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
	this->Palette = GeneralUtils::BuildPalette(this->PaletteFile);
}

void OverlayTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	ObjectTypeExt::SaveToStream(Stm);
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

//DEFINE_HOOK_AGAIN(0x5FEA1E, OverlayTypeClass_LoadFromINI, 0xA)// Section dont exist!
DEFINE_HOOK(0x5FEA11, OverlayTypeClass_LoadFromINI, 0xA)
{
	GET(OverlayTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0x28C, 0x4));

	OverlayTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
