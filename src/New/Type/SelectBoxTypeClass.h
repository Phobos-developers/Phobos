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
	Valueable<Point2D> Offset;
	TranslucencyLevel Translucency;
	Valueable<AffectedHouse> VisibleToHouses;
	Valueable<bool> VisibleToHouses_Observer;
	Valueable<bool> DrawAboveTechno;
	Valueable<SHPStruct*> GroundShape;
	CustomPalette GroundPalette;
	Nullable<Vector3D<int>> GroundFrames;
	Valueable<Point2D> GroundOffset;
	Valueable<bool> Ground_AlwaysDraw;
	Valueable<bool> GroundLine;
	Damageable<ColorStruct> GroundLineColor;
	Valueable<bool> GroundLine_Dashed;

	SelectBoxTypeClass(const char* pTitle = NONE_STR) : Enumerable<SelectBoxTypeClass>(pTitle)
		, Shape { FileSystem::LoadSHPFile("select.shp") }
		, Palette {}
		, Frames {}
		, Offset { Point2D::Empty }
		, Translucency { 0 }
		, VisibleToHouses { AffectedHouse::All }
		, VisibleToHouses_Observer { true }
		, DrawAboveTechno { true }
		, GroundShape { nullptr }
		, GroundPalette {}
		, GroundFrames {}
		, GroundOffset { Point2D::Empty }
		, Ground_AlwaysDraw { true }
		, GroundLine { false }
		, GroundLineColor { { 0,255,0 } }
		, GroundLine_Dashed { false}
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
