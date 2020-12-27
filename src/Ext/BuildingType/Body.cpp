#include "Body.h"
#include <BuildingTypeClass.h>
#include "../../Utilities/trim.h"

template<> const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;

// =============================
// load / save

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	if (pINI->ReadString(pSection, "PowersUp.Owner", "", Phobos::readBuffer)) {
		this->PowersUp_Owner = ParseCanTargetFlags(Phobos::readBuffer, this->PowersUp_Owner);
	}

	//Parse PowersUp.Buildings
	if (pINI->ReadString(pSection, "PowersUp.Buildings", "", this->PowersUp_Buildings_buff)) {
		char* token = strtok(this->PowersUp_Buildings_buff, Phobos::readDelims);
		while (token != 0) {
			token = Trim::FullTrim(token);
			this->PowersUp_Buildings.AddItem(token);
			token = strtok(0, Phobos::readDelims);
		}
		if (!strlen(this->OwnerObject()->PowersUpBuilding)) {
			if (this->PowersUp_Buildings.Count) {
				strcpy(this->OwnerObject()->PowersUpBuilding, PowersUp_Buildings.GetItem(0));
			}
		}
		else {
			PowersUp_Buildings.AddItem(this->OwnerObject()->PowersUpBuilding);
		}
	}
}

void BuildingTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	#define STM_Process(A) Stm->Read(&A, sizeof(A), 0);
	#include "Serialize.hpp"

	{// Load PowersUp_Buildings array
		int count = this->PowersUp_Buildings.Count;
		char* buf = this->PowersUp_Buildings_buff;

		STM_Process(count);
		for (int i = 0; i < count; i++) {
			char tempBuf[sizeof(BuildingTypeClass::ID)];
			Stm->Read(tempBuf, sizeof(tempBuf), 0);
			strcpy(buf, tempBuf);
			this->PowersUp_Buildings.AddItem(buf);

			buf += strlen(buf) + 1;
		}
	}

	#undef STM_Process

	//Stm->Read(&PowersUp_Buildings_buff, sizeof(PowersUp_Buildings_buff), 0)
}

void BuildingTypeExt::ExtData::SaveToStream(IStream* Stm) {
	#define STM_Process(A) Stm->Write(&A, sizeof(A), 0);
	#include "Serialize.hpp"
	#undef STM_Process

	{// Save PowersUp_Buildings array
		int count = this->PowersUp_Buildings.Count;
		Stm->Write(&count, sizeof(count), 0);

		for (int i = 0; i < count; i++) {
			char* item = this->PowersUp_Buildings.GetItem(i);
			Stm->Write(item, sizeof(BuildingTypeClass::ID), 0);
		}
	}
}

// =============================
// container

BuildingTypeExt::ExtContainer::ExtContainer() : Container("BuildingTypeClass") {
}

BuildingTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(45E50C, BuildingTypeClass_CTOR, 6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(45E707, BuildingTypeClass_DTOR, 6)
{
	GET(BuildingTypeClass*, pItem, ESI);

	BuildingTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(465300, BuildingTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(465010, BuildingTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(4652ED, BuildingTypeClass_Load_Suffix, 7)
{
	auto pItem = BuildingTypeExt::ExtMap.Find(BuildingTypeExt::ExtMap.SavingObject);
	IStream* pStm = BuildingTypeExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(46536A, BuildingTypeClass_Save_Suffix, 7)
{
	auto pItem = BuildingTypeExt::ExtMap.Find(BuildingTypeExt::ExtMap.SavingObject);
	IStream* pStm = BuildingTypeExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(464A56, BuildingTypeClass_LoadFromINI, A)
DEFINE_HOOK(464A49, BuildingTypeClass_LoadFromINI, A)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
