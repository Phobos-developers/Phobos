#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Enum.h>

class SelectBoxTypeClass final : public Enumerable<SelectBoxTypeClass>
{
public:
	Valueable<SHPStruct*> Shape;
	CustomPalette Palette;
	Nullable<Vector3D<int>> Frames;
	Valueable<bool> Grounded;
	Valueable<Point2D> Offset;
	TranslucencyLevel Translucency;
	Valueable<AffectedHouse> VisibleToHouses;
	Valueable<bool> VisibleToHouses_Observer;
	Valueable<bool> DrawAboveTechno;

	SelectBoxTypeClass(const char* pTitle = NONE_STR) : Enumerable<SelectBoxTypeClass>(pTitle)
		, Shape { FileSystem::LoadSHPFile("select.shp") }
		, Palette {}
		, Frames {}
		, Grounded { false }
		, Offset { Point2D::Empty }
		, Translucency { 0 }
		, VisibleToHouses { AffectedHouse::All }
		, VisibleToHouses_Observer { true }
		, DrawAboveTechno { true }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
