#include "Body.h"

#include <Themes.h>

template<> const DWORD Extension<SideClass>::Canary = 0x05B10501;
SideExt::ExtContainer SideExt::ExtMap;

void SideExt::ExtData::Initialize()
{
	const char* pID = this->OwnerObject()->ID;

	this->ArrayIndex = SideClass::FindIndex(pID);
	this->Sidebar_GDIPositions = this->ArrayIndex == 0; // true = Allied

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

template <typename T>
void SideExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->ArrayIndex)
		.Process(this->Sidebar_GDIPositions)
		.Process(this->IngameScore_WinTheme)
		.Process(this->IngameScore_LoseTheme)
		;
}
void SideExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<SideClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void SideExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<SideClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool SideExt::LoadGlobals(PhobosStreamReader& Stm) {
	auto ret = Stm
		.Success();

	return ret;
}

bool SideExt::SaveGlobals(PhobosStreamWriter& Stm) {
	return Stm
		.Success();
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
	SideExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(6A48FC, SideClass_Save_Suffix, 5)
{
	SideExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(679A10, SideClass_LoadAllFromINI, 5)
{
	GET_STACK(CCINIClass*, pINI, 0x4);
	SideExt::ExtMap.LoadAllFromINI(pINI); // bwahaha

	return 0;
}

/*
FINE_HOOK(6725C4, RulesClass_Addition_Sides, 8)
{
	GET(SideClass *, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x38);

	SideExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
*/