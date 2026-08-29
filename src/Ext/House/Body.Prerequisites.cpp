#include "body.h"

bool HouseExt::HasGenericPrerequisite(int idx, HouseClass* const pHouse)
{
	if (idx >= 0 || !pHouse)
		return false;

	int absoluteIndex = std::abs(idx);
	if (absoluteIndex >= RulesExt::Global()->GenericPrerequisites.Count)
		return false;

	DynamicVectorClass<int> selectedPrerequisite = RulesExt::Global()->GenericPrerequisites.GetItem(absoluteIndex);
	if (selectedPrerequisite.Count == 0)
		return false;

	for (auto idxItem : selectedPrerequisite)
	{
		if (pHouse->ActiveBuildingTypes.GetItemCount(idxItem) > 0)
			return true;
	}

	return false;
}

int HouseExt::FindGenericPrerequisite(const char* id)
{
	if (BuildingTypeClass::FindIndex(id) >= 0)
		return INT32_MAX;

	if (RulesExt::Global()->GenericPrerequisitesNames.Count == 0)
		RulesExt::FillDefaultPrerequisites();

	int i = 0;
	for (auto str : RulesExt::Global()->GenericPrerequisitesNames)
	{
		if (_strcmpi(id, str) == 0)
			return i;

		--i;
	}

	return INT32_MAX; // Error
}

bool HouseExt::IsAvailableToHouse(HouseClass* const pHouse, TechnoTypeClass* const pItem)
{
	if (!pHouse || !pItem)
		return false;

	const auto pType = pHouse->Type;
	if (!pType)
		return false;

	DWORD const bitHouse = 1u << pType->ArrayIndex2;

	bool inOwners = pItem->InOwners(bitHouse);
	bool inRequired = pItem->InRequiredHouses(bitHouse);
	bool inForbidden = pItem->InForbiddenHouses(bitHouse);

	if (!inOwners || !inRequired || inForbidden)
	{
		if (auto const pParent = pType->FindParentCountry())
		{
			DWORD const bitParent = 1u << pParent->ArrayIndex2;

			if (!inOwners && pItem->InOwners(bitParent))
				inOwners = true;

			if (!inRequired && pItem->InRequiredHouses(bitParent))
				inRequired = true;

			if (!inForbidden && pItem->InForbiddenHouses(bitParent))
				inForbidden = true;
		}
	}

	return inOwners && inRequired && !inForbidden;
}

bool HouseExt::PrerequisitesMet(HouseClass* const pThis, TechnoTypeClass* const pItem, bool skipSecretLabChecks)
{
	if (!pThis || !pItem)
		return false;

	auto pHouseExt = HouseExt::ExtMap.Find(pThis);
	if (!pHouseExt)
		return false;

	auto pItemExt = TechnoTypeExt::ExtMap.Find(pItem);
	if (!pItemExt)
		return false;

	// Check if it appears in Owner=, RequiredHouses= and ForbiddenHouses= (including ParentCountry support)
	if (!HouseExt::IsAvailableToHouse(pThis, pItem))
		return false;

	// If the unit is available after capturing a SecretLab=yes must be evaluated if meets the prerequisite
	if (!skipSecretLabChecks && pItemExt->ConsideredSecretLabTech && !pThis->HasFromSecretLab(pItem))
		return false;

	// Stolen Tech checks (Chrono Commando, Psi Commando, etc.)
	if ((pItem->RequiresStolenAlliedTech && !pThis->Side0TechInfiltrated) ||
		(pItem->RequiresStolenSovietTech && !pThis->Side1TechInfiltrated) ||
		(pItem->RequiresStolenThirdTech && !pThis->Side2TechInfiltrated))
	{
		return false;
	}

	// Prerequisite.RequiredTheaters check
	if (!(pItemExt->PrerequisiteTheaters & (1u << static_cast<int>(ScenarioClass::Instance->Theater))))
		return false;

	// TechLevel check
	if (pThis->TechLevel < pItem->TechLevel)
		return false;

	// BuildLimit checks
	if (pItem->BuildLimit > 0)
	{
		int nInstances = 0;
		for (const auto pTechno : TechnoClass::Array)
		{
			if (pTechno->Owner == pThis
				&& pTechno->GetTechnoType() == pItem
				&& pTechno->IsAlive
				&& pTechno->Health > 0)
			{
				nInstances++;

				if (nInstances >= pItem->BuildLimit)
					return false;
			}
		}
	}
	else if (pItem->BuildLimit == 0)
	{
		return false;
	}

	// Ares Prerequisite.Negative list
	if (pItemExt->Prerequisite_Negative.size() > 0)
	{
		for (int idx : pItemExt->Prerequisite_Negative)
		{
			bool negFound = false;
			if (idx < 0)
			{
				negFound = HouseExt::HasGenericPrerequisite(idx, pThis);
			}
			else
			{
				negFound = pThis->ActiveBuildingTypes.GetItemCount(idx) > 0;
			}

			if (negFound)
				return false;
		}
	}

	// Main prerequisite checks are skipped if a new secret lab object is in process to be unlocked
	if (skipSecretLabChecks)
		return true;

	// PrerequisiteOverride (OR logic: if ANY entry is owned, prerequisites are considered met)
	DynamicVectorClass<int> prerequisiteOverride = pItem->PrerequisiteOverride;
	for (int idx : prerequisiteOverride)
	{
		if (idx < 0)
		{
			if (HouseExt::HasGenericPrerequisite(idx, pThis))
				return true;
		}
		else
		{
			if (pThis->ActiveBuildingTypes.GetItemCount(idx) > 0)
				return true;
		}
	}

	// Main Prerequisite list (AND logic: ALL entries must be satisfied)
	bool prerequisiteMet = true;
	if (pItemExt->Prerequisite.size() > 0)
	{
		for (int idx : pItemExt->Prerequisite)
		{
			bool found = false;

			if (idx < 0)
			{
				// Also check slave miner as alternative for PROC (-6)
				if (idx == -6 &&
					RulesClass::Instance->PrerequisiteProcAlternate != nullptr &&
					pThis->ActiveUnitTypes.GetItemCount(RulesClass::Instance->PrerequisiteProcAlternate->ArrayIndex) > 0)
				{
					found = true;
				}
				else
				{
					found = HouseExt::HasGenericPrerequisite(idx, pThis);
				}
			}
			else
			{
				found = pThis->ActiveBuildingTypes.GetItemCount(idx) > 0;
			}

			if (!found)
			{
				prerequisiteMet = false;
				break;
			}
		}
	}

	// Ares Prerequisite lists (OR logic between lists, AND logic within each list)
	bool prerequisiteListsMet = false;
	if (pItemExt->Prerequisite_Lists.Get() > 0 && !pItemExt->Prerequisite_ListVector.empty())
	{
		for (const auto& list : pItemExt->Prerequisite_ListVector)
		{
			bool listSatisfied = true;
			for (int idx : list)
			{
				bool found = false;

				if (idx < 0)
				{
					if (idx == -6 &&
						RulesClass::Instance->PrerequisiteProcAlternate != nullptr &&
						pThis->ActiveUnitTypes.GetItemCount(RulesClass::Instance->PrerequisiteProcAlternate->ArrayIndex) > 0)
					{
						found = true;
					}
					else
					{
						found = HouseExt::HasGenericPrerequisite(idx, pThis);
					}
				}
				else
				{
					found = pThis->ActiveBuildingTypes.GetItemCount(idx) > 0;
				}

				if (!found)
				{
					listSatisfied = false;
					break;
				}
			}

			if (listSatisfied)
			{
				prerequisiteListsMet = true;
				break;
			}
		}
	}

	bool hasPrereq = !pItemExt->Prerequisite.empty();
	bool hasLists = pItemExt->Prerequisite_Lists.Get() > 0 && !pItemExt->Prerequisite_ListVector.empty();

	if (!hasPrereq && !hasLists)
		return true;

	if (hasPrereq && !hasLists)
		return prerequisiteMet;

	if (!hasPrereq && hasLists)
		return prerequisiteListsMet;

	return prerequisiteMet || prerequisiteListsMet;
}
