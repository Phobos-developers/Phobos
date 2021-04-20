#include "RadTypes.h"

#include "../Utilities/TemplateDef.h"

#include <WarheadTypeClass.h>

Enumerable<RadType>::container_t Enumerable<RadType>::Array;

// pretty nice, eh
const char * Enumerable<RadType>::GetMainSection()
{
	return "RadiationTypes";
}

RadType::RadType(const char* const pTitle)
	: Enumerable<RadType>(pTitle),
	LevelDelay(),
	LightDelay(),
	RadSiteColor(),
	LevelMax(),
	LevelFactor(),
	LightFactor(),
	TintFactor(),
	RadWarhead(),
	DurationMultiple(),
	ApplicationDelay(),
	BuildingApplicationDelay()
{ }

RadType::~RadType() = default;

void RadType::AddDefaults()
{
	FindOrAllocate("Radiation");
}

void RadType::LoadListSection(CCINIClass *pINI)
{
	for (int i = 0; i < pINI->GetKeyCount(GetMainSection()); ++i)
	{
		if (pINI->ReadString(GetMainSection(), pINI->GetKeyName(GetMainSection(), i), "", Phobos::readBuffer))
		{
			FindOrAllocate(Phobos::readBuffer);
			Debug::Log("RadTypes :: LoadListSection check [%s] \n", Phobos::readBuffer);
		}
	}

	for (size_t i = 0; i < Array.size(); ++i) 
	 Array[i]->LoadFromINI(pINI);
}

void RadType::LoadFromINI(CCINIClass *pINI)
{
	const char *section = this->Name;

	INI_EX exINI(pINI);

	this->RadWarhead.Read(exINI, section, "RadSiteWarhead" , true);
	this->RadSiteColor.Read(exINI, section, "RadColor");
	this->DurationMultiple.Read(exINI, section, "RadDurationMultiple");
	this->ApplicationDelay.Read(exINI, section, "RadApplicationDelay");
	this->BuildingApplicationDelay.Read(exINI, section, "RadApplicationDelay.Building");
	this->LevelMax.Read(exINI, section, "RadLevelMax");
	this->LevelDelay.Read(exINI, section, "RadLevelDelay");
	this->LightDelay.Read(exINI, section, "RadLightDelay");
	this->LevelFactor.Read(exINI, section, "RadLevelFactor");
	this->LightFactor.Read(exINI, section, "RadLightFactor");
	this->TintFactor.Read(exINI, section, "RadTintFactor");

	Debug::Log("RadTypes :: LoadFromINI check [%s] \n", section);
//	Debug::Log("RadTypes :: LoadFromINI check [%s]->Warhead \n", this->GetWarhead()->Name);
//	Debug::Log("RadTypes :: LoadFromINI check [%d %d %d]->Color \n", this->GetColor().R , this->GetColor().G, this->GetColor().B);
}

template <typename T>
void RadType::Serialize(T& Stm) {
	Stm
		.Process(this->DurationMultiple)
		.Process(this->ApplicationDelay)
		.Process(this->LevelMax)
		.Process(this->LevelDelay)
		.Process(this->LightDelay)
		.Process(this->BuildingApplicationDelay)
		.Process(this->LevelFactor)
		.Process(this->LightFactor)
		.Process(this->TintFactor)
		.Process(this->RadSiteColor)
		.Process(this->RadWarhead)
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
