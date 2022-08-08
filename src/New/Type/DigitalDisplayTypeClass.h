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
	Nullable<Vector2D<int>> Shape_Interval;
	Valueable<bool> Percentage;
	Valueable<bool> HideMaxValue;
	Valueable<bool> CanSee_Observer;
	Valueable<AffectedHouse> CanSee;
	Valueable<DisplayInfoType> InfoType;

	DigitalDisplayTypeClass(const char* pTitle = NONE_STR) : Enumerable<DigitalDisplayTypeClass>(pTitle)
		, Text_Color({ 0, 255, 0 }, { 255,255,0 }, { 255,0,0 })
		, Text_Background(false)
		, Offset({ 0, 0 })
		, Offset_ShieldDelta()
		, Align(TextAlign::None)
		, AnchorType(HorizontalPosition::Center, VerticalPosition::Top)
		, AnchorType_Building(BuildingSelectBracketPosition::LeftTop)
		, Shape(nullptr)
		, Palette()
		, Shape_Interval()
		, Percentage(false)
		, HideMaxValue(false)
		, CanSee_Observer(true)
		, CanSee(AffectedHouse::All)
		, InfoType(DisplayInfoType::Health)
	{ }

	virtual ~DigitalDisplayTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	void Draw(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield);

private:

	void DisplayText(Point2D& posDraw, int iLength, int iCur, int iMax, bool isBuilding);
	void DisplayShape(Point2D& posDraw, int iLength, int iCur, int iMax, bool isBuilding);

	template <typename T>
	void Serialize(T& Stm);
};
