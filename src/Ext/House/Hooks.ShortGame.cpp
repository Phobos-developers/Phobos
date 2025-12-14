#include <Ext/House/Body.h>

// Fix for Ares bug where deployed Slave Miner prevents defeat in Short Game mode
// Ares hooks at 0x4F8EBD and checks OwnedBuildings, but doesn't account for
// buildings that can undeploy (like deployed Slave Miner) in Short Game mode.
DWORD __cdecl HouseClass_Update_HasBeenDefeated(int param_1)
{
	enum { ReturnNotDefeated = 0x4F8F87 };

	auto* const pThis = *reinterpret_cast<HouseClass**>(param_1 + 0xC);

	auto Eligible = [pThis](FootClass* pFoot) -> bool
	{
		if (pFoot->Owner != pThis)
			return false;

		if (!pFoot->InLimbo)
			return true;

		return pFoot->ParasiteImUsing != nullptr;
	};

	if (GameModeOptionsClass::Instance.ShortGame)
	{
		if (pThis->OwnedBuildings > 0)
		{
			for (auto const& pBld : pThis->Buildings)
			{
				const auto pUndeploysInto = pBld->Type->UndeploysInto;

				if (!pUndeploysInto)
					return ReturnNotDefeated;

				for (auto const pBaseUnit : RulesClass::Instance->BaseUnit)
				{
					if (pUndeploysInto == pBaseUnit)
						return ReturnNotDefeated;
				}
			}
		}

		for (auto const pBaseUnit : RulesClass::Instance->BaseUnit)
		{
			if (pThis->OwnedUnitTypes.GetItemCount(pBaseUnit->ArrayIndex) > 0)
				return ReturnNotDefeated;
		}
	}
	else
	{
		if (pThis->OwnedBuildings)
			return ReturnNotDefeated;

		if (pThis->OwnedUnitTypes.Total)
		{
			for (auto const pTechno : UnitClass::Array)
			{
				if (Eligible(pTechno))
					return ReturnNotDefeated;
			}
		}

		if (pThis->OwnedInfantryTypes.Total)
		{
			for (auto const pTechno : InfantryClass::Array)
			{
				if (Eligible(pTechno))
					return ReturnNotDefeated;
			}
		}

		if (pThis->OwnedAircraftTypes.Total)
		{
			for (auto const pTechno : AircraftClass::Array)
			{
				if (Eligible(pTechno))
					return ReturnNotDefeated;
			}
		}
	}

	pThis->DestroyAll();
	pThis->AcceptDefeat();

	return ReturnNotDefeated;
}
