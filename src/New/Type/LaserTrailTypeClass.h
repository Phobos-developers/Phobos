#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class LaserTrailTypeClass final : public Enumerable<LaserTrailTypeClass>
{
public:
	Valueable<bool> IsHouseColor;
	Valueable<bool> IsRainbowColor;
	ValueableVector<ColorStruct> Colors;
	Valueable<int> TransitionDuration;
	Valueable<int> Duration;
	Valueable<int> Thickness;
	Valueable<int> Distance;
	Valueable<bool> IgnoreVertical;
	Valueable<bool> IsIntense;

	LaserTrailTypeClass(const char* pTitle = NONE_STR) : Enumerable<LaserTrailTypeClass>(pTitle),
		IsHouseColor(false),
		IsRainbowColor(false),
		Colors(),
		TransitionDuration(60),
		Duration(16),
		Thickness(8),
		Distance(32),
		IgnoreVertical(false),
		IsIntense(false)
	{
		this->Colors.push_back({ 255, 0, 0 });
	}

	virtual ~LaserTrailTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};