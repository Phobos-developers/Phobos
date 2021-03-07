#include "Body.h"

template<> const DWORD Extension<TechnoClass>::Canary = 0x55555555;
TechnoExt::ExtContainer TechnoExt::ExtMap;

// =============================
// load / save

void TechnoExt::ExtData::LoadFromStream(IStream* Stm) {
	this->InterceptedBullet.Load(Stm);
}

void TechnoExt::ExtData::SaveToStream(IStream* Stm) {
	this->InterceptedBullet.Save(Stm);
}

// =============================
// container

TechnoExt::ExtContainer::ExtContainer() : Container("TechnoClass") {
}

TechnoExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(6F3260, TechnoClass_CTOR, 5)
{
	GET(TechnoClass*, pItem, ESI);

	TechnoExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6F4500, TechnoClass_DTOR, 5)
{
	GET(TechnoClass*, pItem, ECX);

	TechnoExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(70C250, TechnoClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(70BF50, TechnoClass_SaveLoad_Prefix, 5)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(70C249, TechnoClass_Load_Suffix, 5)
{
	auto pItem = TechnoExt::ExtMap.Find(TechnoExt::ExtMap.SavingObject);
	IStream* pStm = TechnoExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(70C264, TechnoClass_Save_Suffix, 5)
{
	auto pItem = TechnoExt::ExtMap.Find(TechnoExt::ExtMap.SavingObject);
	IStream* pStm = TechnoExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}
