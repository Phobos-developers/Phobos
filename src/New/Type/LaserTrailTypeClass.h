#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class LaserTrailTypeClass final : public Enumerable<LaserTrailTypeClass>
{
public:
	Valueable<LaserTrailDrawType> DrawType;
	Valueable<bool> IsHouseColor;
	Valueable<ColorStruct> Color;
	Valueable<int> Thickness;
	Valueable<bool> IsAlternateColor;
	Nullable<ColorStruct> Bolt_Color1;
	Valueable<bool> Bolt_Disable1;
	Nullable<ColorStruct> Bolt_Color2;
	Valueable<bool> Bolt_Disable2;
	Nullable<ColorStruct> Bolt_Color3;
	Valueable<bool> Bolt_Disable3;
	Valueable<int> Bolt_Arcs;
	Nullable<ColorStruct> Beam_Color;
	Valueable<double> Beam_Amplitude;
	Nullable<int> FadeDuration;
	Valueable<int> SegmentLength;
	Valueable<bool> IgnoreVertical;
	Valueable<bool> IsIntense;
	Valueable<bool> CloakVisible;
	Valueable<bool> CloakVisible_DetectedOnly;
	Valueable<bool> DroppodOnly;

	LaserTrailTypeClass(const char* pTitle = NONE_STR) : Enumerable<LaserTrailTypeClass>(pTitle)
		, DrawType { LaserTrailDrawType::Laser }
		, IsHouseColor { false }
		, Color { { 255, 0, 0 } }
		, Thickness { 4 }
		, IsAlternateColor { false }
		, Bolt_Color1 {}
		, Bolt_Disable1 { false }
		, Bolt_Color2 {}
		, Bolt_Disable2 { false }
		, Bolt_Color3 {}
		, Bolt_Disable3 { false }
		, Bolt_Arcs { 8 }
		, Beam_Color {}
		, Beam_Amplitude { 40.0 }
		, FadeDuration {}
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
