#pragma once

#include <TechnoTypeClass.h>

#include <Utilities/TemplateDef.h>

class ConditionGroup
{
public:
	Valueable<bool> PowerOn;
	Valueable<bool> PowerOff;
	Valueable<int> AmmoExceed;
	Valueable<int> AmmoBelow;
	Valueable<int> AfterDelay;
	Valueable<bool> OwnedByPlayer;
	Valueable<bool> OwnedByAI;
	Valueable<int> MoneyExceed;
	Valueable<int> MoneyBelow;
	Valueable<bool> LowPower;
	Valueable<bool> FullPower;
	Valueable<int> TechLevel;
	DWORD RequiredHouses;
	DWORD ForbiddenHouses;
	Nullable<double> AbovePercent;
	Nullable<double> BelowPercent;
	Valueable<int> PassengersExceed;
	Valueable<int> PassengersBelow;
	Valueable<bool> ShieldActive;
	Valueable<bool> ShieldInactive;
	Nullable<double> ShieldAbovePercent;
	Nullable<double> ShieldBelowPercent;
	Valueable<bool> IsMoving;
	Valueable<bool> IsStationary;
	Valueable<bool> IsDrained;
	Valueable<bool> IsCloaked;
	ValueableVector<TechnoTypeClass*> TechnosDontExist;
	Valueable<bool> TechnosDontExist_Any;
	Valueable<bool> TechnosDontExist_AllowLimboed;
	Valueable<AffectedHouse> TechnosDontExist_Houses;
	ValueableVector<TechnoTypeClass*> TechnosExist;
	Valueable<bool> TechnosExist_Any;
	Valueable<bool> TechnosExist_AllowLimboed;
	Valueable<AffectedHouse> TechnosExist_Houses;
	Valueable<bool> OnAnyCondition;

	ConditionGroup();

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static bool CheckHouseConditions(HouseClass* pOwner, const ConditionGroup condition);
	static bool CheckTechnoConditions(TechnoClass* pTechno, const ConditionGroup condition);
	static bool CheckTechnoConditionsWithTimer(TechnoClass* pTechno, const ConditionGroup condition, CDTimerClass& Timer);
	static bool BatchCheckTechnoExist(HouseClass* pOwner, const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo);
	static void ParseAutoDeath(ConditionGroup& condition, INI_EX& exINI, const char* section);

private:
	template <typename T>
	bool Serialize(T& stm);

	ConditionGroupType type;
};
