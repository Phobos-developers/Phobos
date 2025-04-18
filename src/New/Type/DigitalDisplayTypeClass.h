#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Anchor.h>

class DigitalDisplayTypeClass final : public Enumerable<DigitalDisplayTypeClass>
{
public:
	Damageable<ColorStruct> Text_Color;
	Valueable<bool> Text_Background;
	Valueable<Vector2D<int>> Offset;
	Nullable<Vector2D<int>> Offset_ShieldDelta;
	Valueable<TextAlign> Align;
	Anchor AnchorType;
	Valueable<BuildingSelectBracketPosition> AnchorType_Building;
	Valueable<SHPStruct*> Shape;
	CustomPalette Palette;
	Nullable<Vector2D<int>> Shape_Spacing;
	Valueable<bool> Percentage;
	Nullable<bool> HideMaxValue;
	Valueable<bool> VisibleToHouses_Observer;
	Valueable<AffectedHouse> VisibleToHouses;
	Valueable<DisplayInfoType> InfoType;
	Valueable<int> ValueScaleDivisor;

	DigitalDisplayTypeClass(const char* pTitle = NONE_STR) : Enumerable<DigitalDisplayTypeClass>(pTitle)
		, Text_Color({ 0, 255, 0 }, { 255,255,0 }, { 255,0,0 })
		, Text_Background(false)
		, Offset({ 0, 0 })
		, Offset_ShieldDelta()
		, Align(TextAlign::Right)
		, AnchorType(HorizontalPosition::Right, VerticalPosition::Top)
		, AnchorType_Building(BuildingSelectBracketPosition::Top)
		, Shape(nullptr)
		, Palette()
		, Shape_Spacing()
		, Percentage(false)
		, HideMaxValue()
		, VisibleToHouses_Observer(true)
		, VisibleToHouses(AffectedHouse::All)
		, InfoType(DisplayInfoType::Health)
		, ValueScaleDivisor { 1 }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	void Draw(Point2D position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);

private:

	void DisplayText(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);
	void DisplayShape(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);

	template <typename T>
	void Serialize(T& Stm);
};
