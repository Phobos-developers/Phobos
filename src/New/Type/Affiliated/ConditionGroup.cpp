#include "ConditionGroup.h"
#include <Ext/House/Body.h>
#include <Utilities/EnumFunctions.h>
#include <unordered_set>

bool ConditionGroup::CheckHouseConditions(HouseClass* pOwner, const ConditionGroup& condition)
{
	const bool onAnyCondition = condition.OnAnyCondition;

	// Owning house
	if (pOwner->IsControlledByHuman())
	{
		if (condition.OwnedByPlayer && onAnyCondition)
			return true;

		if (condition.OwnedByAI && !onAnyCondition)
			return false;
	}
	else
	{
		if (condition.OwnedByPlayer && !onAnyCondition)
			return false;

		if (condition.OwnedByAI && onAnyCondition)
			return true;
	}

	// Money
	if (condition.MoneyExceed >= 0)
	{
		if (pOwner->Available_Money() >= condition.MoneyExceed)
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	if (condition.MoneyBelow >= 0)
	{
		if (pOwner->Available_Money() <= condition.MoneyBelow)
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// Power
	if (pOwner->HasLowPower())
	{
		if (condition.LowPower && onAnyCondition)
			return true;

		if (condition.FullPower && !onAnyCondition)
			return false;
	}
	else if (pOwner->HasFullPower())
	{
		if (condition.LowPower && !onAnyCondition)
			return false;

		if (condition.FullPower && onAnyCondition)
			return true;
	}

	// TechLevel
	if (condition.TechLevel > 0)
	{
		if (pOwner->TechLevel >= condition.TechLevel)
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
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
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}

		if (!(condition.ForbiddenHouses & OwnerBits))
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// Listed technos don't exist
	if (!condition.TechnosDontExist.empty())
	{
		if (!ConditionGroup::BatchCheckTechnoExist(pOwner, condition.TechnosDontExist, condition.TechnosDontExist_Houses, !condition.TechnosDontExist_Any, condition.TechnosDontExist_AllowLimboed))
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// Listed technos exist
	if (!condition.TechnosExist.empty())
	{
		if (ConditionGroup::BatchCheckTechnoExist(pOwner, condition.TechnosExist, condition.TechnosExist_Houses, condition.TechnosExist_Any, condition.TechnosDontExist_AllowLimboed))
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!onAnyCondition)
		return true;

	return false;
}

bool ConditionGroup::CheckTechnoConditions(TechnoClass* pTechno, const ConditionGroup& condition)
{
	auto const pType = pTechno->GetTechnoType();
	auto const pOwner = pTechno->Owner;
	const bool onAnyCondition = condition.OnAnyCondition;

	// Process house conditions
	if (pOwner)
	{
		if (ConditionGroup::CheckHouseConditions(pOwner, condition))
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// Powered
	if (condition.PowerOn || condition.PowerOff)
	{
		bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

		if (isActive && pTechno->WhatAmI() == AbstractType::Building)
		{
			auto const pBuilding = static_cast<BuildingClass const*>(pTechno);
			isActive = pBuilding->IsPowerOnline();
		}

		if (isActive)
		{
			if (condition.PowerOn && onAnyCondition)
				return true;

			if (condition.PowerOff && !onAnyCondition)
				return false;
		}
		else
		{
			if (condition.PowerOn && !onAnyCondition)
				return false;

			if (condition.PowerOff && onAnyCondition)
				return true;
		}
	}

	// Health
	if (condition.AbovePercent.isset())
	{
		if (pTechno->GetHealthPercentage() >= condition.AbovePercent.Get())
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	if (condition.BelowPercent.isset())
	{
		if (pTechno->GetHealthPercentage() <= condition.BelowPercent.Get())
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// Ammo
	if (condition.AmmoExceed >= 0)
	{
		if (pType->Ammo > 0 && pTechno->Ammo >= condition.AmmoExceed)
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	if (condition.AmmoBelow >= 0)
	{
		if (pType->Ammo > 0 && pTechno->Ammo <= condition.AmmoBelow)
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// Passengers
	if (condition.PassengersExceed >= 0)
	{
		if (pType->Passengers > 0 && pTechno->Passengers.NumPassengers >= condition.PassengersExceed)
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	if (condition.PassengersBelow >= 0)
	{
		if (pType->Passengers > 0 && pTechno->Passengers.NumPassengers <= condition.PassengersBelow)
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// Move
	if (auto const pFoot = abstract_cast<FootClass*, true>(pTechno))
	{
		if (pFoot->Locomotor->Is_Really_Moving_Now())
		{
			if (condition.IsMoving && onAnyCondition)
				return true;

			if (condition.IsStationary && !onAnyCondition)
				return false;
		}
		else
		{
			if (condition.IsMoving && onAnyCondition)
				return false;

			if (condition.IsStationary && !onAnyCondition)
				return true;
		}
	}

	// Cloak
	if (condition.IsCloaked)
	{
		if (pTechno->CloakState == CloakState::Cloaked)
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// Drain
	if (condition.IsDrained)
	{
		if (pTechno->DrainingMe)
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// Shield
	auto const pExt = TechnoExt::ExtMap.Find(pTechno);

	if (auto const pShieldData = pExt->Shield.get())
	{
		if (pShieldData->IsActive())
		{
			if (condition.ShieldActive && onAnyCondition)
				return true;

			if (condition.ShieldInactive && !onAnyCondition)
				return false;

			if (condition.ShieldAbovePercent.isset() && pShieldData->GetHealthRatio() >= condition.ShieldAbovePercent)
			{
				if (onAnyCondition)
					return true;
			}
			else if (!onAnyCondition)
			{
				return false;
			}

			if (condition.ShieldBelowPercent.isset() && pShieldData->GetHealthRatio() <= condition.ShieldBelowPercent)
			{
				if (onAnyCondition)
					return true;
			}
			else if (!onAnyCondition)
			{
				return false;
			}
		}
		else
		{
			if (condition.ShieldActive && onAnyCondition)
				return false;

			if (condition.ShieldInactive && !onAnyCondition)
				return true;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!onAnyCondition)
		return true;

	return false;
}

bool ConditionGroup::BatchCheckTechnoExist(HouseClass* pOwner, const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo)
{
	std::unordered_set<HouseClass*> validHouses;

	if (affectedHouse == AffectedHouse::Owner)
	{
		validHouses.insert(pOwner);
	}
	else
	{
		for (auto pHouse : HouseClass::Array)
		{
			if (EnumFunctions::CanTargetHouse(affectedHouse, pOwner, pHouse))
				validHouses.insert(pHouse);
		}
	}

	auto existSingleType = [pOwner, validHouses, allowLimbo](TechnoTypeClass* pType)
		{
			for (HouseClass* pHouse : validHouses)
			{
				if (allowLimbo ? HouseExt::ExtMap.Find(pHouse)->CountOwnedPresentAndLimboed(pType) > 0 : pHouse->CountOwnedAndPresent(pType) > 0)
					return true;
			}

			return false;
		};

	return any
		? std::any_of(vTypes.begin(), vTypes.end(), existSingleType)
		: std::all_of(vTypes.begin(), vTypes.end(), existSingleType);
}

bool ConditionGroup::CheckTechnoConditionsWithTimer(TechnoClass* pTechno, const ConditionGroup& condition, CDTimerClass& Timer)
{
	const bool onAnyCondition = condition.OnAnyCondition;

	// Process techno conditions
	if (ConditionGroup::CheckTechnoConditions(pTechno, condition))
	{
		if (onAnyCondition)
			return true;
	}
	else if (!onAnyCondition)
	{
		return false;
	}

	// Countdown ends
	if (condition.AfterDelay > 0)
	{
		if (!Timer.HasStarted())
		{
			Timer.Start(condition.AfterDelay);

			if (!onAnyCondition)
				return false;
		}
		else if (Timer.Completed())
		{
			if (onAnyCondition)
				return true;
		}
		else if (!onAnyCondition)
		{
			return false;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!onAnyCondition)
		return true;

	return false;
}

ConditionGroup::ConditionGroup()
	: PowerOn { false }
	, PowerOff { false }
	, AmmoExceed { -1 }
	, AmmoBelow { -1 }
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
	, AbovePercent {}
	, BelowPercent {}
	, PassengersExceed { -1 }
	, PassengersBelow { -1 }
	, ShieldActive { false }
	, ShieldInactive { false }
	, ShieldAbovePercent {}
	, ShieldBelowPercent {}
	, IsMoving { false }
	, IsStationary { false }
	, IsCloaked { false }
	, IsDrained { false }
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
	condition.AmmoExceed.Read(exINI, pSection, "AutoDeath.AmmoExceed");
	condition.AmmoBelow.Read(exINI, pSection, "AutoDeath.AmmoBelow");
	condition.AfterDelay.Read(exINI, pSection, "AutoDeath.AfterDelay");
	condition.OwnedByPlayer.Read(exINI, pSection, "AutoDeath.OwnedByPlayer");
	condition.OwnedByAI.Read(exINI, pSection, "AutoDeath.OwnedByAI");
	condition.MoneyExceed.Read(exINI, pSection, "AutoDeath.MoneyExceed");
	condition.MoneyBelow.Read(exINI, pSection, "AutoDeath.MoneyBelow");
	condition.LowPower.Read(exINI, pSection, "AutoDeath.LowPower");
	condition.FullPower.Read(exINI, pSection, "AutoDeath.FullPower");
	condition.AbovePercent.Read(exINI, pSection, "AutoDeath.AbovePercent");
	condition.BelowPercent.Read(exINI, pSection, "AutoDeath.BelowPercent");
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
		.Process(this->PowerOn)
		.Process(this->PowerOff)
		.Process(this->AmmoExceed)
		.Process(this->AmmoBelow)
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
		.Process(this->ShieldActive)
		.Process(this->ShieldInactive)
		.Process(this->ShieldAbovePercent)
		.Process(this->ShieldBelowPercent)
		.Process(this->IsMoving)
		.Process(this->IsStationary)
		.Process(this->IsCloaked)
		.Process(this->IsDrained)
		.Process(this->TechnosDontExist)
		.Process(this->TechnosDontExist_Any)
		.Process(this->TechnosDontExist_AllowLimboed)
		.Process(this->TechnosDontExist_Houses)
		.Process(this->TechnosExist)
		.Process(this->TechnosExist_Any)
		.Process(this->TechnosExist_AllowLimboed)
		.Process(this->TechnosExist_Houses)
		.Process(this->OnAnyCondition)
		.Process(this->type)
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
