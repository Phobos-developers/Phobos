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

	Valueable<PartialVector3D<int>> PipBrd {};
	Nullable<SHPStruct*> PipBrdShape {};
	CustomPalette PipBrdPalette {};
	Valueable<int> PipBrdXOffset { 0 };

	Valueable<bool> IsAnimated { false };
	Valueable<bool> IsAnimated_Reverse { false };
	Valueable<int> XOffset { 0 };

	Valueable<DisplayInfoType> InfoType { DisplayInfoType::Health };
	Valueable<int> InfoIndex { 0 };

	HealthBarTypeClass(const char* pTitle = NONE_STR) : Enumerable<HealthBarTypeClass>(pTitle)
	{ }

	Vector3D<int> GetPipBrd(int defaultValue = 0) const
	{
		const auto& pipBrd = this->PipBrd.Get();

		switch (pipBrd.ValueCount)
		{
		case 1:
			return Vector3D<int>(pipBrd.X, pipBrd.X, pipBrd.X);

		case 3:
			return Vector3D<int>(pipBrd.X, pipBrd.Y, pipBrd.Z);

		default:
			break;
		}

		return Vector3D<int>(defaultValue, defaultValue, defaultValue);
	}

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
