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
		, DroppodOnly { false }
	{ }

	virtual ~LaserTrailTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
