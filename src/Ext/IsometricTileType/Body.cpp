#include "Body.h"

#include <ScenarioClass.h>

DynamicVectorClass<LightConvertPalette*> LightConvertPalette::Array;

template<> const DWORD Extension<IsometricTileTypeClass>::Canary = 0x91577125;
IsometricTileTypeExt::ExtContainer IsometricTileTypeExt::ExtMap;
int IsometricTileTypeExt::CurrentTileset = -1;
PhobosMap<LightConvertPalette*, PhobosMap<TintStruct, LightConvertClass*>> IsometricTileTypeExt::TileDrawers;

LightConvertClass* IsometricTileTypeExt::InitDrawer(IsometricTileTypeClass* pType, TintStruct& tint)
{
	// validate the surface
	if (!DSurface::Primary())
		return nullptr;

	auto const pData = IsometricTileTypeExt::ExtMap.Find(pType);
	LightConvertPalette* pLCP = nullptr;
	if (pData && pData->Palette)
		pLCP = pData->Palette;

	auto QueryDrawer = [&tint, pLCP](IsometricTileTypeClass* pType) -> LightConvertClass*
	{
		return IsometricTileTypeExt::TileDrawers.get_or_default(pLCP).get_or_default(tint, nullptr);
	};

	// If no custom palette is set, use vanialla one
	if (!pLCP && tint == TintStruct { 1000,1000,1000 } && LightConvertClass::Array->Count > 0)
		return *LightConvertClass::Array->begin();
	
	ScenarioClass::ScenarioLighting(&tint.Red, &tint.Green, &tint.Blue);

	if (auto const pDrawer = QueryDrawer(pType))
		return pDrawer;

	int nShadeCount = 53;
	if (tint.Red + tint.Green + tint.Blue < 2000)
		nShadeCount = 27;
	
	auto pPalette = pLCP && pLCP->Loaded() ? *pLCP : &FileSystem::ISOx_PAL();

	auto const pDrawer = GameCreate<LightConvertClass>(
		*pPalette, FileSystem::TEMPERAT_PAL(), DSurface::Primary(), tint.Red, tint.Green, tint.Blue,
		LightConvertClass::Array->Count != 0, nullptr, nShadeCount);

	LightConvertClass::Array->AddItem(pDrawer);
	IsometricTileTypeExt::TileDrawers[pLCP][tint] = pDrawer;

	return pDrawer;
}

// =============================
// load / save

void IsometricTileTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	this->Tileset = IsometricTileTypeExt::CurrentTileset;

	char pSection[16];
	sprintf(pSection, "TileSet%04d", this->Tileset.Get());

	if (pINI->GetSection(pSection))
	{
		this->Palette = LightConvertPalette::FindOrAllocate(pINI, pSection, "CustomPalette");

	}
}

template <typename T>
void IsometricTileTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Tileset)
		.Process(this->Palette)
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
		.Process(IsometricTileTypeExt::TileDrawers)
		.Success();
}

bool IsometricTileTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(IsometricTileTypeExt::CurrentTileset)
		.Process(IsometricTileTypeExt::TileDrawers)
		.Success();
}
// =============================
// container

IsometricTileTypeExt::ExtContainer::ExtContainer() : Container("IsometricTileTypeClass") { }

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

DEFINE_HOOK(549D5D, IsometricTileTypeClass_Load_Suffix, 5)
{
	IsometricTileTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(549D8A, IsometricTileTypeClass_Save_Suffix, 6)
{
	IsometricTileTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK(54642E, IsometricTileTypeClass_LoadFromINI, 6)
{
	GET(IsometricTileTypeClass*, pItem, EBP);
	LEA_STACK(CCINIClass*, pINI, STACK_OFFS(0xA10, 0x9D8));

	IsometricTileTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

DEFINE_HOOK(545FA3, IsometricTileTypeClass_LoadFromINI_SetTileSet, 8)
{
	IsometricTileTypeExt::CurrentTileset = R->EDI();

	return 0;
}