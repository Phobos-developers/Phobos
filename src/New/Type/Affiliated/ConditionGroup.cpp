#include "ConditionGroup.h"
#include <Ext/House/Body.h>
#include <Utilities/EnumFunctions.h>

bool ConditionGroup::CheckHouseConditions(HouseClass* pOwner, const ConditionGroup condition)
{
	// Owning house
	if (pOwner->IsControlledByHuman())
	{
		if (condition.OwnedByPlayer && condition.OnAnyCondition)
			return true;

		if (condition.OwnedByAI && !condition.OnAnyCondition)
			return false;
	}
	else
	{
		if (condition.OwnedByPlayer && !condition.OnAnyCondition)
			return false;

		if (condition.OwnedByAI && condition.OnAnyCondition)
			return true;
	}

	// Money
	if (condition.MoneyExceed >= 0)
	{
		if (pOwner->Available_Money() >= condition.MoneyExceed)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	if (condition.MoneyBelow >= 0)
	{
		if (pOwner->Available_Money() <= condition.MoneyBelow)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Power
	if (pOwner->HasLowPower())
	{
		if (condition.LowPower && condition.OnAnyCondition)
			return true;

		if (condition.FullPower && !condition.OnAnyCondition)
			return false;
	}
	else if (pOwner->HasFullPower())
	{
		if (condition.LowPower && !condition.OnAnyCondition)
			return false;

		if (condition.FullPower && condition.OnAnyCondition)
			return true;
	}

	// TechLevel
	if (condition.TechLevel > 0)
	{
		if (pOwner->TechLevel >= condition.TechLevel)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// RequiredHouses & ForbiddenHouses
	if (condition.type == ConditionGroupType::SW)
	{
		const auto OwnerBits = 1u << pOwner->Type->ArrayIndex;

		if (condition.RequiredHouses & OwnerBits)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}

		if (!(condition.ForbiddenHouses & OwnerBits))
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Listed technos don't exist
	if (!condition.TechnosDontExist.empty())
	{
		if (!ConditionGroup::BatchCheckTechnoExist(pOwner, condition.TechnosDontExist, condition.TechnosDontExist_Houses, !condition.TechnosDontExist_Any, condition.TechnosDontExist_AllowLimboed))
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Listed technos exist
	if (!condition.TechnosExist.empty())
	{
		if (ConditionGroup::BatchCheckTechnoExist(pOwner, condition.TechnosExist, condition.TechnosExist_Houses, condition.TechnosExist_Any, condition.TechnosDontExist_AllowLimboed))
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!condition.OnAnyCondition)
		return true;

	return false;
}

bool ConditionGroup::CheckTechnoConditions(const TechnoClass* pTechno, const ConditionGroup condition, CDTimerClass& Timer)
{
	auto const pType = pTechno->GetTechnoType();
	auto const pOwner = pTechno->Owner;

	// Process house conditions
	if (pOwner)
	{
		if (ConditionGroup::CheckHouseConditions(pOwner, condition))
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// No ammo
	if (condition.OnAmmoDepletion)
	{
		if (pType->Ammo > 0 && pTechno->Ammo <= 0)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Passengers
	if (condition.PassengersExceed >= 0)
	{
		if (pType->Passengers > 0 && pTechno->Passengers.NumPassengers >= condition.PassengersExceed)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	if (condition.PassengersBelow >= 0)
	{
		if (pType->Passengers > 0 && pTechno->Passengers.NumPassengers <= condition.PassengersBelow)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Countdown ends
	if (condition.AfterDelay > 0)
	{
		if (!Timer.HasStarted())
		{
			Timer.Start(condition.AfterDelay);

			if (!condition.OnAnyCondition)
				return false;
		}
		else if (Timer.Completed())
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!condition.OnAnyCondition)
		return true;

	return false;
}

bool ConditionGroup::BatchCheckTechnoExist(HouseClass* pOwner, const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo)
{
	auto existSingleType = [pOwner, affectedHouse, allowLimbo](TechnoTypeClass* pType)
		{
			for (HouseClass* pHouse : *HouseClass::Array)
			{
				if (EnumFunctions::CanTargetHouse(affectedHouse, pOwner, pHouse)
					&& (allowLimbo ? HouseExt::ExtMap.Find(pHouse)->CountOwnedPresentAndLimboed(pType) > 0 : pHouse->CountOwnedAndPresent(pType) > 0))
					return true;
			}

			return false;
		};

	return any
		? std::any_of(vTypes.begin(), vTypes.end(), existSingleType)
		: std::all_of(vTypes.begin(), vTypes.end(), existSingleType);
}

ConditionGroup::ConditionGroup()
	: OnAmmoDepletion { false }
	, AfterDelay { 0 }
	, OwnedByPlayer { false }
	, OwnedByAI { false }
	, MoneyExceed { -1 }
	, MoneyBelow { -1 }
	, LowPower { false }
	, FullPower { false }
	, TechLevel { 0 }
	, RequiredHouses { 0xFFFFFFFFu }
	, ForbiddenHouses { 0u }
	, PassengersExceed { -1 }
	, PassengersBelow { -1 }
	, TechnosDontExist {}
	, TechnosDontExist_Any { false }
	, TechnosDontExist_AllowLimboed { false }
	, TechnosDontExist_Houses { AffectedHouse::Owner }
	, TechnosExist {}
	, TechnosExist_Any { true }
	, TechnosExist_AllowLimboed { false }
	, TechnosExist_Houses { AffectedHouse::Owner }
	, OnAnyCondition { true }
{
}

void ConditionGroup::ParseAutoDeath(ConditionGroup& condition, INI_EX& exINI, const char* pSection)
{
	condition.OnAmmoDepletion.Read(exINI, pSection, "AutoDeath.OnAmmoDepletion");
	condition.AfterDelay.Read(exINI, pSection, "AutoDeath.AfterDelay");
	condition.OwnedByPlayer.Read(exINI, pSection, "AutoDeath.OwnedByPlayer");
	condition.OwnedByAI.Read(exINI, pSection, "AutoDeath.OwnedByAI");
	condition.MoneyExceed.Read(exINI, pSection, "AutoDeath.MoneyExceed");
	condition.MoneyBelow.Read(exINI, pSection, "AutoDeath.MoneyBelow");
	condition.LowPower.Read(exINI, pSection, "AutoDeath.LowPower");
	condition.FullPower.Read(exINI, pSection, "AutoDeath.FullPower");
	condition.PassengersExceed.Read(exINI, pSection, "AutoDeath.PassengersExceed");
	condition.PassengersBelow.Read(exINI, pSection, "AutoDeath.PassengersBelow");
	condition.TechnosDontExist.Read(exINI, pSection, "AutoDeath.TechnosDontExist");
	condition.TechnosDontExist_Any.Read(exINI, pSection, "AutoDeath.TechnosDontExist.Any");
	condition.TechnosDontExist_AllowLimboed.Read(exINI, pSection, "AutoDeath.TechnosDontExist.AllowLimboed");
	condition.TechnosDontExist_Houses.Read(exINI, pSection, "AutoDeath.TechnosDontExist.Houses");
	condition.TechnosExist.Read(exINI, pSection, "AutoDeath.TechnosExist");
	condition.TechnosDontExist_Any.Read(exINI, pSection, "AutoDeath.TechnosDontExist.Any");
	condition.TechnosExist_AllowLimboed.Read(exINI, pSection, "AutoDeath.TechnosExist.AllowLimboed");
	condition.TechnosExist_Houses.Read(exINI, pSection, "AutoDeath.TechnosExist.Houses");
	condition.OnAnyCondition.Read(exINI, pSection, "AutoDeath.OnAnyCondition");

	// Type of this condition group must be set
	condition.type = ConditionGroupType::AutoDeath;
}

#pragma region(save/load)

template <typename T>
bool ConditionGroup::Serialize(T& stm)
{
	return stm
		.Process(this->OnAmmoDepletion)
		.Process(this->AfterDelay)
		.Process(this->OwnedByPlayer)
		.Process(this->OwnedByAI)
		.Process(this->MoneyExceed)
		.Process(this->MoneyBelow)
		.Process(this->LowPower)
		.Process(this->FullPower)
		.Process(this->TechLevel)
		.Process(this->RequiredHouses)
		.Process(this->ForbiddenHouses)
		.Process(this->PassengersExceed)
		.Process(this->PassengersBelow)
		.Process(this->TechnosDontExist)
		.Process(this->TechnosDontExist_Any)
		.Process(this->TechnosDontExist_AllowLimboed)
		.Process(this->TechnosDontExist_Houses)
		.Process(this->TechnosExist)
		.Process(this->TechnosExist_Any)
		.Process(this->TechnosExist_AllowLimboed)
		.Process(this->TechnosExist_Houses)
		.Process(this->OnAnyCondition)
		.Success();
}

bool ConditionGroup::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool ConditionGroup::Save(PhobosStreamWriter& stm) const
{
	return const_cast<ConditionGroup*>(this)->Serialize(stm);
}

#pragma endregion(save/load)
