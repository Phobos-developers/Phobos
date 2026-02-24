#include "Body.h"

AITriggerTypeExt::ExtContainer AITriggerTypeExt::ExtMap;

int AITriggerTypeExt::CheckConditions(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	int condition = (int)pThis->ConditionType;

	if (condition < -1) // Invalid, bail out early.
		return 0;
	else if (condition < (int)PhobosAITriggerConditions::NumberOfTechBuildingsExist) // Not Phobos condition
		return -1;

	bool success = false;

	switch ((PhobosAITriggerConditions)condition)
	{
	case PhobosAITriggerConditions::NumberOfTechBuildingsExist:
		success = AITriggerTypeExt::NumberOfTechBuildingsExist(pThis, pOwner);
		break;
	case PhobosAITriggerConditions::NumberOfBridgeRepairHutsExist:
		success = AITriggerTypeExt::NumberOfBridgeRepairHutsExist(pThis);
		break;
	}

	return success;
}

bool AITriggerTypeExt::GetComparatorResult(int operand1, AITriggerConditionComparatorType operatorType, int operand2)
{
	switch (operatorType)
	{
	case AITriggerConditionComparatorType::Less:
		return operand1 < operand2;
		break;
	case AITriggerConditionComparatorType::LessOrEqual:
		return operand1 <= operand2;
		break;
	case AITriggerConditionComparatorType::Equal:
		return operand1 == operand2;
		break;
	case AITriggerConditionComparatorType::GreaterOrEqual:
		return operand1 >= operand2;
		break;
	case AITriggerConditionComparatorType::Greater:
		return operand1 > operand2;
		break;
	case AITriggerConditionComparatorType::NotEqual:
		return operand1 != operand2;
		break;
	default:
		return false;
		break;
	}
}

bool AITriggerTypeExt::NumberOfTechBuildingsExist(AITriggerTypeClass* pThis, HouseClass* pOwner)
{
	int count = 0;

	for (auto const pHouse : HouseClass::Array)
	{
		if (pHouse->IsAlliedWith(pOwner))
			continue;

		// Could possibly be optimized with bespoke tracking but
		// it didn't seem to make much of a difference in testing.
		for (auto const pBuilding : pHouse->Buildings)
		{
			if (!pBuilding->IsAlive || pBuilding->InLimbo)
				continue;

			auto const pType = pBuilding->Type;

			if (pType->NeedsEngineer && pType->Capturable)
				count++;
		}
	}

	return AITriggerTypeExt::GetComparatorResult(count, pThis->Conditions[0].ComparatorType, pThis->Conditions[0].ComparatorOperand);
}

bool AITriggerTypeExt::NumberOfBridgeRepairHutsExist(AITriggerTypeClass* pThis)
{
	int count = 0;
	auto const pHouse = HouseClass::FindCivilianSide();

	for (auto const pBuilding : pHouse->Buildings)
	{
		if (!pBuilding->IsAlive || pBuilding->InLimbo)
			continue;

		auto const pType = pBuilding->Type;

		if (pType->BridgeRepairHut && MapClass::Instance.IsLinkedBridgeDestroyed(pBuilding->GetMapCoords()))
			count++;
	}

	return AITriggerTypeExt::GetComparatorResult(count, pThis->Conditions[0].ComparatorType, pThis->Conditions[0].ComparatorOperand);
}

// =============================
// load / save

template <typename T>
void AITriggerTypeExt::Serialize(T& Stm)
{
	//Stm;
}

void AITriggerTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	AbstractTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AITriggerTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	AbstractTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// container

AITriggerTypeExt::ExtContainer::ExtContainer() : Container("AITriggerTypeClass") {}
AITriggerTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
