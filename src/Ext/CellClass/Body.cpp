#include "Body.h"

template<> const DWORD Extension<CellClass>::Canary = 0x87784321;
CellExt::ExtContainer CellExt::ExtMap;

// ============================ =
// load / save
template <typename T>
void CellExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->NewPowerups)
		;
}
void CellExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<CellClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void CellExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<CellClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool CellExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool CellExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

// =============================
// container

CellExt::ExtContainer::ExtContainer() : Container("CellClass") {};
CellExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(47BDA1, CellClass_CTOR, 5)
{
	GET(CellClass*, pThis, ESI);
	CellExt::ExtMap.FindOrAllocate(pThis);

	return 0;
}

DEFINE_HOOK(47BBE6, CellClass_DTOR, 5)
{
	GET(CellClass*, pThis, ESI);
	CellExt::ExtMap.Remove(pThis);

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

DEFINE_HOOK(483C00, CellClass_Load_Suffix, 5)
{

	CellExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(483C79, CellClass_Save_Suffix, 6)
{

	CellExt::ExtMap.SaveStatic();
	return 0;
}