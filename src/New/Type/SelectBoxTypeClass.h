#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Enum.h>

class SelectBoxTypeClass final : public Enumerable<SelectBoxTypeClass>
{
public:
	Valueable<SHPStruct*> Shape;
	CustomPalette Palette;
	Nullable<Vector3D<int>> Frame;
	Valueable<bool> Grounded;
	Valueable<Point2D> Offset;
	TranslucencyLevel Translucency;
	Valueable<AffectedHouse> Show;
	Valueable<bool> ShowObserver;

	SelectBoxTypeClass(const char* pTitle = NONE_STR) : Enumerable<SelectBoxTypeClass>(pTitle)
		, Shape { nullptr }
		, Palette {}
		, Frame {}
		, Grounded { false }
		, Offset { Point2D::Empty }
		, Translucency { 0 }
		, Show { AffectedHouse::All }
		, ShowObserver { true }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
