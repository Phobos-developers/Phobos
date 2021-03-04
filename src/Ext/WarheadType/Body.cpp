#include "Body.h"
#include <WarheadTypeClass.h>
#include <AnimTypeClass.h>

template<> const DWORD Extension<WarheadTypeClass>::Canary = 0x22222222;
WarheadTypeExt::ExtContainer WarheadTypeExt::ExtMap;

// =============================
// load / save

void WarheadTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	INI_EX exINI(pINI);

	this->SpySat.Read(exINI, pSection, "SpySat");
	this->BigGap.Read(exINI, pSection, "BigGap");
	this->TransactMoney.Read(exINI, pSection, "TransactMoney");
	this->SplashList.Read(exINI, pSection, "SplashList");
	this->SplashList_PickRandom.Read(exINI, pSection, "SplashList.PickRandom");
	this->RemoveDisguise.Read(exINI, pSection, "RemoveDisguise");
	this->RemoveDisguise_AffectAllies.Read(exINI, pSection, "RemoveDisguise.AffectAllies");
	this->RemoveDisguise_ApplyCellSpread.Read(exINI, pSection, "RemoveDisguise.ApplyCellSpread");
}

void WarheadTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	this->SpySat.Load(Stm);
	this->BigGap.Load(Stm);
	this->TransactMoney.Load(Stm);
	this->SplashList.Load(Stm);
	this->SplashList_PickRandom.Load(Stm);
	this->RemoveDisguise.Load(Stm);
	this->RemoveDisguise_AffectAllies.Load(Stm);
	this->RemoveDisguise_ApplyCellSpread.Load(Stm);
}

void WarheadTypeExt::ExtData::SaveToStream(IStream* Stm) const {
	this->SpySat.Save(Stm);
	this->BigGap.Save(Stm);
	this->TransactMoney.Save(Stm);
	this->SplashList.Save(Stm);
	this->SplashList_PickRandom.Save(Stm);
	this->RemoveDisguise.Save(Stm);
	this->RemoveDisguise_AffectAllies.Save(Stm);
	this->RemoveDisguise_ApplyCellSpread.Save(Stm);
}

// =============================
// container

WarheadTypeExt::ExtContainer::ExtContainer() : Container("WarheadTypeClass") {
}

WarheadTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(75D1A9, WarheadTypeClass_CTOR, 7)
{
	GET(WarheadTypeClass*, pItem, EBP);

	WarheadTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(75E5C8, WarheadTypeClass_SDDTOR, 6)
{
	GET(WarheadTypeClass*, pItem, ESI);

	WarheadTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(75E2C0, WarheadTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(75E0C0, WarheadTypeClass_SaveLoad_Prefix, 8)
{
	GET_STACK(WarheadTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WarheadTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(75E2AE, WarheadTypeClass_Load_Suffix, 7)
{
	auto pItem = WarheadTypeExt::ExtMap.Find(WarheadTypeExt::ExtMap.SavingObject);
	IStream* pStm = WarheadTypeExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(75E39C, WarheadTypeClass_Save_Suffix, 5)
{
	auto pItem = WarheadTypeExt::ExtMap.Find(WarheadTypeExt::ExtMap.SavingObject);
	IStream* pStm = WarheadTypeExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(75DEAF, WarheadTypeClass_LoadFromINI, 5)
DEFINE_HOOK(75DEA0, WarheadTypeClass_LoadFromINI, 5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
