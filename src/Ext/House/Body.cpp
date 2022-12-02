#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

#include <ScenarioClass.h>

//Static init

template<> const DWORD Extension<HouseClass>::Canary = 0x11111111;
HouseExt::ExtContainer HouseExt::ExtMap;


int HouseExt::ActiveHarvesterCount(HouseClass* pThis)
{
	int result = 0;
	for (auto pTechno : *TechnoClass::Array)
	{
		if (pTechno->Owner == pThis)
		{
			auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
			result += pTypeExt->Harvester_Counted && TechnoExt::IsHarvesting(pTechno);
		}
	}
	return result;
}

int HouseExt::TotalHarvesterCount(HouseClass* pThis)
{
	int result = 0;

	for (auto pType : RulesExt::Global()->HarvesterTypes)
		result += pThis->CountOwnedAndPresent(pType);

	return result;
}

int HouseExt::CountOwnedLimbo(HouseClass* pThis, BuildingTypeClass const* const pItem)
{
	auto pHouseExt = HouseExt::ExtMap.Find(pThis);
	return pHouseExt->OwnedLimboBuildingTypes.GetItemCount(pItem->ArrayIndex);
}

// Ares
HouseClass* HouseExt::GetHouseKind(OwnerHouseKind const kind, bool const allowRandom, HouseClass* const pDefault, HouseClass* const pInvoker, HouseClass* const pVictim)
{
	switch (kind) {
	case OwnerHouseKind::Invoker:
	case OwnerHouseKind::Killer:
		return pInvoker ? pInvoker : pDefault;
	case OwnerHouseKind::Victim:
		return pVictim ? pVictim : pDefault;
	case OwnerHouseKind::Civilian:
		return HouseClass::FindCivilianSide();
	case OwnerHouseKind::Special:
		return HouseClass::FindSpecial();
	case OwnerHouseKind::Neutral:
		return HouseClass::FindNeutral();
	case OwnerHouseKind::Random:
		if (allowRandom)
		{
			auto& Random = ScenarioClass::Instance->Random;
			return HouseClass::Array->GetItem(
				Random.RandomRanged(0, HouseClass::Array->Count - 1));
		}
		else
		{
			return pDefault;
		}
	case OwnerHouseKind::Default:
	default:
		return pDefault;
	}
}

bool HouseExt::PrerequisitesMet(HouseClass* const pThis, TechnoTypeClass* const pItem, const DynamicVectorClass<BuildingTypeClass*> ownedBuildingTypes)
{
	if (!pThis || !pItem)
		return false;

	auto pHouseExt = HouseExt::ExtMap.Find(pThis);
	if (!pHouseExt)
		return false;

	auto pItemExt = TechnoTypeExt::ExtMap.Find(pItem);
	if (!pItemExt)
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

	for (auto pTechno : *TechnoClass::Array)
	{
		if (pTechno->Owner == pThis && pTechno->GetTechnoType() == pItem && pTechno->IsAlive && pTechno->Health > 0)
			nInstances++;
	}

	if (nInstances >= pItem->BuildLimit)
		return false;

	bool prerequisiteNegativeMet = false; // Only one coincidence is needed

	// Ares Prerequisite.Negative list.
	if (pItemExt->Prerequisite_Negative.size() > 0)
	{
		for (int idx : pItemExt->Prerequisite_Negative)
		{
			if (prerequisiteNegativeMet)
				return false;

			if (idx < 0) // Is even possible this case? I have to check it
			{
				// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
				prerequisiteNegativeMet = HouseExt::HasGenericPrerequisite(idx, ownedBuildingTypes);
			}
			else
			{
				for (auto pObject : ownedBuildingTypes)
				{
					if (prerequisiteNegativeMet)
						break;

					if (idx == pObject->ArrayIndex)
						prerequisiteNegativeMet = true;
				}
			}
		}
	}

	//ValueableVector<int> prerequisite = pItemExt->Prerequisite;
	DynamicVectorClass<int> prerequisiteOverride = pItem->PrerequisiteOverride;

	//UnitTypeClass* prerequisiteProcAlternate = RulesClass::Instance->PrerequisiteProcAlternate;

	bool prerequisiteMet = false; // All in this list must appear in ownedBuildingTypes
	bool prerequisiteOverrideMet = false; // Only one coincidence is needed

	if (prerequisiteOverride.Count > 0)
	{
		for (int idx : prerequisiteOverride)
		{
			if (prerequisiteOverrideMet)
				break;

			if (idx < 0)
			{
				// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
				prerequisiteOverrideMet = HouseExt::HasGenericPrerequisite(idx, ownedBuildingTypes);
			}
			else
			{
				for (auto pObject : ownedBuildingTypes)
				{
					if (prerequisiteOverrideMet)
						break;

					if (idx == pObject->ArrayIndex)
						prerequisiteOverrideMet = true;
				}
			}
		}
	}

	if (pItemExt->Prerequisite.size() > 0)
	{
		bool found = false;
		//if (pItem) Debug::Log("[%s] prereq check????\n", pItem->ID);
		for (int idx : pItemExt->Prerequisite)
		{
			found = false;
			//Debug::Log("\tidx: %d:\n",idx);
			if (idx < 0)
			{
				// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
				found = HouseExt::HasGenericPrerequisite(idx, ownedBuildingTypes);
			}
			else
			{
				//Debug::Log("\t[%s] (idx: %d):\n", TechnoTypeClass::Array->GetItem(idx)->ID, idx);
				for (auto pObject : ownedBuildingTypes)
				{
					if (found)
						break;

					if (idx == pObject->ArrayIndex)
						found = true;

					//Debug::Log("\t\t[%s] (idx: %d) Found? %d\n", pObject->ID, pObject->ArrayIndex, found);
				}
			}

			if (!found)
				break;
		}

		prerequisiteMet = found;
	}
	else
	{
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
					found = HouseExt::HasGenericPrerequisite(idx, ownedBuildingTypes);
				}
				else
				{
					found = false;

					for (auto pObject : ownedBuildingTypes)
					{
						if (idx == pObject->ArrayIndex)
							found = true;

						if (found)
							break;
					}
				}

				if (!found)
					break;
			}
		}

		prerequisiteListsMet = found;
	}

	//Debug::Log("prerequisiteMet: %d, prerequisiteListsMet: %d, prerequisiteOverrideMet: %d\n", prerequisiteMet, prerequisiteListsMet, prerequisiteOverrideMet);
	return prerequisiteMet || prerequisiteListsMet || prerequisiteOverrideMet;
}

bool HouseExt::HasGenericPrerequisite(int idx, const DynamicVectorClass<BuildingTypeClass*> ownedBuildingTypes)
{
	if (idx >= 0)
		return false;

	DynamicVectorClass<int> selectedPrerequisite = RulesExt::Global()->GenericPrerequisites.GetItem(std::abs(idx));

	/*Debug::Log("DEBUG: SELECTED DEFAULT PREREQUISITES: %s [%d]\n", RulesExt::Global()->GenericPrerequisitesNames.GetItem(std::abs(idx)), idx);
	for (int i : selectedPrerequisite)
	{
		auto aaa = TechnoTypeClass::Array->GetItem(i);
		Debug::Log("[%s] -> %d\n", aaa->ID, i);
	}
	Debug::Log("\n");
	*/
	if (selectedPrerequisite.Count == 0)
		return false;

	bool found = false;

	for (auto idxItem : selectedPrerequisite)
	{
		if (found)
			break;

		for (auto pObject : ownedBuildingTypes)
		{
			if (found)
				break;

			if (idxItem == pObject->ArrayIndex)
				found = true;
		}
	}
	//Debug::Log("idx: %d -> found: %d\n", idx, found);

	return found;
}

int HouseExt::FindGenericPrerequisite(const char* id)
{
	if (TechnoTypeClass::FindIndex(id) >= 0)
		return 0;

	if (RulesExt::Global()->GenericPrerequisitesNames.Count == 0)
		RulesExt::FillDefaultPrerequisites();

	int i = 0;
	for (auto str : RulesExt::Global()->GenericPrerequisitesNames)
	{
		if (_strcmpi(id, str) == 0)
			return (-1 * i);

		++i;
	}

	return 0;
}

void HouseExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	const char* pSection = this->OwnerObject()->PlainName;

	INI_EX exINI(pINI);

	ValueableVector<bool> readBaseNodeRepairInfo;
	readBaseNodeRepairInfo.Read(exINI, pSection, "RepairBaseNodes");
	size_t nWritten = readBaseNodeRepairInfo.size();
	if (nWritten > 0)
	{
		for (size_t i = 0; i < 3; i++)
			this->RepairBaseNodes[i] = readBaseNodeRepairInfo[i < nWritten ? i : nWritten - 1];
	}

	Valueable<int> readNewTeamsSelector_MergeUnclassifiedCategoryWith;
	readNewTeamsSelector_MergeUnclassifiedCategoryWith.Read(exINI, pSection, "NewTeamsSelector.MergeUnclassifiedCategoryWith");
	this->NewTeamsSelector_MergeUnclassifiedCategoryWith = readNewTeamsSelector_MergeUnclassifiedCategoryWith.Get();

	Valueable<double> readNewTeamsSelector_UnclassifiedCategoryPercentage;
	readNewTeamsSelector_UnclassifiedCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.UnclassifiedCategoryPercentage");
	this->NewTeamsSelector_UnclassifiedCategoryPercentage = readNewTeamsSelector_UnclassifiedCategoryPercentage.Get();

	Valueable<double> readNewTeamsSelector_GroundCategoryPercentage;
	readNewTeamsSelector_GroundCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.GroundCategoryPercentage");
	this->NewTeamsSelector_GroundCategoryPercentage = readNewTeamsSelector_GroundCategoryPercentage.Get();

	Valueable<double> readNewTeamsSelector_NavalCategoryPercentage;
	readNewTeamsSelector_NavalCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.NavalCategoryPercentage");
	this->NewTeamsSelector_NavalCategoryPercentage = readNewTeamsSelector_NavalCategoryPercentage.Get();

	Valueable<double> readNewTeamsSelector_AirCategoryPercentage;
	readNewTeamsSelector_AirCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.AirCategoryPercentage");
	this->NewTeamsSelector_AirCategoryPercentage = readNewTeamsSelector_AirCategoryPercentage.Get();
}


// =============================
// load / save

template <typename T>
void HouseExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->BuildingCounter)
		.Process(this->Factory_BuildingType)
		.Process(this->Factory_InfantryType)
		.Process(this->Factory_VehicleType)
		.Process(this->Factory_NavyType)
		.Process(this->Factory_AircraftType)
		.Process(this->RepairBaseNodes)
		.Process(this->NewTeamsSelector_MergeUnclassifiedCategoryWith)
		.Process(this->NewTeamsSelector_UnclassifiedCategoryPercentage)
		.Process(this->NewTeamsSelector_GroundCategoryPercentage)
		.Process(this->NewTeamsSelector_NavalCategoryPercentage)
		.Process(this->NewTeamsSelector_AirCategoryPercentage)
		;
}

void HouseExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<HouseClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void HouseExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<HouseClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool HouseExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool HouseExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

HouseExt::ExtContainer::ExtContainer() : Container("HouseClass") {
}

HouseExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);

	HouseExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x4F7371, HouseClass_DTOR, 0x6)
{
	GET(HouseClass*, pItem, ESI);

	HouseExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x504080, HouseClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x503040, HouseClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x504069, HouseClass_Load_Suffix, 0x7)
{
	HouseExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x5046DE, HouseClass_Save_Suffix, 0x7)
{
	HouseExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x50114D, HouseClass_InitFromINI, 0x5)
{
	GET(HouseClass* const, pThis, EBX);
	GET(CCINIClass* const, pINI, ESI);

	HouseExt::ExtMap.LoadFromINI(pThis, pINI);

	return 0;
}
