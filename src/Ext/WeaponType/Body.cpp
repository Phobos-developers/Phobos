#include "Body.h"

template<> const DWORD Extension<WeaponTypeClass>::Canary = 0x22222222;
WeaponTypeExt::ExtContainer WeaponTypeExt::ExtMap;

// =============================
// load / save

void WeaponTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	INI_EX exINI(pINI);

	this->DiskLaser_Radius.Read(exINI, pSection, "DiskLaser.Radius");
	this->DiskLaser_Circumference = (int)(this->DiskLaser_Radius * Math::Pi * 2);
}

void WeaponTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	// this->DiskLaser_Radius.Load(Stm); // There is no need to this. It is used only for LoadFromINIFile
	this->DiskLaser_Circumference.Load(Stm);
}

void WeaponTypeExt::ExtData::SaveToStream(IStream* Stm) const {
	// this->DiskLaser_Radius.Save(Stm); // There is no need to this. It is used only for LoadFromINIFile
	this->DiskLaser_Circumference.Save(Stm);
}

// =============================
// container

WeaponTypeExt::ExtContainer::ExtContainer() : Container("WeaponTypeClass") {
}

WeaponTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(771EE9, WeaponTypeClass_CTOR, 5)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(77311D, WeaponTypeClass_SDDTOR, 6)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(772EB0, WeaponTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(772CD0, WeaponTypeClass_SaveLoad_Prefix, 7)
{
	GET_STACK(WeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WeaponTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(772EA6, WeaponTypeClass_Load_Suffix, 6)
{
	auto pItem = WeaponTypeExt::ExtMap.Find(WeaponTypeExt::ExtMap.SavingObject);
	IStream* pStm = WeaponTypeExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(772F8C, WeaponTypeClass_Save, 5)
{
	auto pItem = WeaponTypeExt::ExtMap.Find(WeaponTypeExt::ExtMap.SavingObject);
	IStream* pStm = WeaponTypeExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(7729C7, WeaponTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(7729D6, WeaponTypeClass_LoadFromINI, 5)
DEFINE_HOOK(7729B0, WeaponTypeClass_LoadFromINI, 5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE4);

	WeaponTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
