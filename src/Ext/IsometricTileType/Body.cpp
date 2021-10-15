#include "Body.h"

#include <ScenarioClass.h>

template<> const DWORD Extension<IsometricTileTypeClass>::Canary = 0x91577125;
IsometricTileTypeExt::ExtContainer IsometricTileTypeExt::ExtMap;
int IsometricTileTypeExt::CurrentTileset = -1;

// =============================
// load / save

void IsometricTileTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	this->Tileset = IsometricTileTypeExt::CurrentTileset;

	char pSection[16];
	sprintf(pSection, "TileSet%04d", this->Tileset.Get());

	if (pINI->GetSection(pSection))
	{

	}
}

template <typename T>
void IsometricTileTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Tileset)
		;
}

void IsometricTileTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<IsometricTileTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void IsometricTileTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<IsometricTileTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool IsometricTileTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(IsometricTileTypeExt::CurrentTileset)
		.Success();
}

bool IsometricTileTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(IsometricTileTypeExt::CurrentTileset)
		.Success();
}
// =============================
// container

IsometricTileTypeExt::ExtContainer::ExtContainer() : Container("IsometricTileTypeClass") { }

IsometricTileTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x5449F2, IsometricTileTypeClass_CTOR, 0x5)
{
	GET(IsometricTileTypeClass*, pItem, EBP);

	IsometricTileTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x544BC2, IsometricTileTypeClass_DTOR, 0x8)
{
	GET(IsometricTileTypeClass*, pItem, ESI);

	IsometricTileTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x549D70, IsometricTileTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x549C80, IsometricTileTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IsometricTileTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	IsometricTileTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x549D5D, IsometricTileTypeClass_Load_Suffix, 0x5)
{
	IsometricTileTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x549D8A, IsometricTileTypeClass_Save_Suffix, 0x6)
{
	IsometricTileTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK(0x54642E, IsometricTileTypeClass_LoadFromINI, 0x6)
{
	GET(IsometricTileTypeClass*, pItem, EBP);
	LEA_STACK(CCINIClass*, pINI, STACK_OFFS(0xA10, 0x9D8));

	IsometricTileTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

DEFINE_HOOK(0x545FA3, IsometricTileTypeClass_LoadFromINI_SetTileSet, 0x8)
{
	IsometricTileTypeExt::CurrentTileset = R->EDI();

	return 0;
}