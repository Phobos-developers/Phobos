#include "Body.h"
#include <Utilities/Macro.h>
#include <Ext/Scenario/Body.h>

#include <comutil.h>
#include  <string>
#include <Ext/Rules/Body.h>
#pragma comment(lib, "comsuppw.lib")

template<> const DWORD Extension<AITriggerTypeClass>::Canary = 0x2C2C2C2C;
AITriggerTypeExt::ExtContainer AITriggerTypeExt::ExtMap;

// =============================
// load / save

void AITriggerTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	// Nothing yet
}

void AITriggerTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	// Nothing yet
}

// =============================
// container

AITriggerTypeExt::ExtContainer::ExtContainer() : Container("AITriggerTypeClass") { }
AITriggerTypeExt::ExtContainer::~ExtContainer() = default;

void AITriggerTypeExt::ProcessCondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int type, int condition)
{
	//AITriggerType is disabled by default
	DisableAITrigger(pAITriggerType);
	switch (static_cast<PhobosAIConditionTypes>(type))
	{
	case PhobosAIConditionTypes::CustomizableAICondition:
		AITriggerTypeExt::CustomizableAICondition(pAITriggerType, pHouse, condition);
		break;
	default:
		break;
	}
	return;
}

void AITriggerTypeExt::DisableAITrigger(AITriggerTypeClass* pAITriggerType)
{
	pAITriggerType->ConditionType = AITriggerCondition::AIOwns;
	pAITriggerType->ConditionObject = nullptr;
	return;
}

void AITriggerTypeExt::EnableAITrigger(AITriggerTypeClass* pAITriggerType)
{
	pAITriggerType->ConditionType = AITriggerCondition::Pool;
	pAITriggerType->ConditionObject = nullptr;
	return;
}

bool AITriggerTypeExt::ReadCustomizableAICondition(HouseClass* pHouse ,int pickMode, int compareMode, int Number, TechnoTypeClass* TechnoType)
{
	//0 = pick enemies(except for neutral); 1 = pick allies(except for neutral); 2 = pick self; 3 = pick all(except for neutral); 
	//4 = pick enemy human players; 5 = pick allied human players; 6 = pick all human players; 
	//7 = pick enemy computer players(except for neutral); 8 = pick allied computer players(except for neutral); 9 = pick all computer players(except for neutral);
	//10 = pick neutral; 11 = pick all(including neutral);
	//int pickMode;

	//0 = "<"; 1 = "<="; 2 = "=="; 3 = ">="; 4 = ">"; 5 = "!="; 
	//int compareMode;

	int count = 0;

	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		auto pTechno = TechnoClass::Array->GetItem(i);
		if (pTechno->GetTechnoType() == TechnoType
			&& pTechno->IsAlive
			&& !pTechno->InLimbo
			&& pTechno->IsOnMap
			&& !pTechno->Absorbed
			&& ((!pTechno->Owner->IsAlliedWith(pHouse) && !pTechno->Owner->IsNeutral() && pickMode == 0)
				|| (pTechno->Owner->IsAlliedWith(pHouse) && !pTechno->Owner->IsNeutral() && pickMode == 1)
				|| (pTechno->Owner == pHouse && pickMode == 2)
				|| (!pTechno->Owner->IsNeutral() && pickMode == 3)
				|| (pTechno->Owner->ControlledByHuman() && !pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 4)
				|| (pTechno->Owner->ControlledByHuman() && pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 5)
				|| (pTechno->Owner->ControlledByHuman() && pickMode == 6)
				|| (!pTechno->Owner->ControlledByHuman() && !pTechno->Owner->IsNeutral() && !pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 7)
				|| (!pTechno->Owner->ControlledByHuman() && !pTechno->Owner->IsNeutral() && pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 8)
				|| (!pTechno->Owner->ControlledByHuman() && !pTechno->Owner->IsNeutral() && pickMode == 9)
				|| (pTechno->Owner->IsNeutral() && pickMode == 10)
				|| (pickMode == 11)
				))

		{
			count++;
		}
	}
	if ((count < Number && compareMode == 0)
		|| (count <= Number && compareMode == 1)
		|| (count == Number && compareMode == 2)
		|| (count >= Number && compareMode == 3)
		|| (count > Number && compareMode == 4)
		|| (count != Number && compareMode == 5)
		)
		return true;
	else
		return false;
}

void AITriggerTypeExt::CustomizableAICondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int condition)
{
	auto AIConditionsLists = RulesExt::Global()->AIConditionsLists;

	int essentialRequirementsCount = -1;
	int leastOptionalRequirementsCount = -1;
	int essentialRequirementsMetCount = 0;
	int optionalRequirementsMetCount = 0;

	if (condition < AIConditionsLists.Count)
	{
		auto thisAICondition = AIConditionsLists.GetItem(condition);

		if (thisAICondition.Count < 2)
		{
			pAITriggerType->IsEnabled = false;
			Debug::Log("DEBUG: [AIConditionsList]: Error parsing line [%d].\n", condition);
			return;
		}

		//parse first string
		auto FirstAIConditionString = thisAICondition.GetItem(0);
		char* context = nullptr;
		char* cur[3];
		cur[0] = strtok_s(FirstAIConditionString.data(), Phobos::readDelims, &context);
		int j = 0;
		while (cur[j])
		{
			j++;
			cur[j] = strtok_s(NULL, ",", &context);
		}
		essentialRequirementsCount = atoi(cur[0]);
		leastOptionalRequirementsCount = atoi(cur[1]);

		//parse other strings
		for (int i = 1; i < thisAICondition.Count; i++)
		{
			auto AIConditionString = thisAICondition.GetItem(i);
			int pickMode = -1;
			int compareMode = -1;
			int Number = -1;
			TechnoTypeClass* TechnoType;

			char* cur2[5];
			cur2[0] = strtok_s(AIConditionString.data(), Phobos::readDelims, &context);
			int k = 0;
			while (cur2[k])
			{
				k++;
				cur2[k] = strtok_s(NULL, Phobos::readDelims, &context);
			}
			TechnoTypeClass* buffer;
			if (Parser<TechnoTypeClass*>::TryParse(cur2[3], &buffer))
			{
				pickMode = atoi(cur2[0]);
				compareMode = atoi(cur2[1]);
				Number = atoi(cur2[2]);
				TechnoType = buffer;
			}
			else
			{
				Debug::Log("DEBUG: [AIConditionsList][%d]: Error parsing [%s]\n", condition, cur2[3]);
				Debug::Log("DEBUG: [AIConditionsList]: Error parsing line [%d].\n", condition);
				pAITriggerType->IsEnabled = false;
				return;
			}
			
			if (essentialRequirementsCount > -1
				&& leastOptionalRequirementsCount > -1
				&& essentialRequirementsCount + leastOptionalRequirementsCount < thisAICondition.Count
				&& pickMode >= 0 && pickMode <= 11
				&& compareMode >= 0 && compareMode <= 5
				&& Number >= 0)
			{
				//essential requirements judgement
				if (i <= essentialRequirementsCount)
				{
					if (ReadCustomizableAICondition(pHouse, pickMode, compareMode, Number, TechnoType))
						essentialRequirementsMetCount++;
				}
				//optional requirements judgement
				else
				{
					if (ReadCustomizableAICondition(pHouse, pickMode, compareMode, Number, TechnoType))
						optionalRequirementsMetCount++;
				}
			}
			else
			{
				Debug::Log("DEBUG: [AIConditionsList]: Error parsing line [%d].\n", condition);
				pAITriggerType->IsEnabled = false;
				return;
			}
		}
	}
	else
	{
		//thoroughly disable it
		pAITriggerType->IsEnabled = false;
		Debug::Log("DEBUG: [AIConditionsList]: Conditin number overflew!.\n");
		return;
	}
	if (essentialRequirementsCount == essentialRequirementsMetCount && leastOptionalRequirementsCount <= optionalRequirementsMetCount)
		EnableAITrigger(pAITriggerType);

	return;
}
