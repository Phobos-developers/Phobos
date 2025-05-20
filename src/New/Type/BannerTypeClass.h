#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class BannerTypeClass final : public Enumerable<BannerTypeClass>
{
public:

	//PCX
	PhobosPCXFile PCX;

	//SHP
	Valueable<SHPStruct*> Shape;
	CustomPalette Palette;

	//CSF
	Valueable<CSFText> CSF;
	Nullable<ColorStruct> CSF_Color;
	Valueable<bool> CSF_Background;
	Valueable<BannerNumberType> CSF_VariableFormat;

	BannerTypeClass(const char* const pTitle) : Enumerable<BannerTypeClass>(pTitle)
		, PCX { }
		, Shape { }
		, Palette { }
		, CSF { }
		, CSF_Color { }
		, CSF_Background { false }
		, CSF_VariableFormat { BannerNumberType::None }
	{ }

	virtual void LoadFromINI(CCINIClass* pINI);
	virtual void LoadFromStream(PhobosStreamReader& stm);
	virtual void SaveToStream(PhobosStreamWriter& stm);

private:

	template <typename T>
	void Serialize(T& Stm);
};
