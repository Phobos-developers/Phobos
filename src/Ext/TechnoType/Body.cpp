#include "Body.h"

#include <TechnoTypeClass.h>
#include <StringTable.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/GeneralUtils.h>

TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;

void TechnoTypeExt::ExtData::Initialize()
{
	this->ShieldType = ShieldTypeClass::FindOrAllocate(NONE_STR);
}

void TechnoTypeExt::ExtData::ApplyTurretOffset(Matrix3D* mtx, double factor)
{
	// Does not verify if the offset actually has all values parsed as it makes no difference, it will be 0 for the unparsed ones either way.
	auto offset = this->TurretOffset.GetEx();
	float x = static_cast<float>(offset->X * factor);
	float y = static_cast<float>(offset->Y * factor);
	float z = static_cast<float>(offset->Z * factor);

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

void TechnoTypeExt::ExtData::ParseBurstFLHs(INI_EX& exArtINI, const char* pArtSection,
	std::vector<std::vector<CoordStruct>>& nFLH, std::vector<std::vector<CoordStruct>>& nEFlh, const char* pPrefixTag)
{
	char tempBuffer[32];
	char tempBufferFLH[48];
	auto pThis = this->OwnerObject();
	bool parseMultiWeapons = pThis->TurretCount > 0 && pThis->WeaponCount > 0;
	auto weaponCount = parseMultiWeapons ? pThis->WeaponCount : 2;
	nFLH.resize(weaponCount);
	nEFlh.resize(weaponCount);

	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%sWeapon%d", pPrefixTag, i + 1);
			auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "%sSecondaryFire" : "%sPrimaryFire";
			_snprintf_s(tempBuffer, sizeof(tempBuffer), prefix, pPrefixTag);

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "%sFLH.Burst%d", tempBuffer, j);
			Nullable<CoordStruct> FLH;
			FLH.Read(exArtINI, pArtSection, tempBufferFLH);

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", tempBuffer, j);
			Nullable<CoordStruct> eliteFLH;
			eliteFLH.Read(exArtINI, pArtSection, tempBufferFLH);

			if (FLH.isset() && !eliteFLH.isset())
				eliteFLH = FLH;
			else if (!FLH.isset() && !eliteFLH.isset())
				break;

			nFLH[i].push_back(FLH.Get());
			nEFlh[i].push_back(eliteFLH.Get());
		}
	}
}

//TODO: YRpp this with proper casting
TechnoTypeClass* TechnoTypeExt::GetTechnoType(ObjectTypeClass* pType)
{
	enum class IUnknownVtbl : DWORD
	{
		AircraftType = 0x7E2868,
		BuildingType = 0x7E4570,
		InfantryType = 0x7EB610,
		UnitType = 0x7F6218,
	};
	auto const vtThis = static_cast<IUnknownVtbl>(VTable::Get(pType));
	if (vtThis == IUnknownVtbl::AircraftType ||
		vtThis == IUnknownVtbl::BuildingType ||
		vtThis == IUnknownVtbl::InfantryType ||
		vtThis == IUnknownVtbl::UnitType)
	{
		return static_cast<TechnoTypeClass*>(pType);
	}

	return nullptr;
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

	this->Spawner_LimitRange.Read(exINI, pSection, "Spawner.LimitRange");
	this->Spawner_ExtraLimitRange.Read(exINI, pSection, "Spawner.ExtraLimitRange");
	this->Spawner_DelayFrames.Read(exINI, pSection, "Spawner.DelayFrames");

	this->Harvester_Counted.Read(exINI, pSection, "Harvester.Counted");
	if (!this->Harvester_Counted.isset() && pThis->Enslaves)
		this->Harvester_Counted = true;
	if (this->Harvester_Counted.Get())
		RulesExt::Global()->HarvesterTypes.AddUnique(pThis);

	this->Promote_IncludeSpawns.Read(exINI, pSection, "Promote.IncludeSpawns");
	this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");
	this->MultiMindControl_ReleaseVictim.Read(exINI, pSection, "MultiMindControl.ReleaseVictim");
	this->NoManualMove.Read(exINI, pSection, "NoManualMove");
	this->InitialStrength.Read(exINI, pSection, "InitialStrength");
	if (this->InitialStrength.isset())
		this->InitialStrength = Math::clamp(this->InitialStrength, 1, pThis->Strength);

	this->ShieldType.Read(exINI, pSection, "ShieldType", true);

	this->Ammo_AddOnDeploy.Read(exINI, pSection, "Ammo.AddOnDeploy");
	this->Ammo_AutoDeployMinimumAmount.Read(exINI, pSection, "Ammo.AutoDeployMinimumAmount");
	this->Ammo_AutoDeployMaximumAmount.Read(exINI, pSection, "Ammo.AutoDeployMaximumAmount");
	this->Ammo_DeployUnlockMinimumAmount.Read(exINI, pSection, "Ammo.DeployUnlockMinimumAmount");
	this->Ammo_DeployUnlockMaximumAmount.Read(exINI, pSection, "Ammo.DeployUnlockMaximumAmount");

	this->VoiceCantDeploy.Read(exINI, pSection, "VoiceCantDeploy");

	this->AutoDeath_Behavior.Read(exINI, pSection, "AutoDeath.Behavior");
	this->AutoDeath_VanishAnimation.Read(exINI, pSection, "AutoDeath.VanishAnimation");
	this->AutoDeath_OnAmmoDepletion.Read(exINI, pSection, "AutoDeath.OnAmmoDepletion");
	this->AutoDeath_AfterDelay.Read(exINI, pSection, "AutoDeath.AfterDelay");
	this->AutoDeath_TechnosDontExist.Read(exINI, pSection, "AutoDeath.TechnosDontExist");
	this->AutoDeath_TechnosDontExist_Any.Read(exINI, pSection, "AutoDeath.TechnosDontExist.Any");
	this->AutoDeath_TechnosDontExist_AllowLimboed.Read(exINI, pSection, "AutoDeath.TechnosDontExist.AllowLimboed");
	this->AutoDeath_TechnosDontExist_Houses.Read(exINI, pSection, "AutoDeath.TechnosDontExist.Houses");
	this->AutoDeath_TechnosExist.Read(exINI, pSection, "AutoDeath.TechnosExist");
	this->AutoDeath_TechnosExist_Any.Read(exINI, pSection, "AutoDeath.TechnosExist.Any");
	this->AutoDeath_TechnosExist_AllowLimboed.Read(exINI, pSection, "AutoDeath.TechnosExist.AllowLimboed");
	this->AutoDeath_TechnosExist_Houses.Read(exINI, pSection, "AutoDeath.TechnosExist.Houses");

	this->Slaved_OwnerWhenMasterKilled.Read(exINI, pSection, "Slaved.OwnerWhenMasterKilled");
	this->SlavesFreeSound.Read(exINI, pSection, "SlavesFreeSound");
	this->SellSound.Read(exINI, pSection, "SellSound");
	this->EVA_Sold.Read(exINI, pSection, "EVA.Sold");

	this->VoiceCreated.Read(exINI, pSection, "VoiceCreated");
	this->CameoPriority.Read(exINI, pSection, "CameoPriority");

	this->WarpOut.Read(exINI, pSection, "WarpOut");
	this->WarpIn.Read(exINI, pSection, "WarpIn");
	this->WarpAway.Read(exINI, pSection, "WarpAway");
	this->ChronoTrigger.Read(exINI, pSection, "ChronoTrigger");
	this->ChronoDistanceFactor.Read(exINI, pSection, "ChronoDistanceFactor");
	this->ChronoMinimumDelay.Read(exINI, pSection, "ChronoMinimumDelay");
	this->ChronoRangeMinimum.Read(exINI, pSection, "ChronoRangeMinimum");
	this->ChronoDelay.Read(exINI, pSection, "ChronoDelay");

	this->WarpInWeapon.Read(exINI, pSection, "WarpInWeapon", true);
	this->WarpInMinRangeWeapon.Read(exINI, pSection, "WarpInMinRangeWeapon", true);
	this->WarpOutWeapon.Read(exINI, pSection, "WarpOutWeapon", true);
	this->WarpInWeapon_UseDistanceAsDamage.Read(exINI, pSection, "WarpInWeapon.UseDistanceAsDamage");

	this->OreGathering_Anims.Read(exINI, pSection, "OreGathering.Anims");
	this->OreGathering_Tiberiums.Read(exINI, pSection, "OreGathering.Tiberiums");
	this->OreGathering_FramesPerDir.Read(exINI, pSection, "OreGathering.FramesPerDir");

	this->DestroyAnim_Random.Read(exINI, pSection, "DestroyAnim.Random");
	this->NotHuman_RandomDeathSequence.Read(exINI, pSection, "NotHuman.RandomDeathSequence");

	this->DefaultDisguise.Read(exINI, pSection, "DefaultDisguise");
	this->UseDisguiseMovementSpeed.Read(exINI, pSection, "UseDisguiseMovementSpeed");

	this->OpenTopped_RangeBonus.Read(exINI, pSection, "OpenTopped.RangeBonus");
	this->OpenTopped_DamageMultiplier.Read(exINI, pSection, "OpenTopped.DamageMultiplier");
	this->OpenTopped_WarpDistance.Read(exINI, pSection, "OpenTopped.WarpDistance");
	this->OpenTopped_IgnoreRangefinding.Read(exINI, pSection, "OpenTopped.IgnoreRangefinding");
	this->OpenTopped_AllowFiringIfDeactivated.Read(exINI, pSection, "OpenTopped.AllowFiringIfDeactivated");
	this->OpenTopped_ShareTransportTarget.Read(exINI, pSection, "OpenTopped.ShareTransportTarget");

	this->AutoFire.Read(exINI, pSection, "AutoFire");
	this->AutoFire_TargetSelf.Read(exINI, pSection, "AutoFire.TargetSelf");

	this->NoSecondaryWeaponFallback.Read(exINI, pSection, "NoSecondaryWeaponFallback");
	this->NoSecondaryWeaponFallback_AllowAA.Read(exINI, pSection, "NoSecondaryWeaponFallback.AllowAA");

	this->JumpjetRotateOnCrash.Read(exINI, pSection, "JumpjetRotateOnCrash");

	this->DeployingAnim_AllowAnyDirection.Read(exINI, pSection, "DeployingAnim.AllowAnyDirection");
	this->DeployingAnim_KeepUnitVisible.Read(exINI, pSection, "DeployingAnim.KeepUnitVisible");
	this->DeployingAnim_ReverseForUndeploy.Read(exINI, pSection, "DeployingAnim.ReverseForUndeploy");
	this->DeployingAnim_UseUnitDrawer.Read(exINI, pSection, "DeployingAnim.UseUnitDrawer");

	this->EnemyUIName.Read(exINI, pSection, "EnemyUIName");
	this->ForceWeapon_Naval_Decloaked.Read(exINI, pSection, "ForceWeapon.Naval.Decloaked");
	this->ForceWeapon_Cloaked.Read(exINI, pSection, "ForceWeapon.Cloaked");
	this->ForceWeapon_Disguised.Read(exINI, pSection, "ForceWeapon.Disguised");
	this->Ammo_Shared.Read(exINI, pSection, "Ammo.Shared");
	this->Ammo_Shared_Group.Read(exINI, pSection, "Ammo.Shared.Group");
	this->SelfHealGainType.Read(exINI, pSection, "SelfHealGainType");
	this->Passengers_SyncOwner.Read(exINI, pSection, "Passengers.SyncOwner");
	this->Passengers_SyncOwner_RevertOnExit.Read(exINI, pSection, "Passengers.SyncOwner.RevertOnExit");

	this->IronCurtain_KeptOnDeploy.Read(exINI, pSection, "IronCurtain.KeptOnDeploy");
	this->IronCurtain_Effect.Read(exINI, pSection, "IronCurtain.Effect");
	this->IronCurtain_KillWarhead.Read(exINI, pSection, "IronCurtain.KillWarhead");

	this->Explodes_KillPassengers.Read(exINI, pSection, "Explodes.KillPassengers");
	this->DeployFireWeapon.Read(exINI, pSection, "DeployFireWeapon");
	this->TargetZoneScanType.Read(exINI, pSection, "TargetZoneScanType");

	this->Insignia.Read(exINI, pSection, "Insignia.%s");
	this->InsigniaFrames.Read(exINI, pSection, "InsigniaFrames");
	this->InsigniaFrame.Read(exINI, pSection, "InsigniaFrame.%s");
	this->Insignia_ShowEnemy.Read(exINI, pSection, "Insignia.ShowEnemy");

	this->TiltsWhenCrushes_Vehicles.Read(exINI, pSection, "TiltsWhenCrushes.Vehicles");
	this->TiltsWhenCrushes_Overlays.Read(exINI, pSection, "TiltsWhenCrushes.Overlays");
	this->CrushForwardTiltPerFrame.Read(exINI, pSection, "CrushForwardTiltPerFrame");
	this->CrushOverlayExtraForwardTilt.Read(exINI, pSection, "CrushOverlayExtraForwardTilt");
	this->CrushSlowdownMultiplier.Read(exINI, pSection, "CrushSlowdownMultiplier");

	this->DigitalDisplay_Disable.Read(exINI, pSection, "DigitalDisplay.Disable");
	this->DigitalDisplayTypes.Read(exINI, pSection, "DigitalDisplayTypes");

	this->AmmoPipFrame.Read(exINI, pSection, "AmmoPipFrame");
	this->EmptyAmmoPipFrame.Read(exINI, pSection, "EmptyAmmoPipFrame");
	this->AmmoPipWrapStartFrame.Read(exINI, pSection, "AmmoPipWrapStartFrame");
	this->AmmoPipSize.Read(exINI, pSection, "AmmoPipSize");
	this->AmmoPipOffset.Read(exINI, pSection, "AmmoPipOffset");

	this->ShowSpawnsPips.Read(exINI, pSection, "ShowSpawnsPips");
	this->SpawnsPipFrame.Read(exINI, pSection, "SpawnsPipFrame");
	this->EmptySpawnsPipFrame.Read(exINI, pSection, "EmptySpawnsPipFrame");
	this->SpawnsPipSize.Read(exINI, pSection, "SpawnsPipSize");
	this->SpawnsPipOffset.Read(exINI, pSection, "SpawnsPipOffset");

	// Ares 0.2
	this->RadarJamRadius.Read(exINI, pSection, "RadarJamRadius");

	// Ares 0.9
	this->InhibitorRange.Read(exINI, pSection, "InhibitorRange");
	this->DesignatorRange.Read(exINI, pSection, "DesignatorRange");

	// Ares 0.A
	this->GroupAs.Read(pINI, pSection, "GroupAs");

	// Ares 0.C
	this->NoAmmoWeapon.Read(exINI, pSection, "NoAmmoWeapon");
	this->NoAmmoAmount.Read(exINI, pSection, "NoAmmoAmount");

	char tempBuffer[32];

	if (this->OwnerObject()->Gunner && this->Insignia_Weapon.empty())
	{
		int weaponCount = this->OwnerObject()->WeaponCount;
		this->Insignia_Weapon.resize(weaponCount);
		this->InsigniaFrame_Weapon.resize(weaponCount);
		this->InsigniaFrames_Weapon.resize(weaponCount);

		for (int i = 0; i < weaponCount; i++)
		{
			Promotable<SHPStruct*> insignia;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "Insignia.Weapon%d.%s", i + 1, "%s");
			insignia.Read(exINI, pSection, tempBuffer);
			this->Insignia_Weapon[i] = insignia;

			Promotable<int> frame = Promotable<int>(-1);
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaFrame.Weapon%d.%s", i + 1, "%s");
			frame.Read(exINI, pSection, tempBuffer);
			this->InsigniaFrame_Weapon[i] = frame;

			Valueable<Vector3D<int>> frames;
			frames = Vector3D<int>(-1, -1, -1);
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "InsigniaFrames.Weapon%d", i + 1);
			frames.Read(exINI, pSection, tempBuffer);
			this->InsigniaFrames_Weapon[i] = frames;
		}
	}

	// Art tags
	INI_EX exArtINI(CCINIClass::INI_Art);
	auto pArtSection = pThis->ImageFile;

	this->TurretOffset.Read(exArtINI, pArtSection, "TurretOffset");
	this->TurretShadow.Read(exArtINI, pArtSection, "TurretShadow");
	this->ShadowIndices.Read(exArtINI, pArtSection, "ShadowIndices");

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

	this->ParseBurstFLHs(exArtINI, pArtSection, this->WeaponBurstFLHs, this->EliteWeaponBurstFLHs, "");
	this->ParseBurstFLHs(exArtINI, pArtSection, this->DeployedWeaponBurstFLHs, this->EliteDeployedWeaponBurstFLHs, "Deployed");
	this->ParseBurstFLHs(exArtINI, pArtSection, this->CrouchedWeaponBurstFLHs, this->EliteCrouchedWeaponBurstFLHs, "Prone");


	this->PronePrimaryFireFLH.Read(exArtINI, pArtSection, "PronePrimaryFireFLH");
	this->ProneSecondaryFireFLH.Read(exArtINI, pArtSection, "ProneSecondaryFireFLH");
	this->DeployedPrimaryFireFLH.Read(exArtINI, pArtSection, "DeployedPrimaryFireFLH");
	this->DeployedSecondaryFireFLH.Read(exArtINI, pArtSection, "DeployedSecondaryFireFLH");

	for (size_t i = 0; ; i++)
	{
		Nullable<CoordStruct> alternateFLH;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "AlternateFLH%u", i);
		alternateFLH.Read(exArtINI, pArtSection, tempBuffer);

		// ww always read all of AlternateFLH0-5
		if (i >= 5U && !alternateFLH.isset())
			break;
		else if (!alternateFLH.isset())
			alternateFLH = this->OwnerObject()->Weapon[0].FLH; // Game defaults to this for AlternateFLH, not 0,0,0

		if (this->AlternateFLHs.size() < i)
			this->AlternateFLHs[i] = alternateFLH;
		else
			this->AlternateFLHs.push_back(alternateFLH);
	}

	// Parasitic types

	bool resetValue = false;
	bool canParse = PassengerDeletionTypeClass::CanParse(exINI, pSection, resetValue);

	if (canParse)
	{
		if (this->PassengerDeletionType == nullptr)
			this->PassengerDeletionType = std::make_unique<PassengerDeletionTypeClass>(this->OwnerObject());

		this->PassengerDeletionType->LoadFromINI(pINI, pSection);
	}
	else if (resetValue)
	{
		this->PassengerDeletionType.reset();
	}


	Nullable<bool> isInterceptor;
	isInterceptor.Read(exINI, pSection, "Interceptor");

	if (isInterceptor)
	{
		if (this->InterceptorType == nullptr)
			this->InterceptorType = std::make_unique<InterceptorTypeClass>(this->OwnerObject());

		this->InterceptorType->LoadFromINI(pINI, pSection);
	}
	else if (isInterceptor.isset())
	{
		this->InterceptorType.reset();
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

		.Process(this->InterceptorType)

		.Process(this->GroupAs)
		.Process(this->RadarJamRadius)
		.Process(this->InhibitorRange)
		.Process(this->DesignatorRange)
		.Process(this->TurretOffset)
		.Process(this->TurretShadow)
		.Process(this->ShadowIndices)
		.Process(this->Spawner_LimitRange)
		.Process(this->Spawner_ExtraLimitRange)
		.Process(this->Spawner_DelayFrames)
		.Process(this->Harvester_Counted)
		.Process(this->Promote_IncludeSpawns)
		.Process(this->ImmuneToCrit)
		.Process(this->MultiMindControl_ReleaseVictim)
		.Process(this->CameoPriority)
		.Process(this->NoManualMove)
		.Process(this->InitialStrength)
		.Process(this->ShieldType)
		.Process(this->PassengerDeletionType)

		.Process(this->Ammo_AddOnDeploy)
		.Process(this->Ammo_AutoDeployMinimumAmount)
		.Process(this->Ammo_AutoDeployMaximumAmount)
		.Process(this->Ammo_DeployUnlockMinimumAmount)
		.Process(this->Ammo_DeployUnlockMaximumAmount)
		.Process(this->VoiceCantDeploy)

		.Process(this->AutoDeath_Behavior)
		.Process(this->AutoDeath_VanishAnimation)
		.Process(this->AutoDeath_OnAmmoDepletion)
		.Process(this->AutoDeath_AfterDelay)
		.Process(this->AutoDeath_TechnosDontExist)
		.Process(this->AutoDeath_TechnosDontExist_Any)
		.Process(this->AutoDeath_TechnosDontExist_AllowLimboed)
		.Process(this->AutoDeath_TechnosDontExist_Houses)
		.Process(this->AutoDeath_TechnosExist)
		.Process(this->AutoDeath_TechnosExist_Any)
		.Process(this->AutoDeath_TechnosExist_AllowLimboed)
		.Process(this->AutoDeath_TechnosExist_Houses)

		.Process(this->Slaved_OwnerWhenMasterKilled)
		.Process(this->SlavesFreeSound)
		.Process(this->SellSound)
		.Process(this->EVA_Sold)

		.Process(this->VoiceCreated)

		.Process(this->WarpOut)
		.Process(this->WarpIn)
		.Process(this->WarpAway)
		.Process(this->ChronoTrigger)
		.Process(this->ChronoDistanceFactor)
		.Process(this->ChronoMinimumDelay)
		.Process(this->ChronoRangeMinimum)
		.Process(this->ChronoDelay)
		.Process(this->WarpInWeapon)
		.Process(this->WarpInMinRangeWeapon)
		.Process(this->WarpOutWeapon)
		.Process(this->WarpInWeapon_UseDistanceAsDamage)

		.Process(this->OreGathering_Anims)
		.Process(this->OreGathering_Tiberiums)
		.Process(this->OreGathering_FramesPerDir)
		.Process(this->LaserTrailData)
		.Process(this->DestroyAnim_Random)
		.Process(this->NotHuman_RandomDeathSequence)
		.Process(this->DefaultDisguise)
		.Process(this->UseDisguiseMovementSpeed)
		.Process(this->WeaponBurstFLHs)
		.Process(this->EliteWeaponBurstFLHs)
		.Process(this->AlternateFLHs)

		.Process(this->OpenTopped_RangeBonus)
		.Process(this->OpenTopped_DamageMultiplier)
		.Process(this->OpenTopped_WarpDistance)
		.Process(this->OpenTopped_IgnoreRangefinding)
		.Process(this->OpenTopped_AllowFiringIfDeactivated)
		.Process(this->OpenTopped_ShareTransportTarget)

		.Process(this->AutoFire)
		.Process(this->AutoFire_TargetSelf)
		.Process(this->NoSecondaryWeaponFallback)
		.Process(this->NoSecondaryWeaponFallback_AllowAA)
		.Process(this->NoAmmoWeapon)
		.Process(this->NoAmmoAmount)
		.Process(this->JumpjetRotateOnCrash)

		.Process(this->DeployingAnim_AllowAnyDirection)
		.Process(this->DeployingAnim_KeepUnitVisible)
		.Process(this->DeployingAnim_ReverseForUndeploy)
		.Process(this->DeployingAnim_UseUnitDrawer)

		.Process(this->EnemyUIName)
		.Process(this->ForceWeapon_Naval_Decloaked)
		.Process(this->ForceWeapon_Cloaked)
		.Process(this->ForceWeapon_Disguised)
		.Process(this->Ammo_Shared)
		.Process(this->Ammo_Shared_Group)
		.Process(this->SelfHealGainType)
		.Process(this->Passengers_SyncOwner)
		.Process(this->Passengers_SyncOwner_RevertOnExit)
		.Process(this->Explodes_KillPassengers)

		.Process(this->PronePrimaryFireFLH)
		.Process(this->ProneSecondaryFireFLH)
		.Process(this->DeployedPrimaryFireFLH)
		.Process(this->DeployedSecondaryFireFLH)
		.Process(this->CrouchedWeaponBurstFLHs)
		.Process(this->EliteCrouchedWeaponBurstFLHs)
		.Process(this->DeployedWeaponBurstFLHs)
		.Process(this->EliteDeployedWeaponBurstFLHs)

		.Process(this->IronCurtain_KeptOnDeploy)
		.Process(this->IronCurtain_Effect)
		.Process(this->IronCurtain_KillWarhead)

		.Process(this->Explodes_KillPassengers)
		.Process(this->DeployFireWeapon)
		.Process(this->TargetZoneScanType)

		.Process(this->Insignia)
		.Process(this->InsigniaFrames)
		.Process(this->InsigniaFrame)
		.Process(this->Insignia_ShowEnemy)
		.Process(this->Insignia_Weapon)
		.Process(this->InsigniaFrame_Weapon)
		.Process(this->InsigniaFrames_Weapon)

		.Process(this->TiltsWhenCrushes_Vehicles)
		.Process(this->TiltsWhenCrushes_Overlays)
		.Process(this->CrushForwardTiltPerFrame)
		.Process(this->CrushOverlayExtraForwardTilt)
		.Process(this->CrushSlowdownMultiplier)

		.Process(this->DigitalDisplay_Disable)
		.Process(this->DigitalDisplayTypes)

		.Process(this->AmmoPipFrame)
		.Process(this->EmptyAmmoPipFrame)
		.Process(this->AmmoPipWrapStartFrame)
		.Process(this->AmmoPipSize)
		.Process(this->AmmoPipOffset)

		.Process(this->ShowSpawnsPips)
		.Process(this->SpawnsPipFrame)
		.Process(this->EmptySpawnsPipFrame)
		.Process(this->SpawnsPipSize)
		.Process(this->SpawnsPipOffset)
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

	TechnoTypeExt::ExtMap.TryAllocate(pItem);

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

#if ANYONE_ACTUALLY_USE_THIS
DEFINE_HOOK(0x679CAF, RulesClass_LoadAfterTypeData_CompleteInitialization, 0x5)
{
	//GET(CCINIClass*, pINI, ESI);

	for (auto const& [pType, pExt] : BuildingTypeExt::ExtMap)
	{
		pExt->CompleteInitialization();
	}

	return 0;
}
#endif

DEFINE_HOOK(0x747E90, UnitTypeClass_LoadFromINI, 0x5)
{
	GET(UnitTypeClass*, pItem, ESI);

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pItem))
	{
		if (!pTypeExt->Harvester_Counted.isset() && pItem->Harvester)
		{
			pTypeExt->Harvester_Counted = true;
			RulesExt::Global()->HarvesterTypes.AddUnique(pItem);
		}
	}

	return 0;
}
