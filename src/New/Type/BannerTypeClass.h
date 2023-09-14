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

	BannerTypeClass(const char* pTitle = NONE_STR);
	~BannerTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& stm) override;
	virtual void SaveToStream(PhobosStreamWriter& stm) override;

private:

	void DetermineType();

	template <typename T>
	void Serialize(T& Stm);
};
