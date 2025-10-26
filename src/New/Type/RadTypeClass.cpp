// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

	if (!pINI->GetSection(section))
		return;

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

	if (this->GetBuildingApplicationDelay())
		Phobos::Optimizations::DisableRadDamageOnBuildings = false;
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

	if (this->GetBuildingApplicationDelay())
		Phobos::Optimizations::DisableRadDamageOnBuildings = false;
}

void RadTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
