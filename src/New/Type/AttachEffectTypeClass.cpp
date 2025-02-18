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

	return std::vector<AttachEffectTypeClass*>(types.begin(), types.end());;
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
}

// AEAttachInfoTypeClass

void AEAttachInfoTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->AttachTypes.Read(exINI, pSection, "AttachEffect.AttachTypes");
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
