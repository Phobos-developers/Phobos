#include "Body.h"

template<> const DWORD Extension<BulletTypeClass>::Canary = 0xF00DF00D;
BulletTypeExt::ExtContainer BulletTypeExt::ExtMap;

// =============================
// load / save

void BulletTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	INI_EX exINI(pINI);

	this->Interceptable.Read(exINI, pSection, "Interceptable");
}

void BulletTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	this->Interceptable.Load(Stm);
}

void BulletTypeExt::ExtData::SaveToStream(IStream* Stm) {
	this->Interceptable.Save(Stm);
}

// =============================
// container

BulletTypeExt::ExtContainer::ExtContainer() : Container("BulletTypeClass") {
}

BulletTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(46BDD9, BulletTypeClass_CTOR, 5)
{
	GET(BulletTypeClass*, pItem, EAX);

	BulletTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(46C8B6, BulletTypeClass_SDDTOR, 6)
{
	GET(BulletTypeClass*, pItem, ESI);

	BulletTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(46C730, BulletTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(46C6A0, BulletTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BulletTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(46C722, BulletTypeClass_Load_Suffix, 4)
{
	auto pItem = BulletTypeExt::ExtMap.Find(BulletTypeExt::ExtMap.SavingObject);
	IStream* pStm = BulletTypeExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(46C74A, BulletTypeClass_Save_Suffix, 3)
{
	auto pItem = BulletTypeExt::ExtMap.Find(BulletTypeExt::ExtMap.SavingObject);
	IStream* pStm = BulletTypeExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(46C429, BulletTypeClass_LoadFromINI, A)
DEFINE_HOOK(46C41C, BulletTypeClass_LoadFromINI, A)
{
	GET(BulletTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x90);

	BulletTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
