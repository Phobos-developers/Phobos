#include "Body.h"

#include <TerrainTypeClass.h>
#include <Utilities/GeneralUtils.h>

template<> const DWORD Extension<TerrainTypeClass>::Canary = 0xBEE78007;
TerrainTypeExt::ExtContainer TerrainTypeExt::ExtMap;

int TerrainTypeExt::ExtData::GetTiberiumGrowthStage()
{
	return GeneralUtils::GetRangedRandomOrSingleValue(this->SpawnsTiberium_GrowthStage.Get());
}

int TerrainTypeExt::ExtData::GetCellsPerAnim()
{
	return GeneralUtils::GetRangedRandomOrSingleValue(this->SpawnsTiberium_CellsPerAnim.Get());
}

// =============================
// load / save

template <typename T>
void TerrainTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->SpawnsTiberium_Type)
		.Process(this->SpawnsTiberium_Range)
		.Process(this->SpawnsTiberium_GrowthStage)
		.Process(this->SpawnsTiberium_CellsPerAnim)
		.Process(this->DestroyAnim)
		.Process(this->DestroySound)
		;
}

void TerrainTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	this->SpawnsTiberium_Type.Read(exINI, pSection, "SpawnsTiberium.Type");
	this->SpawnsTiberium_Range.Read(exINI, pSection, "SpawnsTiberium.Range");
	this->SpawnsTiberium_GrowthStage.Read(exINI, pSection, "SpawnsTiberium.GrowthStage");
	this->SpawnsTiberium_CellsPerAnim.Read(exINI, pSection, "SpawnsTiberium.CellsPerAnim");

	this->DestroyAnim.Read(exINI, pSection, "DestroyAnim");
	this->DestroySound.Read(exINI, pSection, "DestroySound");

	//Strength is already part of ObjecTypeClass::ReadIni Duh!
	//this->TerrainStrength.Read(exINI, pSection, "Strength");
}

void TerrainTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TerrainTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TerrainTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TerrainTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
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

	TerrainTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x71E364, TerrainTypeClass_SDDTOR, 0x6)
{
	GET(TerrainTypeClass*, pItem, ECX);

	TerrainTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x71E1D0, TerrainTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x71E240, TerrainTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TerrainTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TerrainTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x71E235, TerrainTypeClass_Load_Suffix, 0x5)
{
	TerrainTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x71E25A, TerrainTypeClass_Save_Suffix, 0x5)
{
	TerrainTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK(0x71E0A6, TerrainTypeClass_LoadFromINI, 0x5)
{
	GET(TerrainTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x210, -0x4));

	TerrainTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

