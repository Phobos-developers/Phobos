#include "Body.h"
#include <Themes.h>

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

	INI_EX exINI(pINI);
	this->Sidebar_GDIPositions.Read(exINI, pSection, "Sidebar.GDIPositions");
	this->IngameScore_WinTheme = pINI->ReadTheme(pSection, "IngameScore.WinTheme", this->IngameScore_WinTheme);
	this->IngameScore_LoseTheme = pINI->ReadTheme(pSection, "IngameScore.LoseTheme", this->IngameScore_LoseTheme);
}

// =============================
// load / save

void SideExt::ExtData::LoadFromStream(IStream* Stm) {
	this->ArrayIndex.Load(Stm);
	this->Sidebar_GDIPositions.Load(Stm);
	this->IngameScore_WinTheme.Load(Stm);
	this->IngameScore_LoseTheme.Load(Stm);
}

void SideExt::ExtData::SaveToStream(IStream* Stm) const {
	this->ArrayIndex.Save(Stm);
	this->Sidebar_GDIPositions.Save(Stm);
	this->IngameScore_WinTheme.Save(Stm);
	this->IngameScore_LoseTheme.Save(Stm);
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
