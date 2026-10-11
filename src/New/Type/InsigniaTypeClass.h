#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>

class InsigniaTypeClass final : public Enumerable<InsigniaTypeClass>
{
public:
	Promotable<SHPStruct*> Shape;
	CustomPalette Palette;
	CustomPalette Palette_Rookie;
	CustomPalette Palette_Veteran;
	CustomPalette Palette_Elite;
	Promotable<int> Frame;

	InsigniaTypeClass(const char* const pTitle) : Enumerable<InsigniaTypeClass>(pTitle)
		, Shape {}
		, Palette { CustomPalette::PaletteMode::Temperate }
		, Palette_Rookie { CustomPalette::PaletteMode::Temperate }
		, Palette_Veteran { CustomPalette::PaletteMode::Temperate }
		, Palette_Elite { CustomPalette::PaletteMode::Temperate }
		, Frame { -1 }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
