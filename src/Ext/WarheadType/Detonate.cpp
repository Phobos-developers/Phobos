#include "Body.h"

#include <InfantryClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <AnimTypeClass.h>
#include <AnimClass.h>

#include "../../Utilities/Helpers.Alex.h"
#include "../TechnoType/Body.h"

void WarheadTypeExt::ExtData::Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords)
{
	if (pHouse)
	{
		if (this->BigGap)
		{
			for (auto pOtherHouse : *HouseClass::Array)
			{
				if (pOtherHouse->ControlledByHuman() &&	  // Not AI
					!pOtherHouse->IsObserver() &&		 // Not Observer
					!pOtherHouse->Defeated &&			 // Not Defeated
					pOtherHouse != pHouse &&			  // Not pThisHouse
					!pHouse->IsAlliedWith(pOtherHouse))   // Not Allied
				{
					pOtherHouse->ReshroudMap();
				}
			}
		}

		if (this->SpySat)
		{
			MapClass::Instance->Reveal(pHouse);
		}

		if (this->TransactMoney)
		{
			pHouse->TransactMoney(this->TransactMoney);
		}
	}

	this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();

	// List all Warheads here that respect CellSpread
	const bool isCellSpreadWarhead =
		this->RemoveDisguise ||
		this->RemoveMindControl ||
		this->Crit_Chance;

	const float cellSpread = this->OwnerObject()->CellSpread;
	if (cellSpread && isCellSpreadWarhead)
	{
		auto items = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true);
		for (auto pTarget : items)
		{
			this->DetonateOnOneUnit(pHouse, pTarget);
		}
	}
	else if (pBullet && isCellSpreadWarhead)
	{
		if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target))
		{
			this->DetonateOnOneUnit(pHouse, pTarget);
		}
	}
}

void WarheadTypeExt::ExtData::DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner)
{
	if (!pTarget || pTarget->InLimbo || !pTarget->IsAlive || !pTarget->Health)
	{
		return;
	}

	if (!this->CanTargetHouse(pHouse, pTarget))
	{
		return;
	}

	if (this->RemoveDisguise)
	{
		this->ApplyRemoveDisguiseToInf(pHouse, pTarget);
	}

	if (this->RemoveMindControl)
	{
		this->ApplyRemoveMindControl(pHouse, pTarget);
	}

	if (this->Crit_Chance)
	{
		this->ApplyCrit(pHouse, pTarget, pOwner);
	}
}

void WarheadTypeExt::ExtData::ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (auto pController = pTarget->MindControlledBy)
		pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
}

void WarheadTypeExt::ExtData::ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (pTarget->WhatAmI() == AbstractType::Infantry)
	{
		auto pInf = abstract_cast<InfantryClass*>(pTarget);
		if (pInf->IsDisguised())
		{
			pInf->ClearDisguise();
		}
	}
}

void WarheadTypeExt::ExtData::ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner)
{
	//auto& random = ScenarioClass::Instance->Random;
	const double dice = this->RandomBuffer; //double(random.RandomRanged(1, 10)) / 10;

	if (this->Crit_Chance < dice)
	{
		return;
	}

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType()))
	{
		if (pTypeExt->ImmuneToCrit)
			return;
	}

	if (!this->IsCellEligible(pTarget->GetCell(), this->Crit_Affects))
	{
		return;
	}

	if (!this->IsTechnoEligible(pTarget, this->Crit_Affects))
	{
		return;
	}

	pTarget->ReceiveDamage(&this->Crit_ExtraDamage, 0, this->OwnerObject(), pOwner, false, false, pHouse);
}
