#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Anchor.h>

class BarTypeClass final : public Enumerable<BarTypeClass>
{
public:
	Valueable<DisplayInfoType> InfoType;
	Valueable<int> InfoIndex;
	Valueable<double> Bar_ConditionYellow;
	Valueable<double> Bar_ConditionRed;
	Nullable<SHPStruct*> PipBrd_Background_File;
	Valueable<bool> PipBrd_Background_ShowWhenNotSelected;
	Nullable<TranslucencyLevel> PipBrd_Background_Translucency;
	Nullable<SHPStruct*> PipBrd_Foreground_File;
	Valueable<bool> PipBrd_Foreground_ShowWhenNotSelected;
	Nullable<TranslucencyLevel> PipBrd_Foreground_Translucency;
	Valueable<Vector2D<int>> PipBrd_Offset;
	Valueable<Vector2D<int>> Bar_Offset;
	Valueable<SHPStruct*> Pips_File;
	Valueable<Vector3D<int>> Pips_Frames;
	Valueable<int> Pips_EmptyFrame;
	Valueable<int> Pips_Amount;
	Valueable<Vector2D<int>> Pips_PositionDelta;
	Valueable<bool> Pips_DrawBackwards;
	Valueable<bool> Pips_ChangePerSection;

	BarTypeClass(const char* pTitle = NONE_STR) : Enumerable<BarTypeClass>(pTitle)
		, InfoType { DisplayInfoType::Health }
		, InfoIndex { 0 }
		, Bar_ConditionYellow { 0.66 }
		, Bar_ConditionRed { 0.33 }
		, PipBrd_Background_File { }
		, PipBrd_Background_ShowWhenNotSelected { false }
		, PipBrd_Background_Translucency { }
		, PipBrd_Foreground_File { }
		, PipBrd_Foreground_ShowWhenNotSelected { false }
		, PipBrd_Foreground_Translucency { }
		, PipBrd_Offset { { 0, 0 } }
		, Bar_Offset { { 0, 0 } }
		, Pips_File { FileSystem::PIPS_SHP }
		, Pips_Frames { { 16, 17, 18 } }
		, Pips_EmptyFrame { -1 }
		, Pips_Amount { 17 }
		, Pips_PositionDelta { { 2, 0 } }
		, Pips_DrawBackwards { false }
		, Pips_ChangePerSection { false }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
