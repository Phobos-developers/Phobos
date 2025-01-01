#include <Ext/Techno/Body.h>
#include "HandlerFilterClass.h"

HandlerFilterClass::HandlerFilterClass(EventHandlerTypeClass* OwnedBy)
	: OwnerType { OwnedBy }
	, House {}
	, TechnoTypes {}
	, AttachedEffects {}
	, ShieldTypes {}
	, Side {}
	, Country {}
	, Veterancy {}
	, HPPercentage {}
	, Passengers_HasAny {}
	, Passengers_HasType {}
{ }

std::unique_ptr<HandlerFilterClass> HandlerFilterClass::Parse(EventHandlerTypeClass * OwnedBy, INI_EX & exINI, const char* pSection, const char* scopeName, const char* filterName)
{
	auto filter = std::make_unique<HandlerFilterClass>(OwnedBy);
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
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.House", scopeName, filterName);
	House.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.TechnoTypes", scopeName, filterName);
	TechnoTypes.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.AttachedEffects", scopeName, filterName);
	AttachedEffects.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.ShieldTypes", scopeName, filterName);
	ShieldTypes.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Side", scopeName, filterName);
	Side.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Country", scopeName, filterName);
	Country.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Veterancy", scopeName, filterName);
	Veterancy.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.HPPercentage", scopeName, filterName);
	HPPercentage.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers_HasAny", scopeName, filterName);
	Passengers_HasAny.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers_HasType", scopeName, filterName);
	Passengers_HasType.Read(exINI, pSection, tempBuffer);
}

// If "negative == false", the function returns false if any check failed, otherwise it returns true.
// If "negative == true", the function returns false if any check succeed, otherwise it returns true.
// Thus, we simply compare "negative" with the outcome of an entry, if equals then return false.
bool HandlerFilterClass::Check(TechnoClass* pOwner, TechnoClass* pTarget, bool negative) const
{
	TechnoExt::ExtData* pTargetExt = nullptr;

	if (House.isset())
	{
		if (negative == EnumFunctions::CanTargetHouse(House.Get(), pOwner->Owner, pTarget->Owner))
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

	if (!Side.empty())
	{
		bool sideFlag = false;
		for (auto pSide : Side)
		{
			if (pTarget->Owner->Type->SideIndex == pSide->GetArrayIndex()) {
				sideFlag = true;
				break;
			}
		}
		if (negative == sideFlag)
			return false;
	}

	if (!Country.empty())
	{
		if (negative == Country.Contains(pTarget->Owner->Type))
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

	if (Passengers_HasAny.isset())
	{
		if (negative == (Passengers_HasAny.Get() == pTarget->Passengers.NumPassengers > 0))
			return false;
	}

	if (!Passengers_HasType.empty())
	{
		bool passengerFlag = false;
		if (pTarget->Passengers.NumPassengers > 0)
		{
			for (NextObject obj(pTarget->Passengers.FirstPassenger->NextObject); obj; ++obj)
			{
				auto const passenger = abstract_cast<TechnoClass*>(*obj);
				if (Passengers_HasType.Contains(passenger->GetTechnoType()))
				{
					passengerFlag = true;
					break;
				}
			}
		}
		if (negative == passengerFlag)
			return false;
	}

	return true;
}

bool HandlerFilterClass::IsDefined() const
{
	return House.isset()
		|| !TechnoTypes.empty()
		|| !AttachedEffects.empty()
		|| !ShieldTypes.empty()
		|| !Side.empty()
		|| !Country.empty()
		|| Veterancy.isset()
		|| HPPercentage.isset()
		|| Passengers_HasAny.isset()
		|| !Passengers_HasType.empty();
}

template<typename T>
bool HandlerFilterClass::Serialize(T& stm)
{
	return stm
		.Process(this->House)
		.Process(this->TechnoTypes)
		.Process(this->AttachedEffects)
		.Process(this->ShieldTypes)
		.Process(this->Side)
		.Process(this->Country)
		.Process(this->Veterancy)
		.Process(this->HPPercentage)
		.Process(this->Passengers_HasAny)
		.Process(this->Passengers_HasType)
		.Success();
}
