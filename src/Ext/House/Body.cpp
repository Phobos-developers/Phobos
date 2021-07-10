#include "Body.h"
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
//Static init
/*
template<> const DWORD Extension<HouseClass>::Canary = 0x11111111;
HouseExt::ExtContainer HouseExt::ExtMap;
*/

int HouseExt::ActiveHarvesterCount(HouseClass* pThis)
{
	if (!pThis) return 0;

	int result = 0;
	for (auto techno : *TechnoClass::Array) {
		if (auto pTechnoExt = TechnoTypeExt::ExtMap.Find(techno->GetTechnoType())) {
			if (pTechnoExt->IsCountedAsHarvester() && techno->Owner == pThis) {
				if (auto pTechno = TechnoExt::ExtMap.Find(techno)) {
					result += TechnoExt::IsHarvesting(techno);
				}
			}
		}
	}

	return result;
}

int HouseExt::TotalHarvesterCount(HouseClass* pThis)
{
	if (!pThis)	return 0;

	int result = 0;
	for (auto techno : *TechnoTypeClass::Array) {
		if (auto pTechnoExt = TechnoTypeExt::ExtMap.Find(techno)) {
			if (pTechnoExt->IsCountedAsHarvester()) {
				result += pThis->CountOwnedAndPresent(techno);
			}
		}
	}

	return result;
}

// =============================
// load / save

/*
void HouseExt::ExtData::LoadFromStream(IStream* Stm) {
	
}

void HouseExt::ExtData::SaveToStream(IStream* Stm) {
	
}

// =============================
// container

HouseExt::ExtContainer::ExtContainer() : Container("HouseClass") {
}

HouseExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);

	HouseExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x4F7371, HouseClass_DTOR, 0x6)
{
	GET(HouseClass*, pItem, ESI);

	HouseExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x504080, HouseClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x503040, HouseClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x504069, HouseClass_Load_Suffix, 0x7)
{
	auto pItem = HouseExt::ExtMap.Find(HouseExt::ExtMap.SavingObject);
	IStream* pStm = HouseExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);

	return 0;
}

DEFINE_HOOK(0x5046DE, HouseClass_Save_Suffix, 0x7)
{
	auto pItem = HouseExt::ExtMap.Find(HouseExt::ExtMap.SavingObject);
	IStream* pStm = HouseExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}
*/
/*
DEFINE_HOOK(0x50114D, HouseClass_InitFromINI, 0x5)
{
	GET(HouseClass* const, pThis, EBX);
	GET(CCINIClass* const, pINI, ESI);

	HouseExt::ExtMap.LoadFromINI(pThis, pINI);

	return 0;
}*/
