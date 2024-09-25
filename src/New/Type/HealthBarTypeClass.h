#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Anchor.h>

class HealthBarTypeClass final : public Enumerable<HealthBarTypeClass>
{
public:
	Nullable<SHPStruct*> Frame_Background;
	Valueable<bool> Frame_Background_ShowWhenNotSelected;
	Nullable<TranslucencyLevel> Frame_Background_Translucency;
	Nullable<SHPStruct*> Frame_Foreground;
	Valueable<bool> Frame_Foreground_ShowWhenNotSelected;
	Valueable<int> HealthBar_XOffset;
	Valueable<Vector3D<int>> Sections_Pips;
	Valueable<int> Sections_Amount;
	Valueable<int> Sections_Size;

	HealthBarTypeClass(const char* pTitle = NONE_STR) : Enumerable<HealthBarTypeClass>(pTitle)
		, Frame_Background { }
		, Frame_Background_ShowWhenNotSelected { false }
		, Frame_Background_Translucency { }
		, Frame_Foreground { }
		, Frame_Foreground_ShowWhenNotSelected { false }
		, HealthBar_XOffset { 0 }
		, Sections_Pips { { 16,17,18 } }
		, Sections_Amount { 17 }
		, Sections_Size { 2 }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
