#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>

#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Utilities/EnumFunctions.h>

WarheadTypeExt::ExtContainer WarheadTypeExt::ExtMap;

bool WarheadTypeExt::ExtData::CanTargetHouse(HouseClass* pHouse, TechnoClass* pTarget) const
{
	if (pHouse && pTarget)
	{
		if (this->AffectsOwner.Get(this->OwnerObject()->AffectsAllies) && pTarget->Owner == pHouse)
			return true;

		bool isAllies = pHouse->IsAlliedWith(pTarget);

		if (this->OwnerObject()->AffectsAllies && isAllies)
			return pTarget->Owner != pHouse;

		if (this->AffectsEnemies && !isAllies)
			return true;

		return false;
	}

	return true;
}

bool WarheadTypeExt::ExtData::CanAffectTarget(TechnoClass* pTarget, TechnoExt::ExtData* pTargetExt = nullptr) const
{
	if (!pTarget)
		return false;

	if (!this->EffectsRequireVerses)
		return true;

	auto armorType = pTarget->GetTechnoType()->Armor;

	if (!pTargetExt)
		pTargetExt = TechnoExt::ExtMap.Find(pTarget);

	if (pTargetExt)
	{
		if (const auto pShieldData = pTargetExt->Shield.get())
		{
			if (pShieldData->IsActive())
			{
				if (!pShieldData->CanBePenetrated(this->OwnerObject()))
					armorType = pShieldData->GetArmorType();
			}
		}
	}

	return GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), armorType) != 0.0;
}

// Checks if Warhead can affect target that might or might be currently invulnerable.
bool WarheadTypeExt::ExtData::CanAffectInvulnerable(TechnoClass* pTarget) const
{
	if (!pTarget->IsIronCurtained())
		return true;

	return pTarget->ForceShielded ? this->PenetratesForceShield.Get(this->PenetratesIronCurtain) : this->PenetratesIronCurtain;
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse)
{
	WarheadTypeExt::DetonateAt(pThis, pTarget->GetCoords(), pOwner, damage, pFiringHouse, pTarget);
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse, AbstractClass* pTarget)
{
	BulletTypeClass* pType = BulletTypeExt::GetDefaultBulletType();

	if (BulletClass* pBullet = pType->CreateBullet(pTarget, pOwner,
		damage, pThis, 0, pThis->Bright))
	{
		if (pFiringHouse)
		{
			auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
			pBulletExt->FirerHouse = pFiringHouse;
		}

		pBullet->Limbo();
		pBullet->SetLocation(coords);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

bool WarheadTypeExt::ExtData::EligibleForFullMapDetonation(TechnoClass* pTechno, HouseClass* pOwner) const
{
	if (!pTechno || !pTechno->IsOnMap || !pTechno->IsAlive || pTechno->InLimbo || pTechno->IsSinking)
		return false;

	if (pOwner && !EnumFunctions::CanTargetHouse(this->DetonateOnAllMapObjects_AffectHouses, pOwner, pTechno->Owner))
		return false;

	if ((this->DetonateOnAllMapObjects_AffectTypes.size() > 0 &&
		!this->DetonateOnAllMapObjects_AffectTypes.Contains(pTechno->GetTechnoType())) ||
		this->DetonateOnAllMapObjects_IgnoreTypes.Contains(pTechno->GetTechnoType()))
	{
		return false;
	}

	if (this->DetonateOnAllMapObjects_RequireVerses)
	{
		auto const pExt = TechnoExt::ExtMap.Find(pTechno);
		auto armorType = pTechno->GetTechnoType()->Armor;

		if (pExt->Shield && pExt->Shield->IsActive() && !pExt->Shield->CanBePenetrated(this->OwnerObject()))
			armorType = pExt->Shield->GetArmorType();

		if (GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), armorType) == 0.0)
			return false;
	}

	return true;
}

// Wrapper for MapClass::DamageArea() that sets a pointer in WarheadTypeExt::ExtData that is used to figure 'intended' target of the Warhead detonation, if set and there's no CellSpread.
DamageAreaResult WarheadTypeExt::ExtData::DamageAreaWithTarget(const CoordStruct& coords, int damage, TechnoClass* pSource, WarheadTypeClass* pWH, bool affectsTiberium, HouseClass* pSourceHouse, TechnoClass* pTarget)
{
	this->DamageAreaTarget = pTarget;
	auto result = MapClass::DamageArea(coords, damage, pSource, pWH, true, pSourceHouse);
	this->DamageAreaTarget = nullptr;
	return result;
}

// =============================
// load / save

void WarheadTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	// Miscs
	this->SpySat.Read(exINI, pSection, "SpySat");
	this->BigGap.Read(exINI, pSection, "BigGap");
	this->TransactMoney.Read(exINI, pSection, "TransactMoney");
	this->TransactMoney_Display.Read(exINI, pSection, "TransactMoney.Display");
	this->TransactMoney_Display_Houses.Read(exINI, pSection, "TransactMoney.Display.Houses");
	this->TransactMoney_Display_AtFirer.Read(exINI, pSection, "TransactMoney.Display.AtFirer");
	this->TransactMoney_Display_Offset.Read(exINI, pSection, "TransactMoney.Display.Offset");
	this->SplashList.Read(exINI, pSection, "SplashList");
	this->SplashList_PickRandom.Read(exINI, pSection, "SplashList.PickRandom");
	this->SplashList_CreateAll.Read(exINI, pSection, "SplashList.CreateAll");
	this->SplashList_CreationInterval.Read(exINI, pSection, "SplashList.CreationInterval");
	this->SplashList_ScatterMin.Read(exINI, pSection, "SplashList.ScatterMin");
	this->SplashList_ScatterMax.Read(exINI, pSection, "SplashList.ScatterMax");
	this->AnimList_PickRandom.Read(exINI, pSection, "AnimList.PickRandom");
	this->AnimList_CreateAll.Read(exINI, pSection, "AnimList.CreateAll");
	this->AnimList_CreationInterval.Read(exINI, pSection, "AnimList.CreationInterval");
	this->AnimList_ScatterMin.Read(exINI, pSection, "AnimList.ScatterMin");
	this->AnimList_ScatterMax.Read(exINI, pSection, "AnimList.ScatterMax");
	this->CreateAnimsOnZeroDamage.Read(exINI, pSection, "CreateAnimsOnZeroDamage");
	this->Conventional_IgnoreUnits.Read(exINI, pSection, "Conventional.IgnoreUnits");
	this->RemoveDisguise.Read(exINI, pSection, "RemoveDisguise");
	this->RemoveMindControl.Read(exINI, pSection, "RemoveMindControl");
	this->RemoveParasite.Read(exINI, pSection, "RemoveParasite");
	this->DecloakDamagedTargets.Read(exINI, pSection, "DecloakDamagedTargets");
	this->ShakeIsLocal.Read(exINI, pSection, "ShakeIsLocal");
	this->ApplyModifiersOnNegativeDamage.Read(exINI, pSection, "ApplyModifiersOnNegativeDamage");
	this->PenetratesIronCurtain.Read(exINI, pSection, "PenetratesIronCurtain");
	this->PenetratesForceShield.Read(exINI, pSection, "PenetratesForceShield");
	this->Rocker_AmplitudeMultiplier.Read(exINI, pSection, "Rocker.AmplitudeMultiplier");
	this->Rocker_AmplitudeOverride.Read(exINI, pSection, "Rocker.AmplitudeOverride");

	// Crits
	this->Crit_Chance.Read(exINI, pSection, "Crit.Chance");
	this->Crit_ApplyChancePerTarget.Read(exINI, pSection, "Crit.ApplyChancePerTarget");
	this->Crit_ExtraDamage.Read(exINI, pSection, "Crit.ExtraDamage");
	this->Crit_Warhead.Read<true>(exINI, pSection, "Crit.Warhead");
	this->Crit_Warhead_FullDetonation.Read(exINI, pSection, "Crit.Warhead.FullDetonation");
	this->Crit_Affects.Read(exINI, pSection, "Crit.Affects");
	this->Crit_AffectsHouses.Read(exINI, pSection, "Crit.AffectsHouses");
	this->Crit_AnimList.Read(exINI, pSection, "Crit.AnimList");
	this->Crit_AnimList_PickRandom.Read(exINI, pSection, "Crit.AnimList.PickRandom");
	this->Crit_AnimList_CreateAll.Read(exINI, pSection, "Crit.AnimList.CreateAll");
	this->Crit_ActiveChanceAnims.Read(exINI, pSection, "Crit.ActiveChanceAnims");
	this->Crit_AnimOnAffectedTargets.Read(exINI, pSection, "Crit.AnimOnAffectedTargets");
	this->Crit_AffectBelowPercent.Read(exINI, pSection, "Crit.AffectBelowPercent");
	this->Crit_SuppressWhenIntercepted.Read(exINI, pSection, "Crit.SuppressWhenIntercepted");

	this->MindControl_Anim.Read(exINI, pSection, "MindControl.Anim");

	// Shields
	this->Shield_Penetrate.Read(exINI, pSection, "Shield.Penetrate");
	this->Shield_Break.Read(exINI, pSection, "Shield.Break");
	this->Shield_BreakAnim.Read(exINI, pSection, "Shield.BreakAnim");
	this->Shield_HitAnim.Read(exINI, pSection, "Shield.HitAnim");
	this->Shield_SkipHitAnim.Read(exINI, pSection, "Shield.SkipHitAnim");
	this->Shield_HitFlash.Read(exINI, pSection, "Shield.HitFlash");
	this->Shield_BreakWeapon.Read<true>(exINI, pSection, "Shield.BreakWeapon");
	this->Shield_AbsorbPercent.Read(exINI, pSection, "Shield.AbsorbPercent");
	this->Shield_PassPercent.Read(exINI, pSection, "Shield.PassPercent");
	this->Shield_ReceivedDamage_Minimum.Read(exINI, pSection, "Shield.ReceivedDamage.Minimum");
	this->Shield_ReceivedDamage_Maximum.Read(exINI, pSection, "Shield.ReceivedDamage.Maximum");
	this->Shield_ReceivedDamage_MinMultiplier.Read(exINI, pSection, "Shield.ReceivedDamage.MinMultiplier");
	this->Shield_ReceivedDamage_MaxMultiplier.Read(exINI, pSection, "Shield.ReceivedDamage.MaxMultiplier");
	this->Shield_Respawn_Duration.Read(exINI, pSection, "Shield.Respawn.Duration");
	this->Shield_Respawn_Amount.Read(exINI, pSection, "Shield.Respawn.Amount");
	this->Shield_Respawn_Rate_InMinutes.Read(exINI, pSection, "Shield.Respawn.Rate");
	this->Shield_Respawn_Rate = (int)(this->Shield_Respawn_Rate_InMinutes * 900);
	this->Shield_Respawn_RestartTimer.Read(exINI, pSection, "Shield.Respawn.RestartTimer");
	this->Shield_SelfHealing_Duration.Read(exINI, pSection, "Shield.SelfHealing.Duration");
	this->Shield_SelfHealing_Amount.Read(exINI, pSection, "Shield.SelfHealing.Amount");
	this->Shield_SelfHealing_Rate_InMinutes.Read(exINI, pSection, "Shield.SelfHealing.Rate");
	this->Shield_SelfHealing_Rate = (int)(this->Shield_SelfHealing_Rate_InMinutes * 900);
	this->Shield_SelfHealing_RestartInCombat.Read(exINI, pSection, "Shield.SelfHealing.RestartInCombat");
	this->Shield_SelfHealing_RestartInCombat.Read(exINI, pSection, "Shield.SelfHealing.RestartInCombatDelay");
	this->Shield_SelfHealing_RestartTimer.Read(exINI, pSection, "Shield.SelfHealing.RestartTimer");
	this->Shield_AttachTypes.Read(exINI, pSection, "Shield.AttachTypes");
	this->Shield_RemoveTypes.Read(exINI, pSection, "Shield.RemoveTypes");
	this->Shield_ReplaceOnly.Read(exINI, pSection, "Shield.ReplaceOnly");
	this->Shield_RemoveAll.Read(exINI, pSection, "Shield.RemoveAll");
	this->Shield_ReplaceNonRespawning.Read(exINI, pSection, "Shield.ReplaceNonRespawning");
	this->Shield_InheritStateOnReplace.Read(exINI, pSection, "Shield.InheritStateOnReplace");
	this->Shield_MinimumReplaceDelay.Read(exINI, pSection, "Shield.MinimumReplaceDelay");
	this->Shield_AffectTypes.Read(exINI, pSection, "Shield.AffectTypes");
	this->Shield_Penetrate_Types.Read(exINI, pSection, "Shield.Penetrate.Types");
	this->Shield_Break_Types.Read(exINI, pSection, "Shield.Break.Types");
	this->Shield_Respawn_Types.Read(exINI, pSection, "Shield.Respawn.Types");
	this->Shield_SelfHealing_Types.Read(exINI, pSection, "Shield.SelfHealing.Types");

	this->NotHuman_DeathSequence.Read(exINI, pSection, "NotHuman.DeathSequence");
	this->LaunchSW.Read(exINI, pSection, "LaunchSW");
	this->LaunchSW_RealLaunch.Read(exINI, pSection, "LaunchSW.RealLaunch");
	this->LaunchSW_IgnoreInhibitors.Read(exINI, pSection, "LaunchSW.IgnoreInhibitors");
	this->LaunchSW_IgnoreDesignators.Read(exINI, pSection, "LaunchSW.IgnoreDesignators");
	this->LaunchSW_DisplayMoney.Read(exINI, pSection, "LaunchSW.DisplayMoney");
	this->LaunchSW_DisplayMoney_Houses.Read(exINI, pSection, "LaunchSW.DisplayMoney.Houses");
	this->LaunchSW_DisplayMoney_Offset.Read(exINI, pSection, "LaunchSW.DisplayMoney.Offset");

	this->AllowDamageOnSelf.Read(exINI, pSection, "AllowDamageOnSelf");
	this->DebrisAnims.Read(exINI, pSection, "DebrisAnims");
	this->Debris_Conventional.Read(exINI, pSection, "Debris.Conventional");

	this->DetonateOnAllMapObjects.Read(exINI, pSection, "DetonateOnAllMapObjects");
	this->DetonateOnAllMapObjects_Full.Read(exINI, pSection, "DetonateOnAllMapObjects.Full");
	this->DetonateOnAllMapObjects_RequireVerses.Read(exINI, pSection, "DetonateOnAllMapObjects.RequireVerses");
	this->DetonateOnAllMapObjects_AffectTargets.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectTargets");
	this->DetonateOnAllMapObjects_AffectHouses.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectHouses");
	this->DetonateOnAllMapObjects_AffectTypes.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectTypes");
	this->DetonateOnAllMapObjects_IgnoreTypes.Read(exINI, pSection, "DetonateOnAllMapObjects.IgnoreTypes");

	this->Nonprovocative.Read(exINI, pSection, "Nonprovocative");

	this->CombatLightDetailLevel.Read(exINI, pSection, "CombatLightDetailLevel");
	this->CombatLightChance.Read(exINI, pSection, "CombatLightChance");
	this->CLIsBlack.Read(exINI, pSection, "CLIsBlack");
	this->Particle_AlphaImageIsLightFlash.Read(exINI, pSection, "Particle.AlphaImageIsLightFlash");

	this->SuppressRevengeWeapons.Read(exINI, pSection, "SuppressRevengeWeapons");
	this->SuppressRevengeWeapons_Types.Read(exINI, pSection, "SuppressRevengeWeapons.Types");
	this->SuppressReflectDamage.Read(exINI, pSection, "SuppressReflectDamage");
	this->SuppressReflectDamage_Types.Read(exINI, pSection, "SuppressReflectDamage.Types");

	// Convert.From & Convert.To
	TypeConvertGroup::Parse(this->Convert_Pairs, exINI, pSection, AffectedHouse::All);

	// AttachEffect
	this->AttachEffects.LoadFromINI(pINI, pSection);

#ifdef LOCO_TEST_WARHEADS // Enable warheads parsing
	this->InflictLocomotor.Read(exINI, pSection, "InflictLocomotor");
	this->RemoveInflictedLocomotor.Read(exINI, pSection, "RemoveInflictedLocomotor");

	if (this->InflictLocomotor && pThis->Locomotor == _GUID())
	{
		Debug::Log("[Developer warning][%s] InflictLocomotor is specified but Locomotor is not set!", pSection);
		this->InflictLocomotor = false;
	}

	if ((this->InflictLocomotor || this->RemoveInflictedLocomotor) && pThis->IsLocomotor)
	{
		Debug::Log("[Developer warning][%s] InflictLocomotor=yes/RemoveInflictedLocomotor=yes can't be specified while IsLocomotor is set!", pSection);
		this->InflictLocomotor = this->RemoveInflictedLocomotor = false;
	}

	if (this->InflictLocomotor && this->RemoveInflictedLocomotor)
	{
		Debug::Log("[Developer warning][%s] InflictLocomotor=yes and RemoveInflictedLocomotor=yes can't be set simultaneously!", pSection);
		this->InflictLocomotor = this->RemoveInflictedLocomotor = false;
	}
#endif

	// Ares tags
	// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
	this->AffectsEnemies.Read(exINI, pSection, "AffectsEnemies");
	this->AffectsOwner.Read(exINI, pSection, "AffectsOwner");
	this->EffectsRequireVerses.Read(exINI, pSection, "EffectsRequireVerses");

	// List all Warheads here that respect CellSpread
	// Used in WarheadTypeExt::ExtData::Detonate
	this->PossibleCellSpreadDetonate = (
		this->RemoveDisguise
		|| this->RemoveMindControl
		|| this->Shield_Break
		|| this->Shield_Respawn_Duration > 0
		|| this->Shield_SelfHealing_Duration > 0
		|| this->Shield_AttachTypes.size() > 0
		|| this->Shield_RemoveTypes.size() > 0
		|| this->Shield_RemoveAll
		|| this->Convert_Pairs.size() > 0
		|| this->InflictLocomotor
		|| this->RemoveInflictedLocomotor
		|| this->AttachEffects.AttachTypes.size() > 0
		|| this->AttachEffects.RemoveTypes.size() > 0
		|| this->AttachEffects.RemoveGroups.size() > 0
	);

	char tempBuffer[32];
	Nullable<Powerup> crateType;
	Nullable<int> weight;

	for (size_t i = 0; ; i++)
	{
		crateType.Reset();
		weight.Reset();

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "SpawnsCrate%u.Type", i);
		crateType.Read(exINI, pSection, tempBuffer);

		if (i == 0 && !crateType.isset())
			crateType.Read(exINI, pSection, "SpawnsCrate.Type");

		if (!crateType.isset())
			break;

		if (this->SpawnsCrate_Types.size() < i)
			this->SpawnsCrate_Types[i] = crateType;
		else
			this->SpawnsCrate_Types.push_back(crateType);

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "SpawnsCrate%u.Weight", i);
		weight.Read(exINI, pSection, tempBuffer);

		if (i == 0 && !weight.isset())
			weight.Read(exINI, pSection, "SpawnsCrate.Weight");

		if (!weight.isset())
			weight = 1;

		if (this->SpawnsCrate_Weights.size() < i)
			this->SpawnsCrate_Weights[i] = weight;
		else
			this->SpawnsCrate_Weights.push_back(weight);
	}
}

template <typename T>
void WarheadTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->SpySat)
		.Process(this->BigGap)
		.Process(this->TransactMoney)
		.Process(this->TransactMoney_Display)
		.Process(this->TransactMoney_Display_Houses)
		.Process(this->TransactMoney_Display_AtFirer)
		.Process(this->TransactMoney_Display_Offset)
		.Process(this->SplashList)
		.Process(this->SplashList_PickRandom)
		.Process(this->SplashList_CreateAll)
		.Process(this->SplashList_CreationInterval)
		.Process(this->SplashList_ScatterMin)
		.Process(this->SplashList_ScatterMax)
		.Process(this->AnimList_PickRandom)
		.Process(this->AnimList_CreateAll)
		.Process(this->AnimList_CreationInterval)
		.Process(this->AnimList_ScatterMin)
		.Process(this->AnimList_ScatterMax)
		.Process(this->CreateAnimsOnZeroDamage)
		.Process(this->Conventional_IgnoreUnits)
		.Process(this->RemoveDisguise)
		.Process(this->RemoveMindControl)
		.Process(this->RemoveParasite)
		.Process(this->DecloakDamagedTargets)
		.Process(this->ShakeIsLocal)
		.Process(this->ApplyModifiersOnNegativeDamage)
		.Process(this->PenetratesIronCurtain)
		.Process(this->PenetratesForceShield)
		.Process(this->Rocker_AmplitudeMultiplier)
		.Process(this->Rocker_AmplitudeOverride)

		.Process(this->Crit_Chance)
		.Process(this->Crit_ApplyChancePerTarget)
		.Process(this->Crit_ExtraDamage)
		.Process(this->Crit_Warhead)
		.Process(this->Crit_Warhead_FullDetonation)
		.Process(this->Crit_Affects)
		.Process(this->Crit_AffectsHouses)
		.Process(this->Crit_AnimList)
		.Process(this->Crit_AnimList_PickRandom)
		.Process(this->Crit_AnimList_CreateAll)
		.Process(this->Crit_ActiveChanceAnims)
		.Process(this->Crit_AnimOnAffectedTargets)
		.Process(this->Crit_AffectBelowPercent)
		.Process(this->Crit_SuppressWhenIntercepted)

		.Process(this->MindControl_Anim)

		.Process(this->Shield_Penetrate)
		.Process(this->Shield_Break)
		.Process(this->Shield_BreakAnim)
		.Process(this->Shield_HitAnim)
		.Process(this->Shield_SkipHitAnim)
		.Process(this->Shield_HitFlash)
		.Process(this->Shield_BreakWeapon)
		.Process(this->Shield_AbsorbPercent)
		.Process(this->Shield_PassPercent)
		.Process(this->Shield_ReceivedDamage_Minimum)
		.Process(this->Shield_ReceivedDamage_Maximum)
		.Process(this->Shield_ReceivedDamage_MinMultiplier)
		.Process(this->Shield_ReceivedDamage_MaxMultiplier)
		.Process(this->Shield_Respawn_Duration)
		.Process(this->Shield_Respawn_Amount)
		.Process(this->Shield_Respawn_Rate)
		.Process(this->Shield_Respawn_RestartTimer)
		.Process(this->Shield_SelfHealing_Duration)
		.Process(this->Shield_SelfHealing_Amount)
		.Process(this->Shield_SelfHealing_Rate)
		.Process(this->Shield_SelfHealing_RestartInCombat)
		.Process(this->Shield_SelfHealing_RestartInCombatDelay)
		.Process(this->Shield_SelfHealing_RestartTimer)
		.Process(this->Shield_AttachTypes)
		.Process(this->Shield_RemoveTypes)
		.Process(this->Shield_RemoveAll)
		.Process(this->Shield_ReplaceOnly)
		.Process(this->Shield_ReplaceNonRespawning)
		.Process(this->Shield_InheritStateOnReplace)
		.Process(this->Shield_MinimumReplaceDelay)
		.Process(this->Shield_AffectTypes)
		.Process(this->Shield_Penetrate_Types)
		.Process(this->Shield_Break_Types)
		.Process(this->Shield_Respawn_Types)
		.Process(this->Shield_SelfHealing_Types)

		.Process(this->SpawnsCrate_Types)
		.Process(this->SpawnsCrate_Weights)

		.Process(this->NotHuman_DeathSequence)
		.Process(this->LaunchSW)
		.Process(this->LaunchSW_RealLaunch)
		.Process(this->LaunchSW_IgnoreInhibitors)
		.Process(this->LaunchSW_IgnoreDesignators)
		.Process(this->LaunchSW_DisplayMoney)
		.Process(this->LaunchSW_DisplayMoney_Houses)
		.Process(this->LaunchSW_DisplayMoney_Offset)

		.Process(this->AllowDamageOnSelf)
		.Process(this->DebrisAnims)
		.Process(this->Debris_Conventional)

		.Process(this->DetonateOnAllMapObjects)
		.Process(this->DetonateOnAllMapObjects_Full)
		.Process(this->DetonateOnAllMapObjects_RequireVerses)
		.Process(this->DetonateOnAllMapObjects_AffectTargets)
		.Process(this->DetonateOnAllMapObjects_AffectHouses)
		.Process(this->DetonateOnAllMapObjects_AffectTypes)
		.Process(this->DetonateOnAllMapObjects_IgnoreTypes)

		.Process(this->Convert_Pairs)
		.Process(this->AttachEffects)

		.Process(this->SuppressRevengeWeapons)
		.Process(this->SuppressRevengeWeapons_Types)
		.Process(this->SuppressReflectDamage)
		.Process(this->SuppressReflectDamage_Types)

		.Process(this->InflictLocomotor)
		.Process(this->RemoveInflictedLocomotor)

		.Process(this->Nonprovocative)

		.Process(this->CombatLightDetailLevel)
		.Process(this->CombatLightChance)
		.Process(this->CLIsBlack)
		.Process(this->Particle_AlphaImageIsLightFlash)

		// Ares tags
		.Process(this->AffectsEnemies)
		.Process(this->AffectsOwner)
		.Process(this->EffectsRequireVerses)

		.Process(this->WasDetonatedOnAllMapObjects)
		.Process(this->RemainingAnimCreationInterval)
		.Process(this->PossibleCellSpreadDetonate)
		.Process(this->Reflected)
		.Process(this->DamageAreaTarget)
		;
}

void WarheadTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<WarheadTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void WarheadTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<WarheadTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WarheadTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool WarheadTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

// =============================
// container

WarheadTypeExt::ExtContainer::ExtContainer() : Container("WarheadTypeClass") { }

WarheadTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x75D1A9, WarheadTypeClass_CTOR, 0x7)
{
	GET(WarheadTypeClass*, pItem, EBP);

	WarheadTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x75E5C8, WarheadTypeClass_SDDTOR, 0x6)
{
	GET(WarheadTypeClass*, pItem, ESI);

	WarheadTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x75E2C0, WarheadTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x75E0C0, WarheadTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(WarheadTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WarheadTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x75E2AE, WarheadTypeClass_Load_Suffix, 0x7)
{
	WarheadTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x75E39C, WarheadTypeClass_Save_Suffix, 0x5)
{
	WarheadTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x75DEAF, WarheadTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x75DEA0, WarheadTypeClass_LoadFromINI, 0x5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
