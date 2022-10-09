#include "RadTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <GameStrings.h>
#include <WarheadTypeClass.h>

Enumerable<RadTypeClass>::container_t Enumerable<RadTypeClass>::Array;

// pretty nice, eh
const char* Enumerable<RadTypeClass>::GetMainSection()
{
	return "RadiationTypes";
}

void RadTypeClass::AddDefaults()
{
	FindOrAllocate(GameStrings::Radiation);
}

void RadTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->RadWarhead.Read(exINI, section, "RadSiteWarhead", true);
	this->RadWarhead_Detonate.Read(exINI, section, "RadSiteWarhead.Detonate");
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
	this->RadHasOwner.Read(exINI, section, "RadHasOwner");
	this->RadHasInvoker.Read(exINI, section, "RadHasInvoker");
}

template <typename T>
void RadTypeClass::Serialize(T& Stm)
{
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
		.Process(this->RadWarhead_Detonate)
		.Process(this->RadHasOwner)
		.Process(this->RadHasInvoker)
		;
};

void RadTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void RadTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
