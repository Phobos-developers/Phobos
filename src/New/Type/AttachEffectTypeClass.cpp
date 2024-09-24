#include "AttachEffectTypeClass.h"

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

	return std::vector<AttachEffectTypeClass*> (types.begin(), types.end());;
}

AnimTypeClass* AttachEffectTypeClass::GetCumulativeAnimation(int cumulativeCount) const
{
	if (cumulativeCount < 0 || this->CumulativeAnimations.size() < 1)
		return nullptr;

	int index = static_cast<size_t>(cumulativeCount) >= this->CumulativeAnimations.size() ? this->CumulativeAnimations.size() - 1 : cumulativeCount - 1;

	return this->CumulativeAnimations.at(index);
}

template<>
const char* Enumerable<AttachEffectTypeClass>::GetMainSection()
{
	return "AttachEffectTypes";
}

void AttachEffectTypeClass::AddToGroupsMap()
{
	auto const map = &AttachEffectTypeClass::GroupsMap;

	for (auto const group : this->Groups)
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

	if (INIClass::IsBlank(pSection))
		return;

	INI_EX exINI(pINI);

	this->Duration.Read(exINI, pSection, "Duration");
	this->Cumulative.Read(exINI, pSection, "Cumulative");
	this->Cumulative_MaxCount.Read(exINI, pSection, "Cumulative.MaxCount");
	this->Powered.Read(exINI, pSection, "Powered");
	this->DiscardOn.Read(exINI, pSection, "DiscardOn");
	this->DiscardOn_RangeOverride.Read(exINI, pSection, "DiscardOn.RangeOverride");
	this->PenetratesIronCurtain.Read(exINI, pSection, "PenetratesIronCurtain");
	this->PenetratesForceShield.Read(exINI, pSection, "PenetratesForceShield");

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

	this->Tint_Color.Read(exINI, pSection, "Tint.Color");
	this->Tint_Intensity.Read(exINI, pSection, "Tint.Intensity");
	this->Tint_VisibleToHouses.Read(exINI, pSection, "Tint.VisibleToHouses");

	this->FirepowerMultiplier.Read(exINI, pSection, "FirepowerMultiplier");
	this->ArmorMultiplier.Read(exINI, pSection, "ArmorMultiplier");
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
	this->RevengeWeapon_AffectsHouses.Read(exINI, pSection, "RevengeWeapon.AffectsHouses");

	this->ReflectDamage.Read(exINI, pSection, "ReflectDamage");
	this->ReflectDamage_Warhead.Read(exINI, pSection, "ReflectDamage.Warhead");
	this->ReflectDamage_Warhead_Detonate.Read(exINI, pSection, "ReflectDamage.Warhead.Detonate");
	this->ReflectDamage_Multiplier.Read(exINI, pSection, "ReflectDamage.Multiplier");
	this->ReflectDamage_AffectsHouses.Read(exINI, pSection, "ReflectDamage.AffectsHouses");

	this->DisableWeapons.Read(exINI, pSection, "DisableWeapons");

	// Groups
	exINI.ParseStringList(this->Groups, pSection, "Groups");
	AddToGroupsMap();
}

template <typename T>
void AttachEffectTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Duration)
		.Process(this->Cumulative)
		.Process(this->Cumulative_MaxCount)
		.Process(this->Powered)
		.Process(this->DiscardOn)
		.Process(this->DiscardOn_RangeOverride)
		.Process(this->PenetratesIronCurtain)
		.Process(this->PenetratesForceShield)
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
		.Process(this->Tint_Color)
		.Process(this->Tint_Intensity)
		.Process(this->Tint_VisibleToHouses)
		.Process(this->FirepowerMultiplier)
		.Process(this->ArmorMultiplier)
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
		.Process(this->RevengeWeapon_AffectsHouses)
		.Process(this->ReflectDamage)
		.Process(this->ReflectDamage_Warhead)
		.Process(this->ReflectDamage_Warhead_Detonate)
		.Process(this->ReflectDamage_Multiplier)
		.Process(this->ReflectDamage_AffectsHouses)
		.Process(this->DisableWeapons)
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
