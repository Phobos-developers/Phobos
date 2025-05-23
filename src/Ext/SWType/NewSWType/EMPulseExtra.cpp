#include "EMPulseExtra.h"

#include <Ext/House/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/TemplateDef.h>


const char* EMPulseExtra::GetTypeID()
{
	return "EMPulseExtra";
}

void EMPulseExtra::LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI)
{
	INI_EX exINI(pINI);
	const char* pSection = pSW->get_ID();

	//From Ares 0.A
	pData->EMPulse_Cannons.Read(exINI, pSection, "EMPulse.Cannons");
	pData->EMPulse_Linked.Read(exINI, pSection, "EMPulse.Linked");
	pData->EMPulse_TargetSelf.Read(exINI, pSection, "EMPulse.TargetSelf");
	pData->EMPulse_IgnoreMission.Read(exINI, pSection, "EMPulse.IgnoreMission");
}

bool EMPulseExtra::Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer)
{
	HouseClass* pHouse = pSW->Owner;
	SWTypeExt::ExtData* pSWTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);

	for (TechnoTypeClass* pType : pSWTypeExt->EMPulse_Cannons)
	{
		ProcessEMPulseCannon(HouseExt::GetOwnedTechno(pHouse, pType), pSW, cell);
	}

	if (isPlayer && pHouse->IsControlledByCurrentPlayer())
		Unsorted::CurrentSWType = -1;

	return true;
}

inline void EMPulseExtra::ProcessEMPulseCannon(const std::vector<TechnoClass*>& technos, SuperClass* pSW, const CellStruct& cell)
{
	for (TechnoClass* pTechno : technos)
	{
		FireEMPulse(pTechno, pSW, cell);
	}
}

inline void EMPulseExtra::FireEMPulse(TechnoClass* pFirer, SuperClass* pSW, const CellStruct& cell)
{
	if (!TechnoExt::IsActivePower(pFirer))
		return;

	SWTypeExt::ExtData* pSWTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);

	if (!pSWTypeExt->EMPulse_IgnoreMission && pFirer->GetCurrentMission() != Mission::Guard)
		return;

	TechnoExt::ExtData* pFirerExt = TechnoExt::ExtMap.Find(pFirer);

	if (pFirerExt->CurrentFiringSW != nullptr)
		return;

	pFirerExt->CurrentFiringSW = pSW;

	WeaponStruct* pWeapon = pFirer->GetWeapon(0);

	if (pWeapon == nullptr || pWeapon->WeaponType == nullptr)
	{
		pFirerExt->CurrentFiringSW = nullptr;

		return;
	}

	WeaponTypeClass* pWeaponType = pWeapon->WeaponType;

	if (pSWTypeExt->EMPulse_TargetSelf)
	{
		WeaponTypeExt::DetonateAt(pWeaponType, pFirer->GetCenterCoords(), pFirer);

		pFirerExt->CurrentFiringSW = nullptr;

		return;
	}

	AbstractClass* pCell = MapClass::Instance.TryGetCellAt(cell);

	int distance = pFirer->DistanceFrom(pCell);

	if (distance < pWeaponType->MinimumRange || distance > pWeaponType->Range)
	{
		pFirerExt->CurrentFiringSW = nullptr;

		return;
	}

	pFirer->ReceiveCommand(pFirer, RadioCommand::RequestAttack, pCell);
}
