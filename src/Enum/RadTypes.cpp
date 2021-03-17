#include "RadTypes.h"

#include "../Utilities/TemplateDef.h"

#include <WarheadTypeClass.h>

Enumerable<RadType>::container_t Enumerable<RadType>::Array;

// pretty nice, eh
const char* Enumerable<RadType>::GetMainSection()
{
	return "RadiationTypes";
}

RadType::RadType(const char* const pTitle)
	: Enumerable<RadType>(pTitle),
	WH(),
	Color(),
	Duration_Multiple(),
	Application_Delay(),
	BuildingApplication_Delay(),
	Level_Max(),
	Level_Delay(),
	Light_Delay(),
	Level_Factor(),
	Light_Factor(),
	Tint_Factor()
{ }

RadType::~RadType() = default;

void RadType::LoadFromINI(CCINIClass * pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->WH.Read(exINI, section, "Warhead");
	this->Color.Read(exINI, section, "Color");
	this->Duration_Multiple.Read(exINI, section, "DurationMultiple");
	this->Application_Delay.Read(exINI, section, "ApplicationDelay");
	this->BuildingApplication_Delay.Read(exINI, section, "BuildingApplicationDelay");
	this->Level_Max.Read(exINI, section, "LevelMax");
	this->Level_Delay.Read(exINI, section, "LevelDelay");
	this->Light_Delay.Read(exINI, section, "LightDelay");
	this->Level_Factor.Read(exINI, section, "LevelFactor");
	this->Light_Factor.Read(exINI, section, "LightFactor");
	this->Tint_Factor.Read(exINI, section, "TintFactor");
}

template <typename T>
void RadType::Serialize(T& Stm) {
	Stm
		.Process(this->WH)
		.Process(this->Color)
		.Process(this->Duration_Multiple)
		.Process(this->Application_Delay)
		.Process(this->BuildingApplication_Delay)
		.Process(this->Level_Max)
		.Process(this->Level_Delay)
		.Process(this->Light_Delay)
		.Process(this->Level_Factor)
		.Process(this->Light_Factor)
		.Process(this->Tint_Factor)
		;
};

void RadType::LoadFromStream(PhobosStreamReader & Stm)
{
	this->Serialize(Stm);
}

void RadType::SaveToStream(PhobosStreamWriter & Stm)
{
	this->Serialize(Stm);
}
