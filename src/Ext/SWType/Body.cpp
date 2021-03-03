#include "Body.h"
#include <SuperWeaponTypeClass.h>
#include <StringTable.h>

template<> const DWORD Extension<SuperWeaponTypeClass>::Canary = 0x11111111;
SWTypeExt::ExtContainer SWTypeExt::ExtMap;

// =============================
// load / save

void SWTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	INI_EX exINI(pINI);
	this->Money_Amount.Read(exINI, pSection, "Money.Amount");
	this->UIDescription.Read(exINI, pSection, "UIDescription");
}

void SWTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	this->Money_Amount.Load(Stm);
	this->UIDescription.Load(Stm);
}

void SWTypeExt::ExtData::SaveToStream(IStream* Stm) const {
	this->Money_Amount.Save(Stm);
	this->UIDescription.Save(Stm);
}

// =============================
// container

SWTypeExt::ExtContainer::ExtContainer() : Container("SuperWeaponTypeClass") {
}

SWTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(6CE6F6, SuperWeaponTypeClass_CTOR, 5)
{
	GET(SuperWeaponTypeClass*, pItem, EAX);

	SWTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6CEFE0, SuperWeaponTypeClass_SDDTOR, 8)
{
	GET(SuperWeaponTypeClass*, pItem, ECX);

	SWTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(6CE8D0, SuperWeaponTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(6CE800, SuperWeaponTypeClass_SaveLoad_Prefix, A)
{
	GET_STACK(SuperWeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SWTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(6CE8BE, SuperWeaponTypeClass_Load_Suffix, 7)
{
	auto pItem = SWTypeExt::ExtMap.Find(SWTypeExt::ExtMap.SavingObject);
	IStream* pStm = SWTypeExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(6CE8EA, SuperWeaponTypeClass_Save_Suffix, 3)
{
	auto pItem = SWTypeExt::ExtMap.Find(SWTypeExt::ExtMap.SavingObject);
	IStream* pStm = SWTypeExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(6CEE50, SuperWeaponTypeClass_LoadFromINI, A)
DEFINE_HOOK(6CEE43, SuperWeaponTypeClass_LoadFromINI, A)
{
	GET(SuperWeaponTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x3FC);

	SWTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
