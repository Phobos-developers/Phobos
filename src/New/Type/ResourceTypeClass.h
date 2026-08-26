#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <GeneralStructures.h>

class ResourceTypeClass final : public Enumerable<ResourceTypeClass>
{
public:
	// Global HUD Configuration
	static Valueable<ResourceDisplayOrientation> Global_Display_Orientation;
	static Valueable<ResourceDisplayAnchor> Global_Display_Anchor;
	static Valueable<Point2D> Global_Display_BaseOffset;
	static Valueable<int> Global_Display_Spacing;

	// Per-Resource Display Configuration
	Valueable<CSFText> Display_Label;
	Valueable<bool> Display_Label_InvertPosition;
	Valueable<bool> Display_Label_UseSpace;
	Valueable<ColorStruct> Display_Color;
	Valueable<ResourceDisplayCondition> Display_Condition;
	Nullable<Point2D> Display_Offset;

	// Economy and Activation
	Valueable<int> InitialValue;
	Valueable<bool> RequiresCollector;

	// Bounty (Kills)
	Valueable<bool> Bounty_Enabled;
	Valueable<int> Bounty_DefaultValue;
	Valueable<int> Bounty_DefaultFriendlyValue;
	Valueable<bool> Bounty_CanUseStandardPoints;
	Valueable<int> Bounty_MoneyConversion;

	ResourceTypeClass(const char* pTitle = NONE_STR);
	virtual ~ResourceTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI);
	static void LoadGlobalsFromINI(CCINIClass* pINI);

	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};

struct ResourceProductionData
{
	int ResourceIndex;
	int Amount;
	int Delay;
	int Startup;
	Nullable<Point2D> Display_Offset;
	Nullable<ColorStruct> Display_Color;
	Nullable<AffectedHouse> Display_Houses;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(this->ResourceIndex)
			.Process(this->Amount)
			.Process(this->Delay)
			.Process(this->Startup)
			.Process(this->Display_Offset)
			.Process(this->Display_Color)
			.Process(this->Display_Houses);
	}
};

struct ResourceProductionTimer
{
	int ResourceIndex;
	int Delay;
	int Amount;
	int Timer;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(this->ResourceIndex)
			.Process(this->Delay)
			.Process(this->Amount)
			.Process(this->Timer);
	}
};
