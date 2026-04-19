#include "Body.h"

#include <ScenarioClass.h>

IsometricTileTypeExt::ExtContainer IsometricTileTypeExt::ExtMap;
int IsometricTileTypeExt::CurrentTileset = -1;
std::map<std::string, std::vector<LightConvertClass*>> IsometricTileTypeExt::LightConvertEntities {};
bool IsometricTileTypeExt::InRender = false;

IsometricTileTypeExt::ExtData::ExtData(IsometricTileTypeClass* ownerObject)
	: Extension<IsometricTileTypeClass>(ownerObject)
	, Tileset { -1 }
	, PaletteName { "" }
{}

LightConvertClass* IsometricTileTypeExt::GetLightConvert(const char* paletteName, int r, int g, int b, const bool isDefault)
{
	int shadeCount = 53;

	if (r + g + b < 2000)
		shadeCount = 27;

	ScenarioClass::Instance->ScenarioLighting(&r, &g, &b);
	TintStruct tint(r, g, b);

	auto& entities = IsometricTileTypeExt::LightConvertEntities[paletteName];

	if (!entities.empty())
	{
		for (auto const pLightConvert : entities)
		{
			if (pLightConvert->Color1 == tint)
				return pLightConvert;
		}
	}

	LightConvertClass* pLightConvert = GameCreate<LightConvertClass>
		(
			isDefault ? &FileSystem::ISOx_PAL : FileSystem::AllocatePalette(paletteName),
			&FileSystem::TEMPERAT_PAL,
			DSurface::Primary,
			r,
			g,
			b,
			!entities.empty(),
			nullptr,
			shadeCount
		);

	LightConvertClass::Array.AddItem(pLightConvert);
	entities.push_back(pLightConvert);

	return pLightConvert;
}

// =============================
// load / save

void IsometricTileTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	this->Tileset = IsometricTileTypeExt::CurrentTileset;

	char section[0x20];
	sprintf(section, "TileSet%04d", IsometricTileTypeExt::CurrentTileset);

	auto const theater = ScenarioClass::Instance->Theater;
	auto const pExtension = Theater::GetTheater(theater).Extension;
	char pDefault[] = "iso~~~.pal";
	pDefault[3] = pExtension[0];
	pDefault[4] = pExtension[1];
	pDefault[5] = pExtension[2];

	this->PaletteName.Read(pINI, section, "CustomPalette", pDefault);
}

template <typename T>
void IsometricTileTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Tileset)
		.Process(this->PaletteName)
		;
}

void IsometricTileTypeExt::ExtData::LoadFromStream(PhobosStreamReader& stm)
{
	Extension<IsometricTileTypeClass>::LoadFromStream(stm);
	this->Serialize(stm);
}

void IsometricTileTypeExt::ExtData::SaveToStream(PhobosStreamWriter& stm)
{
	Extension<IsometricTileTypeClass>::SaveToStream(stm);
	this->Serialize(stm);
}

void IsometricTileTypeExt::Clear()
{
	IsometricTileTypeExt::LightConvertEntities.clear();
}

// =============================
// container

IsometricTileTypeExt::ExtContainer::ExtContainer() : Container("IsometricTileTypeClass") { }

IsometricTileTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK_AGAIN(0x544A5D, IsometricTileTypeClass_CTOR, 0x6)	// IsometricTileTypeClass::CTOR_Load
DEFINE_HOOK(0x5449F2, IsometricTileTypeClass_CTOR, 0x5)			// IsometricTileTypeClass::CTOR
{
	IsometricTileTypeClass* const pItem = R->Origin() == 0x544A5D ?
		R->ESI<IsometricTileTypeClass* const>() : R->EBP<IsometricTileTypeClass* const>();

	IsometricTileTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x544BC2, IsometricTileTypeClass_DTOR, 0x8)
{
	GET(IsometricTileTypeClass* const, pItem, ESI);

	IsometricTileTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x549D70, IsometricTileTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x549C80, IsometricTileTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IsometricTileTypeClass* const, pItem, 0x4);
	GET_STACK(IStream* const, pStm, 0x8);

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

DEFINE_HOOK(0x545FA3, IsometricTileTypeClass_LoadFromINI_SetTileSet, 0x8)
{
	GET(const int, tileset, EDI);

	IsometricTileTypeExt::CurrentTileset = tileset;

	return 0;
}

DEFINE_HOOK(0x54642E, IsometricTileTypeClass_LoadFromINI, 0x6)
{
	GET(IsometricTileTypeClass* const, pItem, EBP);
	LEA_STACK(CCINIClass* const, pINI, STACK_OFFSET(0xA10, -0x9D8));

	IsometricTileTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x4AD059, IsometricTileTypeClass_LoadFromINI_After, 0x6)
{
	IsometricTileTypeExt::CurrentTileset = -1;

	return 0;
}
