#pragma once

#include <AITriggerTypeClass.h>
#include <Utilities/Container.h>

//this is a 1-based index.
enum class PhobosAIConditionTypes : int
{
	CustomizableAICondition = 1,
};

class AITriggerTypeExt
{
public:
	using base_type = AITriggerTypeClass;

	class ExtData final : public Extension<AITriggerTypeClass>
	{
	public:

		ExtData(AITriggerTypeClass* OwnerObject) : Extension<AITriggerTypeClass>(OwnerObject)
			// Nothing yet
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader & Stm);
		virtual void SaveToStream(PhobosStreamWriter & Stm);
	};

	class ExtContainer final : public Container<AITriggerTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void ProcessCondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int type, int condition);

	static void DisableAITrigger(AITriggerTypeClass* pAITriggerType);
	static void EnableAITrigger(AITriggerTypeClass* pAITriggerType);
	static bool ReadCustomizableAICondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int pickMode, int compareMode, int Number, TechnoTypeClass* TechnoType);
	static void CustomizableAICondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int condition);
	static bool PickValidHouse(HouseClass* pHouse, HouseClass* pThisHouse, int pickMode);

	static ExtContainer ExtMap;

};
