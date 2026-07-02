#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class AttachEffectClass;
class TechnoClass;
class WeaponTypeClass;
class HouseClass;

enum class PeriodicWeaponTargetingMode : unsigned char
{
	Closest = 0,
	All = 1,
	Custom = 2
};

using PeriodicWeaponValidTarget = std::pair<TechnoClass*, int>;
using PeriodicWeaponValidTargetList = std::vector<PeriodicWeaponValidTarget>;

struct PeriodicWeaponTargetingParams
{
	AttachEffectClass* AttachEffect = nullptr;
	TechnoClass* Attached = nullptr;
	WeaponTypeClass* Weapon = nullptr;
	TechnoClass* Firer = nullptr;
	HouseClass* FirerHouse = nullptr;
	const PeriodicWeaponValidTargetList* ValidTargets = nullptr;
};

/// Selects which of the already-filtered, distance-sorted targets to fire at.
/// Return an empty vector to skip firing this interval.
using PeriodicWeaponTargetingCallbackFunc = std::vector<TechnoClass*>(*)(const PeriodicWeaponTargetingParams& params);

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
