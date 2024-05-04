#include "Body.h"

#include <Helpers/Macro.h>

#include <ScenarioClass.h>

#include <Utilities/TemplateDef.h>

IsometricTileTypeExt::ExtContainer IsometricTileTypeExt::ExtMap;
int IsometricTileTypeExt::ExtData::CurrentTileset = -1;
std::map<std::string, std::map<TintStruct, LightConvertClass*>> IsometricTileTypeExt::ExtData::LightConvertEntities;

IsometricTileTypeExt::ExtData::ExtData(IsometricTileTypeClass* ownerObject)
	: Extension<IsometricTileTypeClass>(ownerObject)
	, Tileset { -1 }
	, Palette {}
{}

LightConvertClass* IsometricTileTypeExt::ExtData::GetLightConvert(int r, int g, int b)
{
	int shadeCount = 53;

	if (r + g + b < 2000)
		shadeCount = 27;

	auto& entities = LightConvertEntities[this->Palette.Name];

	ScenarioClass::Instance->ScenarioLighting(&r, &g, &b);
	TintStruct tint(r, g, b);

	if (entities.contains(tint) && entities.at(tint) != nullptr)
		return entities.at(tint);

	LightConvertClass* pLightConvert= GameCreate<LightConvertClass>
		(
			this->Palette.Palette.get(),
			&FileSystem::TEMPERAT_PAL,
			DSurface::Primary,
			r,
			g,
			b,
			!entities.empty(),
			nullptr,
			shadeCount
		);

	LightConvertClass::Array->AddItem(pLightConvert);
	entities[tint] = pLightConvert;

	return pLightConvert;
}

// =============================
// load / save

void IsometricTileTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	this->Tileset = IsometricTileTypeExt::ExtData::CurrentTileset;

	char section[0x20];
	sprintf(section, "TileSet%04d", IsometricTileTypeExt::ExtData::CurrentTileset);

	auto const theater = ScenarioClass::Instance->Theater;
	auto const pExtension = Theater::GetTheater(theater).Extension;
	char pDefault[] = "iso~~~.pal";
	pDefault[3] = pExtension[0];
	pDefault[4] = pExtension[1];
	pDefault[5] = pExtension[2];
	this->Palette.LoadFromINI(pINI, section, "CustomPalette", pDefault);
}

template <typename T>
void IsometricTileTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Tileset)
		.Process(this->Palette)
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

bool IsometricTileTypeExt::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(ExtData::CurrentTileset)
		//.Process(ExtData::LightConvertEntities)
		.Success();
}

bool IsometricTileTypeExt::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(IsometricTileTypeExt::ExtData::CurrentTileset)
		//.Process(ExtData::LightConvertEntities)
		.Success();
}

void IsometricTileTypeExt::Clear()
{
	ExtData::LightConvertEntities.clear();
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

	IsometricTileTypeExt::ExtMap.TryAllocate(pItem);

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
	LEA_STACK(CCINIClass*, pINI, STACK_OFFSET(0xA10, -0x9D8));

	IsometricTileTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

DEFINE_HOOK(0x545FA3, IsometricTileTypeClass_LoadFromINI_SetTileSet, 0x8)
{
	IsometricTileTypeExt::ExtData::CurrentTileset = R->EDI();

	return 0;
}
