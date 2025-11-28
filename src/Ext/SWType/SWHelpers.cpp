#include "Body.h"
#include <MessageListClass.h>

// Universal handler of the rolls-weights system
std::vector<int> SWTypeExt::ExtData::WeightedRollsHandler(ValueableVector<float>* rolls, std::vector<ValueableVector<int>>* weights, size_t size)
{
	bool rollOnce = false;
	size_t rollsSize = rolls->size();
	const size_t weightsSize = weights->size();
	int index;
	std::vector<int> indices;

	// if no RollChances are supplied, do only one roll
	if (rollsSize == 0)
	{
		rollsSize = 1;
		rollOnce = true;
	}

	indices.reserve(rollsSize);

	for (size_t i = 0; i < rollsSize; i++)
	{
		this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();

		if (!rollOnce && this->RandomBuffer > (*rolls)[i])
			continue;

		// If there are more rolls than weight lists, use the last weight list
		const size_t j = std::min(weightsSize - 1, i);
		index = GeneralUtils::ChooseOneWeighted(this->RandomBuffer, &(*weights)[j]);

		// If modder provides more weights than there are objects and we hit one of these, ignore it
		// otherwise add
		if (size_t(index) < size)
			indices.push_back(index);
	}

	return indices;
}

// =============================
// Ares 0.A helpers
// Inhibitors check
//bool SWTypeExt::ExtData::IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno) const
//{
//	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
//	{
//		if (EnumFunctions::CanTargetHouse(this->SW_Inhibitors_Houses, pOwner, pTechno->Owner))
//		{
//			if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pTechno))
//			{
//				if (!pBuilding->IsPowerOnline())
//					return false;
//			}
//
//			return this->SW_AnyInhibitor || this->SW_Inhibitors.Contains(pTechno->GetTechnoType());
//		}
//	}
//
//	return false;
//}

bool SWTypeExt::ExtData::IsInhibitorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	if (!pTechno->IsAlive || !pTechno->Health || pTechno->InLimbo)
		return false;

	const bool deactivated = pTechno->Deactivated;

	if (deactivated && this->SW_InhibiteTypes.empty())
		return false;

	const bool isTemporal = pTechno->TemporalTargetingMe || pTechno->IsBeingWarpedOut();
	bool inactive = deactivated || pTechno->IsUnderEMP();

	if (const auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		inactive |= !pBuilding->IsPowerOnline();

	const auto center = pTechno->GetCenterCoords();
	const double distanceSqr = coords.DistanceFromSquared(CellClass::Coord2Cell(center));
	const auto pTechnoType = pTechno->GetTechnoType();
	const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

	for (const auto signal : this->SW_InhibiteTypes)
	{
		if (inactive && signal->Powered)
			continue;

		if (isTemporal && signal->StopInTemporal)
			continue;

		if (std::ranges::find(pTechnoTypeExt->InhibiteTypes, signal) == pTechnoTypeExt->InhibiteTypes.cend()
			|| !EnumFunctions::CanTargetHouse(signal->Affects.Get(AffectedHouse::Enemies), pOwner, pTechno->Owner))
			continue;

		const int range = signal->Range.Get(pTechnoType->Sight);

		if (distanceSqr <= range * range)
			return true;
	}

	if (!deactivated && EnumFunctions::CanTargetHouse(this->SW_Inhibitors_Houses, pOwner, pTechno->Owner) && (this->SW_AnyInhibitor || this->SW_Inhibitors.Contains(pTechnoType)))
	{
		const int range = pTechnoTypeExt->InhibitorRange.Get(pTechnoType->Sight);

		if (distanceSqr <= range * range)
			return true;
	}

	return false;
}

bool SWTypeExt::ExtData::HasInhibitor(HouseClass* pOwner, const CellStruct& coords) const
{
	// does not allow inhibitors
	if (this->SW_InhibiteTypes.empty() && this->SW_Inhibitors.empty() && !this->SW_AnyInhibitor)
		return false;

	// a single inhibitor in range suffices
	return std::ranges::any_of(TechnoClass::Array, [=, &coords](TechnoClass* pTechno)
		{ return this->IsInhibitorEligible(pOwner, coords, pTechno); }
	);
}

// Designators check
//bool SWTypeExt::ExtData::IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const
//{
//	if (EnumFunctions::CanTargetHouse(this->SW_Designators_Houses, pOwner, pTechno->Owner))
//	{
//		if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
//			return this->SW_AnyDesignator || this->SW_Designators.Contains(pTechno->GetTechnoType());
//	}
//
//	return false;
//}

bool SWTypeExt::ExtData::IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	if (!pTechno->IsAlive || !pTechno->Health || pTechno->InLimbo)
		return false;

	const bool deactivated = pTechno->Deactivated;

	if (deactivated && this->SW_DesignateTypes.empty())
		return false;

	const bool isTemporal = pTechno->TemporalTargetingMe || pTechno->IsBeingWarpedOut();
	bool inactive = deactivated || pTechno->IsUnderEMP();

	if (const auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		inactive |= !pBuilding->IsPowerOnline();

	const auto center = pTechno->GetCenterCoords();
	const double distanceSqr = coords.DistanceFromSquared(CellClass::Coord2Cell(center));
	const auto pTechnoType = pTechno->GetTechnoType();
	const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

	for (const auto signal : this->SW_DesignateTypes)
	{
		if (inactive && signal->Powered)
			continue;

		if (isTemporal && signal->StopInTemporal)
			continue;

		if (std::ranges::find(pTechnoTypeExt->DesignateTypes, signal) == pTechnoTypeExt->DesignateTypes.cend()
			|| !EnumFunctions::CanTargetHouse(signal->Affects.Get(AffectedHouse::Owner), pOwner, pTechno->Owner))
			continue;

		const int range = signal->Range.Get(pTechnoType->Sight);

		if (distanceSqr <= range * range)
			return true;
	}

	if (!deactivated && EnumFunctions::CanTargetHouse(this->SW_Designators_Houses, pOwner, pTechno->Owner) && (this->SW_AnyDesignator || this->SW_Designators.Contains(pTechnoType)))
	{
		const int range = pTechnoTypeExt->DesignatorRange.Get(pTechnoType->Sight);

		if (distanceSqr <= range * range)
			return true;
	}

	return false;
}

bool SWTypeExt::ExtData::HasDesignator(HouseClass* pOwner, const CellStruct& coords) const
{
	// does not require designators
	if (this->SW_DesignateTypes.empty() && this->SW_Designators.empty() && !this->SW_AnyDesignator)
		return true;

	// a single designator in range suffices
	return std::ranges::any_of(TechnoClass::Array, [=, &coords](TechnoClass* pTechno)
		{ return this->IsDesignatorEligible(pOwner, coords, pTechno); });
}

bool SWTypeExt::ExtData::IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const
{
	if (!this->IsLaunchSite(pBuilding))
		return false;

	if (ignoreRange)
		return true;

	// get the range for this building
	const auto range = this->GetLaunchSiteRange(pBuilding);
	const double minRange = range.first;
	const double maxRange = range.second;

	const auto coords = pBuilding->GetCenterCoords();
	const auto center = CellClass::Coord2Cell(coords);
	const int distance = static_cast<int>(Coords.DistanceFrom(center));

	// negative range values just pass the test
	return (minRange < 0.0 || (double)distance >= minRange)
		&& (maxRange < 0.0 || (double)distance <= maxRange);
}

bool SWTypeExt::ExtData::IsLaunchSite(BuildingClass* pBuilding) const
{
	if (pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline())
	{
		auto const pExt = BuildingExt::ExtMap.Find(pBuilding);
		return pExt->HasSuperWeapon(this->OwnerObject()->ArrayIndex, true);
	}

	return false;
}

std::pair<double, double> SWTypeExt::ExtData::GetLaunchSiteRange(BuildingClass* pBuilding) const
{
	return std::make_pair(this->SW_RangeMinimum.Get(), this->SW_RangeMaximum.Get());
}

bool SWTypeExt::ExtData::IsAvailable(HouseClass* pHouse) const
{
	const auto pThis = this->OwnerObject();

	// check whether the optional aux building exists
	if (pThis->AuxBuilding)
	{
		if ((BuildingTypeExt::ExtMap.Find(pThis->AuxBuilding)->PowersUp_Buildings.size() > 0 || BuildingTypeClass::Find(pThis->AuxBuilding->PowersUpBuilding))
			&& BuildingTypeExt::GetUpgradesAmount(pThis->AuxBuilding, pHouse) <= 0)
			return false;
		else if (pHouse->CountOwnedAndPresent(pThis->AuxBuilding) <= 0)
			return false;
	}

	// allow only certain houses, disallow forbidden houses
	const auto OwnerBits = 1u << pHouse->Type->ArrayIndex;

	if (!(this->SW_RequiredHouses & OwnerBits) || (this->SW_ForbiddenHouses & OwnerBits))
		return false;

	// check that any aux building exist and no neg building
	auto IsBuildingPresent = [pHouse](BuildingTypeClass* pType)
		{
			if (pType)
			{
				if (BuildingTypeExt::ExtMap.Find(pType)->PowersUp_Buildings.size() > 0 || BuildingTypeClass::Find(pType->PowersUpBuilding))
					return BuildingTypeExt::GetUpgradesAmount(pType, pHouse) > 0;
				else
					return pHouse->CountOwnedAndPresent(pType) > 0;
			}

			return false;
		};

	const auto& Aux = this->SW_AuxBuildings;

	if (!Aux.empty() && std::none_of(Aux.begin(), Aux.end(), IsBuildingPresent))
		return false;

	const auto& Neg = this->SW_NegBuildings;

	if (std::any_of(Neg.begin(), Neg.end(), IsBuildingPresent))
		return false;

	return true;
}

std::vector<BuildingClass*> SWTypeExt::ExtData::GetEMPulseCannons(HouseClass* pOwner, const CellStruct& cell) const
{
	std::vector<BuildingClass*> emCannons;

	for (auto const& pBuilding : pOwner->Buildings)
	{
		bool eligible = false;
		auto const pType = pBuilding->Type;

		if (!this->EMPulse_Cannons.empty() && this->EMPulse_Cannons.Contains(pType) && pBuilding->IsAlive
			&& pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline())
		{
			eligible = true;
		}
		else if (pType->EMPulseCannon && this->IsLaunchSite(pBuilding))
		{
			eligible = true;
		}

		if (eligible)
		{
			auto range = this->GetEMPulseCannonRange(pBuilding);
			auto const& minRange = range.first;
			auto const& maxRange = range.second;
			auto const center = CellClass::Coord2Cell(pBuilding->GetCenterCoords());
			auto const distance = cell.DistanceFrom(center);

			if ((minRange < 0.0 || distance >= minRange)
				&& (maxRange < 0.0 || distance <= maxRange))
			{
				emCannons.push_back(pBuilding);
			}
		}
	}

	return emCannons;
}

std::pair<double, double> SWTypeExt::ExtData::GetEMPulseCannonRange(BuildingClass* pBuilding) const
{
	if (this->EMPulse_TargetSelf)
		return std::make_pair(-1.0, -1.0);

	if (!pBuilding)
		return std::make_pair(0.0, 0.0);

	if (auto pWeapon = pBuilding->GetWeapon(0)->WeaponType)
	{
		double maxRange = this->SW_RangeMaximum;
		if (maxRange < 0.0)
			maxRange = pWeapon->Range / (double)Unsorted::LeptonsPerCell;

		double minRange = this->SW_RangeMinimum;
		if (minRange < 0.0)
		{
			minRange = pWeapon->MinimumRange / (double)Unsorted::LeptonsPerCell;
		}

		return std::make_pair(minRange, maxRange);
	}

	return std::make_pair(this->SW_RangeMinimum.Get(), this->SW_RangeMaximum.Get());
}

void SWTypeExt::ExtData::PrintMessage(const CSFText& message, HouseClass* pFirer) const
{
	if (message.empty())
		return;

	int color = ColorScheme::FindIndex("Gold");
	if (this->Message_FirerColor)
	{
		// firer color
		if (pFirer)
		{
			color = pFirer->ColorSchemeIndex;
		}
	}
	else
	{
		if (this->Message_ColorScheme > -1)
		{
			// user defined color
			color = this->Message_ColorScheme;
		}
		else if (const auto pCurrent = HouseClass::CurrentPlayer)
		{
			// default way: the current player's color
			color = pCurrent->ColorSchemeIndex;
		}
	}

	// print the message
	MessageListClass::Instance.PrintMessage(message, RulesClass::Instance->MessageDelay, color);
}
