#include "body.h"
#include <BuildingClass.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Rules/Body.h>

bool HouseExt::HasBuildingPrerequisite(HouseClass* const pHouse, int const idxBuildingType)
{
	if (!pHouse || idxBuildingType < 0 || idxBuildingType >= BuildingTypeClass::Array.Count)
		return false;

	if (pHouse->ActiveBuildingTypes.GetItemCount(idxBuildingType) > 0)
		return true;

	// Check if this building type is an upgrade attached to an active building
	auto const pType = BuildingTypeClass::Array.GetItem(idxBuildingType);
	if (!pType)
		return false;

	auto const pTypeExt = BuildingTypeExt::Fetch(pType);
	bool const isUpgrade = (pType->PowersUpBuilding[0] != '\0') || (pTypeExt && !pTypeExt->PowersUp_Buildings.empty());

	if (isUpgrade)
	{
		for (auto const pBld : pHouse->Buildings)
		{
			if (!pBld || !pBld->IsAlive || pBld->Health <= 0)
				continue;

			for (auto const pUpgrade : pBld->Upgrades)
			{
				if (pUpgrade == pType)
					return true;
			}
		}
	}

	return false;
}

bool HouseExt::HasGenericPrerequisite(int idx, HouseClass* const pHouse)
{
	if (idx >= 0 || !pHouse)
		return false;

	int absoluteIndex = std::abs(idx);
	auto const pRulesExt = RulesExt::Global();
	if (absoluteIndex >= pRulesExt->GenericPrerequisites.Count)
		return false;

	// Check buildings and nested generic prerequisites matching this generic prerequisite
	DynamicVectorClass<int> const& selectedPrerequisite = pRulesExt->GenericPrerequisites.GetItem(absoluteIndex);
	for (auto idxItem : selectedPrerequisite)
	{
		if (idxItem < 0)
		{
			if (HouseExt::HasGenericPrerequisite(idxItem, pHouse))
				return true;
		}
		else
		{
			if (HouseExt::HasBuildingPrerequisite(pHouse, idxItem))
				return true;
		}
	}

	// Check alternate technos (vehicles, infantry, aircraft, etc.)
	if (absoluteIndex < pRulesExt->GenericPrerequisitesAlternates.Count)
	{
		DynamicVectorClass<TechnoTypeClass*> const& selectedAlternates = pRulesExt->GenericPrerequisitesAlternates.GetItem(absoluteIndex);
		for (auto pTechnoType : selectedAlternates)
		{
			if (pHouse->CountOwnedNow(pTechnoType) > 0)
				return true;
		}
	}

	return false;
}

bool HouseExt::HasPrerequisite(HouseClass* const pHouse, int const idx)
{
	if (idx < 0)
		return HouseExt::HasGenericPrerequisite(idx, pHouse);

	return HouseExt::HasBuildingPrerequisite(pHouse, idx);
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

	auto pHouseExt = HouseExt::Fetch(pThis);
	if (!pHouseExt)
		return false;

	auto pItemExt = TechnoTypeExt::Fetch(pItem);
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
	else if (pItem->BuildLimit < 0)
	{
		if (pThis->CountOwnedEver(pItem) >= -pItem->BuildLimit)
			return false;
	}
	else if (pItem->BuildLimit == 0)
	{
		return false;
	}

	// Phobos BuildLimitGroup check
	if (HouseExt::ReachedBuildLimit(pThis, pItem, true))
		return false;

	// Ares Prerequisite.Negative list
	if (!pItemExt->Prerequisite_Negative.empty())
	{
		for (int idx : pItemExt->Prerequisite_Negative)
		{
			if (HouseExt::HasPrerequisite(pThis, idx))
				return false;
		}
	}

	// Main prerequisite checks are skipped if a new secret lab object is in process to be unlocked
	if (skipSecretLabChecks)
		return true;

	// PrerequisiteOverride (OR logic: if ANY entry is owned, prerequisites are considered met)
	DynamicVectorClass<int> const& prerequisiteOverride = pItem->PrerequisiteOverride;
	for (int idx : prerequisiteOverride)
	{
		if (HouseExt::HasPrerequisite(pThis, idx))
			return true;
	}

	// Main Prerequisite list (AND logic: ALL entries must be satisfied)
	bool prerequisiteMet = true;
	if (!pItemExt->Prerequisite.empty())
	{
		for (int idx : pItemExt->Prerequisite)
		{
			if (!HouseExt::HasPrerequisite(pThis, idx))
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
				if (!HouseExt::HasPrerequisite(pThis, idx))
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
