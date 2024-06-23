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
	Nullable<int> BuildingApplicationDelay;
	Nullable<double> LevelFactor;
	Nullable<int> LevelMax;
	Nullable<int> LevelDelay;
	Nullable<int> LightDelay;
	Nullable<WarheadTypeClass*> RadWarhead;
	Nullable<bool> RadWarhead_Detonate;
	Nullable<ColorStruct> RadSiteColor;
	Nullable<double> LightFactor;
	Nullable<double> TintFactor;
	Nullable<bool> RadHasOwner;
	Nullable<bool> RadHasInvoker;
	Nullable<bool> DecreasingRadDamage;

public:

	RadTypeClass(const char* const pTitle) : Enumerable<RadTypeClass>(pTitle)
		, LevelDelay { }
		, LightDelay { }
		, RadSiteColor { }
		, LevelMax { }
		, LevelFactor { }
		, LightFactor { }
		, TintFactor { }
		, RadWarhead { }
		, RadWarhead_Detonate { }
		, DurationMultiple { }
		, ApplicationDelay { }
		, BuildingApplicationDelay { }
		, RadHasOwner { }
		, RadHasInvoker { }
		, DecreasingRadDamage { }
	{ }

	virtual ~RadTypeClass() override = default;

	static void AddDefaults();

	WarheadTypeClass* GetWarhead() const
	{
		return this->RadWarhead.Get(RulesClass::Instance->RadSiteWarhead);
	}

	bool GetWarheadDetonate() const
	{
		return this->RadWarhead_Detonate.Get(RulesExt::Global()->RadWarhead_Detonate);
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

	bool GetHasOwner() const
	{
		return this->RadHasOwner.Get(RulesExt::Global()->RadHasOwner);
	}

	bool GetHasInvoker() const
	{
		return this->RadHasInvoker.Get(RulesExt::Global()->RadHasInvoker);
	}

	bool GetDecreasingRadDamage() const
	{
		return this->DecreasingRadDamage.Get(RulesExt::Global()->DecreasingRadDamage);
	}

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
