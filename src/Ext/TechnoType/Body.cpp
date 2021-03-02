#include "Body.h"
#include <TechnoTypeClass.h>
#include <StringTable.h>

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x11111111;
TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;

// =============================
// load / save

void TechnoTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	this->Deployed_RememberTarget = pINI->ReadBool(pSection, "Deployed.RememberTarget", this->Deployed_RememberTarget);
	this->HealthBar_Hide = pINI->ReadBool(pSection, "HealthBar.Hide", this->HealthBar_Hide);
	pINI->ReadString(pSection, "UIDescription", this->UIDescriptionLabel, this->UIDescriptionLabel);
	
	if (IsValidString(this->UIDescriptionLabel))
		this->UIDescription = StringTable::LoadStringA(this->UIDescriptionLabel);
	else
		this->UIDescription = L"";

	this->LowSelectionPriority = pINI->ReadBool(pSection, "LowSelectionPriority", this->LowSelectionPriority);
	this->MindControlRangeLimit = pINI->ReadDouble(pSection, "MindControlRangeLimit", this->MindControlRangeLimit);

	// Phobos 0.A
	pINI->ReadString(pSection, "GroupAs", this->GroupAs, this->GroupAs);
}

void TechnoTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	#define STM_Process(A) Stm->Read(&A, sizeof(A), 0);
	#include "Serialize.hpp"
	#undef STM_Process

	if (IsValidString(this->UIDescriptionLabel))
		this->UIDescription = StringTable::LoadStringA(this->UIDescriptionLabel);
	else
		this->UIDescription = L"";
}

void TechnoTypeExt::ExtData::SaveToStream(IStream* Stm) {
	#define STM_Process(A) Stm->Write(&A, sizeof(A), 0);
	#include "Serialize.hpp"
	#undef STM_Process
}

// =============================
// container

TechnoTypeExt::ExtContainer::ExtContainer() : Container("TechnoTypeClass") {
}

TechnoTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(711835, TechnoTypeClass_CTOR, 5)
{
	GET(TechnoTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(711AE0, TechnoTypeClass_DTOR, 5)
{
	GET(TechnoTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(716DC0, TechnoTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(7162F0, TechnoTypeClass_SaveLoad_Prefix, 6)
{
	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(716DAC, TechnoTypeClass_Load_Suffix, A)
{
	auto pItem = TechnoTypeExt::ExtMap.Find(TechnoTypeExt::ExtMap.SavingObject);
	IStream* pStm = TechnoTypeExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(717094, TechnoTypeClass_Save_Suffix, 5)
{
	auto pItem = TechnoTypeExt::ExtMap.Find(TechnoTypeExt::ExtMap.SavingObject);
	IStream* pStm = TechnoTypeExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(716132, TechnoTypeClass_LoadFromINI, 5)
DEFINE_HOOK(716123, TechnoTypeClass_LoadFromINI, 5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}


// Phobos 0.A source

const char* TechnoTypeExt::ExtData::GetSelectionGroupID() const
{
	return IsValidString(this->GroupAs) ? this->GroupAs : this->OwnerObject()->ID;
}

const char* TechnoTypeExt::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if (auto pExt = TechnoTypeExt::ExtMap.Find(static_cast<TechnoTypeClass*>(pType))) {
		return pExt->GetSelectionGroupID();
	}

	return pType->ID;
}

bool TechnoTypeExt::HasSelectionGroupID(ObjectTypeClass* pType, const char* pID)
{
	auto id = TechnoTypeExt::GetSelectionGroupID(pType);
	return (_strcmpi(id, pID) == 0);
}

