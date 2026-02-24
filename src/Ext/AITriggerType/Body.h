#pragma once
#include <AITriggerTypeClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>

// Vanilla game ones range from -1 to 7, see AITriggerCondition in GeneralDefinitions.h in YRpp.
enum class PhobosAITriggerConditions : unsigned int
{
	NumberOfTechBuildingsExist = 8,
	NumberOfBridgeRepairHutsExist = 9,
};

class AITriggerTypeExt final : public AbstractTypeExt
{
public:
	using base_type = AITriggerTypeClass;

	static constexpr DWORD Canary = 0x9B9B9B9B;

public:
	// Nothing yet

	AITriggerTypeClass* OwnerObject() const
	{
		return static_cast<AITriggerTypeClass*>(this->GetAttachedObject());
	}

	explicit AITriggerTypeExt(AITriggerTypeClass* const OwnerObject) : AbstractTypeExt(OwnerObject)
		// Nothing yet
	{}

	virtual ~AITriggerTypeExt() = default;

	static int CheckConditions(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy);
	static bool GetComparatorResult(int operand1, AITriggerConditionComparatorType operatorType, int operand2);
	static bool NumberOfTechBuildingsExist(AITriggerTypeClass* pThis, HouseClass* pOwner);
	static bool NumberOfBridgeRepairHutsExist(AITriggerTypeClass* pThis);

public:
	class ExtContainer final : public Container<AITriggerTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static AITriggerTypeExt* Fetch(const AITriggerTypeClass* pThis)
	{
		return AbstractExt::Fetch<AITriggerTypeExt>(pThis);
	}

	static AITriggerTypeExt* TryFetch(const AITriggerTypeClass* pThis)
	{
		return AbstractExt::TryFetch<AITriggerTypeExt>(pThis);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
