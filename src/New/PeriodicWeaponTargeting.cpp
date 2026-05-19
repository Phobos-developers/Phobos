#include "PeriodicWeaponTargeting.h"

#include <Utilities/Debug.h>

std::unordered_map<std::string, PeriodicWeaponTargetingCallbackFunc> PeriodicWeaponTargeting::Callbacks;

bool PeriodicWeaponTargeting::Register(const char* name, PeriodicWeaponTargetingCallbackFunc callback)
{
	if (!name || !*name || !callback)
		return false;

	if (IsBuiltinMode(name))
	{
		Debug::Log("[Developer error] PeriodicWeaponTargeting::Register - cannot register builtin mode name '%s'.\n", name);
		return false;
	}

	Callbacks[name] = callback;
	return true;
}

PeriodicWeaponTargetingCallbackFunc PeriodicWeaponTargeting::Find(const char* name)
{
	if (!name || !*name)
		return nullptr;

	const auto it = Callbacks.find(name);

	if (it == Callbacks.end())
		return nullptr;

	return it->second;
}

bool PeriodicWeaponTargeting::IsBuiltinMode(const char* mode)
{
	if (!mode || !*mode)
		return false;

	return !_strcmpi(mode, ModeClosest)
		|| !_strcmpi(mode, "nearest")
		|| !_strcmpi(mode, ModeAll);
}

void PeriodicWeaponTargeting::Clear()
{
	Callbacks.clear();
}
