#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class LaserTrailTypeClass final : public Enumerable<LaserTrailTypeClass>
{
public:
	Valueable<bool> IsHouseColor;
	Valueable<ColorStruct> Color;
	Valueable<int> FadeDuration;
	Valueable<int> Thickness;
	Valueable<int> SegmentLength;
	Valueable<bool> IgnoreVertical;
	Valueable<bool> IsIntense;
	Valueable<bool> CloakVisible;
	Valueable<bool> CloakVisible_DetectedOnly;
	Valueable<bool> DroppodOnly;

	LaserTrailTypeClass(const char* pTitle = NONE_STR) : Enumerable<LaserTrailTypeClass>(pTitle)
		, IsHouseColor { false }
		, Color { { 255, 0, 0 } }
		, FadeDuration { 64 }
		, Thickness { 4 }
		, SegmentLength { 128 }
		, IgnoreVertical { false }
		, IsIntense { false }
		, CloakVisible { false }
		, CloakVisible_DetectedOnly { false }
		, DroppodOnly { false }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
