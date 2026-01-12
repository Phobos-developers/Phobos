#include "Body.h"

#include <Ext/House/Body.h>

// Universal handler of the rolls-weights system
std::vector<int> SWTypeExt::WeightedRollsHandler(ValueableVector<float>* rolls, std::vector<ValueableVector<int>>* weights, size_t size)
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
bool SWTypeExt::IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
	{
		if (!pOwner->IsAlliedWith(pTechno))
		{
			if (const auto pBld = abstract_cast<BuildingClass*, true>(pTechno))
			{
				if (!pBld->IsPowerOnline())
					return false;
			}

			return this->SW_AnyInhibitor || this->SW_Inhibitors.Contains(pTechno->GetTechnoType());
		}
	}

	return false;
}

bool SWTypeExt::IsInhibitorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	if (this->IsInhibitor(pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::Fetch(pType);

		// get the inhibitor's center
		const auto center = pTechno->GetCenterCoords();

		// has to be closer than the inhibitor range (which defaults to Sight)
		const double range = (double)pExt->InhibitorRange.Get(pType->Sight);
		return coords.DistanceFromSquared(CellClass::Coord2Cell(center)) <= range * range;
	}

	return false;
}

bool SWTypeExt::HasInhibitor(HouseClass* pOwner, const CellStruct& coords) const
{
	// does not allow inhibitors
	if (this->SW_Inhibitors.empty() && !this->SW_AnyInhibitor)
		return false;

	// a single inhibitor in range suffices
	return std::any_of(TechnoClass::Array.begin(), TechnoClass::Array.end(), [=, &coords](TechnoClass* pTechno)
		{ return this->IsInhibitorEligible(pOwner, coords, pTechno); }
	);
}

// Designators check
bool SWTypeExt::IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->Owner == pOwner && pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
		return this->SW_AnyDesignator || this->SW_Designators.Contains(pTechno->GetTechnoType());

	return false;
}

bool SWTypeExt::IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	if (this->IsDesignator(pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::Fetch(pType);

		// get the designator's center
		const auto center = pTechno->GetCenterCoords();

		// has to be closer than the designator range (which defaults to Sight)
		const double range = (double)pExt->DesignatorRange.Get(pType->Sight);
		return coords.DistanceFromSquared(CellClass::Coord2Cell(center)) <= range * range;
	}

	return false;
}

bool SWTypeExt::HasDesignator(HouseClass* pOwner, const CellStruct& coords) const
{
	// does not require designators
	if (this->SW_Designators.empty() && !this->SW_AnyDesignator)
		return true;

	// a single designator in range suffices
	return std::any_of(TechnoClass::Array.begin(), TechnoClass::Array.end(), [=, &coords](TechnoClass* pTechno)
		{ return this->IsDesignatorEligible(pOwner, coords, pTechno); });
}

bool SWTypeExt::IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const
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
	const double distanceSq = Coords.DistanceFromSquared(center);

	// negative range values just pass the test
	return (minRange < 0.0 || distanceSq >= minRange * minRange)
		&& (maxRange < 0.0 || distanceSq <= maxRange * maxRange);
}

bool SWTypeExt::IsLaunchSite(BuildingClass* pBuilding) const
{
	if (pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline())
	{
		auto const pExt = BuildingExt::Fetch(pBuilding);
		return pExt->HasSuperWeapon(this->OwnerObject()->ArrayIndex);
	}

	return false;
}

std::pair<double, double> SWTypeExt::GetLaunchSiteRange(BuildingClass* pBuilding) const
{
	return std::make_pair(this->SW_RangeMinimum.Get(), this->SW_RangeMaximum.Get());
}

bool SWTypeExt::IsAvailable(HouseClass* pHouse) const
{
	if (pHouse->TechLevel < this->SW_TechLevel)
		return false;

	const auto pThis = this->OwnerObject();
	const int shots = this->SW_Shots;

	if (shots >= 0 && HouseExt::Fetch(pHouse)->SuperExts[pThis->ArrayIndex].ShotCount >= shots)
		return false;

	if (pHouse->IsControlledByHuman() ? (!this->SW_AllowPlayer) : (!this->SW_AllowAI))
		return false;

	// allow only certain houses, disallow forbidden houses
	const auto ownerBits = 1u << pHouse->Type->ArrayIndex;

	if (!(this->SW_RequiredHouses & ownerBits) || (this->SW_ForbiddenHouses & ownerBits))
		return false;

	auto IsTechnoPresent = [pHouse](TechnoTypeClass* pType)
		{
			const auto pBuildingType = abstract_cast<BuildingTypeClass*, true>(pType);

			// June 7, 2026 - Starkku: PowersUpBuilding is now put in PowersUp_Buildings
			// so removed  BuildingTypeClass::Find(pBuildingType->PowersUpBuilding check here.
			if (pBuildingType && !BuildingTypeExt::Fetch(pBuildingType)->PowersUp_Buildings.empty())
				return BuildingTypeExt::GetUpgradesAmount(pBuildingType, pHouse) > 0;

			return HouseExt::Fetch(pHouse)->CountOwnedPresentAndLimboed(pType) > 0;
		};

	// check whether the optional aux building exists
	if (pThis->AuxBuilding && !IsTechnoPresent(pThis->AuxBuilding))
		return false;

	// check that any aux building exist and no neg building

	const auto& auxBuildings = this->SW_AuxBuildings;

	if (!auxBuildings.empty() && std::ranges::none_of(auxBuildings, IsTechnoPresent))
		return false;

	const auto& negBuildings = this->SW_NegBuildings;

	if (std::ranges::any_of(negBuildings, IsTechnoPresent))
		return false;

	const auto& auxTechnos = this->SW_AuxTechnos;

	if (!auxTechnos.empty() && std::ranges::none_of(auxTechnos, IsTechnoPresent))
		return false;

	const auto& negTechnos = this->SW_NegTechnos;

	if (std::ranges::any_of(negTechnos, IsTechnoPresent))
		return false;

	return true;
}

std::vector<BuildingClass*> SWTypeExt::GetEMPulseCannons(HouseClass* pOwner, const CellStruct& cell) const
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
			const auto range = this->GetEMPulseCannonRange(pBuilding);
			const double& minRange = range.first;
			const double& maxRange = range.second;
			const auto center = CellClass::Coord2Cell(pBuilding->GetCenterCoords());
			const double distanceSq = cell.DistanceFromSquared(center);

			if ((minRange < 0.0 || distanceSq >= minRange * minRange)
				&& (maxRange < 0.0 || distanceSq <= maxRange * maxRange))
			{
				emCannons.push_back(pBuilding);
			}
		}
	}

	return emCannons;
}

std::pair<double, double> SWTypeExt::GetEMPulseCannonRange(BuildingClass* pBuilding) const
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

void SWTypeExt::PrintMessage(const CSFText& message, HouseClass* pFirer) const
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

SuperClass* __stdcall SWTypeExt::IsSuperAvailable(int swIdx, HouseClass* pHouse)
{
	if (const auto pSuper = pHouse->Supers.GetItemOrDefault(swIdx))
	{
		const auto pExt = SWTypeExt::Fetch(pSuper->Type);

		if (pExt->IsAvailable(pHouse))
			return pSuper;
	}

	return nullptr;
}

// Replaces Ares Psychic Dominator SW target eligibility checks if Ares is present.
bool SWTypeExt::EligibleTargetForPsyDomSW(TechnoClass* pTechno)
{
	auto const pTechnoType = pTechno->GetTechnoType();

	// Always ignore Insignificant or civilian-house owned targets - change from Ares and vanilla behaviour.
	if (pTechnoType->Insignificant || pTechno->Owner->Type->MultiplayPassive)
		return false;

	if (auto const pSuper = SWTypeExt::CurrentAIEvaluatedSW)
	{
		auto const pTypeExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		auto const pOwner = pSuper->Owner;
		auto const pTargetHouse = pTechno->Owner;

		// If SW.AIRequiresHouse is explicitly set use that instead of restricting to enemies only.
		if (pTypeExt->SW_AIRequiresHouse.isset())
		{
			if (!EnumFunctions::CanTargetHouse(pTypeExt->SW_AIRequiresHouse, pOwner, pTargetHouse))
				return false;
		}
		else if (pOwner->IsAlliedWith(pTargetHouse))
		{
			return false;
		}

		// If SW.AIRequiresTarget is explicitly set check that here as well.
		if (pTypeExt->SW_AIRequiresTarget.isset() && !EnumFunctions::IsTechnoEligible(pTechno, pTypeExt->SW_AIRequiresTarget))
			return false;

		if (pTypeExt->SW_AITargeting_PsyDom_AllowTypes.size() > 0 && !pTypeExt->SW_AITargeting_PsyDom_AllowTypes.Contains(pTechnoType))
			return false;

		if (pTypeExt->SW_AITargeting_PsyDom_DisallowTypes.size() > 0 && pTypeExt->SW_AITargeting_PsyDom_DisallowTypes.Contains(pTechnoType))
			return false;

		// Skip normal MC immunity etc. checks and only check air & invulnerability separately with toggles to turn them off.
		if (pTypeExt->SW_AITargeting_PsyDom_SkipChecks)
		{
			if (pTechno->IsInAir() && !pTypeExt->SW_AITargeting_PsyDom_AllowAir)
				return false;

			if (pTechno->IsIronCurtained() && !pTypeExt->SW_AITargeting_PsyDom_AllowInvulnerable)
				return false;

			return true;
		}
	}

	return pTechno->CanBePermaMindControlled();
}

// Override Ares' SW AI targeting behaviour based in AI targeting type or whatever else.
bool SWTypeExt::HandleAITargetingOverrides(SuperClass* pSuper, SuperWeaponAITargetingMode aiTargetingType, CellStruct& targetCell, bool& isSuccessful)
{
	// Fix LightningRandom behaving suboptimally with explicit Designators.
	if (aiTargetingType == SuperWeaponAITargetingMode::LightningRandom)
	{
		auto const pTypeExt = SWTypeExt::ExtMap.Find(pSuper->Type);

		if (pTypeExt->SW_AITargeting_Random_SnapOnDesignators)
			return pTypeExt->PickDesignatorCell(pSuper->Owner, pTypeExt->SW_AITargeting_Random_PickFirstDesignator, targetCell, isSuccessful);
	}

	return false;
}

// Overrides target cell with a cell of designator or one in designator range not affected by inhibitor.
bool SWTypeExt::ExtData::PickDesignatorCell(HouseClass* pOwner, bool pickFirst, CellStruct& targetCell, bool& isSuccessful) const
{
	if (!this->SW_Designators.empty())
	{
		std::vector<TechnoClass*> inhibited;
		std::vector<CellStruct> designatorCells;

		// Evaluate technos for designators.
		for (auto* pTechno : TechnoClass::Array)
		{
			if (this->IsDesignator(pOwner, pTechno))
			{
				auto const cell = pTechno->GetMapCoords();

				if (this->HasInhibitor(pOwner, cell))
				{
					inhibited.push_back(pTechno); // Designator is in inhibitor range, defer evaluation.
				}
				else
				{
					// Bail out early if we're picking first match.
					if (pickFirst)
					{
						targetCell = cell;
						isSuccessful = true;
						return true;
					}

					designatorCells.push_back(cell);
				}
			}
		}

		// Pick random cell from found designators.
		if (!designatorCells.empty())
		{
			int index = ScenarioClass::Instance->Random.RandomRanged(0, designatorCells.size() - 1);
			targetCell = designatorCells[index];
			isSuccessful = true;
			return true;
		}

		// Evaluate designators in inhibitor range.
		if (!inhibited.empty())
		{
			// Scramble list of designators if we're not always picking first one.
			if (!pickFirst)
			{
				size_t n = inhibited.size();

				for (size_t i = n - 1; i > 0; --i)
				{
					size_t j = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(i));
					std::swap(inhibited[i], inhibited[j]);
				}
			}

			// Find first cell not within inhibitor range but still within designator's range.
			for (auto const* pTechno : inhibited)
			{
				auto cell = pTechno->GetMapCoords();
				auto const pType = pTechno->GetTechnoType();
				auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pType);
				short maxDistance = static_cast<short>(pTechnoTypeExt->DesignatorRange.Get(pType->Sight) + 0.5);

				for (short dist = 1; dist <= maxDistance; ++dist)
				{
					for (short dx = -dist; dx <= dist; ++dx)
					{
						for (int sign : {-1, 1})
						{
							short x = cell.X + dx;
							short y = cell.Y + static_cast<short>(sign) * dist;

							if (!this->HasInhibitor(pOwner, { x, y }))
							{
								targetCell = { x, y };
								isSuccessful = true;
								return true;
							}
						}
					}

					for (short dy = -dist + 1; dy <= dist - 1; ++dy)
					{
						for (int sign : {-1, 1})
						{
							short x = cell.X + static_cast<short>(sign) * dist;
							short y = cell.Y + dy;

							if (!this->HasInhibitor(pOwner, { x, y }))
							{
								targetCell = { x, y };
								isSuccessful = true;
								return true;
							}
						}
					}
				}
			}
		}

		targetCell = CellStruct::Empty;
		isSuccessful = false;
		return true;
	}

	return false;
}
