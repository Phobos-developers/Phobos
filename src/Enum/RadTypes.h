#pragma once

#include "_Enumerator.hpp"
#include "../Utilities/Template.h"
#include "../Utilities/GeneralUtils.h"
#include "../Phobos.h"

class WarheadTypeClass;

class RadType final : public Enumerable<RadType>
{
private:
	Nullable<int> DurationMultiple;
	Nullable<int> ApplicationDelay;
	Nullable<double> LevelFactor;
	Nullable<int> LevelMax;
	Nullable<int> LevelDelay;
	Nullable<int> LightDelay;
	Nullable<WarheadTypeClass *> RadWarhead;
	Nullable<ColorStruct> RadSiteColor;
	Nullable<double> LightFactor;
	Nullable<double> TintFactor;

public:

	RadType(const char* pTitle);

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

	Nullable<int> BuildingApplicationDelay;

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

	virtual ~RadType() override;
	virtual void LoadFromINI(CCINIClass *pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);
private:
	template <typename T>
	void Serialize(T& Stm);
};