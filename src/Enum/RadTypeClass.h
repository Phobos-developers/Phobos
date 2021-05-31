#pragma once

#include "_Enumerator.hpp"
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>

class WarheadTypeClass;

class RadTypeClass final : public Enumerable<RadTypeClass>
{
private:
	Nullable<int> DurationMultiple;
	Nullable<int> ApplicationDelay;
	Nullable<int> BuildingApplicationDelay;
	Nullable<double> LevelFactor;
	Nullable<int> LevelMax;
	Nullable<int> LevelDelay;
	Nullable<int> LightDelay;
	Nullable<WarheadTypeClass *> RadWarhead;
	Nullable<ColorStruct> RadSiteColor;
	Nullable<double> LightFactor;
	Nullable<double> TintFactor;

public:

	RadTypeClass(const char* const pTitle) : Enumerable<RadTypeClass>(pTitle),
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

	virtual ~RadTypeClass() override = default;

	static void AddDefaults();

	static void LoadListSection(CCINIClass * pINI);

	WarheadTypeClass* GetWarhead() const
	{
		return this->RadWarhead.Get(RulesClass::Instance->RadSiteWarhead);
	}

	const ColorStruct& GetColor() const
	{
		return *this->RadSiteColor.GetEx(&RulesClass::Instance->RadColor);
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
		return this->BuildingApplicationDelay.Get(RulesExt::Global()->RadApplicationDelay_Building);
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

	virtual void LoadFromINI(CCINIClass *pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};