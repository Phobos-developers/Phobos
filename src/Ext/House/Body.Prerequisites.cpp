#include "body.h"

bool HouseExt::PrerequisitesMet(HouseClass* const pThis, TechnoTypeClass* const pItem, std::map<BuildingTypeClass*, int> ownedBuildings, bool skipSecretLabChecks)
{
	if (!pThis || !pItem)
		return false;

	auto pHouseExt = HouseExt::ExtMap.Find(pThis);
	if (!pHouseExt)
		return false;

	auto pItemExt = TechnoTypeExt::ExtMap.Find(pItem);
	if (!pItemExt)
		return false;

	// If the unit is available after capturing a SecretLab=yes must be evaluated if meets the prerequisite
	if (!skipSecretLabChecks && pItemExt->ConsideredSecretLabTech && !pThis->HasFromSecretLab(pItem))
		return false;

	// Check if it appears in Owner=, RequiredHouses= and ForbiddenHouses=
	// Note: if RequiredHouses = tag doesn't exist InRequiredHouses() always returns TRUE
	if (!pThis->InOwners(pItem) || !pThis->InRequiredHouses(pItem) || pThis->InForbiddenHouses(pItem))
		return false;

	// Prerequisite.RequiredTheaters check
	if (pItemExt->Prerequisite_RequiredTheaters.size() > 0)
	{
		int currentTheaterIndex = (int)ScenarioClass::Instance->Theater;
		if (pItemExt->Prerequisite_RequiredTheaters.IndexOf(currentTheaterIndex) < 0)
			return false;
	}

	// TechLevel check
	if (pThis->TechLevel < pItem->TechLevel)
		return false;

	// BuildLimit checks
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

	if (pItem->BuildLimit < 1)
		return false;

	bool prerequisiteNegativeMet = false; // Only one coincidence is needed

	// Ares Prerequisite.Negative list
	if (pItemExt->Prerequisite_Negative.size() > 0)
	{
		for (int idx : pItemExt->Prerequisite_Negative)
		{
			if (idx < 0) // Can be used generic prerequisites in this Ares tag? I have to investigate it but for now we support it...
			{
				// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
				prerequisiteNegativeMet = HouseExt::HasGenericPrerequisite(idx, ownedBuildings);
			}
			else
			{
				if (ownedBuildings.count(BuildingTypeClass::Array.GetItem(idx)) > 0)
					prerequisiteNegativeMet = true;
			}

			if (prerequisiteNegativeMet)
				return false;
		}
	}

	// Main prerequisite checks are skipped if a new secret lab object is in process to be unlocked
	if (skipSecretLabChecks)
		return true;

	DynamicVectorClass<int> prerequisiteOverride = pItem->PrerequisiteOverride;

	bool prerequisiteMet = false; // All buildings must appear in the buildings list owner by the house
	bool prerequisiteOverrideMet = false; // This tag uses an OR comparator: Only one coincidence is needed

	if (prerequisiteOverride.Count > 0)
	{
		for (int idx : prerequisiteOverride)
		{
			if (prerequisiteOverrideMet)
				break;

			if (idx < 0)
			{
				// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
				prerequisiteOverrideMet = HouseExt::HasGenericPrerequisite(idx, ownedBuildings);
			}
			else
			{
				if (ownedBuildings.count(BuildingTypeClass::Array.GetItem(idx)) > 0)
					prerequisiteOverrideMet = true;
			}
		}
	}

	if (pItemExt->Prerequisite.size() > 0)
	{
		bool found = false;

		for (int idx : pItemExt->Prerequisite)
		{
			found = false;

			if (idx < 0)
			{
				// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
				found = HouseExt::HasGenericPrerequisite(idx, ownedBuildings);
			}
			else
			{
				auto debugType = BuildingTypeClass::Array.GetItem(idx);
				if (ownedBuildings.count(BuildingTypeClass::Array.GetItem(idx)) > 0)
					found = true;
			}

			if (!found)
				break;
		}

		prerequisiteMet = found;
	}
	else
	{
		// No prerequisites list means that always is buildable
		prerequisiteMet = true;
	}

	bool prerequisiteListsMet = false;

	// Ares Prerequisite lists
	if (pItemExt->Prerequisite_Lists.Get() > 0)
	{
		bool found = false;

		for (auto list : pItemExt->Prerequisite_ListVector)
		{
			if (found)
				break;

			for (int idx : list)
			{
				if (idx < 0)
				{
					// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
					found = HouseExt::HasGenericPrerequisite(idx, ownedBuildings);
				}
				else
				{
					found = false;

					if (ownedBuildings.count(BuildingTypeClass::Array.GetItem(idx)) > 0)
						found = true;
				}

				if (!found)
					break;
			}
		}

		prerequisiteListsMet = found;
	}

	return prerequisiteMet || prerequisiteListsMet || prerequisiteOverrideMet;
}

bool HouseExt::HasGenericPrerequisite(int idx, std::map<BuildingTypeClass*, int> ownedBuildings)

{
	if (idx >= 0)
		return false;

	DynamicVectorClass<int> selectedPrerequisite = RulesExt::Global()->GenericPrerequisites.GetItem(std::abs(idx));
	const char* selectedPrerequisiteName = RulesExt::Global()->GenericPrerequisitesNames[std::abs(idx)];// Only used for easy debug

	if (selectedPrerequisite.Count == 0)
		return false;

	bool found = false;

	for (auto idxItem : selectedPrerequisite)
	{
		if (found)
			break;

		auto debugType = BuildingTypeClass::Array.GetItem(idxItem);
		if (ownedBuildings.count(BuildingTypeClass::Array.GetItem(idxItem)) > 0)
			found = true;
	}

	return found;
}

int HouseExt::FindGenericPrerequisite(const char* id)
{
	if (BuildingTypeClass::FindIndex(id) >= 0)
		return INT32_MAX;

	if (RulesExt::Global()->GenericPrerequisitesNames.Count == 0)
		RulesExt::FillDefaultPrerequisites(); // needed!

	int i = 0;
	for (auto str : RulesExt::Global()->GenericPrerequisitesNames)
	{
		if (_strcmpi(id, str) == 0)
			return i;

		--i;
	}

	return INT32_MAX; // Error
}
