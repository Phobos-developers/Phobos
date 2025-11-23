#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>

class SWSignalTypeClass final : public Enumerable<SWSignalTypeClass>
{
public:
	Nullable<int> Range {};
	Nullable<AffectedHouse> Affects {};

	SWSignalTypeClass(const char* pTitle = NONE_STR) : Enumerable<SWSignalTypeClass>(pTitle)
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& stm);
	void SaveToStream(PhobosStreamWriter& stm);

private:
	template <typename T>
	void Serialize(T& stm);
};
