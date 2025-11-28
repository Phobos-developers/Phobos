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
	const bool hasInhibitType = !pExt->SW_InhibitTypes.empty();
	const bool hasInhibitor = !pExt->SW_Inhibitors.empty() || pExt->SW_AnyInhibitor;

	if (hasDesignateType || hasDesignator || hasInhibitType || hasInhibitor)
	{
		if (hasDesignateType || hasDesignator)
			data.NeedsDesignator = true;

		for (const auto pTechno : TechnoClass::Array)
		{
			if (!pTechno->IsAlive || !pTechno->Health || pTechno->InLimbo)
				continue;

			const bool deactivated = pTechno->Deactivated;

			if (deactivated && !hasDesignateType && !hasInhibitType)
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
			const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
			const auto pTechnoTypeExt = pTechnoExt->TypeExtData;
			const auto pTechnoType = pTechnoTypeExt->OwnerObject();
			const auto pTechnoOwner = pTechno->Owner;
			const int sight = pTechnoType->Sight;

			if (hasDesignateType)
			{
				for (const auto signal : pExt->SW_DesignateTypes)
				{
					if (inactive && signal->Powered)
						continue;

					if (isTemporal && signal->StopInTemporal)
						continue;

					if (!EnumFunctions::CanTargetHouse(signal->Affects.Get(AffectedHouse::Owner), pOwner, pTechnoOwner))
						continue;

					bool findSignal = std::ranges::find(pTechnoTypeExt->DesignateTypes, signal) == pTechnoTypeExt->DesignateTypes.cend();

					if (!findSignal && pTechnoExt->AE.HasDesignator)
					{
						for (const auto& attachEffect : pTechnoExt->AttachedEffects)
						{
							if (signal == attachEffect->GetType()->DesignateType)
							{
								findSignal = true;
								break;
							}
						}
					}

					if (!findSignal)
						continue;

					const int range = signal->Range.Get(sight);

					if (range > 0)
						data.Designators.emplace_back(range * range, cell);
				}
			}

			if (hasDesignator && !deactivated
				&& EnumFunctions::CanTargetHouse(pExt->SW_Designators_Houses, pOwner, pTechnoOwner)
				&& (pExt->SW_AnyDesignator || pExt->SW_Designators.Contains(pTechnoType)))
			{
				const int range = pTechnoTypeExt->DesignatorRange.Get(sight);

				if (range > 0)
					data.Designators.emplace_back(range * range, cell);
			}

			if (hasInhibitType)
			{
				for (const auto signal : pExt->SW_InhibitTypes)
				{
					if (inactive && signal->Powered)
						continue;

					if (isTemporal && signal->StopInTemporal)
						continue;

					if (!EnumFunctions::CanTargetHouse(signal->Affects.Get(AffectedHouse::Enemies), pOwner, pTechnoOwner))
						continue;

					bool findSignal = std::ranges::find(pTechnoTypeExt->InhibitTypes, signal) == pTechnoTypeExt->InhibitTypes.cend();

					if (!findSignal && pTechnoExt->AE.HasInhibitor)
					{
						for (const auto& attachEffect : pTechnoExt->AttachedEffects)
						{
							if (signal == attachEffect->GetType()->InhibitType)
							{
								findSignal = true;
								break;
							}
						}
					}

					if (!findSignal)
						continue;

					const int range = signal->Range.Get(sight);

					if (range > 0)
						data.Inhibitors.emplace_back(range * range, cell);
				}
			}

			if (hasInhibitor && buildingOnline && !deactivated
				&& EnumFunctions::CanTargetHouse(pExt->SW_Inhibitors_Houses, pOwner, pTechnoOwner)
				&& (pExt->SW_AnyInhibitor || pExt->SW_Inhibitors.Contains(pTechnoType)))
			{
				const int range = pTechnoTypeExt->InhibitorRange.Get(sight);

				if (range > 0)
					data.Inhibitors.emplace_back(range * range, cell);
			}
		}
	}

	return data;
}
