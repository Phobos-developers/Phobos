#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>

namespace
{
	template <typename T>
	std::pair<T, T> ApplyPowerEnhancers(TechnoTypeClass* pType, T output, HouseClass* pHouse, TechnoClass* pPowerPlant, bool enhance = true)
	{
		T amount = 0;
		float factor = 1.0f;
		std::map<int, int> applied; // index, count

		if (enhance)
		{
			for (const auto pEnhancer : HouseExt::Fetch(pHouse)->PowerPlantEnhancers)
			{
				if (!TechnoExt::IsActive(pEnhancer) || !pEnhancer->HasPower)
					continue;

				const auto pEnhancerType = pEnhancer->Type;
				const auto pExt = BuildingTypeExt::Fetch(pEnhancerType);
				if (std::find(pExt->PowerPlantEnhancer_AffectTypes.begin(), pExt->PowerPlantEnhancer_AffectTypes.end(), pType) == pExt->PowerPlantEnhancer_AffectTypes.end())
					continue;

				const int range = pExt->PowerPlantEnhancer_Range.Get();
				if (range > 0 && (!pPowerPlant || pEnhancer->DistanceFrom(pPowerPlant) > range))
					continue;

				const int max = pExt->PowerPlantEnhancer_MaxCount;
				if (max > 0)
				{
					const auto it = applied.find(pEnhancerType->ArrayIndex);
					if (it != applied.cend() && it->second >= max)
						continue;
				}

				factor *= pExt->PowerPlantEnhancer_Factor;
				amount += pExt->PowerPlantEnhancer_Amount;
				++applied[pEnhancerType->ArrayIndex];
			}
		}

		if constexpr (std::is_same_v<T, int>)
			return { static_cast<T>(std::round(output * factor)), amount };
		else
			return { static_cast<T>(std::round(static_cast<double>(output) * factor)), amount };
	}

	// The host and each installed slot contribute once, including repeated types.
	template <typename F>
	void ForEachPowerComponent(BuildingClass* pBuilding, F action)
	{
		action(pBuilding->Type, true);
		if (pBuilding->UpgradeLevel)
		{
			for (const auto pUpgrade : pBuilding->Upgrades)
			{
				if (pUpgrade)
					action(pUpgrade, false);
			}
		}
	}

	template <typename T, typename F>
	T CalculatePowerOutput(BuildingClass* pBuilding, F getComponent)
	{
		if (!pBuilding->HasPower || pBuilding->IsUnderEMP())
			return 0;

		T power = 0;
		T extraPower = 0;
		ForEachPowerComponent(pBuilding, [&](BuildingTypeClass* pType, bool host)
		{
			const auto [output, extra] = getComponent(pType, host);
			power += output;
			extraPower += extra;
		});

		if (power + extraPower <= 0)
			return 0;

		// Preserve the real-power rounding: sum all components before scaling,
		// and leave the enhancer's additive amount independent of damage.
		const double factor = BuildingTypeExt::Fetch(pBuilding->Type)->PowerPlant_DamageFactor;
		if (factor == 1.0)
			power = static_cast<T>(power * pBuilding->GetHealthPercentage());
		else if (factor != 0.0)
		{
			const double scale = 1.0 - factor + factor * pBuilding->GetHealthPercentage();
			power = static_cast<T>(power * scale);
			// Only damage-induced sign changes are floored; Amount is not scaled.
			if (scale < 0.0)
				power = Math::max(power, static_cast<T>(0));
		}

		return power + extraPower;
	}

	template <typename T>
	T GetBasePowerOutput(BuildingClass* pBuilding)
	{
		const auto pType = pBuilding->Type;
		T output = pType->PowerBonus;
		if (pBuilding->HasExtraPowerBonus)
			output += pType->ExtraPowerBonus;
		if ((pType->UnitAbsorb || pType->InfantryAbsorb) && pType->ExtraPowerBonus > 0 && pBuilding->Passengers.NumPassengers > 0)
			output += static_cast<T>(pType->ExtraPowerBonus) * pBuilding->Passengers.NumPassengers;
		return output;
	}
}

std::pair<int, int> BuildingTypeExt::GetEnhancedPower(TechnoTypeClass* pType, int output, HouseClass* pHouse, TechnoClass* pPowerPlant)
{
	return ApplyPowerEnhancers<int>(pType, output, pHouse, pPowerPlant);
}

std::pair<std::int64_t, std::int64_t> BuildingTypeExt::GetEnhancedPower(TechnoTypeClass* pType, std::int64_t output, HouseClass* pHouse, TechnoClass* pSource)
{
	return ApplyPowerEnhancers<std::int64_t>(pType, output, pHouse, pSource);
}

std::pair<std::int64_t, std::int64_t> BuildingTypeExt::GetUnitPower(TechnoTypeClass* pType, std::int64_t power, HouseClass* pHouse, TechnoClass* pSource)
{
	// Match buildings: enhance supply (including Amount on zero supply),
	// retain base drain, and perform the pre-damage supply check.
	const auto [output, extra] = GetEnhancedPower(pType, Math::max(power, 0LL), pHouse, pSource);
	return { Math::max(output + extra, 0LL), Math::max(-power, 0LL) };
}

int BuildingTypeExt::GetPowerOutput(BuildingClass* pBuilding, int baseOutput)
{
	// Native GetPowerOutput checks warped-out state before reaching our hook.
	return CalculatePowerOutput<int>(pBuilding, [&](BuildingTypeClass* pType, bool host)
	{
		return GetEnhancedPower(pType, host ? baseOutput : pType->PowerBonus, pBuilding->Owner, pBuilding);
	});
}

std::int64_t BuildingTypeExt::GetAIPower(BuildingTypeClass* pType)
{
	const auto& power = BuildingTypeExt::Fetch(pType)->AI_Power;
	return power.isset() ? power.Get() : static_cast<std::int64_t>(pType->PowerBonus) - pType->PowerDrain;
}

std::int64_t BuildingTypeExt::GetAIPower(BuildingClass* pBuilding, bool defaultAffectedByEnhancers, bool outputDisabled, std::int64_t* pDrain)
{
	if (pDrain)
		*pDrain = 0;
	if (!pBuilding->IsAlive || pBuilding->Health <= 0 || pBuilding->IsBeingWarpedOut() || !pBuilding->HasPower)
		return 0;

	const auto baseOutput = GetBasePowerOutput<std::int64_t>(pBuilding);
	const auto output = CalculatePowerOutput<std::int64_t>(pBuilding, [&](BuildingTypeClass* pType, bool host)
	{
		const auto pExt = TechnoTypeExt::Fetch(pType);
		const bool enhance = pExt->AI_Power_AffectedByEnhancers.Get(defaultAffectedByEnhancers);
		const auto nativeOutput = host ? baseOutput : pType->PowerBonus;
		if (!pExt->AI_Power.isset()
			|| pExt->AI_Power.Get() == static_cast<std::int64_t>(pType->PowerBonus) - pType->PowerDrain)
		{
			// Preserve the real int/float rounding when the base is unchanged.
			const auto [power, extra] = ApplyPowerEnhancers<int>(pType,
				static_cast<int>(nativeOutput), pBuilding->Owner, pBuilding, enhance);
			return std::pair<std::int64_t, std::int64_t> { power, extra };
		}

		const auto value = static_cast<std::int64_t>(pExt->AI_Power.Get());
		const auto componentOutput = Math::max(value, 0LL) + (host ? baseOutput - pType->PowerBonus : 0);
		return ApplyPowerEnhancers<std::int64_t>(pType, componentOutput, pBuilding->Owner, pBuilding, enhance);
	});

	// Native drain has no health or EMP scaling. AI.Power replaces only its
	// base contribution; the host's extra drain is retained independently.
	std::int64_t drain = 0;
	ForEachPowerComponent(pBuilding, [&](BuildingTypeClass* pType, bool host)
	{
		const auto& power = TechnoTypeExt::Fetch(pType)->AI_Power;
		const auto nativePower = static_cast<std::int64_t>(pType->PowerBonus) - pType->PowerDrain;
		drain += !power.isset() || power.Get() == nativePower
			? pType->PowerDrain : Math::max(-static_cast<std::int64_t>(power.Get()), 0LL);
		if (host && pBuilding->HasExtraPowerDrain)
			drain += pType->ExtraPowerDrain;
	});
	if (pDrain)
		*pDrain = drain;
	return (outputDisabled ? 0 : output) - drain;
}

std::int64_t BuildingTypeExt::GetAIPowerDelta(BuildingClass* pBuilding, bool defaultAffectedByEnhancers, bool outputDisabled, std::int64_t* pDrainDelta, bool* pAdjusted)
{
	if (pDrainDelta)
		*pDrainDelta = 0;
	if (pAdjusted)
		*pAdjusted = false;
	bool adjusted = false;
	ForEachPowerComponent(pBuilding, [&](BuildingTypeClass* pType, bool)
	{
		const auto pExt = BuildingTypeExt::Fetch(pType);
		adjusted |= pExt->AI_Power.isset()
			|| (!pExt->AI_Power_AffectedByEnhancers.Get(defaultAffectedByEnhancers)
				&& !HouseExt::Fetch(pBuilding->Owner)->PowerPlantEnhancers.empty());
		if (pAdjusted && pExt->AI_Power.isset())
			*pAdjusted |= pExt->AI_Power.Get() != static_cast<std::int64_t>(pType->PowerBonus) - pType->PowerDrain;
	});
	if (!adjusted)
		return 0;

	std::int64_t aiDrain;
	const auto power = BuildingTypeExt::GetAIPower(pBuilding, defaultAffectedByEnhancers, outputDisabled, &aiDrain);
	const int output = outputDisabled ? 0 : pBuilding->GetPowerOutput();
	const int drain = pBuilding->GetPowerDrain();
	const auto delta = power - output + drain;
	if (pDrainDelta)
		*pDrainDelta = aiDrain - drain;
	if (pAdjusted)
		*pAdjusted |= delta != 0 || aiDrain != drain;
	return delta;
}
