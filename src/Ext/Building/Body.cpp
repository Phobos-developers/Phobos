#include "Body.h"
#include <BuildingClass.h>

template<> const DWORD Extension<BuildingClass>::Canary = 0x87654321;
BuildingExt::ExtContainer BuildingExt::ExtMap;

// =============================
// load / save

void BuildingExt::ExtData::LoadFromStream(IStream* Stm) {

}

void BuildingExt::ExtData::SaveToStream(IStream* Stm) const {

}

// =============================
// container

BuildingExt::ExtContainer::ExtContainer() : Container("BuildingClass") {
}

BuildingExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(43BCBD, BuildingClass_CTOR, 6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(43C022, BuildingClass_DTOR, 6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(454190, BuildingClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(453E20, BuildingClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BuildingClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(45417E, BuildingClass_Load_Suffix, 5)
{
	auto pItem = BuildingExt::ExtMap.Find(BuildingExt::ExtMap.SavingObject);
	IStream* pStm = BuildingExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(454244, BuildingClass_Save_Suffix, 7)
{
	auto pItem = BuildingExt::ExtMap.Find(BuildingExt::ExtMap.SavingObject);
	IStream* pStm = BuildingExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}
