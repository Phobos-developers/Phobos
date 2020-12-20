#include "Body.h"
#include <SuperWeaponTypeClass.h>
#include <StringTable.h>

template<> const DWORD Extension<SuperWeaponTypeClass>::Canary = 0x11111111;
SuperWeaponTypeExt::ExtContainer SuperWeaponTypeExt::ExtMap;

// =============================
// load / save

void SuperWeaponTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	pINI->ReadString(pSection, "UIDescription", "", this->UIDescriptionLabel);

	if (strlen(this->UIDescriptionLabel) != 0)
		this->UIDescription = StringTable::LoadStringA(this->UIDescriptionLabel);
	else
		this->UIDescription = L"";
}

void SuperWeaponTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	#define STM_Process(A) Stm->Read(&A, sizeof(A), 0);
	#include "Serialize.hpp"
	#undef STM_Process

	if (strlen(this->UIDescriptionLabel) != 0)
		this->UIDescription = StringTable::LoadStringA(this->UIDescriptionLabel);
	else
		this->UIDescription = L"";
}

void SuperWeaponTypeExt::ExtData::SaveToStream(IStream* Stm) {
	#define STM_Process(A) Stm->Write(&A, sizeof(A), 0);
	#include "Serialize.hpp"
	#undef STM_Process
}

// =============================
// container

SuperWeaponTypeExt::ExtContainer::ExtContainer() : Container("SuperWeaponTypeClass") {
}

SuperWeaponTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(6CE6F6, SuperWeaponTypeClass_CTOR, 5)
{
	GET(SuperWeaponTypeClass*, pItem, EAX);

	SuperWeaponTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6CEFE0, SuperWeaponTypeClass_SDDTOR, 8)
{
	GET(SuperWeaponTypeClass*, pItem, ECX);

	SuperWeaponTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(6CE8D0, SuperWeaponTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(6CE800, SuperWeaponTypeClass_SaveLoad_Prefix, A)
{
	GET_STACK(SuperWeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SuperWeaponTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(6CE8BE, SuperWeaponTypeClass_Load_Suffix, 7)
{
	auto pItem = SuperWeaponTypeExt::ExtMap.Find(SuperWeaponTypeExt::ExtMap.SavingObject);
	IStream* pStm = SuperWeaponTypeExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(6CE8EA, SuperWeaponTypeClass_Save_Suffix, 3)
{
	auto pItem = SuperWeaponTypeExt::ExtMap.Find(SuperWeaponTypeExt::ExtMap.SavingObject);
	IStream* pStm = SuperWeaponTypeExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(6CEE50, SuperWeaponTypeClass_LoadFromINI, A)
DEFINE_HOOK(6CEE43, SuperWeaponTypeClass_LoadFromINI, A)
{
	GET(SuperWeaponTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x3FC);

	SuperWeaponTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
