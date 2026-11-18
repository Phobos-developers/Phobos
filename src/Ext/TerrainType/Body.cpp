#include "Body.h"

#include <Ext/Anim/Body.h>

#include <algorithm>

TerrainTypeExt::ExtContainer TerrainTypeExt::ExtMap;

int TerrainTypeExt::GetTiberiumGrowthStage()
{
	return GeneralUtils::GetRangedRandomOrSingleValue(this->SpawnsTiberium_GrowthStage.Get());
}

int TerrainTypeExt::GetCellsPerAnim()
{
	return GeneralUtils::GetRangedRandomOrSingleValue(this->SpawnsTiberium_CellsPerAnim.Get());
}

void TerrainTypeExt::PlayDestroyEffects(const CoordStruct& coords)
{
	VocClass::PlayIndexAtPos(this->DestroySound, coords);
	AnimExt::CreateRandomAnim(this->DestroyAnim, coords);
}

void TerrainTypeExt::Remove(TerrainClass* pTerrain)
{
	if (!pTerrain)
		return;

	RectangleStruct rect = RectangleStruct {};
	rect = *pTerrain->GetRenderDimensions(&rect);
	TacticalClass::Instance->RegisterDirtyArea(rect, false);
	pTerrain->Disappear(true);
	pTerrain->UnInit();
}

TerrainClass* TerrainTypeExt::FindCustomFoundationTerrain(const CellClass& cell, bool onBridge)
{
	// Unlike GetTerrain, this also sees objects while the scenario is loading.
	for (auto pObject = onBridge ? cell.AltObject : cell.FirstObject; pObject; pObject = pObject->NextObject)
	{
		if (auto const pTerrain = abstract_cast<TerrainClass*>(pObject))
		{
			if (auto const pExt = TryFetch(pTerrain->Type); pExt && pExt->HasCustomFoundation())
				return pTerrain;
		}
	}

	return nullptr;
}

void TerrainTypeExt::LoadCustomFoundation()
{
	auto const pThis = this->OwnerObject();
	const char* pSection = pThis->ImageFile;
	INI_EX art(CCINIClass::INI_Art);
	if (!art.ReadString(pSection, "Foundation") || _stricmp(art.value(), "Custom") != 0)
	{
		this->CustomFoundation.clear();
		return;
	}

	// Never use the engine's numeric foundation index for a custom foundation.
	// The engine uses that index to restore its stock data on load and on INI rereads.
	this->CustomFoundation.clear();
	int width = 0;
	int height = 0;
	if (!art.ReadInteger(pSection, "Foundation.X", &width) || width < 1 || width > MapSide)
	{
		Debug::INIParseFailed(pSection, "Foundation.X", art.value(), "Expected a width between 1 and 512");
		return;
	}
	if (!art.ReadInteger(pSection, "Foundation.Y", &height) || height < 1 || height > MapSide)
	{
		Debug::INIParseFailed(pSection, "Foundation.Y", art.value(), "Expected a height between 1 and 512");
		return;
	}

	std::vector<CellStruct> cells;
	for (int i = 0; i < width * height; ++i)
	{
		char key[32];
		sprintf_s(key, "Foundation.%d", i);
		if (!art.ReadString(pSection, key))
			break;

		int offset[2] {};
		char trailing = 0;
		if (sscanf_s(art.value(), " %d , %d %c", &offset[0], &offset[1], &trailing, 1u) != 2
			|| offset[0] < 0 || offset[0] >= width || offset[1] < 0 || offset[1] >= height)
		{
			Debug::INIParseFailed(pSection, key, art.value(), "Expected an X,Y offset inside the foundation dimensions");
			return;
		}

		cells.emplace_back(static_cast<short>(offset[0]), static_cast<short>(offset[1]));
	}

	std::ranges::sort(cells, [](const CellStruct& lhs, const CellStruct& rhs)
	{
		return lhs.Y != rhs.Y ? lhs.Y < rhs.Y : lhs.X < rhs.X;
	});
	if (cells.empty() || cells.front() != CellStruct::Empty)
	{
		Debug::INIParseFailed(pSection, "Foundation", "Custom",
			"The footprint must include (0,0) because terrain placement and removal update the anchor cell. Falling back to 1x1.");
		return;
	}

	cells.erase(std::unique(cells.begin(), cells.end()), cells.end());
	cells.push_back(FoundationEnd);
	this->CustomFoundation = std::move(cells);
	pThis->FoundationData = this->CustomFoundation.data();
}

// =============================
// load / save

template <typename T>
void TerrainTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->SpawnsTiberium_Type)
		.Process(this->SpawnsTiberium_Range)
		.Process(this->SpawnsTiberium_GrowthStage)
		.Process(this->SpawnsTiberium_CellsPerAnim)
		.Process(this->SpawnsTiberium_Particle)
		.Process(this->DestroyAnim)
		.Process(this->DestroySound)
		.Process(this->MinimapColor)
		.Process(this->IsPassable)
		.Process(this->CanBeBuiltOn)
		.Process(this->HasDamagedFrames)
		.Process(this->HasCrumblingFrames)
		.Process(this->CrumblingSound)
		.Process(this->AnimationLength)
		.Process(this->PaletteFile)
		;
}

void TerrainTypeExt::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->SpawnsTiberium_Type.Read(exINI, pSection, "SpawnsTiberium.Type");
	this->SpawnsTiberium_Range.Read(exINI, pSection, "SpawnsTiberium.Range");
	this->SpawnsTiberium_GrowthStage.Read(exINI, pSection, "SpawnsTiberium.GrowthStage");
	this->SpawnsTiberium_CellsPerAnim.Read(exINI, pSection, "SpawnsTiberium.CellsPerAnim");
	this->SpawnsTiberium_Particle.Read(exINI, pSection, "SpawnsTiberium.Particle");

	this->DestroyAnim.Read(exINI, pSection, "DestroyAnim");
	this->DestroySound.Read(exINI, pSection, "DestroySound");

	this->MinimapColor.Read(exINI, pSection, "MinimapColor");

	this->IsPassable.Read(exINI, pSection, "IsPassable");
	this->CanBeBuiltOn.Read(exINI, pSection, "CanBeBuiltOn");

	this->HasDamagedFrames.Read(exINI, pSection, "HasDamagedFrames");
	this->HasCrumblingFrames.Read(exINI, pSection, "HasCrumblingFrames");
	this->CrumblingSound.Read(exINI, pSection, "CrumblingSound");
	this->AnimationLength.Read(exINI, pSection, "AnimationLength");

	//Strength is already part of ObjecTypeClass::ReadIni Duh!
	//this->TerrainStrength.Read(exINI, pSection, "Strength");

	this->LoadCustomFoundation();

	this->PaletteFile.Read(&CCINIClass::INI_Art, pThis->ImageFile, "Palette");
	this->Palette = GeneralUtils::BuildPalette(this->PaletteFile);

	if (GeneralUtils::IsValidString(this->PaletteFile) && !this->Palette)
		Debug::Log("[Developer warning] [%s] has Palette=%s set but no palette file was loaded (missing file or wrong filename). Missing palettes cause issues with lighting recalculations.\n", pThis->ImageFile, this->PaletteFile.data());
}

void TerrainTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	ObjectTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
	if (Stm.HasRemainingData())
		Stm.Process(this->CustomFoundation);
	this->Palette = GeneralUtils::BuildPalette(this->PaletteFile);
}

void TerrainTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	ObjectTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
	Stm.Process(this->CustomFoundation);
}

void TerrainTypeExt::PostLoad()
{
	// The footprint lives in the extension, so the owner must point at the loaded storage.
	if (this->HasCustomFoundation())
		this->OwnerObject()->FoundationData = this->CustomFoundation.data();
}

bool TerrainTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TerrainTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

TerrainTypeExt::ExtContainer::ExtContainer() : Container("TerrainTypeClass") { }
TerrainTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x71DBC0, TerrainTypeClass_CTOR, 0x7)
{
	GET(TerrainTypeClass*, pItem, ESI);

	TerrainTypeExt::ExtMap.TryAllocate(pItem);

	// Override the default value (true) from game constructor.
	pItem->RadarInvisible = false;

	return 0;
}

DEFINE_HOOK_AGAIN(0x71DC04, TerrainTypeClass_SDDTOR, 0x6)
DEFINE_HOOK(0x71E364, TerrainTypeClass_SDDTOR, 0x6)
{
	GET(TerrainTypeClass*, pItem, ECX);

	TerrainTypeExt::ExtMap.Remove(pItem);

	// FileSystem owns cached SHP references; raw theater images are freed below.
	if (pItem->Image && !pItem->ImageAllocated && pItem->Image->IsReference())
		pItem->Image = nullptr;

	return 0;
}

DEFINE_HOOK(0x71E0A6, TerrainTypeClass_LoadFromINI, 0x5)
{
	GET(TerrainTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0x210, 0x4));

	TerrainTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
