#pragma once

#include <Utilities/Anchor.h>
#include <Utilities/Constructs.h>

class HugeBar
{
private:

	static std::vector<TechnoClass*> Technos;

public:

	static void InitializeHugeBar(TechnoClass* pTechno);
	static void ProcessHugeBar();

	Valueable<double> HugeBar_RectWidthPercentage;
	Valueable<Point2D> HugeBar_RectWH;
	Damageable<ColorStruct> HugeBar_Pips_Color1;
	Damageable<ColorStruct> HugeBar_Pips_Color2;

	Valueable<SHPStruct*> HugeBar_Shape;
	Valueable<SHPStruct*> HugeBar_Pips_Shape;
	CustomPalette HugeBar_Palette;
	CustomPalette HugeBar_Pips_Palette;
	Damageable<int> HugeBar_Frame;
	Damageable<int> HugeBar_Pips_Frame;
	Valueable<int> HugeBar_Pips_Spacing;

	Valueable<Point2D> HugeBar_Offset;
	Nullable<Point2D> HugeBar_Pips_Offset;
	Valueable<int> HugeBar_Pips_Num;

	Damageable<ColorStruct> Value_Text_Color;

	Valueable<SHPStruct*> Value_Shape;
	CustomPalette Value_Palette;
	Valueable<int> Value_Num_BaseFrame;
	Valueable<int> Value_Sign_BaseFrame;
	Valueable<int> Value_Shape_Spacing;

	Valueable<bool> DisplayValue;
	Valueable<bool> Value_Percentage;
	Valueable<Point2D> Value_Offset;
	Anchor Anchor;
	DisplayInfoType InfoType;

	Valueable<bool> VisibleToHouses_Observer;
	Valueable<AffectedHouse> VisibleToHouses;

	HugeBar() = default;
	HugeBar(DisplayInfoType infoType);

	void LoadFromINI(CCINIClass* pINI);

	void DrawHugeBar(int iCurrent, int iMax);
	void HugeBar_DrawValue(Point2D& posDraw, int iCurrent, int iMax);

	static void InvalidatePointer(void* ptr, bool removed);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);

private:

	template <typename T>
	bool Serialize(T& stm);
};
