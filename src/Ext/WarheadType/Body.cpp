#include "Body.h"

#include <HouseClass.h>

template<> const DWORD Extension<WarheadTypeClass>::Canary = 0x22222222;
WarheadTypeExt::ExtContainer WarheadTypeExt::ExtMap;

bool WarheadTypeExt::ExtData::CanTargetHouse(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (pHouse && pTarget) {
		if (this->AffectsOwner && pTarget->Owner == pHouse) {
			return true;
		}

		bool isAllies = pHouse->IsAlliedWith(pTarget);

		if (this->OwnerObject()->AffectsAllies && isAllies) {
			return true;
		}

		if (this->AffectsEnemies && !isAllies) {
			return true;
		}

		return false;
	}
	return true;
}

bool WarheadTypeExt::ExtData::IsCellEligible(CellClass* const pCell, SuperWeaponTarget allowed)
{
	if (allowed & SuperWeaponTarget::AllCells) {
		if (pCell->LandType == LandType::Water) {
			// check whether it supports water
			return (allowed & SuperWeaponTarget::Water) != SuperWeaponTarget::None;
		}
		else {
			// check whether it supports non-water
			return (allowed & SuperWeaponTarget::Land) != SuperWeaponTarget::None;
		}
	}
	return true;
}

bool WarheadTypeExt::ExtData::IsTechnoEligible(TechnoClass* const pTechno, SuperWeaponTarget allowed)
{
	if (allowed & SuperWeaponTarget::AllContents) {
		if (pTechno) {
			switch (pTechno->WhatAmI()) {
			case AbstractType::Infantry:
				return (allowed & SuperWeaponTarget::Infantry) != SuperWeaponTarget::None;
			case AbstractType::Unit:
			case AbstractType::Aircraft:
				return (allowed & SuperWeaponTarget::Unit) != SuperWeaponTarget::None;
			case AbstractType::Building:
				return (allowed & SuperWeaponTarget::Building) != SuperWeaponTarget::None;
			}
		}
		else {
			// is the target cell allowed to be empty?
			return (allowed & SuperWeaponTarget::NoContent) != SuperWeaponTarget::None;
		}
	}
	return true;
}

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
	this->RemoveMindControl.Read(exINI, pSection, "RemoveMindControl");

	this->Crit_Chance.Read(exINI, pSection, "Crit.Chance");
	this->Crit_Damage.Read(exINI, pSection, "Crit.Damage");
	this->Crit_Affects.Read(exINI, pSection, "Crit.Affects");
	this->Crit_Anims.Read(exINI, pSection, "Crit.Anims");

	// Ares tags
	// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
	this->AffectsEnemies.Read(exINI, pSection, "AffectsEnemies");
	this->AffectsOwner.Read(exINI, pSection, "AffectsOwner");
}

template <typename T>
void WarheadTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->SpySat)
		.Process(this->BigGap)
		.Process(this->TransactMoney)

		.Process(this->SplashList)
		.Process(this->SplashList_PickRandom)

		.Process(this->RemoveDisguise)
		.Process(this->RemoveMindControl)

		.Process(this->Crit_Chance)
		.Process(this->Crit_Damage)
		.Process(this->Crit_Affects)
		.Process(this->Crit_Anims)

		// Ares tags
		.Process(this->AffectsEnemies)
		.Process(this->AffectsOwner)
		;
}

void WarheadTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<WarheadTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void WarheadTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<WarheadTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WarheadTypeExt::LoadGlobals(PhobosStreamReader& Stm) {
	return Stm.Success();
}

bool WarheadTypeExt::SaveGlobals(PhobosStreamWriter& Stm) {
	return Stm.Success();
}

// =============================
// container

WarheadTypeExt::ExtContainer::ExtContainer() : Container("WarheadTypeClass") {
}

WarheadTypeExt::ExtContainer::~ExtContainer() = default;

void WarheadTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {
	
}

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
	WarheadTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(75E39C, WarheadTypeClass_Save_Suffix, 5)
{
	WarheadTypeExt::ExtMap.SaveStatic();
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
