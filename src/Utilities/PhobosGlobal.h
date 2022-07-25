#pragma once
#include <Utilities/TemplateDef.h>
#include <Ext/Techno/Body.h>
#include <set>
#include <map>

class PhobosGlobal
{
public:
	std::multimap<int, TechnoClass*, std::greater<int>> Techno_HugeBar;

	bool Save(PhobosStreamWriter& stm);
	bool Load(PhobosStreamReader& stm);

	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);

	static PhobosGlobal* Global();

	PhobosGlobal() :
		Techno_HugeBar()
	{ }

	~PhobosGlobal() = default;

	static void Clear();
	static void PointerGotInvalid(void* ptr, bool bRemoved);
	void InvalidatePointer(void* ptr);

	void Reset();

private:
	template <typename T>
	bool Serialize(T& stm);

	static PhobosGlobal GlobalObject;
};
