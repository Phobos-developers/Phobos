#include "Body.h"

//Static init
template<> const DWORD Extension<SideClass>::Canary = 0x05B10501;
SideExt::ExtContainer SideExt::ExtMap;

void SideExt::ExtData::Initialize()
{
	const char* pID = this->OwnerObject()->ID;

	this->ArrayIndex = SideClass::FindIndex(pID);

	if(this->ArrayIndex == 0) {
		// Allied
		Sidebar_GDIPositions = true;
	}
	else {
		// Other
		Sidebar_GDIPositions = false;
	}
};

void SideExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	this->Sidebar_GDIPositions = pINI->ReadBool(pSection, "Sidebar.GDIPositions", this->Sidebar_GDIPositions);
}

// =============================
// load / save

void SideExt::ExtData::LoadFromStream(IStream* Stm) {
	#define STM_Process(A) Stm->Read(&A, sizeof(A), 0);
	#include "Serialize.hpp"
	#undef STM_Process
}

void SideExt::ExtData::SaveToStream(IStream* Stm) {
	#define STM_Process(A) Stm->Write(&A, sizeof(A), 0);
	#include "Serialize.hpp"
	#undef STM_Process
}

// =============================
// container

SideExt::ExtContainer::ExtContainer() : Container("SideClass") {
}

SideExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(6A4609, SideClass_CTOR, 7)
{
	GET(SideClass*, pItem, ESI);

	SideExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6A499F, SideClass_SDDTOR, 6)
{
	GET(SideClass*, pItem, ESI);

	SideExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(6A48A0, SideClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(6A4780, SideClass_SaveLoad_Prefix, 6)
{
	GET_STACK(SideClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SideExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(6A488B, SideClass_Load_Suffix, 6)
{
	auto pItem = SideExt::ExtMap.Find(SideExt::ExtMap.SavingObject);
	IStream* pStm = SideExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(6A48FC, SideClass_Save_Suffix, 5)
{
	auto pItem = SideExt::ExtMap.Find(SideExt::ExtMap.SavingObject);
	IStream* pStm = SideExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK(679A10, SideClass_LoadAllFromINI, 5)
{
	GET_STACK(CCINIClass*, pINI, 0x4);
	SideExt::ExtMap.LoadAllFromINI(pINI);

	return 0;
}
