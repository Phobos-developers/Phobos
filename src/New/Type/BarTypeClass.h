#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Anchor.h>

class BarTypeClass final : public Enumerable<BarTypeClass>
{
public:
	Nullable<SHPStruct*> Board_Background_File;
	Valueable<bool> Board_Background_ShowWhenNotSelected;
	Nullable<TranslucencyLevel> Board_Background_Translucency;
	Nullable<SHPStruct*> Board_Foreground_File;
	Valueable<bool> Board_Foreground_ShowWhenNotSelected;
	Nullable<TranslucencyLevel> Board_Foreground_Translucency;
	Valueable<Vector2D<int>> Board_Offset;
	Valueable<Vector2D<int>> Bar_Offset;
	Valueable<bool> Sections_DrawBackwards;
	Valueable<SHPStruct*> Sections_Pips_File;
	Valueable<Vector3D<int>> Sections_Pips;
	Valueable<int> Sections_EmptyPip;
	Valueable<int> Sections_Amount;
	Valueable<Vector2D<int>> Sections_PositionDelta;

	BarTypeClass(const char* pTitle = NONE_STR) : Enumerable<BarTypeClass>(pTitle)
		, Board_Background_File { }
		, Board_Background_ShowWhenNotSelected { false }
		, Board_Background_Translucency { }
		, Board_Foreground_File { }
		, Board_Foreground_ShowWhenNotSelected { false }
		, Board_Foreground_Translucency { }
		, Board_Offset { { 0, 0 } }
		, Bar_Offset { { 0, 0 } }
		, Sections_DrawBackwards { false }
		, Sections_Pips_File { FileSystem::PIPS_SHP() }
		, Sections_Pips { { 16, 17, 18 } }
		, Sections_EmptyPip { -1 }
		, Sections_Amount { 17 }
		, Sections_PositionDelta { { 2, 0 } }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
