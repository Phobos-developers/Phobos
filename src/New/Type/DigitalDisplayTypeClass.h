#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>

class DigitalDisplayTypeClass final : public Enumerable<DigitalDisplayTypeClass>
{
public:
	Valueable<ColorStruct> Text_ColorHigh;
	Valueable<ColorStruct> Text_ColorMid;
	Valueable<ColorStruct> Text_ColorLow;
	Valueable<bool> Text_Background;
	Valueable<Vector2D<int>> Offset;
	Nullable<Vector2D<int>> Offset_WithoutShield;
	Valueable<TextAlign> Align;
	PhobosFixedString<0x10> Anchor;
	Valueable<bool> UseShape;
	Valueable<SHPStruct*> Shape;
	PhobosFixedString<0x20> Palette;
	Valueable<Vector2D<int>> Shape_Interval;
	Valueable<Vector2D<int>> Shape_Interval_Building;
	Valueable<bool> Percentage;
	Valueable<bool> HideStrength;
	Valueable<DisplayInfoType> InfoType;

	enum class AnchorType : int
	{
		Left = 0,
		Right = 1,
		Top = 2,
		TopLeft = 2,
		TopRight = 3
	};

	SHPStruct* SHPFile;
	ConvertClass* PALFile;
	AnchorType Anchoring;

	DigitalDisplayTypeClass(const char* pTitle = NONE_STR) : Enumerable<DigitalDisplayTypeClass>(pTitle)
		, Text_ColorHigh({ 0, 255, 0 })
		, Text_ColorMid({ 255, 255, 0 })
		, Text_ColorLow({ 255, 0, 0 })
		, Text_Background(false)
		, Offset({ 0, 0 })
		, Offset_WithoutShield()
		, Align(TextAlign::None)
		, Anchor("")
		, UseShape(false)
		, Shape(nullptr)
		, Palette("")
		, Shape_Interval({ 8, 0 })
		, Shape_Interval_Building({ 8, 4 })
		, Anchoring(AnchorType::TopLeft)
		, Percentage(false)
		, HideStrength(false)
		, InfoType(DisplayInfoType::Health)
	{ }

	virtual ~DigitalDisplayTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	void Draw(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield);

private:

	void DisplayText(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield);
	void DisplayShape(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield);

	template <typename T>
	void Serialize(T& Stm);
};
