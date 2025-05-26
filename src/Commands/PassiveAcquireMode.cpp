#include "PassiveAcquireMode.h"

#include "Ext/Techno/Body.h"
#include <Ext/Event/Body.h>

const char* AggressiveModeClass::GetName() const
{
	return "AggressiveMode";
}

const wchar_t* AggressiveModeClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AGGRESSIVE_MODE", L"Aggressive Mode");
}

const wchar_t* AggressiveModeClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* AggressiveModeClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AGGRESSIVE_MODE_DESC", L"Aggressive Mode");
}

void AggressiveModeClass::Execute(WWKey eInput) const
{
	std::vector<TechnoClass*> TechnoVectorAggressive;
	std::vector<TechnoClass*> TechnoVectorNonAggressive;

	// Get current selected units.
	// If all selected units are at Aggressive mode, we should cancel their Aggressive mode.
	// Otherwise, we should turn them into Aggressive mode.
	bool isAnySelectedUnitTogglable = false;
	bool isAllSelectedUnitAggressiveMode = true;

	auto processATechno = [&](TechnoClass* pTechno)
		{
			const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

			// If not togglable then exclude it from the iteration.
			if (!pTechnoExt->CanTogglePassiveAcquireMode())
				return;

			isAnySelectedUnitTogglable = true;

			if (pTechnoExt->GetPassiveAcquireMode() == PassiveAcquireMode::Aggressive)
			{
				TechnoVectorAggressive.push_back(pTechno);
			}
			else
			{
				isAllSelectedUnitAggressiveMode = false;
				TechnoVectorNonAggressive.push_back(pTechno);
			}
			return;
		};

	for (const auto& pUnit : ObjectClass::CurrentObjects)
	{
		// try to cast to TechnoClass
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pUnit);

		// if not a techno or is in berserk or is not controlled by the local player then ignore it
		if (!pTechno || pTechno->Berzerk || !pTechno->Owner->IsControlledByCurrentPlayer())
			continue;

		processATechno(pTechno);

		if (auto pPassenger = pTechno->Passengers.GetFirstPassenger())
		{
			for (; pPassenger; pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject))
				processATechno(pPassenger);
		}

		if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		{
			for (auto pOccupier : pBuilding->Occupants)
				processATechno(pOccupier);
		}
	}

	// If this boolean is false, then none of the selected units are togglable, meaning this hotket doesn't need to do anything.
	if (isAnySelectedUnitTogglable)
	{
		// If all selected units are Aggressive mode, then cancel their Aggressive mode;
		// otherwise, make all selected units Aggressive mode.
		if (isAllSelectedUnitAggressiveMode)
		{
			for (const auto& pTechno : TechnoVectorAggressive)
				EventExt::RaiseTogglePassiveAcquireMode(pTechno, PassiveAcquireMode::Normal);

			wchar_t buffer[0x100];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("MSG:AGGRESSIVE_MODE_OFF", L"%i unit(s) ceased Aggressive Mode."), TechnoVectorAggressive.size());
			MessageListClass::Instance.PrintMessage(buffer);
		}
		else
		{
			for (const auto& pTechno : TechnoVectorNonAggressive)
				EventExt::RaiseTogglePassiveAcquireMode(pTechno, PassiveAcquireMode::Aggressive);

			wchar_t buffer[0x100];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("MSG:AGGRESSIVE_MODE_ON", L"%i unit(s) entered Aggressive Mode."), TechnoVectorNonAggressive.size());
			MessageListClass::Instance.PrintMessage(buffer);
		}
	}
}

const char* CeaseFireModeClass::GetName() const
{
	return "CeaseFireMode";
}

const wchar_t* CeaseFireModeClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CEASEFIRE_MODE", L"Cease Fire Mode");
}

const wchar_t* CeaseFireModeClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* CeaseFireModeClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CEASEFIRE_MODE_DESC", L"Cease Fire Mode");
}

void CeaseFireModeClass::Execute(WWKey eInput) const
{
	std::vector<TechnoClass*> TechnoVectorCeaseFire;
	std::vector<TechnoClass*> TechnoVectorNonCeaseFire;

	// Get current selected units.
	// If all selected units are at CeaseFire mode, we should cancel their CeaseFire mode.
	// Otherwise, we should turn them into CeaseFire mode.
	bool isAnySelectedUnitTogglable = false;
	bool isAllSelectedUnitCeaseFireMode = true;

	auto processATechno = [&](TechnoClass* pTechno)
		{
			const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

			// If not togglable then exclude it from the iteration.
			if (!pTechnoExt->CanTogglePassiveAcquireMode())
				return;

			isAnySelectedUnitTogglable = true;

			if (pTechnoExt->GetPassiveAcquireMode() == PassiveAcquireMode::CeaseFire)
			{
				TechnoVectorCeaseFire.push_back(pTechno);
			}
			else
			{
				isAllSelectedUnitCeaseFireMode = false;
				TechnoVectorNonCeaseFire.push_back(pTechno);
			}
			return;
		};

	for (const auto& pUnit : ObjectClass::CurrentObjects)
	{
		// try to cast to TechnoClass
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pUnit);

		// if not a techno or is in berserk or is not controlled by the local player then ignore it
		if (!pTechno || pTechno->Berzerk || !pTechno->Owner->IsControlledByCurrentPlayer())
			continue;

		processATechno(pTechno);

		if (auto pPassenger = pTechno->Passengers.GetFirstPassenger())
		{
			for (; pPassenger; pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject))
				processATechno(pPassenger);
		}

		if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		{
			for (auto pOccupier : pBuilding->Occupants)
				processATechno(pOccupier);
		}
	}

	// If this boolean is false, then none of the selected units are togglable, meaning this hotket doesn't need to do anything.
	if (isAnySelectedUnitTogglable)
	{
		// If all selected units are CeaseFire mode, then cancel their CeaseFire mode;
		// otherwise, make all selected units CeaseFire mode.
		if (isAllSelectedUnitCeaseFireMode)
		{
			for (const auto& pTechno : TechnoVectorCeaseFire)
				EventExt::RaiseTogglePassiveAcquireMode(pTechno, PassiveAcquireMode::Normal);

			wchar_t buffer[0x100];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("MSG:CEASEFIRE_MODE_OFF", L"%i unit(s) ceased Cease Fire Mode."), TechnoVectorCeaseFire.size());
			MessageListClass::Instance.PrintMessage(buffer);
		}
		else
		{
			for (const auto& pTechno : TechnoVectorNonCeaseFire)
				EventExt::RaiseTogglePassiveAcquireMode(pTechno, PassiveAcquireMode::CeaseFire);

			wchar_t buffer[0x100];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("MSG:CEASEFIRE_MODE_ON", L"%i unit(s) entered Cease Fire Mode."), TechnoVectorNonCeaseFire.size());
			MessageListClass::Instance.PrintMessage(buffer);
		}
	}
}
