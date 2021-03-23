#pragma once

#include "../Utilities/Template.h"
#include "../Utilities/TemplateDef.h"
#include "../Utilities/GeneralUtils.h"

#include <WarheadTypeClass.h>

class RadType {
public:
	PhobosFixedString<0x20> ID;
	Valueable<int> DurationMultiple;
	Valueable<int> ApplicationDelay;
	Valueable<int> LevelMax;
	Valueable<int> LevelDelay;
	Valueable<int> LightDelay;
	Valueable<int> BuildingApplicationDelay;
	Valueable<double> LevelFactor;
	Valueable<double> LightFactor;
	Valueable<double> TintFactor;
	Valueable<ColorStruct> RadSiteColor;
	Valueable<WarheadTypeClass*> RadWarhead;

	// Set default values
	// RadType::Read method will later read the new values from the section specified in the ID field
	RadType(const char* id = "Radiation") :
		ID(id),
		DurationMultiple(1),
		ApplicationDelay(16),
		LevelMax(500),
		LevelDelay(90),
		LightDelay(90),
		BuildingApplicationDelay(0),
		LevelFactor(0.2f),
		LightFactor(0.1f),
		TintFactor(1.0f),
		RadSiteColor(ColorStruct{ 0,255,0 }),
		RadWarhead(nullptr)
	{
		RadWarhead = WarheadTypeClass::FindOrAllocate("RadSite");
	}

	void Read(CCINIClass* const pINI, const char* section, const char* pKey);
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);
private:
	template <typename T>
	void Serialize(T& Stm);
};
