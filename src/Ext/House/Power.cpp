#include "Body.h"

#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>

#include <AircraftClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <limits>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

void HouseExt::RecheckUnitPower(HouseClass* const pThis)
{
	if (!Phobos::Config::UnitPowerDrain)
		return;

	for (const auto pEnhancer : HouseExt::Fetch(pThis)->PowerPlantEnhancers)
	{
		if (!TechnoExt::IsActive(pEnhancer) || !pEnhancer->HasPower)
			continue;

		const auto pExt = BuildingTypeExt::Fetch(pEnhancer->Type);
		if (pExt->PowerPlantEnhancer_Range.Get() <= 0)
			continue;

		for (const auto pType : pExt->PowerPlantEnhancer_Buildings)
		{
			if (pType && pType->WhatAmI() != AbstractType::BuildingType)
			{
				// Moving units can enter or leave this range without a gain/loss
				// event. Refresh once in this house's update before the native gate.
				pThis->RecheckPower = true;
				return;
			}
		}
	}
}

void HouseExt::UpdateUnitPower(HouseClass* const pThis)
{
	if (!Phobos::Config::UnitPowerDrain)
		return;

	std::int64_t output = pThis->PowerOutput;
	std::int64_t drain = pThis->PowerDrain;
	std::unordered_set<const TechnoTypeClass*> enhancedTypes;
	for (const auto pEnhancer : HouseExt::Fetch(pThis)->PowerPlantEnhancers)
	{
		if (!TechnoExt::IsActive(pEnhancer) || !pEnhancer->HasPower)
			continue;

		for (const auto pType : BuildingTypeExt::Fetch(pEnhancer->Type)->PowerPlantEnhancer_Buildings)
		{
			if (pType && pType->WhatAmI() != AbstractType::BuildingType)
				enhancedTypes.insert(pType);
		}
	}
	auto addPower = [&output, &drain](std::int64_t power, int count)
	{
		if (power > 0)
			output += power * count;
		else
			drain -= power * count;
	};

	struct CountedPower
	{
		int Remaining;
		int Power;
	};

	auto updatePower = [&](const auto& types, const auto& instances)
	{
		std::unordered_map<const TechnoTypeClass*, CountedPower> counted;
		for (const auto pType : types)
		{
			const int count = pThis->CountOwnedAndPresent(pType);
			if (count == 0)
				continue;

			const int power = TechnoTypeExt::Fetch(pType)->Power;
			addPower(power, count);
			if (enhancedTypes.contains(pType))
				counted.emplace(pType, CountedPower { count, power });
		}
		if (counted.empty())
			return;

		// The counters remain authoritative, including entries with no matching
		// live instance. Replace at most one counted contribution per instance.
		for (const auto pUnit : instances)
		{
			if (!pUnit || !pUnit->Type || pUnit->Owner != pThis || !pUnit->IsAlive || pUnit->Health <= 0 || pUnit->InLimbo)
				continue;
			if constexpr (std::is_same_v<std::remove_cvref_t<decltype(*pUnit)>, InfantryClass>)
			{
				// Native infantry gain does not add either category to its active
				// counter. They must not consume another infantry's counted slot.
				if (pUnit->Technician || pUnit->Type->DontScore)
					continue;
			}

			const auto it = counted.find(pUnit->Type);
			if (it == counted.end() || it->second.Remaining <= 0)
				continue;

			auto& countedPower = it->second;
			--countedPower.Remaining;
			const auto [realOutput, realDrain] = BuildingTypeExt::GetUnitPower(pUnit->Type,
				static_cast<std::int64_t>(countedPower.Power), pThis, pUnit);
			addPower(countedPower.Power, -1);
			output += realOutput;
			drain += realDrain;
		}
	};

	updatePower(InfantryTypeClass::Array, InfantryClass::Array);
	updatePower(UnitTypeClass::Array, UnitClass::Array);
	updatePower(AircraftTypeClass::Array, AircraftClass::Array);
	// Buildings already contributed through native UpdatePower.
	pThis->PowerOutput = static_cast<int>(std::clamp(output,
		static_cast<std::int64_t>(std::numeric_limits<int>::min()), static_cast<std::int64_t>(std::numeric_limits<int>::max())));
	pThis->PowerDrain = static_cast<int>(std::clamp(drain,
		static_cast<std::int64_t>(std::numeric_limits<int>::min()), static_cast<std::int64_t>(std::numeric_limits<int>::max())));
}

