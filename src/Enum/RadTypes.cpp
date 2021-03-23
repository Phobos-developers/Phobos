#include "RadTypes.h"

void RadType::Read(CCINIClass* const pINI, const char* pSection, const char* pKey) {
	INI_EX exINI(pINI);

	this->ID.Read(pINI, pSection, "RadType");
	const char* section = this->ID;

	if (pINI->GetSection(section)) {
		this->DurationMultiple.Read(exINI, section, "RadDurationMultiple");
		this->ApplicationDelay.Read(exINI, section, "RadApplicationDelay");
		this->BuildingApplicationDelay.Read(exINI, section, "RadApplicationDelay.Building");
		this->LevelMax.Read(exINI, section, "RadLevelMax");
		this->LevelDelay.Read(exINI, section, "RadLevelDelay");
		this->LightDelay.Read(exINI, section, "RadLightDelay");
		this->LevelFactor.Read(exINI, section, "RadLevelFactor");
		this->LightFactor.Read(exINI, section, "RadLightFactor");
		this->TintFactor.Read(exINI, section, "RadTintFactor");
		this->RadWarhead.Read(exINI, section, "RadSiteWarhead");
		this->RadSiteColor.Read(exINI, section, "RadColor");
	}
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
		;
};

void RadType::LoadFromStream(PhobosStreamReader & Stm)
{
	Stm.Process(this->ID);
	if (GeneralUtils::IsValidString(this->ID))
	{
	char warheadID[sizeof(this->RadWarhead->ID)];
	Stm.Process(warheadID);
	RadWarhead = WarheadTypeClass::FindOrAllocate(warheadID);
	this->Serialize(Stm);
	}
}

void RadType::SaveToStream(PhobosStreamWriter & Stm)
{
	Stm.Process(this->ID);
	if (GeneralUtils::IsValidString(this->ID))
	{
	Stm.Process(this->RadWarhead->ID);
	this->Serialize(Stm);
	}
}
