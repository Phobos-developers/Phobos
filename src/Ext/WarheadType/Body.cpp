#include "Body.h"
#include <WarheadTypeClass.h>
#include <AnimTypeClass.h>
#include <AnimClass.h>
#include <HouseClass.h>
#include <TechnoClass.h>
#include <ScenarioClass.h>

#include "../TechnoType/Body.h"
#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/trim.h"

template<> const DWORD Extension<WarheadTypeClass>::Canary = 0x22222222;
WarheadTypeExt::ExtContainer WarheadTypeExt::ExtMap;

void WarheadTypeExt::ExtData::ApplyCrit(const CoordStruct& coords, TechnoClass* const Owner)
{
	auto& random = ScenarioClass::Instance->Random;
	if (random.RandomRanged(1, 10) / 10 >= this->CritChance)
	{
		auto const pWH = this->OwnerObject();
		pWH->CellSpread = this->CritSpread;

		auto items = Helpers::Alex::getCellSpreadItems(coords, this->OwnerObject()->CellSpread, true);
		for (const auto curTechno : items) {
			if (!curTechno || curTechno->InLimbo || !curTechno->IsAlive || !curTechno->Health) {
				continue;
			}

			if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(curTechno->GetTechnoType())) {
				if (pTypeExt->ImmuneToCrit)
					continue;
			}

			if (Owner && !WarheadTypeExt::CanAffectTarget(curTechno, Owner->Owner, this->OwnerObject())) {
				continue;
			}

			auto damage = this->CritDamage;
			curTechno->ReceiveDamage(&damage, 0, pWH, Owner, false, false, Owner->Owner);

			if (this->CritAnims.Count) {
				GameCreate<AnimClass>(this->CritAnims.GetItem(
					this->CritAnims.Count > 1 ?
					random.RandomRanged(0, this->CritAnims.Count - 1) : 0),
					curTechno->GetCoords());
			}
		}
	}
}

bool WarheadTypeExt::CanAffectTarget(TechnoClass* const pTarget, HouseClass* const pSourceHouse, WarheadTypeClass* const pWarhead) {
	if (pSourceHouse && pTarget && pWarhead) {
		// apply AffectsAllies if owner and target house are allied
		if (pSourceHouse->IsAlliedWith(pTarget->Owner)) {
			return pWarhead->AffectsAllies;
		}

		// this warhead is designed to ignore enemy units
		const auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead);
		return pExt->AffectsEnemies;
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

	this->SpySat = pINI->ReadBool(pSection, "SpySat", this->SpySat);
	this->BigGap = pINI->ReadBool(pSection, "BigGap", this->BigGap);
	this->TransactMoney = pINI->ReadInteger(pSection, "TransactMoney", this->TransactMoney);
	this->AffectsEnemies = pINI->ReadBool(pSection, "AffectsEnemies", this->AffectsEnemies);
	this->CritDamage = pINI->ReadInteger(pSection, "CritDamage", this->CritDamage);
	this->CritSpread = pINI->ReadDouble(pSection, "CritSpread", this->CritSpread);
	this->CritChance = pINI->ReadDouble(pSection, "CritChance", this->CritChance);
	pINI->ReadString(pSection, "CritAffects", this->CritAffects, this->CritAffects);

	if (pINI->ReadString(pSection, "CritAnims", this->CritAnimsBuffer, this->CritAnimsBuffer)) {
		char* context = nullptr;
		this->CritAnims.Clear();
		for (char* cur = strtok_s(this->CritAnimsBuffer, Phobos::readDelims, &context); cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context)) {
			if (auto item = AnimTypeClass::Find(Trim::FullTrim(cur)))
				this->CritAnims.AddItem(item);
		}
	}

	if (pINI->ReadString(pSection, "SplashList", this->SplashList_Buffer, this->SplashList_Buffer)) {
		char* context = nullptr;
		this->SplashList.Clear();
		for (char* cur = strtok_s(this->SplashList_Buffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context)) {
			if (auto splash = AnimTypeClass::Find(Trim::FullTrim(cur))) this->SplashList.AddItem(splash);
		}
		this->SplashList_PickRandom = pINI->ReadBool(pSection, "SplashList.PickRandom", this->SplashList_PickRandom);
	}
}

void WarheadTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	#define STM_Process(A) Stm->Read(&A, sizeof(A), 0);
	#include "Serialize.hpp"

	if (this->CritAnimsBuffer[0])
	{
		char* context = nullptr;
		this->CritAnims.Clear();
		for (char* cur = strtok_s(this->CritAnimsBuffer, Phobos::readDelims, &context); cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context)) {
			if (auto item = AnimTypeClass::Find(Trim::FullTrim(cur)))
				this->CritAnims.AddItem(item);
		}
	}

	{// Load SplashList array
		int count = this->SplashList.Count;
		char* buf = this->SplashList_Buffer;

		STM_Process(count);
		for (int i = 0; i < count; i++) {
			char tempBuf[sizeof(AnimTypeClass::ID)];
			Stm->Read(tempBuf, sizeof(tempBuf), 0);
			strcpy_s(buf, (sizeof(this->SplashList_Buffer) - (buf - this->SplashList_Buffer)), tempBuf);
			this->SplashList.AddItem(AnimTypeClass::Find(buf));
			buf += strlen(buf) + 1;
		}
	}
	#undef STM_Process
}

void WarheadTypeExt::ExtData::SaveToStream(IStream* Stm) {
	#define STM_Process(A) Stm->Write(&A, sizeof(A), 0);
	#include "Serialize.hpp"

	{// Save SplashList array
		int count = this->SplashList.Count;
		Stm->Write(&count, sizeof(count), 0);

		for (int i = 0; i < count; i++) {
			char* item = this->SplashList.GetItem(i)->ID;
			Stm->Write(item, sizeof(AnimTypeClass::ID), 0);
		}
	}
	#undef STM_Process
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
