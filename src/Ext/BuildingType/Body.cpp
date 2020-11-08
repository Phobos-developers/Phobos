#include "Body.h"
#include <BuildingTypeClass.h>

template<> const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;

// =============================
// load / save

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	pINI->ReadString(pSection, "PowersUp.Owner", "", Phobos::readBuffer);
	this->PowersUp_Owner = ParseCanTargetFlags(Phobos::readBuffer, this->PowersUp_Owner);
}

void BuildingTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	#define STM_Process(A) Stm->Read(&A, sizeof(A), 0);
	#include "Serialize.hpp"
	#undef STM_Process
}

void BuildingTypeExt::ExtData::SaveToStream(IStream* Stm) {
	#define STM_Process(A) Stm->Write(&A, sizeof(A), 0);
	#include "Serialize.hpp"
	#undef STM_Process
}

// =============================
// container

BuildingTypeExt::ExtContainer::ExtContainer() : Container("BuildingTypeClass") {
}

BuildingTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(45E50C, BuildingTypeClass_CTOR, 6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(45E707, BuildingTypeClass_DTOR, 6)
{
	GET(BuildingTypeClass*, pItem, ESI);

	BuildingTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(465300, BuildingTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(465010, BuildingTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(4652ED, BuildingTypeClass_Load_Suffix, 7)
{
	auto pItem = BuildingTypeExt::ExtMap.Find(BuildingTypeExt::ExtMap.SavingObject);
	IStream* pStm = BuildingTypeExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(46536A, BuildingTypeClass_Save_Suffix, 7)
{
	auto pItem = BuildingTypeExt::ExtMap.Find(BuildingTypeExt::ExtMap.SavingObject);
	IStream* pStm = BuildingTypeExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(464A56, BuildingTypeClass_LoadFromINI, A)
DEFINE_HOOK(464A49, BuildingTypeClass_LoadFromINI, A)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
