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

void RadType::Load(IStream* Stm) {
	PhobosStreamReader::Process(Stm, this->ID);

	if (GeneralUtils::IsValidString(this->ID))
	{
		this->DurationMultiple.Load(Stm);
		this->ApplicationDelay.Load(Stm);
		this->LevelFactor.Load(Stm);
		this->LevelMax.Load(Stm);
		this->LevelDelay.Load(Stm);
		this->LightDelay.Load(Stm);
		this->BuildingApplicationDelay.Load(Stm);

		char warheadID[sizeof(this->RadWarhead->ID)];
		PhobosStreamReader::Process(Stm, warheadID);
		RadWarhead = WarheadTypeClass::FindOrAllocate(warheadID);

		this->RadSiteColor.Load(Stm);
		this->LightFactor.Load(Stm);
		this->TintFactor.Load(Stm);
	}
}

void RadType::Save(IStream* Stm) {
	PhobosStreamWriter::Process(Stm, this->ID);

	if (GeneralUtils::IsValidString(this->ID))
	{
		this->DurationMultiple.Save(Stm);
		this->ApplicationDelay.Save(Stm);
		this->LevelFactor.Save(Stm);
		this->LevelMax.Save(Stm);
		this->LevelDelay.Save(Stm);
		this->LightDelay.Save(Stm);
		this->BuildingApplicationDelay.Save(Stm);

		PhobosStreamWriter::Process(Stm, this->RadWarhead->ID);

		this->RadSiteColor.Save(Stm);
		this->LightFactor.Save(Stm);
		this->TintFactor.Save(Stm);
	}
}

