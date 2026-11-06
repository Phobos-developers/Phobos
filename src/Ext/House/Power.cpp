#include "Body.h"

#include <Ext/BuildingType/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Utilities/AresHelper.h>

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <limits>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace
{
	// Consumed later in the same UpdatePower call, after Ares has applied its
	// house-wide zero floor. It includes auxiliary power and all local drains.
	HouseClass* powerOutputContext = nullptr;
	int powerOutputBeforeDrainClamp = 0;
}

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

		for (const auto pType : pExt->PowerPlantEnhancer_AffectTypes)
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

HouseExt::PowerAdjustment HouseExt::UpdateUnitPower(HouseClass* const pThis)
{
	if (!Phobos::Config::UnitPowerDrain)
		return {};

	std::int64_t output = pThis->PowerOutput;
	std::int64_t drain = pThis->PowerDrain;
	std::int64_t aiDelta = 0;
	std::int64_t aiDrainDelta = 0;
	bool aiAdjusted = false;
	const bool affectedByEnhancers = RulesExt::Global()->AI_Power_AffectedByEnhancers;
	std::unordered_set<const TechnoTypeClass*> enhancedTypes;
	for (const auto pEnhancer : HouseExt::Fetch(pThis)->PowerPlantEnhancers)
	{
		if (!TechnoExt::IsActive(pEnhancer) || !pEnhancer->HasPower)
			continue;

		for (const auto pType : BuildingTypeExt::Fetch(pEnhancer->Type)->PowerPlantEnhancer_AffectTypes)
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
		int AIPower;
		bool EnhanceAI;
	};

	auto updatePower = [&](const auto& types, const auto& instances)
	{
		std::unordered_map<const TechnoTypeClass*, CountedPower> counted;
		for (const auto pType : types)
		{
			const int count = pThis->CountOwnedAndPresent(pType);
			if (count == 0)
				continue;

			const auto pExt = TechnoTypeExt::Fetch(pType);
			const int power = pExt->Power;
			const int aiPower = pExt->AI_Power.Get(power);
			addPower(power, count);
			aiDelta += (static_cast<std::int64_t>(aiPower) - power) * count;
			aiDrainDelta += (Math::max(-static_cast<std::int64_t>(aiPower), 0LL)
				- Math::max(-static_cast<std::int64_t>(power), 0LL)) * count;
			aiAdjusted |= aiPower != power;
			if (enhancedTypes.contains(pType))
				counted.emplace(pType, CountedPower { count, power, aiPower,
					pExt->AI_Power_AffectedByEnhancers.Get(affectedByEnhancers) });
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
			const auto realPower = realOutput - realDrain;
			std::int64_t aiOutput = Math::max(static_cast<std::int64_t>(countedPower.AIPower), 0LL);
			std::int64_t aiDrain = Math::max(-static_cast<std::int64_t>(countedPower.AIPower), 0LL);
			if (countedPower.EnhanceAI)
			{
				if (countedPower.AIPower == countedPower.Power)
				{
					aiOutput = realOutput;
					aiDrain = realDrain;
				}
				else
				{
					const auto enhanced = BuildingTypeExt::GetUnitPower(pUnit->Type,
						static_cast<std::int64_t>(countedPower.AIPower), pThis, pUnit);
					aiOutput = enhanced.first;
					aiDrain = enhanced.second;
				}
			}
			const auto aiPower = aiOutput - aiDrain;
			// Replace the same counted slot on both grids; unmatched counter
			// entries retain their respective unenhanced base contributions.
			aiDelta += aiPower - realPower
				- (static_cast<std::int64_t>(countedPower.AIPower) - countedPower.Power);
			aiDrainDelta += aiDrain - realDrain
				- (Math::max(-static_cast<std::int64_t>(countedPower.AIPower), 0LL)
					- Math::max(-static_cast<std::int64_t>(countedPower.Power), 0LL));
			aiAdjusted |= aiPower != realPower || aiDrain != realDrain;
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
	// Retain the raw-to-cache correction even when only a building overrides
	// its AI contribution. The actual grid stores saturated ints.
	return { aiDelta, aiDrainDelta,
		output - drain - (static_cast<std::int64_t>(pThis->PowerOutput) - pThis->PowerDrain),
		drain - pThis->PowerDrain, aiAdjusted };
}

void HouseExt::RecordPowerOutputBeforeDrainClamp(HouseClass* const pThis)
{
	powerOutputContext = pThis;
	powerOutputBeforeDrainClamp = pThis->PowerOutput;
}

// Keep the AI adjustment in the same snapshot as the real power totals. Reading
// live buildings during a later decision could mix new state with an old cache.
std::int64_t HouseExt::UpdateAIPower(HouseClass* const pThis, PowerAdjustment unitPower)
{
	std::int64_t delta = unitPower.Net;
	std::int64_t aiDrain = static_cast<std::int64_t>(pThis->PowerDrain) + unitPower.Drain;
	bool adjusted = unitPower.Adjusted;
	const bool affectedByEnhancers = RulesExt::Global()->AI_Power_AffectedByEnhancers;
	const bool blackout = pThis->PowerBlackoutTimer.GetTimeLeft() != 0;
	const bool outputDisabled = blackout || pThis->IsBeingDrained;
	const bool capturedOutput = powerOutputContext == pThis;
	const auto realOutputBeforeClamp = powerOutputBeforeDrainClamp;
	powerOutputContext = nullptr;
	std::int64_t buildingOutputDelta = 0;
	std::int64_t localDrainDelta = 0;
	bool aiFullDrain = false;

	for (auto const pBuilding : pThis->Buildings)
	{
		if (!pBuilding || pBuilding->Owner != pThis || pBuilding->InLimbo || !pBuilding->IsOnMap)
		{
			continue;
		}

		std::int64_t drainDelta;
		bool buildingAdjusted;
		const auto buildingDelta = BuildingTypeExt::GetAIPowerDelta(pBuilding, affectedByEnhancers,
			capturedOutput ? false : outputDisabled, &drainDelta, &buildingAdjusted);
		delta += buildingDelta;
		aiDrain += drainDelta;
		adjusted |= buildingAdjusted;
		buildingOutputDelta += buildingDelta + drainDelta;

		if (capturedOutput && pBuilding->IsBeingDrained())
		{
			const auto pDrainer = pBuilding->DrainingMe;
			if (!pDrainer)
				continue;
			const auto pBuildingExt = TechnoTypeExt::Fetch(pBuilding->Type);
			const auto pDrainerExt = TechnoTypeExt::Fetch(pDrainer->GetTechnoType());
			const std::int64_t realOutput = pBuilding->GetPowerOutput();
			std::int64_t buildingAIDrain;
			const auto aiNet = BuildingTypeExt::GetAIPower(pBuilding, affectedByEnhancers, false, &buildingAIDrain);
			const auto aiOutput = aiNet + buildingAIDrain;
			if (AresHelper::CanUseAres && (pBuildingExt->Drain_Local || pDrainerExt->Drain_Local))
			{
				auto localDrain = [&](std::int64_t output)
				{
					if (output <= 0)
						return 0LL;
					for (const std::int64_t amount : { pDrainerExt->Drain_Amount.Get(), pBuildingExt->Drain_Amount.Get() })
					{
						if (amount > 0)
							output = Math::min(output, amount);
						else if (amount != std::numeric_limits<int>::min())
							output = Math::max(output, -amount);
						// Ares negates its signed int before comparison; INT_MIN
						// stays negative and cannot raise a positive drain amount.
					}
					return output;
				};
				localDrainDelta += localDrain(realOutput) - localDrain(aiOutput);
			}
			else
				aiFullDrain |= aiOutput > 0;
		}
	}

	if (capturedOutput)
	{
		const auto aiOutput = blackout || aiFullDrain ? 0LL : Math::max(0LL,
			static_cast<std::int64_t>(realOutputBeforeClamp) + buildingOutputDelta + localDrainDelta);
		// Clamp both output lines independently; actual overdrain was discarded
		// by Ares before this hook. Unit power is added after that native step.
		const auto realOutput = outputDisabled ? 0LL : Math::max(0LL, static_cast<std::int64_t>(realOutputBeforeClamp));
		delta += aiOutput - realOutput - buildingOutputDelta;
	}

	if (adjusted)
	{
		delta += unitPower.OverflowNet;
		aiDrain += unitPower.OverflowDrain;
	}
	HouseExt::Fetch(pThis)->AIPowerDelta = delta;
	return aiDrain;
}
