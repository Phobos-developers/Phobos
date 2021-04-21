#include "Body.h"

template<> const DWORD Extension<CellClass>::Canary = 0xFDC49191;
CellExt::ExtContainer CellExt::ExtMap;

// =============================
// load / save

template <typename T>
void CellExt::ExtData::Serialize(T& Stm) {
	Stm
		
		;
}

void CellExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<CellClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void CellExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<CellClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}


// =============================
// container

CellExt::ExtContainer::ExtContainer() : Container("CellClass") {
}

CellExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(47B390, CellClass_CTOR, 6)
{
	GET(CellClass*, pItem, ESI);

	CellExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(47BB60, CellClass_DTOR, 6)
{
	GET(CellClass*, pItem, ECX);

	CellExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(483C10, CellClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(4839F0, CellClass_SaveLoad_Prefix, 7)
{
	GET_STACK(CellClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	CellExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(483C05, CellClass_Load_Suffix, 3)
{
	CellExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(483C7C, CellClass_Save_Suffix, 3)
{
	CellExt::ExtMap.SaveStatic();
	return 0;
}