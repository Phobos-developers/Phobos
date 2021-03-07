#include "Body.h"

template<> const DWORD Extension<BulletClass>::Canary = 0x2A2A2A2A;
BulletExt::ExtContainer BulletExt::ExtMap;

// =============================
// load / save

void BulletExt::ExtData::LoadFromStream(IStream* Stm) {
	this->Intercepted.Load(Stm);
	this->ShouldIntercept.Load(Stm);
}

void BulletExt::ExtData::SaveToStream(IStream* Stm) {
	this->Intercepted.Save(Stm);
	this->ShouldIntercept.Save(Stm);
}

// =============================
// container

BulletExt::ExtContainer::ExtContainer() : Container("BulletClass") {
}

BulletExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(4664BA, BulletClass_CTOR, 5)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(4665E9, BulletClass_DTOR, A)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(46AFB0, BulletClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(46AE70, BulletClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BulletClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(46AF97, BulletClass_Load_Suffix, 7)
DEFINE_HOOK(46AF9E, BulletClass_Load_Suffix, 7)
{
	auto pItem = BulletExt::ExtMap.Find(BulletExt::ExtMap.SavingObject);
	IStream* pStm = BulletExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(46AFC4, BulletClass_Save_Suffix, 3)
{
	auto pItem = BulletExt::ExtMap.Find(BulletExt::ExtMap.SavingObject);
	IStream* pStm = BulletExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}
