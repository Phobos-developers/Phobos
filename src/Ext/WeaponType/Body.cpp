#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

WeaponTypeExt::ExtContainer WeaponTypeExt::ExtMap;

bool WeaponTypeExt::IsAllowedTarget(WeaponTypeClass* pWeapon, TechnoClass* pTarget, bool useWeaponTargeting, TechnoClass* pSource, HouseClass* pOwner)
{
	if (!pWeapon || !pTarget)
		return false;

	if (!pOwner && pSource)
		pOwner = pSource->Owner;

	auto const pWH = pWeapon->Warhead;

	if (useWeaponTargeting)
	{
		auto const pType = pTarget->GetTechnoType();

		if (!pType->LegalTarget || GeneralUtils::GetWarheadVersusArmor(pWH, pTarget, pType) == 0.0)
			return false;

		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

		if (pWeaponExt->SkipWeaponPicking)
			return true;

		if (!pOwner
			|| !EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, pTarget->Owner)
			|| !EnumFunctions::IsCellEligible(pTarget->GetCell(), pWeaponExt->CanTarget, true, true)
			|| !EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget)
			|| !pWeaponExt->IsHealthInThreshold(pTarget)
			|| !pWeaponExt->IsVeterancyInThreshold(pTarget))
		{
			return false;
		}

		if (!pWeaponExt->HasRequiredAttachedEffects(pTarget, pSource))
			return false;
	}
	else
	{
		if (!pOwner || !WarheadTypeExt::ExtMap.Find(pWH)->CanTargetHouse(pOwner, pTarget))
			return false;
	}

	return true;
}

bool WeaponTypeExt::ExtData::HasRequiredAttachedEffects(TechnoClass* pTarget, TechnoClass* pFirer) const
{
	const bool hasRequiredTypes = this->AttachEffect_RequiredTypes.size() > 0;
	const bool hasDisallowedTypes = this->AttachEffect_DisallowedTypes.size() > 0;
	const bool hasRequiredGroups = this->AttachEffect_RequiredGroups.size() > 0;
	const bool hasDisallowedGroups = this->AttachEffect_DisallowedGroups.size() > 0;

	if (hasRequiredTypes || hasDisallowedTypes || hasRequiredGroups || hasDisallowedGroups)
	{
		auto pTechno = pTarget;

		if (this->AttachEffect_CheckOnFirer)
			pTechno = pFirer;

		if (!pTechno)
			return true;

		auto const pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
		auto const pWH = this->OwnerObject()->Warhead;

		if (hasDisallowedTypes && pTechnoExt->HasAttachedEffects(this->AttachEffect_DisallowedTypes, false, this->AttachEffect_IgnoreFromSameSource, pFirer, pWH, &this->AttachEffect_DisallowedMinCounts, &this->AttachEffect_DisallowedMaxCounts))
			return false;

		if (hasDisallowedGroups && pTechnoExt->HasAttachedEffects(AttachEffectTypeClass::GetTypesFromGroups(this->AttachEffect_DisallowedGroups), false, this->AttachEffect_IgnoreFromSameSource, pFirer, pWH, &this->AttachEffect_DisallowedMinCounts, &this->AttachEffect_DisallowedMaxCounts))
			return false;

		if (hasRequiredTypes && !pTechnoExt->HasAttachedEffects(this->AttachEffect_RequiredTypes, true, this->AttachEffect_IgnoreFromSameSource, pFirer, pWH, &this->AttachEffect_RequiredMinCounts, &this->AttachEffect_RequiredMaxCounts))
			return false;

		if (hasRequiredGroups && !pTechnoExt->HasAttachedEffects(AttachEffectTypeClass::GetTypesFromGroups(this->AttachEffect_RequiredGroups), true, this->AttachEffect_IgnoreFromSameSource, pFirer, pWH, &this->AttachEffect_RequiredMinCounts, &this->AttachEffect_RequiredMaxCounts))
			return false;
	}

	return true;
}

bool WeaponTypeExt::ExtData::IsHealthInThreshold(TechnoClass* pTarget) const
{
	return TechnoExt::IsHealthInThreshold(pTarget, this->CanTarget_MinHealth, this->CanTarget_MaxHealth);
}

bool WeaponTypeExt::ExtData::IsVeterancyInThreshold(TechnoClass* pTarget) const
{
	return EnumFunctions::CanTargetVeterancy(this->CanTargetVeterancy, pTarget);
}

void WeaponTypeExt::ExtData::Initialize()
{
	this->RadType = RadTypeClass::FindOrAllocate(GameStrings::Radiation);
}

int WeaponTypeExt::ExtData::GetBurstDelay(int burstIndex) const
{
	int burstDelay = -1;

	if (burstIndex == 0)
		return 0;
	else if (this->Burst_Delays.size() > (unsigned)burstIndex)
		burstDelay = this->Burst_Delays[burstIndex - 1];
	else if (this->Burst_Delays.size() > 0)
		burstDelay = this->Burst_Delays[this->Burst_Delays.size() - 1];

	return burstDelay;
}

// =============================
// load / save

void WeaponTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);
	char tempBuffer[0x40];

	this->DiskLaser_Radius.Read(exINI, pSection, "DiskLaser.Radius");
	this->ProjectileRange.Read(exINI, pSection, "ProjectileRange");

	for (int idx = 0; idx < 3; ++idx)
	{
		_snprintf_s(tempBuffer, _TRUNCATE, "Bolt.Color%d", idx + 1);
		this->Bolt_Color[idx].Read(exINI, pSection, tempBuffer);

		_snprintf_s(tempBuffer, _TRUNCATE, "Bolt.Disable%d", idx + 1);
		this->Bolt_Disable[idx].Read(exINI, pSection, tempBuffer);
	}

	this->Bolt_ParticleSystem.Read<false, true>(exINI, pSection, "Bolt.ParticleSystem");
	this->Bolt_Arcs.Read(exINI, pSection, "Bolt.Arcs");
	this->Bolt_Duration.Read(exINI, pSection, "Bolt.Duration");
	this->Bolt_FollowFLH.Read(exINI, pSection, "Bolt.FollowFLH");

	this->RadType.Read<true>(exINI, pSection, "RadType");

	this->Strafing.Read(exINI, pSection, "Strafing");
	this->Strafing_Shots.Read(exINI, pSection, "Strafing.Shots");
	this->Strafing_SimulateBurst.Read(exINI, pSection, "Strafing.SimulateBurst");
	this->Strafing_UseAmmoPerShot.Read(exINI, pSection, "Strafing.UseAmmoPerShot");
	this->Strafing_TargetCell.Read(exINI, pSection, "Strafing.TargetCell");
	this->Strafing_EndDelay.Read(exINI, pSection, "Strafing.EndDelay");
	this->CanTarget.Read(exINI, pSection, "CanTarget");
	this->CanTargetHouses.Read(exINI, pSection, "CanTargetHouses");
	this->CanTarget_MaxHealth.Read(exINI, pSection, "CanTarget.MaxHealth");
	this->CanTarget_MinHealth.Read(exINI, pSection, "CanTarget.MinHealth");
	this->CanTargetVeterancy.Read(exINI, pSection, "CanTargetVeterancy");
	this->CanTarget_IronCurtained.Read(exINI, pSection, "CanTarget.IronCurtained");
	this->AutoTarget_IronCurtained.Read(exINI, pSection, "AutoTarget.IronCurtained");
	this->Burst_Delays.Read(exINI, pSection, "Burst.Delays");
	this->Burst_FireWithinSequence.Read(exINI, pSection, "Burst.FireWithinSequence");
	this->Burst_NoDelay.Read(exINI, pSection, "Burst.NoDelay");
	this->AreaFire_Target.Read(exINI, pSection, "AreaFire.Target");
	this->FeedbackWeapon.Read<true>(exINI, pSection, "FeedbackWeapon");
	this->Laser_IsSingleColor.Read(exINI, pSection, "IsSingleColor");
	this->LaserZAdjust.Read(exINI, pSection, "LaserZAdjust");
	this->EBoltZAdjust.Read(exINI, pSection, "EBoltZAdjust");
	this->EBoltZAdjust_ClampInitialDepthForBuilding.Read(exINI, pSection, "EBoltZAdjust.ClampInitialDepthForBuilding");
	this->VisualScatter.Read(exINI, pSection, "VisualScatter");
	this->ROF_RandomDelay.Read(exINI, pSection, "ROF.RandomDelay");
	this->ChargeTurret_Delays.Read(exINI, pSection, "ChargeTurret.Delays");
	this->OmniFire_TurnToTarget.Read(exINI, pSection, "OmniFire.TurnToTarget");
	this->FireOnce_ResetSequence.Read(exINI, pSection, "FireOnce.ResetSequence");
	this->TurretRecoil_Suppress.Read(exINI, pSection, "TurretRecoil.Suppress");
	this->ExtraWarheads.Read(exINI, pSection, "ExtraWarheads");
	this->ExtraWarheads_DamageOverrides.Read(exINI, pSection, "ExtraWarheads.DamageOverrides");
	this->ExtraWarheads_DetonationChances.Read(exINI, pSection, "ExtraWarheads.DetonationChances");
	this->ExtraWarheads_RollChances.Read(exINI, pSection, "ExtraWarheads.RollChances");

	// ExtraWarheads.RandomWeights
	for (size_t i = 0; ; ++i)
	{
		ValueableVector<int> weights3;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "ExtraWarheads.RandomWeights%d", i);
		weights3.Read(exINI, pSection, tempBuffer);

		if (!weights3.size())
			break;

		this->ExtraWarheads_WeightsData.emplace_back(std::move(weights3));
	}
	ValueableVector<int> weights3;
	weights3.Read(exINI, pSection, "ExtraWarheads.RandomWeights");
	if (weights3.size())
	{
		if (this->ExtraWarheads_WeightsData.size())
			this->ExtraWarheads_WeightsData[0] = std::move(weights3);
		else
			this->ExtraWarheads_WeightsData.emplace_back(std::move(weights3));
	}

	this->ExtraWarheads_FullDetonation.Read(exINI, pSection, "ExtraWarheads.FullDetonation");
	this->AmbientDamage_Warhead.Read<true>(exINI, pSection, "AmbientDamage.Warhead");
	this->AmbientDamage_IgnoreTarget.Read(exINI, pSection, "AmbientDamage.IgnoreTarget");
	this->AttachEffect_RequiredTypes.Read(exINI, pSection, "AttachEffect.RequiredTypes");
	this->AttachEffect_DisallowedTypes.Read(exINI, pSection, "AttachEffect.DisallowedTypes");
	exINI.ParseStringList(this->AttachEffect_RequiredGroups, pSection, "AttachEffect.RequiredGroups");
	exINI.ParseStringList(this->AttachEffect_DisallowedGroups, pSection, "AttachEffect.DisallowedGroups");
	this->AttachEffect_RequiredMinCounts.Read(exINI, pSection, "AttachEffect.RequiredMinCounts");
	this->AttachEffect_RequiredMaxCounts.Read(exINI, pSection, "AttachEffect.RequiredMaxCounts");
	this->AttachEffect_DisallowedMinCounts.Read(exINI, pSection, "AttachEffect.DisallowedMinCounts");
	this->AttachEffect_DisallowedMaxCounts.Read(exINI, pSection, "AttachEffect.DisallowedMaxCounts");
	this->AttachEffect_CheckOnFirer.Read(exINI, pSection, "AttachEffect.CheckOnFirer");
	this->AttachEffect_IgnoreFromSameSource.Read(exINI, pSection, "AttachEffect.IgnoreFromSameSource");
	this->KeepRange.Read(exINI, pSection, "KeepRange");
	this->KeepRange_AllowAI.Read(exINI, pSection, "KeepRange.AllowAI");
	this->KeepRange_AllowPlayer.Read(exINI, pSection, "KeepRange.AllowPlayer");
	this->KeepRange_EarlyStopFrame.Read(exINI, pSection, "KeepRange.EarlyStopFrame");
	this->KickOutPassengers.Read(exINI, pSection, "KickOutPassengers");
	this->Beam_Color.Read(exINI, pSection, "Beam.Color");
	this->Beam_Duration.Read(exINI, pSection, "Beam.Duration");
	this->Beam_Amplitude.Read(exINI, pSection, "Beam.Amplitude");
	this->Beam_IsHouseColor.Read(exINI, pSection, "Beam.IsHouseColor");
	this->LaserThickness.Read(exINI, pSection, "LaserThickness");
	this->DelayedFire_Duration.Read(exINI, pSection, "DelayedFire.Duration");
	this->DelayedFire_SkipInTransport.Read(exINI, pSection, "DelayedFire.SkipInTransport");
	this->DelayedFire_Animation.Read(exINI, pSection, "DelayedFire.Animation");
	this->DelayedFire_OpenToppedAnimation.Read(exINI, pSection, "DelayedFire.OpenToppedAnimation");
	this->DelayedFire_AnimIsAttached.Read(exINI, pSection, "DelayedFire.AnimIsAttached");
	this->DelayedFire_CenterAnimOnFirer.Read(exINI, pSection, "DelayedFire.CenterAnimOnFirer");
	this->DelayedFire_RemoveAnimOnNoDelay.Read(exINI, pSection, "DelayedFire.RemoveAnimOnNoDelay");
	this->DelayedFire_PauseFiringSequence.Read(exINI, pSection, "DelayedFire.PauseFiringSequence");
	this->DelayedFire_OnlyOnInitialBurst.Read(exINI, pSection, "DelayedFire.OnlyOnInitialBurst");
	this->DelayedFire_AnimOffset.Read(exINI, pSection, "DelayedFire.AnimOffset");
	this->DelayedFire_AnimOnTurret.Read(exINI, pSection, "DelayedFire.AnimOnTurret");
	this->ExtraRange_TargetMoving.Read(exINI, GameStrings::General, "ExtraRange.TargetMoving");
	this->ExtraRange_FirerMoving.Read(exINI, GameStrings::General, "ExtraRange.FirerMoving");
	this->ExtraRange_Prefiring.Read(exINI, GameStrings::General, "ExtraRange.Prefiring");
	this->ExtraRange_Prefiring_IncludeBurst.Read(exINI, GameStrings::General, "ExtraRange.Prefiring.IncludeBurst");
	this->AttackFriendlies.Read(exINI, pSection, "AttackFriendlies");
	this->AttackCursorOnFriendlies.Read(exINI, pSection, "AttackCursorOnFriendlies");
	this->AttackNoThreatBuildings.Read(exINI, pSection, "AttackNoThreatBuildings");
	this->CylinderRangefinding.Read(exINI, pSection, "CylinderRangefinding");
	this->Anim_Update.Read(exINI, pSection, "Anim.Update");

	// handle SkipWeaponPicking
	if (this->CanTarget != AffectedTarget::All || this->CanTargetHouses != AffectedHouse::All
		|| this->CanTarget_MaxHealth < 1.0 || this->CanTarget_MinHealth > 0.0
		|| this->CanTargetVeterancy != AffectedVeterancy::All
		|| this->AttachEffect_RequiredTypes.size() || this->AttachEffect_RequiredGroups.size()
		|| this->AttachEffect_DisallowedTypes.size() || this->AttachEffect_DisallowedGroups.size())
	{
		this->SkipWeaponPicking = false;
	}
}

template <typename T>
void WeaponTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DiskLaser_Radius)
		.Process(this->ProjectileRange)
		.Process(this->Bolt_Color)
		.Process(this->Bolt_Disable)
		.Process(this->Bolt_ParticleSystem)
		.Process(this->Bolt_Arcs)
		.Process(this->Bolt_Duration)
		.Process(this->Bolt_FollowFLH)
		.Process(this->Strafing)
		.Process(this->Strafing_Shots)
		.Process(this->Strafing_SimulateBurst)
		.Process(this->Strafing_UseAmmoPerShot)
		.Process(this->Strafing_TargetCell)
		.Process(this->Strafing_EndDelay)
		.Process(this->CanTarget)
		.Process(this->CanTargetHouses)
		.Process(this->CanTarget_MaxHealth)
		.Process(this->CanTarget_MinHealth)
		.Process(this->CanTargetVeterancy)
		.Process(this->CanTarget_IronCurtained)
		.Process(this->AutoTarget_IronCurtained)
		.Process(this->RadType)
		.Process(this->Burst_Delays)
		.Process(this->Burst_FireWithinSequence)
		.Process(this->Burst_NoDelay)
		.Process(this->AreaFire_Target)
		.Process(this->FeedbackWeapon)
		.Process(this->Laser_IsSingleColor)
		.Process(this->LaserZAdjust)
		.Process(this->EBoltZAdjust)
		.Process(this->EBoltZAdjust_ClampInitialDepthForBuilding)
		.Process(this->VisualScatter)
		.Process(this->ROF_RandomDelay)
		.Process(this->ChargeTurret_Delays)
		.Process(this->OmniFire_TurnToTarget)
		.Process(this->FireOnce_ResetSequence)
		.Process(this->TurretRecoil_Suppress)
		.Process(this->ExtraWarheads)
		.Process(this->ExtraWarheads_DamageOverrides)
		.Process(this->ExtraWarheads_DetonationChances)
		.Process(this->ExtraWarheads_RollChances)
		.Process(this->ExtraWarheads_WeightsData)
		.Process(this->ExtraWarheads_FullDetonation)
		.Process(this->AmbientDamage_Warhead)
		.Process(this->AmbientDamage_IgnoreTarget)
		.Process(this->AttachEffect_RequiredTypes)
		.Process(this->AttachEffect_DisallowedTypes)
		.Process(this->AttachEffect_RequiredGroups)
		.Process(this->AttachEffect_DisallowedGroups)
		.Process(this->AttachEffect_RequiredMinCounts)
		.Process(this->AttachEffect_RequiredMaxCounts)
		.Process(this->AttachEffect_DisallowedMinCounts)
		.Process(this->AttachEffect_DisallowedMaxCounts)
		.Process(this->AttachEffect_CheckOnFirer)
		.Process(this->AttachEffect_IgnoreFromSameSource)
		.Process(this->KeepRange)
		.Process(this->KeepRange_AllowAI)
		.Process(this->KeepRange_AllowPlayer)
		.Process(this->KeepRange_EarlyStopFrame)
		.Process(this->KickOutPassengers)
		.Process(this->Beam_Color)
		.Process(this->Beam_Duration)
		.Process(this->Beam_Amplitude)
		.Process(this->Beam_IsHouseColor)
		.Process(this->LaserThickness)
		.Process(this->SkipWeaponPicking)
		.Process(this->DelayedFire_Duration)
		.Process(this->DelayedFire_SkipInTransport)
		.Process(this->DelayedFire_Animation)
		.Process(this->DelayedFire_OpenToppedAnimation)
		.Process(this->DelayedFire_AnimIsAttached)
		.Process(this->DelayedFire_CenterAnimOnFirer)
		.Process(this->DelayedFire_RemoveAnimOnNoDelay)
		.Process(this->DelayedFire_PauseFiringSequence)
		.Process(this->DelayedFire_OnlyOnInitialBurst)
		.Process(this->DelayedFire_AnimOffset)
		.Process(this->DelayedFire_AnimOnTurret)
		.Process(this->ExtraRange_TargetMoving)
		.Process(this->ExtraRange_FirerMoving)
		.Process(this->ExtraRange_Prefiring)
		.Process(this->ExtraRange_Prefiring_IncludeBurst)
		.Process(this->AttackFriendlies)
		.Process(this->AttackCursorOnFriendlies)
		.Process(this->AttackNoThreatBuildings)
		.Process(this->CylinderRangefinding)
		.Process(this->Anim_Update)
		;
};

void WeaponTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<WeaponTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);

}

void WeaponTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<WeaponTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WeaponTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(OldRadius)
		.Success();
}

bool WeaponTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(OldRadius)
		.Success();
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, HouseClass* pFiringHouse)
{
	WeaponTypeExt::DetonateAt(pThis, pTarget->GetCoords(), pOwner, pThis->Damage, pFiringHouse, pTarget);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse)
{
	WeaponTypeExt::DetonateAt(pThis, pTarget->GetCoords(), pOwner, damage, pFiringHouse, pTarget);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, HouseClass* pFiringHouse, AbstractClass* pTarget)
{
	WeaponTypeExt::DetonateAt(pThis, coords, pOwner, pThis->Damage, pFiringHouse, pTarget);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse, AbstractClass* pTarget)
{
	BulletExt::Detonate(coords, pOwner, damage, pFiringHouse, pTarget, pThis->Bright, pThis, pThis->Warhead);
}

int WeaponTypeExt::GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer)
{
	int range = 0;

	if (!pThis && !pFirer)
		return range;
	else if (pFirer && pFirer->CanOccupyFire())
		range = RulesClass::Instance->OccupyWeaponRange * Unsorted::LeptonsPerCell;
	else if (pThis && pFirer)
		range = pThis->Range;
	else
		return range;

	if (range == -512)
		return range;

	return WeaponTypeExt::GetRangeWithModifiers(pThis, pFirer, range);
}

int WeaponTypeExt::GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer, int range)
{
	auto pTechno = pFirer;

	if (auto const pTransport = pTechno->Transporter)
	{
		auto const pTypeExt = TechnoExt::ExtMap.Find(pTransport)->TypeExtData;

		if (pTypeExt->OpenTopped_UseTransportRangeModifiers && pTypeExt->OwnerObject()->OpenTopped)
			pTechno = pTransport;
	}

	auto const pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

	if (!pTechnoExt->AE.HasRangeModifier)
		return range;

	double extraRange = 0.0;

	for (auto const& attachEffect : pTechnoExt->AttachedEffects)
	{
		if (!attachEffect->IsActive())
			continue;

		auto const type = attachEffect->GetType();

		if (type->WeaponRange_Multiplier == 1.0 && type->WeaponRange_ExtraRange == 0.0)
			continue;

		if (type->WeaponRange_AllowWeapons.size() > 0 && !type->WeaponRange_AllowWeapons.Contains(pThis))
			continue;

		if (type->WeaponRange_DisallowWeapons.size() > 0 && type->WeaponRange_DisallowWeapons.Contains(pThis))
			continue;

		range = static_cast<int>(range * Math::max(type->WeaponRange_Multiplier, 0.0));
		extraRange += type->WeaponRange_ExtraRange;
	}

	range += static_cast<int>(extraRange * Unsorted::LeptonsPerCell);

	return Math::max(range, 0);
}

int WeaponTypeExt::GetTechnoKeepRange(WeaponTypeClass* pThis, TechnoClass* pFirer, bool isMinimum)
{
	if (!pThis)
		return 0;

	const auto pExt = WeaponTypeExt::ExtMap.Find(pThis);
	const auto keepRange = pExt->KeepRange.Get();

	if (!keepRange || !pFirer || pFirer->Transporter)
		return 0;

	const auto absType = pFirer->WhatAmI();

	if (absType != AbstractType::Infantry && absType != AbstractType::Unit)
		return 0;

	const auto pHouse = pFirer->Owner;

	if (pHouse && pHouse->IsControlledByHuman())
	{
		if (!pExt->KeepRange_AllowPlayer)
			return 0;
	}
	else if (!pExt->KeepRange_AllowAI)
	{
		return 0;
	}

	if (pFirer->RearmTimer.GetTimeLeft() < pExt->KeepRange_EarlyStopFrame)
		return 0;

	if (!pFirer->RearmTimer.InProgress())
	{
		const auto spawnManager = pFirer->SpawnManager;

		if (!spawnManager || spawnManager->Status != SpawnManagerStatus::CoolDown)
			return 0;

		const auto spawnsNumber = pFirer->GetTechnoType()->SpawnsNumber;

		for (int i = 0; i < spawnsNumber; i++)
		{
			const auto status = spawnManager->SpawnedNodes[i]->Status;

			if (status == SpawnNodeStatus::TakeOff || status == SpawnNodeStatus::Returning)
				return 0;
		}
	}

	if (isMinimum)
		return (keepRange > 0) ? keepRange : 0;

	if (keepRange > 0)
		return 0;

	const auto checkRange = -keepRange - 128;
	const auto pTarget = pFirer->Target;

	if (pTarget && pFirer->DistanceFrom(pTarget) >= checkRange)
		return (checkRange > 443) ? checkRange : 443; // 1.73 * Unsorted::LeptonsPerCell

	return -keepRange;
}

int WeaponTypeExt::GetWeaponIndex(TechnoClass* pTechno, WeaponTypeClass* pWeapon)
{
	if (!pTechno || !pWeapon)
		return -1;

	for (int i = 0; i < TechnoTypeClass::MaxWeapons; ++i)
	{
		auto const pWeaponStruct = pTechno->GetWeapon(i);

		if (pWeaponStruct && pWeaponStruct->WeaponType == pWeapon)
			return i;
	}

	return -1;
}

namespace InRangeEvaluation
{
	static bool IsChasing(TechnoClass* pThis, AbstractClass* pTarget)
	{
		if ((pThis->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
			return false;

		const auto pFootTarget = abstract_cast<FootClass*>(pTarget);

		if (!pFootTarget || !pFootTarget->Locomotor.GetInterfacePtr()->Is_Really_Moving_Now())
			return false;

		return true;
	}

	static bool IsMovingFire(TechnoClass* pThis)
	{
		const auto pFoot = abstract_cast<FootClass*>(pThis);

		if (!pFoot || !pFoot->Locomotor.GetInterfacePtr()->Is_Really_Moving_Now())
			return false;

		return true;
	}

	static bool IsPrefiring(TechnoClass* pThis, WeaponTypeClass* pWeapon)
	{
		const auto pTypeExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		const int currentBurst = pThis->CurrentBurstIndex % pWeapon->Burst;

		if (pTypeExt->ExtraRange_Prefiring_IncludeBurst.Get(RulesExt::Global()->ExtraRange_Prefiring_IncludeBurst) && currentBurst != 0)
			return true;

		const auto pTechnoExt = TechnoExt::ExtMap.Find(pThis);

		if (pTechnoExt->DelayedFireTimer.InProgress())
			return true;

		switch (pThis->WhatAmI())
		{
		case AbstractType::Unit:
		{
			const auto pUnit = static_cast<UnitClass*>(pThis);
			int syncFrame = -1;

			if (currentBurst == 0)
				syncFrame = pUnit->Type->FiringSyncFrame0;
			else if (currentBurst == 1)
				syncFrame = pUnit->Type->FiringSyncFrame1;

			if (syncFrame == -1)
				return false;

			return pUnit->CurrentFiringFrame >= syncFrame;
		}
		case AbstractType::Aircraft:
		{
			const auto pAircraft = static_cast<AircraftClass*>(pThis);
			const auto status = (AirAttackStatus)pAircraft->MissionStatus;
			return status == AirAttackStatus::FireAtTarget
				|| status == AirAttackStatus::FireAtTarget2
				|| status == AirAttackStatus::FireAtTarget2_Strafe
				|| status == AirAttackStatus::FireAtTarget3_Strafe
				|| status == AirAttackStatus::FireAtTarget4_Strafe
				|| status == AirAttackStatus::FireAtTarget5_Strafe;
		}
		case AbstractType::Building:
		{
			const auto pBuilding = static_cast<BuildingClass*>(pThis);
			const auto pExt = BuildingExt::ExtMap.Find(pBuilding);
			return pBuilding->DelayBeforeFiring || pExt->IsFiringNow;
		}
		case AbstractType::Infantry:
		{
			const auto pInfantry = static_cast<InfantryClass*>(pThis);
			return pInfantry->IsFiring;
		}
		default:
			return false;
		}
	}
}

int WeaponTypeExt::EvaluateInRangeMaximumRange(WeaponTypeClass* pWeapon, TechnoClass* pTechno, AbstractClass* pTarget)
{
	if (!pWeapon || !pTechno)
		return 0;

	if (const int keepRange = WeaponTypeExt::GetTechnoKeepRange(pWeapon, pTechno, false))
		return keepRange;

	int range = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pTechno);

	if (range == -512)
		return range;

	const auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	const auto prefiringExtraRange = pExt->ExtraRange_Prefiring.Get(RulesExt::Global()->ExtraRange_Prefiring);

	if (prefiringExtraRange && InRangeEvaluation::IsPrefiring(pTechno, pWeapon))
		range += prefiringExtraRange;

	const auto targetMovingExtraRange = pExt->ExtraRange_TargetMoving.isset()
		? pExt->ExtraRange_TargetMoving.Get() : (!RulesExt::Global()->ExtraRange_TargetMoving_CloseRangeOnly || pTechno->GetTechnoType()->CloseRange
		? RulesExt::Global()->ExtraRange_TargetMoving : Leptons(0));

	if (targetMovingExtraRange && pTarget && InRangeEvaluation::IsChasing(pTechno, pTarget))
		range += targetMovingExtraRange;

	const auto firerMovingExtraRange = pExt->ExtraRange_FirerMoving.Get(RulesExt::Global()->ExtraRange_FirerMoving);

	if (firerMovingExtraRange && InRangeEvaluation::IsMovingFire(pTechno))
		range += firerMovingExtraRange;

	return range;
}

int WeaponTypeExt::EvaluateInRangeMinimumRange(WeaponTypeClass* pWeapon, TechnoClass* pTechno)
{
	if (!pWeapon || !pTechno)
		return 0;

	if (const int keepRange = WeaponTypeExt::GetTechnoKeepRange(pWeapon, pTechno, true))
		return keepRange;

	return pWeapon->MinimumRange;
}

int WeaponTypeExt::ApplyInRangeOccupyBonus(TechnoClass* pTechno, int range)
{
	if (!pTechno)
		return range;

	const int occupyRange = WeaponTypeExt::GetRangeWithModifiers(nullptr, pTechno) / Unsorted::LeptonsPerCell;

	return range + occupyRange;
}

int WeaponTypeExt::EvaluateInRangeDistance(TechnoClass* pTechno, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	if (!pTechno || !pTarget || !pWeapon || !pWeapon->Projectile)
		return 0;

	if (pTechno->IsInAir() || pWeapon->Projectile->Arcing || pTechno->WhatAmI() == AbstractType::Aircraft)
		return pTarget->DistanceFrom(pTechno);

	return pTarget->DistanceFrom3D(pTechno);
}

bool WeaponTypeExt::IsTargetInWeaponRange(TechnoClass* pSource, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	if (!pSource || !pTarget || !pWeapon)
		return false;

	int maxRange = WeaponTypeExt::EvaluateInRangeMaximumRange(pWeapon, pSource, pTarget);

	if (!maxRange)
		return false;

	maxRange = WeaponTypeExt::ApplyInRangeOccupyBonus(pSource, maxRange);

	const int distance = WeaponTypeExt::EvaluateInRangeDistance(pSource, pTarget, pWeapon);

	if (maxRange != -512 && distance > maxRange)
		return false;

	const int minRange = WeaponTypeExt::EvaluateInRangeMinimumRange(pWeapon, pSource);

	if (minRange > 0 && distance < minRange)
		return false;

	if (WeaponTypeExt::CheckInRangeObstacle(pSource, pTarget, pWeapon))
		return false;

	return true;
}

// =============================
// container

WeaponTypeExt::ExtContainer::ExtContainer() : Container("WeaponTypeClass") { }

WeaponTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x771EE9, WeaponTypeClass_CTOR, 0x5)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x77311D, WeaponTypeClass_SDDTOR, 0x6)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x772EB0, WeaponTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x772CD0, WeaponTypeClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(WeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WeaponTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x772EA6, WeaponTypeClass_Load_Suffix, 0x6)
{
	WeaponTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x772F8C, WeaponTypeClass_Save, 0x5)
{
	WeaponTypeExt::ExtMap.SaveStatic();

	return 0;
}

//DEFINE_HOOK_AGAIN(0x7729D6, WeaponTypeClass_LoadFromINI, 0x5)// Section dont exist!
DEFINE_HOOK_AGAIN(0x7729C7, WeaponTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x7729B0, WeaponTypeClass_LoadFromINI, 0x5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE4);

	WeaponTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
