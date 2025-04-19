#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include "DigitalDisplayTypeClass.h"

class HealthBarTypeClass final : public Enumerable<HealthBarTypeClass>
{
public:
	Nullable<Vector3D<int>> Pips {};
	Nullable<Vector3D<int>> Pips_Building {};
	Nullable<int> PipsEmpty {};
	Valueable<Point2D> PipsInterval { { 2,0 } };
	Valueable<Point2D> PipsInterval_Building { { -4,2 } };
	Nullable<int> PipsLength {};
	Valueable<SHPStruct*> PipsShape { FileSystem::PIPS_SHP };
	CustomPalette PipsPalette {};

	Nullable<int> PipBrd {};
	Nullable<SHPStruct*> PipBrdShape {};
	CustomPalette PipBrdPalette {};
	Valueable<int> PipBrdXOffset { 0 };

	Valueable<int> XOffset { 0 };

	HealthBarTypeClass(const char* pTitle = NONE_STR) : Enumerable<HealthBarTypeClass>(pTitle)
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
