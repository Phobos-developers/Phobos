#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>

class DigitalDisplayTypeClass final : public Enumerable<DigitalDisplayTypeClass>
{
public:
	//static DigitalDisplayTypeClass BuildingsDefaultHP;
	//static DigitalDisplayTypeClass BuildingsDefaultSP;
	//static DigitalDisplayTypeClass UnitsDefaultHP;
	//static DigitalDisplayTypeClass UnitsDefaultSP;
	Valueable<ColorStruct> Text_ColorHigh;
	Valueable<ColorStruct> Text_ColorMid;
	Valueable<ColorStruct> Text_ColorLow;
	Valueable<bool> Text_Background;
	Valueable<Vector2D<int>> Offset;
	Nullable<Vector2D<int>> Offset_WithoutShield;
	PhobosFixedString<0x10> Align;
	PhobosFixedString<0x10> Anchor;
	Valueable<bool> UseSHP;
	PhobosFixedString<0x20> SHP_SHPFile;
	PhobosFixedString<0x20> SHP_PALFile;
	Valueable<Vector2D<int>> SHP_Interval;
	Valueable<Vector2D<int>> SHP_Interval_Building;
	Valueable<bool> Percentage;
	Valueable<bool> HideStrength;
	
	enum class AlignType : int
	{
		Default = 0,
		Left = 1,
		Center = 2,
		Right = 3
	};

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
	AlignType Alignment;
	AnchorType Anchoring;

	DigitalDisplayTypeClass(const char* pTitle = NONE_STR) : Enumerable<DigitalDisplayTypeClass>(pTitle)
		, Text_ColorHigh({ 0, 255, 0 })
		, Text_ColorMid({ 255, 255, 0 })
		, Text_ColorLow({ 255, 0, 0 })
		, Text_Background(false)
		, Offset({ 0, 0 })
		, Offset_WithoutShield()
		, Align("")
		, Anchor("")
		, UseSHP(false)
		, SHP_SHPFile("number.shp")
		, SHP_PALFile("")
		, SHP_Interval({ 8, 0 })
		, SHP_Interval_Building({ 8, 4 })
		, Alignment(AlignType::Default)
		, Anchoring(AnchorType::TopLeft)
		, Percentage(false)
		, HideStrength(false)
	{ }
	
	virtual ~DigitalDisplayTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static void DigitalDisplay(TechnoClass* pThis, Point2D* pLocation, bool Shield);
	static void DigitalDisplayText(TechnoClass* pThis, DigitalDisplayTypeClass* pDisplayType, Point2D Pos, bool Shield);
	static void DigitalDisplaySHP(TechnoClass* pThis, DigitalDisplayTypeClass* pDisplayType, Point2D Pos, bool Shield);

private:
	template <typename T>
	void Serialize(T& Stm);
};
