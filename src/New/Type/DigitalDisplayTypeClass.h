#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>
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
	Valueable<bool> UseSHP;
	PhobosFixedString<0x20> SHP_SHPFile;
	PhobosFixedString<0x20> SHP_PALFile;
	Valueable<int> SHP_Interval;
	SHPStruct* SHPFile;
	ConvertClass* PALFile;

	DigitalDisplayTypeClass(const char* pTitle = NONE_STR) : Enumerable<DigitalDisplayTypeClass>(pTitle)
		,Text_ColorHigh({ 0, 255, 0 })
		,Text_ColorMid({ 255, 255, 0 })
		,Text_ColorLow({ 255, 0, 0 })
		,Text_Background(false)
		,Offset({ 0, 0 })
		,Offset_WithoutShield()
		,UseSHP(false)
		,SHP_SHPFile("number.shp")
		,SHP_PALFile("")
		,SHP_Interval(8)
	{ }
	
	virtual ~DigitalDisplayTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
