#include "RadTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <WarheadTypeClass.h>

template<>
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

	this->DurationMultiple.Read(exINI, section, "RadDurationMultiple");
	this->ApplicationDelay.Read(exINI, section, "RadApplicationDelay");
	this->ApplicationDelay_Building.Read(exINI, section, "RadApplicationDelay.Building");
	this->BuildingDamageMaxCount.Read(exINI, section, "RadBuildingDamageMaxCount");
	this->LevelMax.Read(exINI, section, "RadLevelMax");
	this->LevelDelay.Read(exINI, section, "RadLevelDelay");
	this->LightDelay.Read(exINI, section, "RadLightDelay");
	this->LevelFactor.Read(exINI, section, "RadLevelFactor");
	this->LightFactor.Read(exINI, section, "RadLightFactor");
	this->TintFactor.Read(exINI, section, "RadTintFactor");
	this->Color.Read(exINI, section, "RadColor");
	this->SiteWarhead.Read<true>(exINI, section, "RadSiteWarhead");
	this->SiteWarhead_Detonate.Read(exINI, section, "RadSiteWarhead.Detonate");
	this->SiteWarhead_Detonate_Full.Read(exINI, section, "RadSiteWarhead.Detonate.Full");
	this->HasOwner.Read(exINI, section, "RadHasOwner");
	this->HasInvoker.Read(exINI, section, "RadHasInvoker");
}

template <typename T>
void RadTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->DurationMultiple)
		.Process(this->ApplicationDelay)
		.Process(this->ApplicationDelay_Building)
		.Process(this->BuildingDamageMaxCount)
		.Process(this->LevelMax)
		.Process(this->LevelDelay)
		.Process(this->LightDelay)
		.Process(this->LevelFactor)
		.Process(this->LightFactor)
		.Process(this->TintFactor)
		.Process(this->Color)
		.Process(this->SiteWarhead)
		.Process(this->SiteWarhead_Detonate)
		.Process(this->SiteWarhead_Detonate_Full)
		.Process(this->HasOwner)
		.Process(this->HasInvoker)
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
