#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class BannerTypeClass final : public Enumerable<BannerTypeClass>
{
public:

	//PCX
	Valueable<BSurface*> PCX;

	//SHP
	Valueable<SHPStruct*> Shape;
	CustomPalette Palette;

	//CSF
	Valueable<CSFText> CSF;
	Valueable<ColorStruct> CSF_Color;
	Valueable<bool> CSF_Background;

	//VariableFormat
	Valueable<BannerNumberType> VariableFormat;
	Valueable<CSFText> VariableFormat_Label;

	BannerType BannerType;

	BannerTypeClass(const char* pTitle) : Enumerable<BannerTypeClass>(pTitle)
		, PCX { }
		, Shape { }
		, Palette { }
		, CSF { }
		, CSF_Color(Drawing::TooltipColor)
		, CSF_Background { false }
		, VariableFormat { }
		, VariableFormat_Label { }
		, BannerType(BannerType::None)
	{ }

	virtual void LoadFromINI(CCINIClass* pINI);
	virtual void LoadFromStream(PhobosStreamReader& stm);
	virtual void SaveToStream(PhobosStreamWriter& stm);

private:

	void DetermineType();

	template <typename T>
	void Serialize(T& Stm);
};
