#include "Body.h"

#include <TechnoTypeClass.h>
#include <StringTable.h>
#include <Matrix3D.h>

#include "../BulletType/Body.h"
#include "../Techno/Body.h"

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x11111111;
TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;

void TechnoTypeExt::ExtData::ApplyTurretOffset(Matrix3D* mtx, double factor)
{
	float x = static_cast<float>(this->TurretOffset.GetEx()->X * factor);
	float y = static_cast<float>(this->TurretOffset.GetEx()->Y * factor);
	float z = static_cast<float>(this->TurretOffset.GetEx()->Z * factor);

	mtx->Translate(x, y, z);
}

void TechnoTypeExt::ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor)
{
	auto ext = TechnoTypeExt::ExtMap.Find(pType);

	ext->ApplyTurretOffset(mtx, factor);
}

void TechnoTypeExt::ApplyMindControlRangeLimit(TechnoClass* pThis)
{
	if (auto Capturer = pThis->MindControlledBy) {
		auto pCapturerExt = TechnoTypeExt::ExtMap.Find(Capturer->GetTechnoType());
		if (pCapturerExt->MindControlRangeLimit > 0 && pThis->DistanceFrom(Capturer) > pCapturerExt->MindControlRangeLimit * 256.0) {
			Capturer->CaptureManager->FreeUnit(pThis);
			if (!pThis->IsHumanControlled) {
				pThis->QueueMission(Mission::Hunt, 0);
			};
		}
	}
}
void TechnoTypeExt::ApplyBuildingDeployerTargeting(TechnoClass* pThis)
{
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pThis->WhatAmI() == AbstractType::Building) {
		// Prevent target loss when vehicles are deployed into buildings.
		if (pTypeData->Deployed_RememberTarget)
		{
			auto currentMission = pThis->CurrentMission;
			// With this the vehicle will not forget who is the target until the deploy process finish
			if (pThis->Target > 0 &&
				currentMission != Mission::Construction &&
				currentMission != Mission::Guard &&
				currentMission != Mission::Attack &&
				currentMission != Mission::Selling
				) {
				pThis->QueueMission(Mission::Construction, 0);
				pThis->LastTarget = pThis->Target;
			}
			else if (pThis->Target == 0 && currentMission == Mission::Construction) {
				// Just when the deployment into structure ended the vehicle forgot the target. Just attack the original target.
				pThis->Target = pThis->LastTarget;
				pThis->QueueMission(Mission::Attack, 0);
			}
		}
	}
}
void TechnoTypeExt::ApplyInterceptor(TechnoClass* pThis)
{
	auto pData = TechnoExt::ExtMap.Find(pThis);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData->Interceptor && !pThis->Target &&
		!(pThis->WhatAmI() == AbstractType::Aircraft && pThis->GetHeight() <= 0))
	{
		for (auto const& pBullet : *BulletClass::Array) {
			if (auto pBulletTypeData = BulletTypeExt::ExtMap.Find(pBullet->Type)) {
				if (!pBulletTypeData->Interceptable)
					continue;
			}

			const double guardRange = pThis->Veterancy.IsElite() ?
				pTypeData->Interceptor_EliteGuardRange * 256 : pTypeData->Interceptor_GuardRange * 256;

			if (pBullet->Location.DistanceFrom(pThis->Location) > guardRange)
				continue;

			if (pBullet->Location.DistanceFrom(pBullet->TargetCoords) >
				double(ScenarioClass::Instance->Random.RandomRanged(128, (int)guardRange / 10)) * 10)
				continue;

			if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target)) {
				if (pThis->Owner->IsAlliedWith(pTarget)) {
					pThis->SetTarget(pBullet);
					pData->InterceptedBullet = pBullet;
					break;
				}
			}
		}
	}
}
void TechnoTypeExt::ApplyPowered_KillSpawns(TechnoClass* pThis)
{
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pThis->WhatAmI() == AbstractType::Building) {
		auto pBuilding = abstract_cast<BuildingClass*>(pThis);

		if (pTypeData->Powered_KillSpawns && pBuilding->Type->Powered && !pBuilding->IsPowerOnline()) {
			if (auto pManager = pBuilding->SpawnManager) {
				pManager->ResetTarget();

				for (auto pItem : pManager->SpawnedNodes) {
					if (pItem->Status == SpawnNodeStatus::Attacking || pItem->Status == SpawnNodeStatus::Returning) {
						pItem->Unit->ReceiveDamage(
							&pItem->Unit->Health,
							0,
							RulesClass::Global()->C4Warhead,
							nullptr, false, false, nullptr);
					}
				}
			}
		}
	}
}
void TechnoTypeExt::ApplySpawn_LimitRange(TechnoClass* pThis)
{
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData->Spawn_LimitedRange) {
		if (auto pManager = pThis->SpawnManager) {
			auto pTechnoType = pThis->GetTechnoType();
			int weaponRange = 0;
			int weaponRangeExtra = pTypeData->Spawn_LimitedExtraRange * 256;

			auto setWeaponRange = [&weaponRange](WeaponTypeClass* pWeaponType)
			{
				if (pWeaponType)
					if (pWeaponType->Spawner && pWeaponType->Range > weaponRange)
						weaponRange = pWeaponType->Range;
			};

			setWeaponRange(pTechnoType->Weapon[0].WeaponType);
			setWeaponRange(pTechnoType->Weapon[1].WeaponType);
			setWeaponRange(pTechnoType->EliteWeapon[0].WeaponType);
			setWeaponRange(pTechnoType->EliteWeapon[1].WeaponType);

			weaponRange += weaponRangeExtra;
			if (pManager->Target && (pThis->DistanceFrom(pManager->Target) > weaponRange))
				pManager->ResetTarget();

		}
	}
}

// Ares 0.A source

const char* TechnoTypeExt::ExtData::GetSelectionGroupID() const
{
	return GeneralUtils::IsValidString(this->GroupAs) ? this->GroupAs : this->OwnerObject()->ID;
}

const char* TechnoTypeExt::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if (auto pExt = TechnoTypeExt::ExtMap.Find(static_cast<TechnoTypeClass*>(pType))) {
		return pExt->GetSelectionGroupID();
	}

	return pType->ID;
}

bool TechnoTypeExt::HasSelectionGroupID(ObjectTypeClass* pType, const char* pID)
{
	auto id = TechnoTypeExt::GetSelectionGroupID(pType);
	return (_strcmpi(id, pID) == 0);
}

// =============================
// load / save

void TechnoTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	INI_EX exINI(pINI);

	this->Deployed_RememberTarget.Read(exINI, pSection, "Deployed.RememberTarget");
	this->HealthBar_Hide.Read(exINI, pSection, "HealthBar.Hide");
	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->LowSelectionPriority.Read(exINI, pSection, "LowSelectionPriority");
	this->MindControlRangeLimit.Read(exINI, pSection, "MindControlRangeLimit");
	this->Interceptor.Read(exINI, pSection, "Interceptor");
	this->Interceptor_GuardRange.Read(exINI, pSection, "Interceptor.GuardRange");
	this->Interceptor_EliteGuardRange.Read(exINI, pSection, "Interceptor.EliteGuardRange");
	this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");
	this->Spawn_LimitedRange.Read(exINI, pSection, "Spawner.LimitRange");
	this->Spawn_LimitedExtraRange.Read(exINI, pSection, "Spawner.ExtraLimitRange");

	// Ares 0.A
	this->GroupAs.Read(pINI, pSection, "GroupAs");

	//Art tags
	INI_EX exArtINI(CCINIClass::INI_Art);

	this->TurretOffset.Read(exArtINI, pThis->ImageFile, "TurretOffset");
}

void TechnoTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	this->Deployed_RememberTarget.Load(Stm);
	this->HealthBar_Hide.Load(Stm);
	this->UIDescription.Load(Stm);
	this->LowSelectionPriority.Load(Stm);
	this->MindControlRangeLimit.Load(Stm);
	this->Interceptor.Load(Stm);
	this->Interceptor_GuardRange.Load(Stm);
	this->Interceptor_EliteGuardRange.Load(Stm);
	PhobosStreamReader::Process(Stm, this->GroupAs);
	this->TurretOffset.Load(Stm);
	this->Powered_KillSpawns.Load(Stm);
	this->Spawn_LimitedRange.Load(Stm);
	this->Spawn_LimitedExtraRange.Load(Stm);
}

void TechnoTypeExt::ExtData::SaveToStream(IStream* Stm) const {
	this->Deployed_RememberTarget.Save(Stm);
	this->HealthBar_Hide.Save(Stm);
	this->UIDescription.Save(Stm);
	this->LowSelectionPriority.Save(Stm);
	this->MindControlRangeLimit.Save(Stm);
	this->Interceptor.Save(Stm);
	this->Interceptor_GuardRange.Save(Stm);
	this->Interceptor_EliteGuardRange.Save(Stm);
	PhobosStreamWriter::Process(Stm, this->GroupAs);
	this->TurretOffset.Save(Stm);
	this->Powered_KillSpawns.Save(Stm);
	this->Spawn_LimitedRange.Save(Stm);
	this->Spawn_LimitedExtraRange.Save(Stm);
}

// =============================
// container

TechnoTypeExt::ExtContainer::ExtContainer() : Container("TechnoTypeClass") {
}

TechnoTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(711835, TechnoTypeClass_CTOR, 5)
{
	GET(TechnoTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(711AE0, TechnoTypeClass_DTOR, 5)
{
	GET(TechnoTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(716DC0, TechnoTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(7162F0, TechnoTypeClass_SaveLoad_Prefix, 6)
{
	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(716DAC, TechnoTypeClass_Load_Suffix, A)
{
	auto pItem = TechnoTypeExt::ExtMap.Find(TechnoTypeExt::ExtMap.SavingObject);
	IStream* pStm = TechnoTypeExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(717094, TechnoTypeClass_Save_Suffix, 5)
{
	auto pItem = TechnoTypeExt::ExtMap.Find(TechnoTypeExt::ExtMap.SavingObject);
	IStream* pStm = TechnoTypeExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(716132, TechnoTypeClass_LoadFromINI, 5)
DEFINE_HOOK(716123, TechnoTypeClass_LoadFromINI, 5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
