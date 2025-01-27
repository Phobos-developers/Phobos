#include <Ext/Techno/Body.h>
#include "HandlerFilterClass.h"
#include <Ext/House/Body.h>

HandlerFilterClass::HandlerFilterClass()
	: HasAnyTechnoCheck { false }
#pragma region TechnoFilters
	, Abstract { }
	, IsInAir {}
	, TechnoTypes {}
	, AttachedEffects {}
	, AEStacks_Types {}
	, AEStacks_Methods {}
	, AEStacks_Values {}
	, ShieldTypes {}
	, Veterancy {}
	, HPPercentage {}
	, IsPassenger {}
	, IsParasited {}
	, IsParasiting {}
	, IsBunkered {}
	, IsMindControlled {}
	, IsMindControlled_Perma {}
	, MindControlling_Any {}
	, MindControlling_Type {}
	, Passengers_Any {}
	, Passengers_Type {}
	, Upgrades_Any {}
	, Upgrades_Type {}
#pragma endregion
#pragma region HouseFilters
	, HasAnyHouseCheck { false }
	, House {}
	, Sides {}
	, Countries {}
	, Buildings {}
	, Emblems {}
	, IsHuman {}
	, IsAI {}
	, IsLowPower {}
	, HouseCompare_Params {}
	, HouseCompare_Methods {}
	, HouseCompare_Values {}
#pragma endregion
{ }

std::unique_ptr<HandlerFilterClass> HandlerFilterClass::Parse(INI_EX & exINI, const char* pSection, const char* actorName, const char* filterName)
{
	auto filter = std::make_unique<HandlerFilterClass>();
	filter.get()->LoadFromINI(exINI, pSection, actorName, filterName);
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

void HandlerFilterClass::LoadFromINI(INI_EX& exINI, const char* pSection, const char* actorName, const char* filterName)
{
	char tempBuffer[64];

#pragma region TechnoFilters
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Abstract", actorName, filterName);
	Abstract.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsInAir", actorName, filterName);
	IsInAir.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.TechnoTypes", actorName, filterName);
	TechnoTypes.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.AttachedEffects", actorName, filterName);
	AttachedEffects.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.AEStacks.Types", actorName, filterName);
	AEStacks_Types.Read(exINI, pSection, tempBuffer);
	if (!AEStacks_Types.empty())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.AEStacks.Methods", actorName, filterName);
		AEStacks_Methods.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.AEStacks.Values", actorName, filterName);
		AEStacks_Values.Read(exINI, pSection, tempBuffer);
		// if the counts do not match then it is invalid input
		if (AEStacks_Types.size() != AEStacks_Methods.size()
			|| AEStacks_Types.size() != AEStacks_Values.size())
		{
			AEStacks_Types.clear();
			AEStacks_Methods.clear();
			AEStacks_Values.clear();
		}
	}
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.ShieldTypes", actorName, filterName);
	ShieldTypes.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Veterancy", actorName, filterName);
	Veterancy.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.HPPercentage", actorName, filterName);
	HPPercentage.Read(exINI, pSection, tempBuffer);

	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsPassenger", actorName, filterName);
	IsPassenger.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsParasited", actorName, filterName);
	IsParasited.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsParasiting", actorName, filterName);
	IsParasiting.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsBunkered", actorName, filterName);
	IsBunkered.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsMindControlled", actorName, filterName);
	IsMindControlled.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsMindControlled.Perma", actorName, filterName);
	IsMindControlled_Perma.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.MindControlling.Any", actorName, filterName);
	MindControlling_Any.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.MindControlling.Type", actorName, filterName);
	MindControlling_Type.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Any", actorName, filterName);
	Passengers_Any.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Type", actorName, filterName);
	Passengers_Type.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Upgrades.Any", actorName, filterName);
	Upgrades_Any.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Upgrades.Type", actorName, filterName);
	Upgrades_Type.Read(exINI, pSection, tempBuffer);

	HasAnyTechnoCheck = IsDefinedAnyTechnoCheck();
#pragma endregion

#pragma region HouseFilters
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.House", actorName, filterName);
	House.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Sides", actorName, filterName);
	Sides.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Countries", actorName, filterName);
	Countries.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Buildings", actorName, filterName);
	Buildings.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Emblems", actorName, filterName);
	Emblems.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsHuman", actorName, filterName);
	IsHuman.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsAI", actorName, filterName);
	IsAI.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.IsLowPower", actorName, filterName);
	IsLowPower.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.HouseCompare.Params", actorName, filterName);
	HouseCompare_Params.Read(exINI, pSection, tempBuffer);
	if (!HouseCompare_Params.empty())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.HouseCompare.Methods", actorName, filterName);
		HouseCompare_Methods.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.HouseCompare.Values", actorName, filterName);
		HouseCompare_Values.Read(exINI, pSection, tempBuffer);
		// if the counts do not match then it is invalid input
		if (HouseCompare_Params.size() != HouseCompare_Methods.size()
			|| HouseCompare_Params.size() != HouseCompare_Values.size())
		{
			HouseCompare_Params.clear();
			HouseCompare_Methods.clear();
			HouseCompare_Values.clear();
		}
	}

	HasAnyHouseCheck = IsDefinedAnyHouseCheck();
#pragma endregion
}

bool HandlerFilterClass::Check(HouseClass* pHouse, AbstractClass* pTarget, bool negative) const
{
	if (HasAnyTechnoCheck)
	{
		if (!pTarget)
			return false;
		if (auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
		{
			if (!CheckForTechno(pHouse, pTargetTechno, negative))
				return false;
		}
		else
		{
			return false;
		}
	}

	if (HasAnyHouseCheck)
	{
		if (!pTarget)
			return false;
		auto pTargetHouse = HandlerCompClass::GetOwningHouseOfActor(pTarget);
		if (!pTargetHouse)
			return false;
		if (!CheckForHouse(pHouse, pTargetHouse, negative))
			return false;
	}

	return true;
}

// If "negative == false", the function returns false if any check failed, otherwise it returns true.
// If "negative == true", the function returns false if any check succeed, otherwise it returns true.
// Thus, we simply compare "negative" with the outcome of an entry, if equals then return false.
bool HandlerFilterClass::CheckForTechno(HouseClass* pHouse, TechnoClass* pTarget, bool negative) const
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

	if (!AEStacks_Types.empty())
	{
		if (!pTargetExt)
			pTargetExt = TechnoExt::ExtMap.Find(pTarget);
		static std::map<AttachEffectTypeClass*, int> mapAE;
		for (auto const& attachEffect : pTargetExt->AttachedEffects)
		{
			if (attachEffect->IsActive())
				mapAE[attachEffect->GetType()]++;
		}
		for (size_t i = 0; i < AEStacks_Types.size(); i++)
		{
			auto const pAEType = AEStacks_Types.at(i);
			if (EnumFunctions::ParameterCompare(mapAE[pAEType],
				AEStacks_Methods.at(i),
				AEStacks_Values.at(i)) == negative)
			{
				return false;
			}
		}
		mapAE.clear();
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

	if (IsPassenger.isset())
	{
		if (negative == (IsPassenger.Get() == (HandlerCompClass::GetTransportingTechno(pTarget) != nullptr)))
			return false;
	}

	if (IsParasited.isset())
	{
		if (negative == (IsParasited.Get() == (HandlerCompClass::GetParasiteTechno(pTarget) != nullptr)))
			return false;
	}

	if (IsParasiting.isset())
	{
		if (negative == (IsParasiting.Get() == (HandlerCompClass::GetHostTechno(pTarget) != nullptr)))
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

	if (MindControlling_Any.isset() || !MindControlling_Type.empty())
	{
		bool mcAnyFlag = pTarget->CaptureManager && pTarget->CaptureManager->IsControllingSomething();

		if (MindControlling_Any.isset())
		{
			if (negative == (MindControlling_Any.Get() == mcAnyFlag))
				return false;
		}

		if (!MindControlling_Type.empty())
		{
			bool mcFlag = false;
			if (mcAnyFlag)
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
				for (NextObject obj(pTarget->Passengers.FirstPassenger); obj; ++obj)
				{
					auto const pPassenger = static_cast<TechnoClass*>(*obj);
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

bool HandlerFilterClass::CheckForHouse(HouseClass* pHouse, HouseClass* pTargetHouse, bool negative) const
{
	if (House.isset())
	{
		if (negative == EnumFunctions::CanTargetHouse(House.Get(), pHouse, pTargetHouse))
			return false;
	}

	if (!Sides.empty())
	{
		bool sideFlag = false;
		for (auto pSide : Sides)
		{
			if (pTargetHouse->Type->SideIndex == pSide->GetArrayIndex())
			{
				sideFlag = true;
				break;
			}
		}
		if (negative == sideFlag)
			return false;
	}

	if (!Countries.empty())
	{
		if (negative == Countries.Contains(pTargetHouse->Type))
			return false;
	}

	if (!Buildings.empty())
	{
		bool buildingFlag = false;
		if (pTargetHouse->OwnedBuildings > 0)
		{
			for (auto const& pBld : pTargetHouse->Buildings)
			{
				if (Buildings.Contains(pBld->Type))
				{
					buildingFlag = true;
					goto _EndOwnerBuildingSearch_;
				}
				for (const auto& pUpgrade : pBld->Upgrades)
				{
					if (Buildings.Contains(pUpgrade))
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

	if (!Emblems.empty())
	{
		bool emblemFlag = false;
		auto const pTargetHouseExt = HouseExt::ExtMap.Find(pTargetHouse);
		for (auto const pEmblemType : Emblems)
		{
			if (pTargetHouseExt->PlayerEmblems.contains(pEmblemType))
			{
				emblemFlag = true;
				goto _EndEmblemSearch_;
			}
		}
	_EndEmblemSearch_:;
		if (negative == emblemFlag)
			return false;
	}

	if (IsHuman.isset())
	{
		if (negative == pTargetHouse->IsControlledByHuman())
			return false;
	}

	if (IsAI.isset())
	{
		if (negative != pTargetHouse->IsControlledByHuman())
			return false;
	}

	if (IsLowPower.isset())
	{
		if (negative != (IsLowPower.Get() == pTargetHouse->HasLowPower()))
			return false;
	}

	if (!HouseCompare_Params.empty())
	{
		for (size_t i = 0; i < HouseCompare_Params.size(); i++)
		{
			if (EnumFunctions::HouseParameterCompare(pTargetHouse,
				HouseCompare_Params.at(i),
				HouseCompare_Methods.at(i),
				HouseCompare_Values.at(i)) == negative)
			{
				return false;
			}
		}
	}

	return true;
}

bool HandlerFilterClass::IsDefined() const
{
	return HasAnyTechnoCheck || HasAnyHouseCheck;
}

bool HandlerFilterClass::IsDefinedAnyTechnoCheck() const
{
	return Abstract.isset()
		|| IsInAir.isset()
		|| !TechnoTypes.empty()
		|| !AttachedEffects.empty()
		|| !AEStacks_Types.empty()
		|| !ShieldTypes.empty()
		|| Veterancy.isset()
		|| HPPercentage.isset()
		|| IsPassenger.isset()
		|| IsParasited.isset()
		|| IsParasiting.isset()
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

bool HandlerFilterClass::IsDefinedAnyHouseCheck() const
{
	return House.isset()
		|| !Sides.empty()
		|| !Countries.empty()
		|| !Buildings.empty()
		|| !Emblems.empty()
		|| IsHuman.isset()
		|| IsAI.isset()
		|| IsLowPower.isset()
		|| !HouseCompare_Params.empty();
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
#pragma region TechnoFilters
		.Process(this->HasAnyTechnoCheck)
		.Process(this->Abstract)
		.Process(this->IsInAir)
		.Process(this->TechnoTypes)
		.Process(this->AttachedEffects)
		.Process(this->AEStacks_Types)
		.Process(this->AEStacks_Methods)
		.Process(this->AEStacks_Values)
		.Process(this->ShieldTypes)
		.Process(this->Veterancy)
		.Process(this->HPPercentage)
		.Process(this->IsBunkered)
		.Process(this->IsMindControlled)
		.Process(this->IsMindControlled_Perma)
		.Process(this->MindControlling_Any)
		.Process(this->MindControlling_Type)
		.Process(this->Passengers_Any)
		.Process(this->Passengers_Type)
		.Process(this->Upgrades_Any)
		.Process(this->Upgrades_Type)
#pragma endregion
#pragma region HouseFilters
		.Process(this->HasAnyHouseCheck)
		.Process(this->House)
		.Process(this->Sides)
		.Process(this->Countries)
		.Process(this->Buildings)
		.Process(this->Emblems)
		.Process(this->IsHuman)
		.Process(this->IsAI)
		.Process(this->IsLowPower)
		.Process(this->HouseCompare_Params)
		.Process(this->HouseCompare_Methods)
		.Process(this->HouseCompare_Values)
#pragma endregion
		.Success();
}
