#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class AttachEffectClass;
class TechnoClass;
class WeaponTypeClass;
class HouseClass;

using PeriodicWeaponValidTarget = std::pair<TechnoClass*, int>;
using PeriodicWeaponValidTargetList = std::vector<PeriodicWeaponValidTarget>;

/// Selects which of the already-filtered, distance-sorted targets to fire at.
/// Return an empty vector to skip firing this interval.
using PeriodicWeaponTargetingCallbackFunc = std::vector<TechnoClass*>(*)(
	AttachEffectClass* pAttachEffect,
	TechnoClass* pAttached,
	WeaponTypeClass* pWeapon,
	TechnoClass* pFirer,
	HouseClass* pFirerHouse,
	const PeriodicWeaponValidTargetList& validTargets);

class PeriodicWeaponTargeting
{
public:
	static constexpr const char* ModeClosest = "closest";
	static constexpr const char* ModeAll = "all";

	static bool Register(const char* name, PeriodicWeaponTargetingCallbackFunc callback);
	static PeriodicWeaponTargetingCallbackFunc Find(const char* name);
	static bool IsBuiltinMode(const char* mode);
	static void Clear();

private:
	static std::unordered_map<std::string, PeriodicWeaponTargetingCallbackFunc> Callbacks;
};
