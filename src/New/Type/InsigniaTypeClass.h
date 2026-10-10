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
		, Palette {}
		, Palette_Rookie {}
		, Palette_Veteran {}
		, Palette_Elite {}
		, Frame { -1 }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
