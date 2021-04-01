#include "Body.h"

#include <TerrainTypeClass.h>

template<> const DWORD Extension<TerrainTypeClass>::Canary = 0xBEE78007;
TerrainTypeExt::ExtContainer TerrainTypeExt::ExtMap;

// =============================
// load / save

template <typename T>
void TerrainTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->SpawnsTiberium_Type)
		;
}

void TerrainTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	INI_EX exINI(pINI);
	this->SpawnsTiberium_Type.Read(exINI, pSection, "SpawnsTiberium.Type");
}

void TerrainTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<TerrainTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TerrainTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<TerrainTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TerrainTypeExt::LoadGlobals(PhobosStreamReader& Stm) {
	return Stm
		.Success();
}

bool TerrainTypeExt::SaveGlobals(PhobosStreamWriter& Stm) {
	return Stm
		.Success();
}

// =============================
// container

TerrainTypeExt::ExtContainer::ExtContainer() : Container("TerrainTypeClass") {
}

TerrainTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(71DBC2, TerrainTypeClass_CTOR, 5)
{
	GET(TerrainTypeClass*, pItem, ESI);

	TerrainTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(71E360, TerrainTypeClass_SDDTOR, A)
{
	GET(TerrainTypeClass*, pItem, ECX);

	TerrainTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(71E1D0, TerrainTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(71E240, TerrainTypeClass_SaveLoad_Prefix, 8)
{
	GET_STACK(TerrainTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TerrainTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(71E235, TerrainTypeClass_Load_Suffix, 4)
{
	TerrainTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(71E25A, TerrainTypeClass_Save_Suffix, 3)
{
	TerrainTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(71E0B4, TerrainTypeClass_LoadFromINI, A)
DEFINE_HOOK(71E0A7, TerrainTypeClass_LoadFromINI, A)
{
	GET(TerrainTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x20C, 0x4));

	TerrainTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

