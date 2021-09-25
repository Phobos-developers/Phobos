#include "Body.h"

#include <TechnoTypeClass.h>
#include <StringTable.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/GeneralUtils.h>

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x11111111;
TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;

void TechnoTypeExt::ExtData::Initialize()
{
	this->ShieldType = ShieldTypeClass::FindOrAllocate(NONE_STR);
}

void TechnoTypeExt::ExtData::ApplyTurretOffset(Matrix3D* mtx, double factor)
{
	float x = static_cast<float>(this->TurretOffset.GetEx()->X * factor);
	float y = static_cast<float>(this->TurretOffset.GetEx()->Y * factor);
	float z = static_cast<float>(this->TurretOffset.GetEx()->Z * factor);

	mtx->Translate(x, y, z);
}

void TechnoTypeExt::ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor)
{
	if (auto ext = TechnoTypeExt::ExtMap.Find(pType))
		ext->ApplyTurretOffset(mtx, factor);
}

// Ares 0.A source
const char* TechnoTypeExt::ExtData::GetSelectionGroupID() const
{
	return GeneralUtils::IsValidString(this->GroupAs) ? this->GroupAs : this->OwnerObject()->ID;
}

const char* TechnoTypeExt::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if (auto pExt = TechnoTypeExt::ExtMap.Find(static_cast<TechnoTypeClass*>(pType)))
		return pExt->GetSelectionGroupID();

	return pType->ID;
}

bool TechnoTypeExt::HasSelectionGroupID(ObjectTypeClass* pType, const char* pID)
{
	auto id = TechnoTypeExt::GetSelectionGroupID(pType);

	return (_strcmpi(id, pID) == 0);
}

bool TechnoTypeExt::ExtData::IsCountedAsHarvester()
{
	auto pThis = this->OwnerObject();
	UnitTypeClass* pUnit = nullptr;

	if (pThis->WhatAmI() == AbstractType::UnitType)
		pUnit = abstract_cast<UnitTypeClass*>(pThis);

	if (this->Harvester_Counted.Get(pThis->Enslaves || pUnit && (pUnit->Harvester || pUnit->Enslaves)))
		return true;

	return false;
}

// =============================
// load / save

void TechnoTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->HealthBar_Hide.Read(exINI, pSection, "HealthBar.Hide");
	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->LowSelectionPriority.Read(exINI, pSection, "LowSelectionPriority");
	this->MindControlRangeLimit.Read(exINI, pSection, "MindControlRangeLimit");
	this->Interceptor.Read(exINI, pSection, "Interceptor");
	this->Interceptor_GuardRange.Read(exINI, pSection, "Interceptor.GuardRange");
	this->Interceptor_MinimumGuardRange.Read(exINI, pSection, "Interceptor.MinimumGuardRange");
	this->Interceptor_EliteGuardRange.Read(exINI, pSection, "Interceptor.EliteGuardRange");
	this->Interceptor_EliteMinimumGuardRange.Read(exINI, pSection, "Interceptor.EliteMinimumGuardRange");
	this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");
	this->Spawn_LimitedRange.Read(exINI, pSection, "Spawner.LimitRange");
	this->Spawn_LimitedExtraRange.Read(exINI, pSection, "Spawner.ExtraLimitRange");
	this->Harvester_Counted.Read(exINI, pSection, "Harvester.Counted");
	this->Promote_IncludeSpawns.Read(exINI, pSection, "Promote.IncludeSpawns");
	this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");
	this->MultiMindControl_ReleaseVictim.Read(exINI, pSection, "MultiMindControl.ReleaseVictim");
	this->NoManualMove.Read(exINI, pSection, "NoManualMove");
	this->InitialStrength.Read(exINI, pSection, "InitialStrength");
	this->ShieldType.Read(exINI, pSection, "ShieldType", true);
	this->CameoPriority.Read(exINI, pSection, "CameoPriority");

	this->WarpOut.Read(exINI, pSection, "WarpOut");
	this->WarpIn.Read(exINI, pSection, "WarpIn");
	this->WarpAway.Read(exINI, pSection, "WarpAway");
	this->ChronoTrigger.Read(exINI, pSection, "ChronoTrigger");
	this->ChronoDistanceFactor.Read(exINI, pSection, "ChronoDistanceFactor");
	this->ChronoMinimumDelay.Read(exINI, pSection, "ChronoMinimumDelay");
	this->ChronoRangeMinimum.Read(exINI, pSection, "ChronoRangeMinimum");
	this->ChronoDelay.Read(exINI, pSection, "ChronoDelay");

	this->OreGathering_Anims.Read(exINI, pSection, "OreGathering.Anims");
	this->OreGathering_Tiberiums.Read(exINI, pSection, "OreGathering.Tiberiums");
	this->OreGathering_FramesPerDir.Read(exINI, pSection, "OreGathering.FramesPerDir");

	this->DestroyAnim_Random.Read(exINI, pSection, "DestroyAnim.Random");
	this->NotHuman_RandomDeathSequence.Read(exINI, pSection, "NotHuman.RandomDeathSequence");

	this->DefaultDisguise.Read(exINI, pSection, "DefaultDisguise");

	this->OpenTopped_RangeBonus.Read(exINI, pSection, "OpenTopped.RangeBonus");
	this->OpenTopped_DamageMultiplier.Read(exINI, pSection, "OpenTopped.DamageMultiplier");
	this->OpenTopped_WarpDistance.Read(exINI, pSection, "OpenTopped.WarpDistance");

	// Ares 0.A
	this->GroupAs.Read(pINI, pSection, "GroupAs");

	// Art tags
	INI_EX exArtINI(CCINIClass::INI_Art);
	auto pArtSection = pThis->ImageFile;

	this->TurretOffset.Read(exArtINI, pArtSection, "TurretOffset");

	char tempBuffer[32];
	for (size_t i = 0; ; ++i)
	{
		NullableIdx<LaserTrailTypeClass> trail;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.Type", i);
		trail.Read(exArtINI, pArtSection, tempBuffer);

		if (!trail.isset())
			break;

		Valueable<CoordStruct> flh;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.FLH", i);
		flh.Read(exArtINI, pArtSection, tempBuffer);

		Valueable<bool> isOnTurret;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.IsOnTurret", i);
		isOnTurret.Read(exArtINI, pArtSection, tempBuffer);

		this->LaserTrailData.push_back({ ValueableIdx<LaserTrailTypeClass>(trail), flh, isOnTurret });
	}

	bool parseMultiWeapons = pThis->TurretCount > 0 && pThis->WeaponCount > 0;
	auto weaponCount = parseMultiWeapons ? pThis->WeaponCount : 2;
	this->WeaponBurstFLHs.resize(weaponCount);
	this->EliteWeaponBurstFLHs.resize(weaponCount);
	char tempBufferFLH[48];

	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "Weapon%d", i + 1);
			auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "SecondaryFire" : "PrimaryFire";

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "%sFLH.Burst%d", prefix, j);
			Nullable<CoordStruct> FLH;
			FLH.Read(exArtINI, pArtSection, tempBufferFLH);

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", prefix, j);
			Nullable<CoordStruct> eliteFLH;
			eliteFLH.Read(exArtINI, pArtSection, tempBufferFLH);

			if (FLH.isset() & !eliteFLH.isset())
				eliteFLH = FLH;
			else if (!FLH.isset() && !eliteFLH.isset())
				break;

			WeaponBurstFLHs[i].AddItem(FLH.Get());
			EliteWeaponBurstFLHs[i].AddItem(eliteFLH.Get());
		}
	}
}

template <typename T>
void TechnoTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->HealthBar_Hide)
		.Process(this->UIDescription)
		.Process(this->LowSelectionPriority)
		.Process(this->MindControlRangeLimit)
		.Process(this->Interceptor)
		.Process(this->Interceptor_GuardRange)
		.Process(this->Interceptor_MinimumGuardRange)
		.Process(this->Interceptor_EliteGuardRange)
		.Process(this->Interceptor_EliteMinimumGuardRange)
		.Process(this->GroupAs)
		.Process(this->TurretOffset)
		.Process(this->Powered_KillSpawns)
		.Process(this->Spawn_LimitedRange)
		.Process(this->Spawn_LimitedExtraRange)
		.Process(this->Harvester_Counted)
		.Process(this->Promote_IncludeSpawns)
		.Process(this->ImmuneToCrit)
		.Process(this->MultiMindControl_ReleaseVictim)
		.Process(this->CameoPriority)
		.Process(this->NoManualMove)
		.Process(this->InitialStrength)
		.Process(this->ShieldType)
		.Process(this->WarpOut)
		.Process(this->WarpIn)
		.Process(this->WarpAway)
		.Process(this->ChronoTrigger)
		.Process(this->ChronoDistanceFactor)
		.Process(this->ChronoMinimumDelay)
		.Process(this->ChronoRangeMinimum)
		.Process(this->ChronoDelay)
		.Process(this->OreGathering_Anims)
		.Process(this->OreGathering_Tiberiums)
		.Process(this->OreGathering_FramesPerDir)
		.Process(this->LaserTrailData)
		.Process(this->DestroyAnim_Random)
		.Process(this->NotHuman_RandomDeathSequence)
		.Process(this->DefaultDisguise)
		.Process(this->WeaponBurstFLHs)
		.Process(this->EliteWeaponBurstFLHs)
		.Process(this->OpenTopped_RangeBonus)
		.Process(this->OpenTopped_DamageMultiplier)
		.Process(this->OpenTopped_WarpDistance)
		;
}
void TechnoTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TechnoTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TechnoTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TechnoTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TechnoTypeExt::ExtData::LaserTrailDataEntry::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool TechnoTypeExt::ExtData::LaserTrailDataEntry::Save(PhobosStreamWriter& stm) const
{
	return const_cast<LaserTrailDataEntry*>(this)->Serialize(stm);
}

template <typename T>
bool TechnoTypeExt::ExtData::LaserTrailDataEntry::Serialize(T& stm)
{
	return stm
		.Process(this->idxType)
		.Process(this->FLH)
		.Process(this->IsOnTurret)
		.Success();
}

// =============================
// container

TechnoTypeExt::ExtContainer::ExtContainer() : Container("TechnoTypeClass") { }
TechnoTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x711835, TechnoTypeClass_CTOR, 0x5)
{
	GET(TechnoTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x711AE0, TechnoTypeClass_DTOR, 0x5)
{
	GET(TechnoTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x716DC0, TechnoTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x7162F0, TechnoTypeClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x716DAC, TechnoTypeClass_Load_Suffix, 0xA)
{
	TechnoTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x717094, TechnoTypeClass_Save_Suffix, 0x5)
{
	TechnoTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x716132, TechnoTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x716123, TechnoTypeClass_LoadFromINI, 0x5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679CAF, RulesClass_LoadAfterTypeData_CompleteInitialization, 0x5)
{
	//GET(CCINIClass*, pINI, ESI);

	for (auto const& pType : *BuildingTypeClass::Array)
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pType);
		pExt->CompleteInitialization();
	}

	return 0;
}
