#pragma once
#include <Utilities/TemplateDef.h>
#include <Ext/Techno/Body.h>
#include <set>
#include <map>

class PhobosGlobal
{
public:
	std::map<int, std::set<TriggerClass*>> RandomTriggerPool;

	bool Save(PhobosStreamWriter& stm);
	bool Load(PhobosStreamReader& stm);

	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);

	static PhobosGlobal* Global();

	PhobosGlobal() :
		RandomTriggerPool()
	{ }

	~PhobosGlobal() = default;

	void InvalidatePointer(void* ptr, bool bRemoved) { };

	static void Clear();
	void Reset();

private:
	template <typename T>
	bool Serialize(T& stm);

	template <typename T>
	static bool SerializeGlobal(T& stm);

	template <typename T>
	static bool ProcessTechnoType(T& stm);

	template <typename T>
	static bool ProcessTechno(T& stm);

	static PhobosGlobal GlobalObject;
};
