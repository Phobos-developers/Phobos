#include <Ext/Techno/Body.h>
#include "HandlerFilterClass.h"

HandlerFilterClass::HandlerFilterClass()
	: Abstract {}
	, IsInAir {}
	, TechnoTypes {}
	, AttachedEffects {}
	, ShieldTypes {}
	, Veterancy {}
	, HPPercentage {}
	, Owner_House {}
	, Owner_Sides {}
	, Owner_Countries {}
	, Owner_Buildings {}
	, Owner_IsHuman {}
	, Owner_IsAI {}
	, IsBunkered {}
	, IsMindControlled {}
	, IsMindControlled_Perma {}
	, MindControlling_Any {}
	, MindControlling_Type {}
	, Passengers_Any {}
	, Passengers_Type {}
	, Upgrades_Any {}
	, Upgrades_Type {}
{ }

std::unique_ptr<HandlerFilterClass> HandlerFilterClass::Parse(INI_EX & exINI, const char* pSection, const char* scopeName, const char* filterName)
{
	auto filter = std::make_unique<HandlerFilterClass>();
	filter.get()->LoadFromINI(exINI, pSection, scopeName, filterName);
	if (filter.get()->IsDefined())
	{
		return filter;
	}
	else
	{
		filter.reset();
		return nullptr;
	}
}

void HandlerFilterClass::LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* filterName)
{
	char tempBuffer[64];

	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Abstract", scopeName, filterName);
	Abstract.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsInAir", scopeName, filterName);
	IsInAir.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.TechnoTypes", scopeName, filterName);
	TechnoTypes.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.AttachedEffects", scopeName, filterName);
	AttachedEffects.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.ShieldTypes", scopeName, filterName);
	ShieldTypes.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Veterancy", scopeName, filterName);
	Veterancy.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.HPPercentage", scopeName, filterName);
	HPPercentage.Read(exINI, pSection, tempBuffer);

	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Owner.House", scopeName, filterName);
	Owner_House.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Owner.Sides", scopeName, filterName);
	Owner_Sides.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Owner.Countries", scopeName, filterName);
	Owner_Countries.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Owner.Buildings", scopeName, filterName);
	Owner_Buildings.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Owner.IsHuman", scopeName, filterName);
	Owner_IsHuman.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Owner.IsAI", scopeName, filterName);
	Owner_IsAI.Read(exINI, pSection, tempBuffer);

	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsBunkered", scopeName, filterName);
	IsBunkered.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsMindControlled", scopeName, filterName);
	IsMindControlled.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsMindControlled.Perma", scopeName, filterName);
	IsMindControlled_Perma.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.MindControlling.Any", scopeName, filterName);
	MindControlling_Any.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.MindControlling.Type", scopeName, filterName);
	MindControlling_Type.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Any", scopeName, filterName);
	Passengers_Any.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Type", scopeName, filterName);
	Passengers_Type.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Upgrades.Any", scopeName, filterName);
	Upgrades_Any.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Upgrades.Type", scopeName, filterName);
	Upgrades_Type.Read(exINI, pSection, tempBuffer);
}

// If "negative == false", the function returns false if any check failed, otherwise it returns true.
// If "negative == true", the function returns false if any check succeed, otherwise it returns true.
// Thus, we simply compare "negative" with the outcome of an entry, if equals then return false.
bool HandlerFilterClass::Check(std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pOwner, TechnoClass* pTarget, bool negative) const
{
	TechnoExt::ExtData* pTargetExt = nullptr;

	if (Abstract.isset())
	{
		if (negative == (EnumFunctions::IsCellEligible(pTarget->GetCell(), Abstract.Get(), true, true)
			&& EnumFunctions::IsTechnoEligible(pTarget, Abstract.Get(), false)))
			return false;
	}

	if (IsInAir.isset())
	{
		if (negative == (IsInAir.Get() == pTarget->IsInAir()))
			return false;
	}

	if (!TechnoTypes.empty())
	{
		if (negative == TechnoTypes.Contains(pTarget->GetTechnoType()))
			return false;
	}

	if (!AttachedEffects.empty())
	{
		if (!pTargetExt)
			pTargetExt = TechnoExt::ExtMap.Find(pTarget);
		if (negative == pTargetExt->HasAttachedEffects(AttachedEffects, false, false, nullptr, nullptr, nullptr, nullptr))
			return false;
	}

	if (!ShieldTypes.empty())
	{
		if (!pTargetExt)
			pTargetExt = TechnoExt::ExtMap.Find(pTarget);
		if (negative == (pTargetExt->CurrentShieldType && ShieldTypes.Contains(pTargetExt->CurrentShieldType)))
			return false;
	}

	if (Veterancy.isset())
	{
		if (negative == EnumFunctions::CheckVeterancy(pTarget, Veterancy.Get()))
			return false;
	}

	if (HPPercentage.isset())
	{
		if (negative == EnumFunctions::CheckHPPercentage(pTarget, HPPercentage.Get()))
			return false;
	}

	if (Owner_House.isset())
	{
		if (negative == EnumFunctions::CanTargetHouse(Owner_House.Get(), pOwner->Owner, pTarget->Owner))
			return false;
	}

	if (!Owner_Sides.empty())
	{
		bool sideFlag = false;
		for (auto pSide : Owner_Sides)
		{
			if (pTarget->Owner->Type->SideIndex == pSide->GetArrayIndex()) {
				sideFlag = true;
				break;
			}
		}
		if (negative == sideFlag)
			return false;
	}

	if (!Owner_Countries.empty())
	{
		if (negative == Owner_Countries.Contains(pTarget->Owner->Type))
			return false;
	}

	if (!Owner_Buildings.empty())
	{
		bool buildingFlag = false;
		if (pTarget->Owner->OwnedBuildings > 0)
		{
			for (auto const& pBld : pTarget->Owner->Buildings)
			{
				if (Owner_Buildings.Contains(pBld->Type))
				{
					buildingFlag = true;
					goto _EndOwnerBuildingSearch_;
				}
				for (const auto& pUpgrade : pBld->Upgrades)
				{
					if (Owner_Buildings.Contains(pUpgrade))
					{
						buildingFlag = true;
						goto _EndOwnerBuildingSearch_;
					}
				}
			}
		_EndOwnerBuildingSearch_:;
		}
		if (negative == buildingFlag)
			return false;
	}

	if (Owner_IsHuman.isset())
	{
		if (negative == pTarget->Owner->IsControlledByHuman())
			return false;
	}

	if (Owner_IsAI.isset())
	{
		if (negative != pTarget->Owner->IsControlledByHuman())
			return false;
	}

	if (IsBunkered.isset())
	{
		if (negative == (pTarget->BunkerLinkedItem != nullptr))
			return false;
	}

	if (IsMindControlled.isset())
	{
		if (negative == pTarget->IsMindControlled())
			return false;
	}

	if (IsMindControlled_Perma.isset())
	{
		if (negative == (pTarget->MindControlledByAUnit && !pTarget->MindControlledBy))
			return false;
	}

	if (MindControlling_Any.isset())
	{
		if (negative == (pTarget->CaptureManager && pTarget->CaptureManager->IsControllingSomething()))
			return false;
	}

	if (!MindControlling_Type.empty())
	{
		bool mcFlag = false;
		if ((pTarget->CaptureManager && pTarget->CaptureManager->IsControllingSomething()))
		{
			for (auto node : pTarget->CaptureManager->ControlNodes)
			{
				if (node->Unit && MindControlling_Type.Contains(node->Unit->GetTechnoType()))
				{
					mcFlag = true;
					break;
				}
			}
		}
		if (negative == mcFlag)
			return false;
	}

	if (Passengers_Any.isset() || !Passengers_Type.empty())
	{
		bool passengerAnyFlag = pTarget->Passengers.NumPassengers > 0;

		if (Passengers_Any.isset())
		{
			if (negative == (Passengers_Any.Get() == passengerAnyFlag))
				return false;
		}

		if (!Passengers_Type.empty())
		{
			bool passengerFlag = false;
			if (passengerAnyFlag)
			{
				for (NextObject obj(pTarget->Passengers.FirstPassenger->NextObject); obj; ++obj)
				{
					auto const pPassenger = abstract_cast<TechnoClass*>(*obj);
					if (Passengers_Type.Contains(pPassenger->GetTechnoType()))
					{
						passengerFlag = true;
						break;
					}
				}
			}
			if (negative == passengerFlag)
				return false;
		}
	}

	if (Upgrades_Any.isset() || !Upgrades_Type.empty())
	{
		bool upgradesAnyFlag = false;
		bool upgradesTypeFlag = Upgrades_Type.empty(); // if no upgrades types specified then skip upgrades type check

		if (auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
		{
			for (auto const& pBuildingUpgrade : pBuilding->Upgrades)
			{
				upgradesAnyFlag = true;
				upgradesTypeFlag = upgradesTypeFlag || Upgrades_Type.Contains(pBuildingUpgrade);
				if (upgradesAnyFlag && upgradesTypeFlag)
					break;
			}
		}

		if (Upgrades_Any.isset())
		{
			if (negative == (Upgrades_Any.Get() == upgradesAnyFlag))
				return false;
		}

		if (!Upgrades_Type.empty())
		{
			if (negative == upgradesTypeFlag)
				return false;
		}
	}

	return true;
}

bool HandlerFilterClass::IsDefined() const
{
	return Abstract.isset()
		|| IsInAir.isset()
		|| !TechnoTypes.empty()
		|| !AttachedEffects.empty()
		|| !ShieldTypes.empty()
		|| Veterancy.isset()
		|| HPPercentage.isset()
		|| Owner_House.isset()
		|| !Owner_Sides.empty()
		|| !Owner_Countries.empty()
		|| !Owner_Buildings.empty()
		|| Owner_IsHuman.isset()
		|| Owner_IsAI.isset()
		|| IsBunkered.isset()
		|| IsMindControlled.isset()
		|| IsMindControlled_Perma.isset()
		|| MindControlling_Any.isset()
		|| !MindControlling_Type.empty()
		|| Passengers_Any.isset()
		|| !Passengers_Type.empty()
		|| Upgrades_Any.isset()
		|| !Upgrades_Type.empty();
}

bool HandlerFilterClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool HandlerFilterClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<HandlerFilterClass*>(this)->Serialize(stm);
}

template<typename T>
bool HandlerFilterClass::Serialize(T& stm)
{
	return stm
		.Process(this->Abstract)
		.Process(this->IsInAir)
		.Process(this->TechnoTypes)
		.Process(this->AttachedEffects)
		.Process(this->ShieldTypes)
		.Process(this->Veterancy)
		.Process(this->HPPercentage)
		.Process(this->Owner_House)
		.Process(this->Owner_Sides)
		.Process(this->Owner_Countries)
		.Process(this->Owner_Buildings)
		.Process(this->Owner_IsHuman)
		.Process(this->Owner_IsAI)
		.Process(this->IsBunkered)
		.Process(this->IsMindControlled)
		.Process(this->IsMindControlled_Perma)
		.Process(this->MindControlling_Any)
		.Process(this->MindControlling_Type)
		.Process(this->Passengers_Any)
		.Process(this->Passengers_Type)
		.Process(this->Upgrades_Any)
		.Process(this->Upgrades_Type)
		.Success();
}
