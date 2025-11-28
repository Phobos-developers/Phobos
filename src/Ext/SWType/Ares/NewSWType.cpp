#include "NewSWType.h"

TargetingData& AresNewSWType::GetTargetingData(TargetingData& data, SuperWeaponTypeClass** pExt_Ares, HouseClass* pOwner) const
{
	data.TypeExt_Ares = pExt_Ares;
	data.Owner = pOwner;
	data.NeedsLaunchSite = false;
	data.NeedsDesignator = false;
	std::memset(&data.LaunchSites, 0, sizeof(data.LaunchSites));
	std::memset(&data.Designators, 0, sizeof(data.Designators));
	std::memset(&data.Inhibitors, 0, sizeof(data.Inhibitors));

	// get launchsite data
	const auto& [minRange, maxRange] = this->GetLaunchSiteRange(pExt_Ares);

	if (minRange >= 0.0 || maxRange >= 0.0)
	{
		data.NeedsLaunchSite = true;

		for (const auto pBuilding : pOwner->Buildings)
		{
			if (this->IsLaunchSite(pExt_Ares, pBuilding))
			{
				const auto& [minSiteRange, maxSiteRange] = this->GetLaunchSiteRange(pExt_Ares, pBuilding);
				const auto center = pBuilding->GetCoords();
				data.LaunchSites.emplace_back(pBuilding, CellClass::Coord2Cell(center), minSiteRange, maxSiteRange);
			}
		}
	}

	const auto pExt = SWTypeExt::ExtMap.Find(*pExt_Ares);
	const bool hasDesignateType = !pExt->SW_DesignateTypes.empty();
	const bool hasDesignator = !pExt->SW_Designators.empty() || pExt->SW_AnyDesignator;
	const bool hasInhibiteType = !pExt->SW_InhibiteTypes.empty();
	const bool hasInhibitor = !pExt->SW_Inhibitors.empty() || pExt->SW_AnyInhibitor;

	if (hasDesignateType || hasDesignator || hasInhibiteType || hasInhibitor)
	{
		if (hasDesignateType || hasDesignator)
			data.NeedsDesignator = true;

		for (const auto pTechno : TechnoClass::Array)
		{
			if (!pTechno->IsAlive || !pTechno->Health || pTechno->InLimbo)
				continue;

			const bool deactivated = pTechno->Deactivated;

			if (deactivated && !hasDesignateType && !hasInhibiteType)
				continue;

			const bool isTemporal = pTechno->TemporalTargetingMe || pTechno->IsBeingWarpedOut();
			bool inactive = deactivated || pTechno->IsUnderEMP();
			bool buildingOnline = true;

			if (const auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
			{
				buildingOnline = pBuilding->IsPowerOnline();
				inactive |= !buildingOnline;
			}

			const auto center = pTechno->GetCoords();
			const auto cell = CellClass::Coord2Cell(center);
			const auto pTechnoType = pTechno->GetTechnoType();
			const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

			if (hasDesignateType)
			{
				for (const auto signal : pExt->SW_DesignateTypes)
				{
					if (inactive && signal->Powered)
						continue;

					if (isTemporal && signal->StopInTemporal)
						continue;

					if (std::ranges::find(pTechnoTypeExt->DesignateTypes, signal) == pTechnoTypeExt->DesignateTypes.cend()
						|| !EnumFunctions::CanTargetHouse(signal->Affects.Get(AffectedHouse::Owner), pOwner, pTechno->Owner))
						continue;

					const int range = signal->Range.Get(pTechnoType->Sight);

					if (range > 0)
						data.Designators.emplace_back(range * range, cell);
				}
			}

			if (hasDesignator && !deactivated
				&& EnumFunctions::CanTargetHouse(pExt->SW_Designators_Houses, pOwner, pTechno->Owner)
				&& (pExt->SW_AnyDesignator || pExt->SW_Designators.Contains(pTechnoType)))
			{
				const int range = pTechnoTypeExt->DesignatorRange.Get(pTechnoType->Sight);

				if (range > 0)
					data.Designators.emplace_back(range * range, cell);
			}

			if (hasInhibiteType)
			{
				for (const auto signal : pExt->SW_InhibiteTypes)
				{
					if (inactive && signal->Powered)
						continue;

					if (isTemporal && signal->StopInTemporal)
						continue;

					if (std::ranges::find(pTechnoTypeExt->InhibiteTypes, signal) == pTechnoTypeExt->InhibiteTypes.cend()
						|| !EnumFunctions::CanTargetHouse(signal->Affects.Get(AffectedHouse::Enemies), pOwner, pTechno->Owner))
						continue;

					const int range = signal->Range.Get(pTechnoType->Sight);

					if (range > 0)
						data.Inhibitors.emplace_back(range * range, cell);
				}
			}

			if (hasInhibitor && buildingOnline && !deactivated
				&& EnumFunctions::CanTargetHouse(pExt->SW_Inhibitors_Houses, pOwner, pTechno->Owner)
				&& (pExt->SW_AnyInhibitor || pExt->SW_Inhibitors.Contains(pTechnoType)))
			{
				const int range = pTechnoTypeExt->InhibitorRange.Get(pTechnoType->Sight);

				if (range > 0)
					data.Inhibitors.emplace_back(range * range, cell);
			}
		}
	}

	return data;
}
