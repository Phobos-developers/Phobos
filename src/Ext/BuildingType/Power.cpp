#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Techno/Body.h>

namespace
{
	template <typename T>
	std::pair<T, T> ApplyPowerEnhancers(TechnoTypeClass* pType, T output, HouseClass* pHouse, TechnoClass* pPowerPlant)
	{
		T amount = 0;
		float factor = 1.0f;
		std::map<int, int> applied; // index, count

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

	int GetRealPower(BuildingClass* pBuilding, int baseOutput)
	{
		if (!pBuilding->HasPower || pBuilding->IsUnderEMP())
			return 0;

		int power = 0;
		int extraPower = 0;
		ForEachPowerComponent(pBuilding, [&](BuildingTypeClass* pType, bool host)
		{
			const auto [output, extra] = BuildingTypeExt::GetEnhancedPower(pType,
				host ? baseOutput : pType->PowerBonus, pBuilding->Owner, pBuilding);
			power += output;
			extraPower += extra;
		});

		if (power + extraPower <= 0)
			return 0;

		// Preserve the real-power rounding: sum all components before scaling,
		// and leave the enhancer's additive amount independent of damage.
		const double factor = BuildingTypeExt::Fetch(pBuilding->Type)->PowerPlant_DamageFactor;
		if (factor == 1.0)
			power = static_cast<int>(power * pBuilding->GetHealthPercentage());
		else if (factor != 0.0)
		{
			const double scale = 1.0 - factor + factor * pBuilding->GetHealthPercentage();
			power = static_cast<int>(power * scale);
			// Only damage-induced sign changes are floored; Amount is not scaled.
			if (scale < 0.0)
				power = Math::max(power, 0);
		}

		return power + extraPower;
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
	return GetRealPower(pBuilding, baseOutput);
}
