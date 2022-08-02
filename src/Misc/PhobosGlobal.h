#pragma once
#include <Utilities/TemplateDef.h>
#include <Ext/Techno/Body.h>
#include <set>
#include <map>

class PhobosGlobal
{
public:
	std::map<int, std::vector<TriggerClass*>> RandomTriggerPool;

	bool Save(PhobosStreamWriter& stm);
	bool Load(PhobosStreamReader& stm);

	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);

	static PhobosGlobal* Global();

	PhobosGlobal() :
		RandomTriggerPool()
	{ }

	~PhobosGlobal() = default;

	static void Clear();
	static void PointerGotInvalid(void* ptr, bool bRemoved) { }

	void Reset();

private:
	template <typename T>
	bool Serialize(T& stm);

	static PhobosGlobal GlobalObject;
};
