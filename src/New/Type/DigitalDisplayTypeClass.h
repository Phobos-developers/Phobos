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
	PhobosFixedString<0x10> Align;
	PhobosFixedString<0x10> Anchor;
	Valueable<bool> UseSHP;
	PhobosFixedString<0x20> SHP_SHPFile;
	PhobosFixedString<0x20> SHP_PALFile;
	Valueable<Vector2D<int>> SHP_Interval;
	Valueable<Vector2D<int>> SHP_Interval_Building;
	Valueable<bool> Percentage;
	Valueable<bool> HideStrength;
	PhobosFixedString<0x20> InfoType;

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

	enum class Info : int
	{
		Health = 0,
		Shield = 1,
		Ammo = 2,
		MindControl = 3,
		Spawns = 4,
		Passengers = 5,
		Tiberium = 6,
		Experience = 7,
		Occupants = 8,
		GattlingStage = 9
	};

	SHPStruct* SHPFile;
	ConvertClass* PALFile;
	AlignType Alignment;
	AnchorType Anchoring;
	Info InfoClass;

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
		, InfoType("Health")
		, InfoClass(Info::Health)
	{
	}

	virtual ~DigitalDisplayTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	void SetDisplayInfo();
	void Draw(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield);

private:

	void DisplayText(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield);
	void DisplayShape(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield);

	template <typename T>
	void Serialize(T& Stm);
};
