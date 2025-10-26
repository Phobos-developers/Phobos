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
#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>

class WarheadTypeClass;

class RadTypeClass final : public Enumerable<RadTypeClass>
{
private:
	Nullable<int> DurationMultiple;
	Nullable<int> ApplicationDelay;
	Nullable<int> ApplicationDelay_Building;
	Nullable<int> BuildingDamageMaxCount;
	Nullable<int> LevelMax;
	Nullable<int> LevelDelay;
	Nullable<int> LightDelay;
	Nullable<double> LevelFactor;
	Nullable<double> LightFactor;
	Nullable<double> TintFactor;
	Nullable<ColorStruct> Color;
	Nullable<WarheadTypeClass*> SiteWarhead;
	Nullable<bool> SiteWarhead_Detonate;
	Nullable<bool> SiteWarhead_Detonate_Full;
	Nullable<bool> HasOwner;
	Nullable<bool> HasInvoker;

public:

	RadTypeClass(const char* const pTitle) : Enumerable<RadTypeClass>(pTitle)
		, DurationMultiple { }
		, ApplicationDelay { }
		, ApplicationDelay_Building { }
		, BuildingDamageMaxCount { }
		, LevelMax { }
		, LevelDelay { }
		, LightDelay { }
		, LevelFactor { }
		, LightFactor { }
		, TintFactor { }
		, Color { }
		, SiteWarhead { }
		, SiteWarhead_Detonate { }
		, SiteWarhead_Detonate_Full { }
		, HasOwner { }
		, HasInvoker { }
	{ }

	static void AddDefaults();

	WarheadTypeClass* GetWarhead() const
	{
		return this->SiteWarhead.Get(RulesClass::Instance->RadSiteWarhead);
	}

	bool GetWarheadDetonate() const
	{
		return this->SiteWarhead_Detonate.Get(RulesExt::Global()->RadSiteWarhead_Detonate);
	}

	bool GetWarheadDetonateFull() const
	{
		return this->SiteWarhead_Detonate_Full.Get(RulesExt::Global()->RadSiteWarhead_Detonate_Full);
	}

	const ColorStruct& GetColor() const
	{
		return *this->Color.GetEx(&RulesClass::Instance->RadColor);
	}

	int GetDurationMultiple() const
	{
		return this->DurationMultiple.Get(RulesClass::Instance->RadDurationMultiple);
	}

	int GetApplicationDelay() const
	{
		return this->ApplicationDelay.Get(RulesClass::Instance->RadApplicationDelay);
	}

	int GetBuildingApplicationDelay() const
	{
		return this->ApplicationDelay_Building.Get(RulesExt::Global()->RadApplicationDelay_Building);
	}

	int GetBuildingDamageMaxCount() const
	{
		return this->BuildingDamageMaxCount.Get(RulesExt::Global()->RadBuildingDamageMaxCount);
	}

	int GetLevelMax() const
	{
		return this->LevelMax.Get(RulesClass::Instance->RadLevelMax);
	}

	int GetLevelDelay() const
	{
		return this->LevelDelay.Get(RulesClass::Instance->RadLevelDelay);
	}

	int GetLightDelay() const
	{
		return this->LightDelay.Get(RulesClass::Instance->RadLightDelay);
	}

	double GetLevelFactor() const
	{
		return this->LevelFactor.Get(RulesClass::Instance->RadLevelFactor);
	}

	double GetLightFactor() const
	{
		return this->LightFactor.Get(RulesClass::Instance->RadLightFactor);
	}

	double GetTintFactor() const
	{
		return this->TintFactor.Get(RulesClass::Instance->RadTintFactor);
	}

	bool GetHasOwner() const
	{
		return this->HasOwner.Get(RulesExt::Global()->RadHasOwner);
	}

	bool GetHasInvoker() const
	{
		return this->HasInvoker.Get(RulesExt::Global()->RadHasInvoker);
	}

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
