#include "AttachEffectTypeClass.h"

#include <Ext/TEvent/Body.h>

// Used to match groups names to AttachEffectTypeClass instances. Do not iterate due to undetermined order being prone to desyncs.
std::unordered_map<std::string, std::set<AttachEffectTypeClass*>> AttachEffectTypeClass::GroupsMap;

bool AttachEffectTypeClass::HasGroup(const std::string& groupID) const
{
	for (auto const& group : this->Groups)
	{
		if (!group.compare(groupID))
			return true;
	}

	return false;
}

bool AttachEffectTypeClass::HasGroups(const std::vector<std::string>& groupIDs, bool requireAll) const
{
	size_t foundCount = 0;

	for (auto const& group : this->Groups)
	{
		for (auto const& requiredGroup : groupIDs)
		{
			if (!group.compare(requiredGroup))
			{
				if (!requireAll)
					return true;

				foundCount++;
			}
		}
	}

	return !requireAll ? false : foundCount >= groupIDs.size();
}

std::vector<AttachEffectTypeClass*> AttachEffectTypeClass::GetTypesFromGroups(const std::vector<std::string>& groupIDs)
{
	std::set<AttachEffectTypeClass*> types;
	auto const map = &AttachEffectTypeClass::GroupsMap;

	for (auto const& group : groupIDs)
	{
		if (map->contains(group))
		{
			auto const values = &map->at(group);
			types.insert(values->begin(), values->end());
		}
	}

	return std::vector<AttachEffectTypeClass*>(types.begin(), types.end());
}

void AttachEffectTypeClass::HandleEvent(TechnoClass* pTarget)
{
	if (const auto pTag = pTarget->AttachedTag)
		pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::AttachedIsUnderAttachedEffect, pTarget, CellStruct::Empty);
}

template<>
const char* Enumerable<AttachEffectTypeClass>::GetMainSection()
{
	return "AttachEffectTypes";
}

void AttachEffectTypeClass::AddToGroupsMap()
{
	auto const map = &AttachEffectTypeClass::GroupsMap;

	for (auto const& group : this->Groups)
	{
		if (!map->contains(group))
		{
			map->insert(std::make_pair(group, std::set<AttachEffectTypeClass*>{this}));
		}
		else
		{
			auto const values = &map->at(group);
			values->insert(this);
		}
	}
}

void AttachEffectTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;

	if (INIClass::IsBlank(pSection) || !pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->Duration.Read(exINI, pSection, "Duration");
	this->Duration_ApplyFirepowerMult.Read(exINI, pSection, "Duration.ApplyFirepowerMult");
	this->Duration_ApplyArmorMultOnTarget.Read(exINI, pSection, "Duration.ApplyArmorMultOnTarget");
	this->Cumulative.Read(exINI, pSection, "Cumulative");
	this->Cumulative_MaxCount.Read(exINI, pSection, "Cumulative.MaxCount");
	this->Powered.Read(exINI, pSection, "Powered");
	this->DiscardOn.Read(exINI, pSection, "DiscardOn");
	this->DiscardOn_RangeOverride.Read(exINI, pSection, "DiscardOn.RangeOverride");
	this->DiscardOn_MoveBasedOnDestination.Read(exINI, pSection, "DiscardOn.MoveBasedOnDestination");
	this->PenetratesIronCurtain.Read(exINI, pSection, "PenetratesIronCurtain");
	this->PenetratesForceShield.Read(exINI, pSection, "PenetratesForceShield");
	this->AffectTypes.Read(exINI, pSection, "AffectTypes");
	this->IgnoreTypes.Read(exINI, pSection, "IgnoreTypes");
	if (exINI.ReadString(pSection, "AffectTargets") > 0)
	{
		Debug::Log("[Developer warning][%s] AffectTargets is deprecated and has been replaced by AffectsTarget! If both are set, the latter will be used.\n", pSection);
	}
	this->AffectsTarget.Read(exINI, pSection, "AffectTargets"); // Temporary solution for the INI tags renaming issue, see #2093
	this->AffectsTarget.Read(exINI, pSection, "AffectsTarget");

	this->Animation.Read(exINI, pSection, "Animation");
	this->CumulativeAnimations.Read(exINI, pSection, "CumulativeAnimations");
	this->CumulativeAnimations_RestartOnChange.Read(exINI, pSection, "CumulativeAnimations.RestartOnChange");
	this->Animation_ResetOnReapply.Read(exINI, pSection, "Animation.ResetOnReapply");
	this->Animation_OfflineAction.Read(exINI, pSection, "Animation.OfflineAction");
	this->Animation_TemporalAction.Read(exINI, pSection, "Animation.TemporalAction");
	this->Animation_UseInvokerAsOwner.Read(exINI, pSection, "Animation.UseInvokerAsOwner");
	this->Animation_HideIfAttachedWith.Read(exINI, pSection, "Animation.HideIfAttachedWith");

	this->ExpireWeapon.Read<true>(exINI, pSection, "ExpireWeapon");
	this->ExpireWeapon_TriggerOn.Read(exINI, pSection, "ExpireWeapon.TriggerOn");
	this->ExpireWeapon_CumulativeOnlyOnce.Read(exINI, pSection, "ExpireWeapon.CumulativeOnlyOnce");
	this->ExpireWeapon_UseInvokerAsOwner.Read(exINI, pSection, "ExpireWeapon.UseInvokerAsOwner");
	this->Next.Read(exINI, pSection, "Next");

	this->Tint_Color.Read(exINI, pSection, "Tint.Color");
	this->Tint_Intensity.Read(exINI, pSection, "Tint.Intensity");
	this->Tint_VisibleToHouses.Read(exINI, pSection, "Tint.VisibleToHouses");

	this->FirepowerMultiplier.Read(exINI, pSection, "FirepowerMultiplier");
	this->ArmorMultiplier.Read(exINI, pSection, "ArmorMultiplier");
	this->ArmorMultiplier_AllowWarheads.Read(exINI, pSection, "ArmorMultiplier.AllowWarheads");
	this->ArmorMultiplier_DisallowWarheads.Read(exINI, pSection, "ArmorMultiplier.DisallowWarheads");
	this->SpeedMultiplier.Read(exINI, pSection, "SpeedMultiplier");
	this->ROFMultiplier.Read(exINI, pSection, "ROFMultiplier");
	this->ROFMultiplier_ApplyOnCurrentTimer.Read(exINI, pSection, "ROFMultiplier.ApplyOnCurrentTimer");

	this->Cloakable.Read(exINI, pSection, "Cloakable");
	this->ForceDecloak.Read(exINI, pSection, "ForceDecloak");

	this->WeaponRange_Multiplier.Read(exINI, pSection, "WeaponRange.Multiplier");
	this->WeaponRange_ExtraRange.Read(exINI, pSection, "WeaponRange.ExtraRange");
	this->WeaponRange_AllowWeapons.Read(exINI, pSection, "WeaponRange.AllowWeapons");
	this->WeaponRange_DisallowWeapons.Read(exINI, pSection, "WeaponRange.DisallowWeapons");

	this->Crit_Multiplier.Read(exINI, pSection, "Crit.Multiplier");
	this->Crit_ExtraChance.Read(exINI, pSection, "Crit.ExtraChance");
	this->Crit_AllowWarheads.Read(exINI, pSection, "Crit.AllowWarheads");
	this->Crit_DisallowWarheads.Read(exINI, pSection, "Crit.DisallowWarheads");

	this->RevengeWeapon.Read<true>(exINI, pSection, "RevengeWeapon");
	if (exINI.ReadString(pSection, "RevengeWeapon.AffectsHouses") > 0)
	{
		Debug::Log("[Developer warning][%s] RevengeWeapon.AffectsHouses is deprecated and has been replaced by RevengeWeapon.AffectsHouse! If both are set, the latter will be used.\n", pSection);
	}
	this->RevengeWeapon_AffectsHouse.Read(exINI, pSection, "RevengeWeapon.AffectsHouses"); // Temporary solution for the INI tags renaming issue, see #2093
	this->RevengeWeapon_AffectsHouse.Read(exINI, pSection, "RevengeWeapon.AffectsHouse");
	this->RevengeWeapon_UseInvokerAsOwner.Read(exINI, pSection, "RevengeWeapon.UseInvokerAsOwner");

	this->ReflectDamage.Read(exINI, pSection, "ReflectDamage");
	this->ReflectDamage_Warhead.Read(exINI, pSection, "ReflectDamage.Warhead");
	this->ReflectDamage_Warhead_Detonate.Read(exINI, pSection, "ReflectDamage.Warhead.Detonate");
	this->ReflectDamage_Multiplier.Read(exINI, pSection, "ReflectDamage.Multiplier");
	if (exINI.ReadString(pSection, "ReflectDamage.AffectsHouses") > 0)
	{
		Debug::Log("[Developer warning][%s] ReflectDamage.AffectsHouses is deprecated and has been replaced by ReflectDamage.AffectsHouse! If both are set, the latter will be used.\n", pSection);
	}
	this->ReflectDamage_AffectsHouse.Read(exINI, pSection, "ReflectDamage.AffectsHouses"); // Temporary solution for the INI tags renaming issue, see #2093
	this->ReflectDamage_AffectsHouse.Read(exINI, pSection, "ReflectDamage.AffectsHouse");
	this->ReflectDamage_Chance.Read(exINI, pSection, "ReflectDamage.Chance");
	this->ReflectDamage_Override.Read(exINI, pSection, "ReflectDamage.Override");
	this->ReflectDamage_UseInvokerAsOwner.Read(exINI, pSection, "ReflectDamage.UseInvokerAsOwner");

	this->DisableWeapons.Read(exINI, pSection, "DisableWeapons");
	this->Unkillable.Read(exINI, pSection, "Unkillable");
	this->LaserTrail_Type.Read(exINI, pSection, "LaserTrail.Type");

	// Vector
	this->Vector_TimeStep.Read(exINI, pSection, "Vector.TimeStep");
	if (this->Vector_TimeStep < 1)
		this->Vector_TimeStep = 1;
	this->Vector_DisabledFrames.Read(exINI, pSection, "Vector.DisabledFrames");
	this->Vector_SyncFacing.Read(exINI, pSection, "Vector.SyncFacing");
	this->Vector_OriginIsOnWorld.Read(exINI, pSection, "Vector.OriginIsOnWorld");
	this->Vector_OriginIsOnBody.Read(exINI, pSection, "Vector.OriginIsOnBody");
	this->Vector_Origin.Read(exINI, pSection, "Vector.Origin");
	this->Vector_OriginFLH.Read(exINI, pSection, "Vector.OriginFLH");
	this->Vector_OriginNoUpdate.Read(exINI, pSection, "Vector.OriginNoUpdate");
	this->Vector_Force.Read(exINI, pSection, "Vector.Force");
	this->Vector_Freeze.Read(exINI, pSection, "Vector.Freeze");
	this->Vector_AllowedTilt.Read(exINI, pSection, "Vector.AllowedTilt");
	this->Vector_NormalVector.Read(exINI, pSection, "Vector.NormalVector");
	this->Vector_NormalRandomF.Read(exINI, pSection, "Vector.NormalRandomF");
	this->Vector_NormalRandomL.Read(exINI, pSection, "Vector.NormalRandomL");
	this->Vector_NormalRandomH.Read(exINI, pSection, "Vector.NormalRandomH");
	this->Vector_NormalRandomFMin.Read(exINI, pSection, "Vector.NormalRandomF.Min");
	this->Vector_NormalRandomFMax.Read(exINI, pSection, "Vector.NormalRandomF.Max");
	this->Vector_NormalRandomLMin.Read(exINI, pSection, "Vector.NormalRandomL.Min");
	this->Vector_NormalRandomLMax.Read(exINI, pSection, "Vector.NormalRandomL.Max");
	this->Vector_NormalRandomHMin.Read(exINI, pSection, "Vector.NormalRandomH.Min");
	this->Vector_NormalRandomHMax.Read(exINI, pSection, "Vector.NormalRandomH.Max");
	this->Vector_NormalFAnglePerStep.Read(exINI, pSection, "Vector.NormalFAnglePerStep");
	this->Vector_NormalLAnglePerStep.Read(exINI, pSection, "Vector.NormalLAnglePerStep");
	this->Vector_NormalHAnglePerStep.Read(exINI, pSection, "Vector.NormalHAnglePerStep");
	this->Vector_MoveTo.Read(exINI, pSection, "Vector.MoveTo");
	this->Vector_GrowRate.Read(exINI, pSection, "Vector.GrowRate");
	this->Vector_AnglePerStep.Read(exINI, pSection, "Vector.AnglePerStep");
	this->Vector_CircleRadius.Read(exINI, pSection, "Vector.CircleRadius");
	this->Vector_CircleSpeed.Read(exINI, pSection, "Vector.CircleSpeed");
	this->Vector_CircleSpeedAcceleration.Read(exINI, pSection, "Vector.CircleSpeedAcceleration");
	this->Vector_CircleMaxSpeed.Read(exINI, pSection, "Vector.CircleMaxSpeed");
	this->Vector_CircleMinSpeed.Read(exINI, pSection, "Vector.CircleMinSpeed");
	this->Vector_CircleAnglePerStep.Read(exINI, pSection, "Vector.CircleAnglePerStep");
	this->Vector_CircleAngleAcceleration.Read(exINI, pSection, "Vector.CircleAngleAcceleration");
	this->Vector_CircleOrigin.Read(exINI, pSection, "Vector.CircleOrigin");
	this->Vector_AllowOriginTilt.Read(exINI, pSection, "Vector.AllowOriginTilt");
	this->Vector_CircleRadiusGrow.Read(exINI, pSection, "Vector.CircleRadiusGrow");
	this->Vector_CircleMaxRadius.Read(exINI, pSection, "Vector.CircleMaxRadius");
	this->Vector_CircleMinRadius.Read(exINI, pSection, "Vector.CircleMinRadius");
	this->Vector_CircleEndOnMaxRadius.Read(exINI, pSection, "Vector.CircleEndOnMaxRadius");
	this->Vector_CircleEndOnMinRadius.Read(exINI, pSection, "Vector.CircleEndOnMinRadius");
	this->Vector_TargetFLH.Read(exINI, pSection, "Vector.TargetFLH");
	this->Vector_ReachTarget.Read(exINI, pSection, "Vector.ReachTarget");
	this->Vector_ReachTargetEarlyEnd.Read(exINI, pSection, "Vector.ReachTargetEarlyEnd");
	this->Vector_ArcHeight.Read(exINI, pSection, "Vector.ArcHeight");
	this->Vector_ArcPeakPercent.Read(exINI, pSection, "Vector.ArcPeakPercent");
	this->Vector_ArcPeakRandomPercent.Read(exINI, pSection, "Vector.ArcPeakRandomPercent");
	this->Vector_ArcPeakRandomPercentMin.Read(exINI, pSection, "Vector.ArcPeakRandomPercent.Min");
	this->Vector_ArcPeakRandomPercentMax.Read(exINI, pSection, "Vector.ArcPeakRandomPercent.Max");
	this->Vector_ArcRotation.Read(exINI, pSection, "Vector.ArcRotation");
	this->Vector_InitialSpeed.Read(exINI, pSection, "Vector.InitialSpeed");
	this->Vector_MaxSpeed.Read(exINI, pSection, "Vector.MaxSpeed");
	this->Vector_MinSpeed.Read(exINI, pSection, "Vector.MinSpeed");
	this->Vector_Acceleration.Read(exINI, pSection, "Vector.Acceleration");
	this->Vector_AllowFallingDestroy.Read(exINI, pSection, "Vector.AllowFallingDestroy");
	this->Vector_FallingDestroyHeight.Read(exINI, pSection, "Vector.FallingDestroyHeight");
	this->Vector_AffectTechno.Read(exINI, pSection, "Vector.AffectTechno");
	this->Vector_AffectBullets.Read(exINI, pSection, "Vector.AffectBullets");
	this->Vector_CircleRandomRadiusMin.Read(exINI, pSection, "Vector.CircleRandomRadius.Min");
	this->Vector_CircleRandomRadiusMax.Read(exINI, pSection, "Vector.CircleRandomRadius.Max");
	this->Vector_CircleRandomAngleMin.Read(exINI, pSection, "Vector.CircleRandomAngle.Min");
	this->Vector_CircleRandomAngleMax.Read(exINI, pSection, "Vector.CircleRandomAngle.Max");
	this->Vector_CircleRandomAngleMin2.Read(exINI, pSection, "Vector.CircleRandomAngle.Min2");
	this->Vector_CircleRandomAngleMax2.Read(exINI, pSection, "Vector.CircleRandomAngle.Max2");
	this->Vector_CircleMaxAngle.Read(exINI, pSection, "Vector.CircleMaxAngle");
	this->Vector_CircleMinAngle.Read(exINI, pSection, "Vector.CircleMinAngle");
	this->Vector_TargetOffsetFMin.Read(exINI, pSection, "Vector.TargetOffsetF.Min");
	this->Vector_TargetOffsetFMax.Read(exINI, pSection, "Vector.TargetOffsetF.Max");
	this->Vector_TargetOffsetLMin.Read(exINI, pSection, "Vector.TargetOffsetL.Min");
	this->Vector_TargetOffsetLMax.Read(exINI, pSection, "Vector.TargetOffsetL.Max");
	this->Vector_TargetOffsetHMin.Read(exINI, pSection, "Vector.TargetOffsetH.Min");
	this->Vector_TargetOffsetHMax.Read(exINI, pSection, "Vector.TargetOffsetH.Max");
	this->Vector_ArcRandomHeightMin.Read(exINI, pSection, "Vector.ArcRandomHeight.Min");
	this->Vector_ArcRandomHeightMax.Read(exINI, pSection, "Vector.ArcRandomHeight.Max");
	this->Vector_ArcRandomRotationMin.Read(exINI, pSection, "Vector.ArcRandomRotation.Min");
	this->Vector_ArcRandomRotationMax.Read(exINI, pSection, "Vector.ArcRandomRotation.Max");
	this->Vector_RandomSpeedMin.Read(exINI, pSection, "Vector.RandomSpeed.Min");
	this->Vector_RandomSpeedMax.Read(exINI, pSection, "Vector.RandomSpeed.Max");
	this->Vector_OriginMoveTo.Read(exINI, pSection, "Vector.Origin.MoveTo");
	this->Vector_OriginGrowRate.Read(exINI, pSection, "Vector.Origin.GrowRate");
	this->Vector_OriginTargetFLH.Read(exINI, pSection, "Vector.Origin.TargetFLH");
	this->Vector_OriginInitialSpeed.Read(exINI, pSection, "Vector.Origin.InitialSpeed");
	this->Vector_OriginAcceleration.Read(exINI, pSection, "Vector.Origin.Acceleration");
	this->Vector_OriginMaxSpeed.Read(exINI, pSection, "Vector.Origin.MaxSpeed");
	this->Vector_OriginMinSpeed.Read(exINI, pSection, "Vector.Origin.MinSpeed");
	this->Vector_OriginTargetOffsetFMin.Read(exINI, pSection, "Vector.Origin.TargetOffsetF.Min");
	this->Vector_OriginTargetOffsetFMax.Read(exINI, pSection, "Vector.Origin.TargetOffsetF.Max");
	this->Vector_OriginTargetOffsetLMin.Read(exINI, pSection, "Vector.Origin.TargetOffsetL.Min");
	this->Vector_OriginTargetOffsetLMax.Read(exINI, pSection, "Vector.Origin.TargetOffsetL.Max");
	this->Vector_OriginTargetOffsetHMin.Read(exINI, pSection, "Vector.Origin.TargetOffsetH.Min");
	this->Vector_OriginTargetOffsetHMax.Read(exINI, pSection, "Vector.Origin.TargetOffsetH.Max");
	this->Vector_OriginReachTarget.Read(exINI, pSection, "Vector.Origin.ReachTarget");
	this->Vector_OriginArcHeight.Read(exINI, pSection, "Vector.Origin.ArcHeight");
	this->Vector_OriginCircleRadius.Read(exINI, pSection, "Vector.Origin.CircleRadius");
	this->Vector_OriginCircleSpeed.Read(exINI, pSection, "Vector.Origin.CircleSpeed");
	this->Vector_OriginCircleAnglePerStep.Read(exINI, pSection, "Vector.Origin.CircleAnglePerStep");
	this->Vector_OriginLissajous.Read(exINI, pSection, "Vector.Origin.Lissajous");
	this->Vector_OriginCircleRadiusGrow.Read(exINI, pSection, "Vector.Origin.CircleRadiusGrow");
	this->Vector_OriginCircleMaxRadius.Read(exINI, pSection, "Vector.Origin.CircleMaxRadius");
	this->Vector_OriginCircleMinRadius.Read(exINI, pSection, "Vector.Origin.CircleMinRadius");
	this->Vector_OriginCircleEndOnMaxRadius.Read(exINI, pSection, "Vector.Origin.CircleEndOnMaxRadius");
	this->Vector_OriginCircleEndOnMinRadius.Read(exINI, pSection, "Vector.Origin.CircleEndOnMinRadius");
	this->Vector_OriginNormalVector.Read(exINI, pSection, "Vector.Origin.NormalVector");
	this->Vector_OriginNormalFAnglePerStep.Read(exINI, pSection, "Vector.Origin.NormalFAnglePerStep");
	this->Vector_OriginNormalLAnglePerStep.Read(exINI, pSection, "Vector.Origin.NormalLAnglePerStep");
	this->Vector_OriginNormalHAnglePerStep.Read(exINI, pSection, "Vector.Origin.NormalHAnglePerStep");
	this->Vector_OriginAllowedTilt.Read(exINI, pSection, "Vector.Origin.AllowedTilt");
	this->Vector_OriginCircleOffset.Read(exINI, pSection, "Vector.Origin.CircleOffset");
	this->Vector_OriginOrigin.Read(exINI, pSection, "Vector.Origin.Origin");
	this->Vector_OriginOriginFLH.Read(exINI, pSection, "Vector.Origin.OriginFLH");
	this->Vector_NormalFAngleRMin.Read(exINI, pSection, "Vector.NormalFAngleRanges.Min");
	this->Vector_NormalFAngleRMax.Read(exINI, pSection, "Vector.NormalFAngleRanges.Max");
	this->Vector_NormalFAngleRMin2.Read(exINI, pSection, "Vector.NormalFAngleRanges.Min2");
	this->Vector_NormalFAngleRMax2.Read(exINI, pSection, "Vector.NormalFAngleRanges.Max2");
	this->Vector_NormalLAngleRMin.Read(exINI, pSection, "Vector.NormalLAngleRanges.Min");
	this->Vector_NormalLAngleRMax.Read(exINI, pSection, "Vector.NormalLAngleRanges.Max");
	this->Vector_NormalLAngleRMin2.Read(exINI, pSection, "Vector.NormalLAngleRanges.Min2");
	this->Vector_NormalLAngleRMax2.Read(exINI, pSection, "Vector.NormalLAngleRanges.Max2");
	this->Vector_NormalHAngleRMin.Read(exINI, pSection, "Vector.NormalHAngleRanges.Min");
	this->Vector_NormalHAngleRMax.Read(exINI, pSection, "Vector.NormalHAngleRanges.Max");
	this->Vector_NormalHAngleRMin2.Read(exINI, pSection, "Vector.NormalHAngleRanges.Min2");
	this->Vector_NormalHAngleRMax2.Read(exINI, pSection, "Vector.NormalHAngleRanges.Max2");
	this->Vector_OriginNormalFAngleRMin.Read(exINI, pSection, "Vector.Origin.NormalFAngleRanges.Min");
	this->Vector_OriginNormalFAngleRMax.Read(exINI, pSection, "Vector.Origin.NormalFAngleRanges.Max");
	this->Vector_OriginNormalFAngleRMin2.Read(exINI, pSection, "Vector.Origin.NormalFAngleRanges.Min2");
	this->Vector_OriginNormalFAngleRMax2.Read(exINI, pSection, "Vector.Origin.NormalFAngleRanges.Max2");
	this->Vector_OriginNormalLAngleRMin.Read(exINI, pSection, "Vector.Origin.NormalLAngleRanges.Min");
	this->Vector_OriginNormalLAngleRMax.Read(exINI, pSection, "Vector.Origin.NormalLAngleRanges.Max");
	this->Vector_OriginNormalLAngleRMin2.Read(exINI, pSection, "Vector.Origin.NormalLAngleRanges.Min2");
	this->Vector_OriginNormalLAngleRMax2.Read(exINI, pSection, "Vector.Origin.NormalLAngleRanges.Max2");
	this->Vector_OriginNormalHAngleRMin.Read(exINI, pSection, "Vector.Origin.NormalHAngleRanges.Min");
	this->Vector_OriginNormalHAngleRMax.Read(exINI, pSection, "Vector.Origin.NormalHAngleRanges.Max");
	this->Vector_OriginNormalHAngleRMin2.Read(exINI, pSection, "Vector.Origin.NormalHAngleRanges.Min2");
	this->Vector_OriginNormalHAngleRMax2.Read(exINI, pSection, "Vector.Origin.NormalHAngleRanges.Max2");

	// Groups
	exINI.ParseStringList(this->Groups, pSection, "Groups");
	AddToGroupsMap();
}

template <typename T>
void AttachEffectTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Duration)
		.Process(this->Duration_ApplyFirepowerMult)
		.Process(this->Duration_ApplyArmorMultOnTarget)
		.Process(this->Cumulative)
		.Process(this->Cumulative_MaxCount)
		.Process(this->Powered)
		.Process(this->DiscardOn)
		.Process(this->DiscardOn_RangeOverride)
		.Process(this->DiscardOn_MoveBasedOnDestination)
		.Process(this->PenetratesIronCurtain)
		.Process(this->PenetratesForceShield)
		.Process(this->AffectTypes)
		.Process(this->IgnoreTypes)
		.Process(this->AffectsTarget)
		.Process(this->Animation)
		.Process(this->CumulativeAnimations)
		.Process(this->CumulativeAnimations_RestartOnChange)
		.Process(this->Animation_ResetOnReapply)
		.Process(this->Animation_OfflineAction)
		.Process(this->Animation_TemporalAction)
		.Process(this->Animation_UseInvokerAsOwner)
		.Process(this->Animation_HideIfAttachedWith)
		.Process(this->ExpireWeapon)
		.Process(this->ExpireWeapon_TriggerOn)
		.Process(this->ExpireWeapon_CumulativeOnlyOnce)
		.Process(this->ExpireWeapon_UseInvokerAsOwner)
		.Process(this->Next)
		.Process(this->Tint_Color)
		.Process(this->Tint_Intensity)
		.Process(this->Tint_VisibleToHouses)
		.Process(this->FirepowerMultiplier)
		.Process(this->ArmorMultiplier)
		.Process(this->ArmorMultiplier_AllowWarheads)
		.Process(this->ArmorMultiplier_DisallowWarheads)
		.Process(this->SpeedMultiplier)
		.Process(this->ROFMultiplier)
		.Process(this->ROFMultiplier_ApplyOnCurrentTimer)
		.Process(this->Cloakable)
		.Process(this->ForceDecloak)
		.Process(this->WeaponRange_Multiplier)
		.Process(this->WeaponRange_ExtraRange)
		.Process(this->WeaponRange_AllowWeapons)
		.Process(this->WeaponRange_DisallowWeapons)
		.Process(this->Crit_Multiplier)
		.Process(this->Crit_ExtraChance)
		.Process(this->Crit_AllowWarheads)
		.Process(this->Crit_DisallowWarheads)
		.Process(this->RevengeWeapon)
		.Process(this->RevengeWeapon_AffectsHouse)
		.Process(this->RevengeWeapon_UseInvokerAsOwner)
		.Process(this->ReflectDamage)
		.Process(this->ReflectDamage_Warhead)
		.Process(this->ReflectDamage_Warhead_Detonate)
		.Process(this->ReflectDamage_Multiplier)
		.Process(this->ReflectDamage_AffectsHouse)
		.Process(this->ReflectDamage_Chance)
		.Process(this->ReflectDamage_Override)
		.Process(this->ReflectDamage_UseInvokerAsOwner)
		.Process(this->DisableWeapons)
		.Process(this->Unkillable)
		.Process(this->LaserTrail_Type)
		.Process(this->Vector_TimeStep)
		.Process(this->Vector_DisabledFrames)
		.Process(this->Vector_SyncFacing)
		.Process(this->Vector_OriginIsOnWorld)
		.Process(this->Vector_OriginIsOnBody)
		.Process(this->Vector_Origin)
		.Process(this->Vector_OriginFLH)
		.Process(this->Vector_OriginNoUpdate)
		.Process(this->Vector_Force)
		.Process(this->Vector_Freeze)
		.Process(this->Vector_AllowedTilt)
		.Process(this->Vector_NormalVector)
		.Process(this->Vector_NormalRandomF)
		.Process(this->Vector_NormalRandomL)
		.Process(this->Vector_NormalRandomH)
		.Process(this->Vector_NormalRandomFMin)
		.Process(this->Vector_NormalRandomFMax)
		.Process(this->Vector_NormalRandomLMin)
		.Process(this->Vector_NormalRandomLMax)
		.Process(this->Vector_NormalRandomHMin)
		.Process(this->Vector_NormalRandomHMax)
		.Process(this->Vector_NormalFAnglePerStep)
		.Process(this->Vector_NormalLAnglePerStep)
		.Process(this->Vector_NormalHAnglePerStep)
		.Process(this->Vector_MoveTo)
		.Process(this->Vector_GrowRate)
		.Process(this->Vector_AnglePerStep)
		.Process(this->Vector_CircleRadius)
		.Process(this->Vector_CircleSpeed)
		.Process(this->Vector_CircleSpeedAcceleration)
		.Process(this->Vector_CircleMaxSpeed)
		.Process(this->Vector_CircleMinSpeed)
		.Process(this->Vector_CircleAnglePerStep)
		.Process(this->Vector_CircleAngleAcceleration)
		.Process(this->Vector_CircleOrigin)
		.Process(this->Vector_AllowOriginTilt)
		.Process(this->Vector_CircleRadiusGrow)
		.Process(this->Vector_CircleMaxRadius)
		.Process(this->Vector_CircleMinRadius)
		.Process(this->Vector_CircleEndOnMaxRadius)
		.Process(this->Vector_CircleEndOnMinRadius)
		.Process(this->Vector_TargetFLH)
		.Process(this->Vector_ReachTarget)
		.Process(this->Vector_ReachTargetEarlyEnd)
		.Process(this->Vector_ArcHeight)
		.Process(this->Vector_ArcPeakPercent)
		.Process(this->Vector_ArcPeakRandomPercent)
		.Process(this->Vector_ArcPeakRandomPercentMin)
		.Process(this->Vector_ArcPeakRandomPercentMax)
		.Process(this->Vector_ArcRotation)
		.Process(this->Vector_InitialSpeed)
		.Process(this->Vector_MaxSpeed)
		.Process(this->Vector_MinSpeed)
		.Process(this->Vector_Acceleration)
		.Process(this->Vector_AllowFallingDestroy)
		.Process(this->Vector_FallingDestroyHeight)
		.Process(this->Vector_AffectTechno)
		.Process(this->Vector_AffectBullets)
		.Process(this->Vector_CircleRandomRadiusMin)
		.Process(this->Vector_CircleRandomRadiusMax)
		.Process(this->Vector_CircleRandomAngleMin)
		.Process(this->Vector_CircleRandomAngleMax)
		.Process(this->Vector_CircleRandomAngleMin2)
		.Process(this->Vector_CircleRandomAngleMax2)
		.Process(this->Vector_CircleMaxAngle)
		.Process(this->Vector_CircleMinAngle)
		.Process(this->Vector_TargetOffsetFMin)
		.Process(this->Vector_TargetOffsetFMax)
		.Process(this->Vector_TargetOffsetLMin)
		.Process(this->Vector_TargetOffsetLMax)
		.Process(this->Vector_TargetOffsetHMin)
		.Process(this->Vector_TargetOffsetHMax)
		.Process(this->Vector_ArcRandomHeightMin)
		.Process(this->Vector_ArcRandomHeightMax)
		.Process(this->Vector_ArcRandomRotationMin)
		.Process(this->Vector_ArcRandomRotationMax)
		.Process(this->Vector_RandomSpeedMin)
		.Process(this->Vector_RandomSpeedMax)
		.Process(this->Vector_OriginMoveTo)
		.Process(this->Vector_OriginGrowRate)
		.Process(this->Vector_OriginTargetFLH)
		.Process(this->Vector_OriginInitialSpeed)
		.Process(this->Vector_OriginAcceleration)
		.Process(this->Vector_OriginMaxSpeed)
		.Process(this->Vector_OriginMinSpeed)
		.Process(this->Vector_OriginTargetOffsetFMin)
		.Process(this->Vector_OriginTargetOffsetFMax)
		.Process(this->Vector_OriginTargetOffsetLMin)
		.Process(this->Vector_OriginTargetOffsetLMax)
		.Process(this->Vector_OriginTargetOffsetHMin)
		.Process(this->Vector_OriginTargetOffsetHMax)
		.Process(this->Vector_OriginReachTarget)
		.Process(this->Vector_OriginArcHeight)
		.Process(this->Vector_OriginCircleRadius)
		.Process(this->Vector_OriginCircleSpeed)
		.Process(this->Vector_OriginCircleAnglePerStep)
		.Process(this->Vector_OriginLissajous)
		.Process(this->Vector_OriginCircleRadiusGrow)
		.Process(this->Vector_OriginCircleMaxRadius)
		.Process(this->Vector_OriginCircleMinRadius)
		.Process(this->Vector_OriginCircleEndOnMaxRadius)
		.Process(this->Vector_OriginCircleEndOnMinRadius)
		.Process(this->Vector_OriginNormalVector)
		.Process(this->Vector_OriginNormalFAnglePerStep)
		.Process(this->Vector_OriginNormalLAnglePerStep)
		.Process(this->Vector_OriginNormalHAnglePerStep)
		.Process(this->Vector_OriginAllowedTilt)
		.Process(this->Vector_OriginCircleOffset)
		.Process(this->Vector_OriginOrigin)
		.Process(this->Vector_OriginOriginFLH)
		.Process(this->Vector_NormalFAngleRMin)
		.Process(this->Vector_NormalFAngleRMax)
		.Process(this->Vector_NormalFAngleRMin2)
		.Process(this->Vector_NormalFAngleRMax2)
		.Process(this->Vector_NormalLAngleRMin)
		.Process(this->Vector_NormalLAngleRMax)
		.Process(this->Vector_NormalLAngleRMin2)
		.Process(this->Vector_NormalLAngleRMax2)
		.Process(this->Vector_NormalHAngleRMin)
		.Process(this->Vector_NormalHAngleRMax)
		.Process(this->Vector_NormalHAngleRMin2)
		.Process(this->Vector_NormalHAngleRMax2)
		.Process(this->Vector_OriginNormalFAngleRMin)
		.Process(this->Vector_OriginNormalFAngleRMax)
		.Process(this->Vector_OriginNormalFAngleRMin2)
		.Process(this->Vector_OriginNormalFAngleRMax2)
		.Process(this->Vector_OriginNormalLAngleRMin)
		.Process(this->Vector_OriginNormalLAngleRMax)
		.Process(this->Vector_OriginNormalLAngleRMin2)
		.Process(this->Vector_OriginNormalLAngleRMax2)
		.Process(this->Vector_OriginNormalHAngleRMin)
		.Process(this->Vector_OriginNormalHAngleRMax)
		.Process(this->Vector_OriginNormalHAngleRMin2)
		.Process(this->Vector_OriginNormalHAngleRMax2)
		.Process(this->Groups)
		;
}

void AttachEffectTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
	AddToGroupsMap();
}

void AttachEffectTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

// AE type-related enum etc. parsers
namespace detail
{
	template <>
	inline bool read<DiscardCondition>(DiscardCondition& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = DiscardCondition::None;

			auto str = parser.value();
			char* context = nullptr;
			for (auto cur = strtok_s(str, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				if (!_strcmpi(cur, "none"))
				{
					parsed |= DiscardCondition::None;
				}
				else if (!_strcmpi(cur, "entry"))
				{
					parsed |= DiscardCondition::Entry;
				}
				else if (!_strcmpi(cur, "move"))
				{
					parsed |= DiscardCondition::Move;
				}
				else if (!_strcmpi(cur, "stationary"))
				{
					parsed |= DiscardCondition::Stationary;
				}
				else if (!_strcmpi(cur, "drain"))
				{
					parsed |= DiscardCondition::Drain;
				}
				else if (!_strcmpi(cur, "inrange"))
				{
					parsed |= DiscardCondition::InRange;
				}
				else if (!_strcmpi(cur, "outofrange"))
				{
					parsed |= DiscardCondition::OutOfRange;
				}
				else if (!_strcmpi(cur, "firing"))
				{
					parsed |= DiscardCondition::Firing;
				}
				else
				{
					Debug::INIParseFailed(pSection, pKey, cur, "Expected a discard condition type");
					return false;
				}
			}

			value = parsed;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<ExpireWeaponCondition>(ExpireWeaponCondition& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = ExpireWeaponCondition::None;

			auto str = parser.value();
			char* context = nullptr;
			for (auto cur = strtok_s(str, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				if (!_strcmpi(cur, "none"))
				{
					parsed |= ExpireWeaponCondition::None;
				}
				else if (!_strcmpi(cur, "expire"))
				{
					parsed |= ExpireWeaponCondition::Expire;
				}
				else if (!_strcmpi(cur, "remove"))
				{
					parsed |= ExpireWeaponCondition::Remove;
				}
				else if (!_strcmpi(cur, "death"))
				{
					parsed |= ExpireWeaponCondition::Death;
				}
				else if (!_strcmpi(cur, "discard"))
				{
					parsed |= ExpireWeaponCondition::Discard;
				}
				else if (!_strcmpi(cur, "all"))
				{
					parsed |= ExpireWeaponCondition::All;
				}
				else
				{
					Debug::INIParseFailed(pSection, pKey, cur, "Expected a expire weapon trigger condition type");
					return false;
				}
			}

			value = parsed;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<VectorOrigin>(VectorOrigin& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();

			if (!_strcmpi(str, "Self"))
				value = VectorOrigin::Self;
			else if (!_strcmpi(str, "Launcher"))
				value = VectorOrigin::Launcher;
			else if (!_strcmpi(str, "Target"))
				value = VectorOrigin::Target;
			else if (!_strcmpi(str, "Source"))
				value = VectorOrigin::Source;
			else
				value = VectorOrigin::Self;

			return true;
		}

		return false;
	}
}

// AEAttachInfoTypeClass

void AEAttachInfoTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->AttachTypes.Read(exINI, pSection, "AttachEffect.AttachTypes");
	this->CumulativeSourceMaxCount.Read(exINI, pSection, "AttachEffect.CumulativeSourceMaxCount");
	this->CumulativeRefreshAll.Read(exINI, pSection, "AttachEffect.CumulativeRefreshAll");
	this->CumulativeRefreshAll_OnAttach.Read(exINI, pSection, "AttachEffect.CumulativeRefreshAll.OnAttach");
	this->CumulativeRefreshSameSourceOnly.Read(exINI, pSection, "AttachEffect.CumulativeRefreshSameSourceOnly");
	this->RemoveTypes.Read(exINI, pSection, "AttachEffect.RemoveTypes");
	exINI.ParseStringList(this->RemoveGroups, pSection, "AttachEffect.RemoveGroups");
	this->CumulativeRemoveMinCounts.Read(exINI, pSection, "AttachEffect.CumulativeRemoveMinCounts");
	this->CumulativeRemoveMaxCounts.Read(exINI, pSection, "AttachEffect.CumulativeRemoveMaxCounts");
	this->DurationOverrides.Read(exINI, pSection, "AttachEffect.DurationOverrides");
	this->Delays.Read(exINI, pSection, "AttachEffect.Delays");
	this->InitialDelays.Read(exINI, pSection, "AttachEffect.InitialDelays");
	this->RecreationDelays.Read(exINI, pSection, "AttachEffect.RecreationDelays");
}

AEAttachParams AEAttachInfoTypeClass::GetAttachParams(unsigned int index, bool selfOwned) const
{
	AEAttachParams info { };

	if (this->DurationOverrides.size() > 0)
		info.DurationOverride = this->DurationOverrides[this->DurationOverrides.size() > index ? index : this->DurationOverrides.size() - 1];

	if (selfOwned)
	{
		if (this->Delays.size() > 0)
			info.Delay = this->Delays[this->Delays.size() > index ? index : this->Delays.size() - 1];

		if (this->InitialDelays.size() > 0)
			info.InitialDelay = this->InitialDelays[this->InitialDelays.size() > index ? index : this->InitialDelays.size() - 1];

		if (this->RecreationDelays.size() > 0)
			info.RecreationDelay = this->RecreationDelays[this->RecreationDelays.size() > index ? index : this->RecreationDelays.size() - 1];
	}
	else
	{
		info.CumulativeSourceMaxCount = this->CumulativeSourceMaxCount;
		info.CumulativeRefreshAll = this->CumulativeRefreshAll;
		info.CumulativeRefreshAll_OnAttach = this->CumulativeRefreshAll_OnAttach;
		info.CumulativeRefreshSameSourceOnly = this->CumulativeRefreshSameSourceOnly;
	}

	return info;
}

#pragma region(save/load)

template <class T>
bool AEAttachInfoTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->AttachTypes)
		.Process(this->CumulativeSourceMaxCount)
		.Process(this->CumulativeRefreshAll)
		.Process(this->CumulativeRefreshAll_OnAttach)
		.Process(this->CumulativeRefreshSameSourceOnly)
		.Process(this->RemoveTypes)
		.Process(this->RemoveGroups)
		.Process(this->CumulativeRemoveMinCounts)
		.Process(this->CumulativeRemoveMaxCounts)
		.Process(this->DurationOverrides)
		.Process(this->Delays)
		.Process(this->InitialDelays)
		.Process(this->RecreationDelays)
		.Success();
}

bool AEAttachInfoTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool AEAttachInfoTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AEAttachInfoTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)
