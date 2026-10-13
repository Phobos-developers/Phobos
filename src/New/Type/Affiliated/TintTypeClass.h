#pragma once

#include <Utilities/TemplateDef.h>

class TintTypeClass
{
public:

	Valueable<ColorStruct> Color { ColorStruct { 0,0,0 } };
	Valueable<double> Intensity { 0.0 } ;
	Valueable<AffectedHouse> VisibleToHouses { AffectedHouse::All };
	Valueable<bool> Cumulative { true };
	bool Enabled { false };

	TintTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};
