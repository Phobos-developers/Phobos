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
	PhobosFixedString<0x20> Content_Palette;
	Valueable<CSFText> Content_CSF;
	// internal
	BannerType Type;
	wchar_t Text[256];
	SHPStruct* ImageSHP;
	ConvertClass* Palette;

	BannerTypeClass(const char* pTitle = NONE_STR) : Enumerable<BannerTypeClass>(pTitle)
		, Type(BannerType::CSF)
	{ }

	virtual ~BannerTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
