#include "Body.h"

#include <ConvertClass.h>

template<> const DWORD Extension<IsometricTileTypeClass>::Canary = 0x23434657;
IsometricTileTypeExt::ExtContainer IsometricTileTypeExt::ExtMap;

std::unordered_map<std::string, BytePalette*> IsometricTileTypeExt::Palettes;

void IsometricTileTypeExt::ExtData::GetSectionName(char* buffer) {
	sprintf(buffer, "TileSet%04d", this->TileSetNumber.Get());
}

// =============================
// load / save

int CurrentTileSetNumber = -1;

void IsometricTileTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	this->TileSetNumber = CurrentTileSetNumber;

	char pSection[16];
	this->GetSectionName(pSection);

	if (pINI->GetSection(pSection))
	{
		this->CustomPalette.Read(pINI, pSection, "CustomPalette");

		if (this->CustomPalette) {
			std::string s(this->CustomPalette);

			if (!Palettes[s]) {
				Debug::Log("[Palette] Loading new custom palette %s\n", s.c_str());
				Palettes[s] = FileSystem::AllocatePalette(s.c_str());
			}

			this->Palette = Palettes[s];
		}
	}
}
template <typename T>
void IsometricTileTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->CustomPalette)
		;
}

void IsometricTileTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<IsometricTileTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void IsometricTileTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<IsometricTileTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

IsometricTileTypeExt::ExtContainer::ExtContainer() : Container("IsometricTileTypeClass") {
}

IsometricTileTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(5449F2, IsometricTileTypeClass_CTOR, 5)
{
	GET(IsometricTileTypeClass*, pItem, EBP);

	IsometricTileTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(544BC2, IsometricTileTypeClass_DTOR, 8)
{
	GET(IsometricTileTypeClass*, pItem, ESI);

	IsometricTileTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(549D70, IsometricTileTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(549C80, IsometricTileTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(IsometricTileTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	IsometricTileTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(549D8A, IsometricTileTypeClass_Save_Suffix, 6)
{
	IsometricTileTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(549D5D, IsometricTileTypeClass_Load_Suffix, 5)
{
	IsometricTileTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(545FA3, IsometricTileTypeClass_LoadFromINI_SetTileSetNumber, 8)
{
	CurrentTileSetNumber = R->EDI<int>();

	return 0;
}

DEFINE_HOOK(54642E, IsometricTileTypeClass_LoadFromINI, 6)
{
	GET(IsometricTileTypeClass*, pItem, EBP);
	LEA_STACK(CCINIClass*, pINI, 0xA10 - 0x9D8);

	IsometricTileTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
