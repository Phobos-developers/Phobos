#include "Body.h"

#include <Utilities/GeneralUtils.h>

static constexpr DWORD Canary = 0x1111111A;
HouseTypeExt::ExtContainer HouseTypeExt::ExtMap;

void HouseTypeExt::ExtData::Initialize()
{
}

// =============================
// load / save

void HouseTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->BattlePoints.Read(exINI, pSection, "BattlePoints");
	this->BattlePoints_CanReuseStandardPoints.Read(exINI, pSection, "BattlePoints.CanUseStandardPoints");
}

void HouseTypeExt::ExtData::CompleteInitialization()
{
	auto const pThis = this->OwnerObject();
	UNREFERENCED_PARAMETER(pThis);
}

template <typename T>
void HouseTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->BattlePoints)
		.Process(this->BattlePoints_CanReuseStandardPoints)
		;
}

void HouseTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<HouseTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void HouseTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<HouseTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool HouseTypeExt::ExtContainer::Load(HouseTypeClass* pThis, IStream* pStm)
{
	HouseTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);
	return pData != nullptr;
};

bool HouseTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool HouseTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

// =============================
// container

HouseTypeExt::ExtContainer::ExtContainer() : Container("HouseTypeClass") { }

HouseTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x511635, HouseTypeClass_CTOR_1, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x511643, HouseTypeClass_CTOR_2, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x5127CF, HouseTypeClass_DTOR, 0x6)
{
	GET(HouseTypeClass*, pItem, ESI);

	HouseTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x512480, HouseTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x512290, HouseTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x51246D, HouseTypeClass_Load_Suffix, 0x5)
{
	HouseTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x51255C, HouseTypeClass_Save_Suffix, 0x5)
{
	HouseTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x51215A, HouseTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x51214F, HouseTypeClass_LoadFromINI, 0x5)
{
	GET(HouseTypeClass*, pItem, EBX);
	GET_BASE(CCINIClass*, pINI, 0x8);

	HouseTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
