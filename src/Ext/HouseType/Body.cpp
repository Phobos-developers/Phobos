#include "Body.h"

#include <Ext/Side/Body.h>

template<> const DWORD Extension<HouseTypeClass>::Canary = 0xAFFEAFFE;
HouseTypeExt::ExtContainer HouseTypeExt::ExtMap;

void HouseTypeExt::ExtData::Initialize() {}

void HouseTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	INI_EX exINI(pINI);

	if (SuperWeaponTypeClass::Array->IsAllocated)
	{
		char tempBuffer[32];
		for (size_t i = 0; i <= this->ScoreSuperWeaponData.size(); ++i)
		{
			NullableIdx<SuperWeaponTypeClass> swType;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "ScoreSuperWeapon%d.Type", i);
			swType.Read(exINI, pSection, tempBuffer);

			if (!swType.isset())
				continue;

			Valueable<int> score;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "ScoreSuperWeapon%d.Score", i);
			score.Read(exINI, pSection, tempBuffer);

			Valueable<bool> oneTime;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "ScoreSuperWeapon%d.OneTime", i);
			oneTime.Read(exINI, pSection, tempBuffer);

			Valueable<bool> initialReady;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "ScoreSuperWeapon%d.InitialReady", i);
			initialReady.Read(exINI, pSection, tempBuffer);

			if (i == ScoreSuperWeaponData.size())
				this->ScoreSuperWeaponData.push_back({ ValueableIdx<SuperWeaponTypeClass>(swType), score, oneTime, initialReady, false });
			else
				this->ScoreSuperWeaponData[i] = { ValueableIdx<SuperWeaponTypeClass>(swType), score, oneTime, initialReady, false };
		}
	}
}

template <typename T>
void HouseTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->ScoreSuperWeaponData)
		;
}

void HouseTypeExt::ExtData::LoadFromStream(PhobosStreamReader &Stm)
{
	Extension<HouseTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void HouseTypeExt::ExtData::SaveToStream(PhobosStreamWriter &Stm)
{
	Extension<HouseTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool HouseTypeExt::ExtData::ScoreSuperWeaponDataEntry::Load(PhobosStreamReader &Stm, bool registerForChange)
{
	return this->Serialize(Stm);
}

bool HouseTypeExt::ExtData::ScoreSuperWeaponDataEntry::Save(PhobosStreamWriter &Stm) const
{
	return const_cast<ScoreSuperWeaponDataEntry*>(this)->Serialize(Stm);
}

template <typename T>
bool HouseTypeExt::ExtData::ScoreSuperWeaponDataEntry::Serialize(T& stm)
{
	return stm
		.Process(this->IdxType)
		.Process(this->Score)
		.Process(this->OneTime)
		.Process(this->InitialReady)
		.Process(this->AlreadyGranted)
		.Success();
}

bool HouseTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool HouseTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

HouseTypeExt::ExtContainer::ExtContainer() : Container("HouseTypeClass") {}
HouseTypeExt::ExtContainer::~ExtContainer() = default;
void HouseTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

// =============================
// container hooks

DEFINE_HOOK(0x511635, HouseTypeClass_CTOR_1, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x511643, HouseTypeClass_CTOR_2, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.FindOrAllocate(pItem);
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