#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Anchor.h>

class BarTypeClass final : public Enumerable<BarTypeClass>
{
public:
	Nullable<SHPStruct*> BoardBG_File;
	Valueable<bool> BoardBG_ShowWhenNotSelected;
	Nullable<TranslucencyLevel> BoardBG_Translucency;
	Nullable<SHPStruct*> BoardFG_File;
	Valueable<bool> BoardFG_ShowWhenNotSelected;
	Nullable<TranslucencyLevel> BoardFG_Translucency;
	Valueable<Vector2D<int>> Board_Offset;
	Valueable<Vector2D<int>> Bar_Offset;
	Valueable<bool> Sections_DrawBackwards;
	Valueable<Vector3D<int>> Sections_Pips;
	Valueable<int> Sections_EmptyPip;
	Valueable<int> Sections_Amount;
	Valueable<Vector2D<int>> Sections_PositionDelta;

	BarTypeClass(const char* pTitle = NONE_STR) : Enumerable<BarTypeClass>(pTitle)
		, BoardBG_File { }
		, BoardBG_ShowWhenNotSelected { false }
		, BoardBG_Translucency { }
		, BoardFG_File { }
		, BoardFG_ShowWhenNotSelected { false }
		, BoardFG_Translucency { }
		, Board_Offset { { 0,0 } }
		, Bar_Offset { { 0,0 } }
		, Sections_DrawBackwards { false }
		, Sections_Pips { { 16,17,18 } }
		, Sections_EmptyPip { -1 }
		, Sections_Amount { 17 }
		, Sections_PositionDelta { { 2,0 } }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
