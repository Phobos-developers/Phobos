#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/Enum.h>

class BannerTypeClass final : public Enumerable<BannerTypeClass>
{
public:
	// read from INI
	PhobosFixedString<0x20> Content_PCX;
	PhobosFixedString<0x20> Content_SHP;
	PhobosFixedString<0x20> Content_SHP_Palette;
	Valueable<CSFText> Content_CSF;
	Nullable<ColorStruct> Content_CSF_Color;
	Valueable<bool> Content_CSF_DrawBackground;
	// internal
	BannerType Type;
	wchar_t Text[256];
	SHPStruct* ImageSHP;
	ConvertClass* Palette;

	BannerTypeClass(const char* pTitle = NONE_STR) : Enumerable<BannerTypeClass>(pTitle)
		, Content_PCX()
		, Content_SHP()
		, Content_SHP_Palette()
		, Content_CSF()
		, Content_CSF_Color()
		, Content_CSF_DrawBackground(false)
		, Type(BannerType::CSF)
		, Text()
		, ImageSHP()
		, Palette()
	{ }

	virtual ~BannerTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
