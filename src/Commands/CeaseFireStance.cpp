#include "CeaseFireStance.h"

#include "Ext/Techno/Body.h"
#include <Ext/Event/Body.h>

const char* CeaseFireStanceClass::GetName() const
{
	return "CeaseFireStance";
}

const wchar_t* CeaseFireStanceClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CEASEFIRE_STANCE", L"Cease Fire Stance");
}

const wchar_t* CeaseFireStanceClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* CeaseFireStanceClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CEASEFIRE_STANCE_DESC", L"Cease Fire Stance");
}

void CeaseFireStanceClass::Execute(WWKey eInput) const
{
	std::vector<TechnoClass*> TechnoVectorCeaseFire;
	std::vector<TechnoClass*> TechnoVectorNonCeaseFire;

	// Get current selected units.
	// If all selected units are at CeaseFire stance, we should cancel their CeaseFire stance.
	// Otherwise, we should turn them into CeaseFire stance.
	bool isAnySelectedUnitTogglable = false;
	bool isAllSelectedUnitCeaseFireStance = true;

	auto processATechno = [&](TechnoClass* pTechno)
		{
			const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

			// If not togglable then exclude it from the iteration.
			if (!pTechnoExt->CanToggleCeaseFireStance())
				return;

			isAnySelectedUnitTogglable = true;

			if (pTechnoExt->GetCeaseFireStance())
			{
				TechnoVectorCeaseFire.push_back(pTechno);
			}
			else
			{
				isAllSelectedUnitCeaseFireStance = false;
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
		// If all selected units are CeaseFire stance, then cancel their CeaseFire stance;
		// otherwise, make all selected units CeaseFire stance.
		if (isAllSelectedUnitCeaseFireStance)
		{
			for (const auto& pTechno : TechnoVectorCeaseFire)
				EventExt::RaiseToggleCeaseFireStance(pTechno);

			wchar_t buffer[0x100];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("MSG:CeaseFire_STANCE_OFF", L"%i unit(s) ceased CeaseFire Stance."), TechnoVectorCeaseFire.size());
			MessageListClass::Instance.PrintMessage(buffer);
		}
		else
		{
			for (const auto& pTechno : TechnoVectorNonCeaseFire)
				EventExt::RaiseToggleCeaseFireStance(pTechno);

			wchar_t buffer[0x100];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("MSG:CeaseFire_STANCE_ON", L"%i unit(s) entered CeaseFire Stance."), TechnoVectorNonCeaseFire.size());
			MessageListClass::Instance.PrintMessage(buffer);
		}
	}
}
