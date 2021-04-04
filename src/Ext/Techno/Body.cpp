#include "Body.h"
#include <BuildingClass.h>
#include <HouseClass.h>

template<> const DWORD Extension<TechnoClass>::Canary = 0x55555555;
TechnoExt::ExtContainer TechnoExt::ExtMap;

bool TechnoExt::IsHarvesting(TechnoClass* pThis) {
	if (pThis->InLimbo) {
		return false;
	}

	if (auto slave = pThis->SlaveManager) {
		if (slave->State != SlaveManagerStatus::Ready){
			return true;
		}
	}

	if (pThis->WhatAmI() == AbstractType::Building) {
		if (pThis->IsPowerOnline()) {
			return true;
		}
	}

	auto mission = pThis->GetCurrentMission();
	if ((mission == Mission::Harvest || mission == Mission::Unload || mission == Mission::Enter)
		&& TechnoExt::HasAvailableDock(pThis)) {
		return true;
	}

	return false;
}

bool TechnoExt::HasAvailableDock(TechnoClass* pThis) {
	for (auto pBld : pThis->GetTechnoType()->Dock) {
		if (pThis->Owner->CountOwnedAndPresent(pBld)) {
			return true;
		}
	}
	return false;
}

// =============================
// load / save

template <typename T>
void TechnoExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->InterceptedBullet)
        .Process(this->ShieldData)
        ;
}

void TechnoExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<TechnoClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TechnoExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<TechnoClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TechnoExt::LoadGlobals(PhobosStreamReader& Stm) {
	return Stm
		.Success();
}

bool TechnoExt::SaveGlobals(PhobosStreamWriter& Stm) {
	return Stm
		.Success();
}

// =============================
// container

TechnoExt::ExtContainer::ExtContainer() : Container("TechnoClass") {
}

TechnoExt::ExtContainer::~ExtContainer() = default;

void TechnoExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {

}

// =============================
// container hooks

DEFINE_HOOK(6F3260, TechnoClass_CTOR, 5)
{
	GET(TechnoClass*, pItem, ESI);

	TechnoExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6F4500, TechnoClass_DTOR, 5)
{
	GET(TechnoClass*, pItem, ECX);

	//TechnoExt::ExtData *pItemExt = TechnoExt::ExtMap.Find(pItem);
	TechnoExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(70C250, TechnoClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(70BF50, TechnoClass_SaveLoad_Prefix, 5)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(70C249, TechnoClass_Load_Suffix, 5)
{
	TechnoExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(70C264, TechnoClass_Save_Suffix, 5)
{
	TechnoExt::ExtMap.SaveStatic();
	return 0;
}