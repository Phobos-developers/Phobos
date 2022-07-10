#include <HouseClass.h>
#include <SuperClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/SWType/Body.h>

#include <Utilities/Macro.h>

// most code copy from Ares 0.A
// abandon Ares hook
// 50AF10 = HouseClass_UpdateSuperWeaponsOwned, 5
// 50B1D0 = HouseClass_UpdateSuperWeaponsUnavailable, 6

struct SWStatus
{
	bool Available;
	bool PowerSourced;
	bool Charging;
};

void __forceinline UpdateStatus(BuildingClass* pBuilding, SWStatus& status, int idxSW)
{
	if (idxSW > -1)
	{
		status.Available = true;

		if (!status.Charging && pBuilding->HasPower && !pBuilding->IsUnderEMP())
		{
			status.PowerSourced = true;

			if (!pBuilding->IsBeingWarpedOut()
				&& (pBuilding->CurrentMission != Mission::Construction)
				&& (pBuilding->CurrentMission != Mission::Selling)
				&& (pBuilding->QueuedMission != Mission::Construction)
				&& (pBuilding->QueuedMission != Mission::Selling))
			{
				status.Charging = true;
			}
		}
	}
}

std::vector<SWStatus> __forceinline GetSuperWeaponStatuses(HouseClass* pHouse, std::vector<SWStatus>& vStatus)
{
	vStatus = std::move(std::vector<SWStatus>(pHouse->Supers.Count));

	if (!pHouse->Defeated)
	{
		for (BuildingClass* pBuilding : pHouse->Buildings)
		{
			if (pBuilding->IsAlive && !pBuilding->InLimbo)
			{
				BuildingTypeClass* pBuildingType = pBuilding->Type;
				BuildingTypeExt::ExtData* pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType);

				for (const BuildingTypeClass* pUpgrade : pBuilding->Upgrades)
				{
					if (pUpgrade != nullptr)
					{
						BuildingTypeExt::ExtData* pUpgradeExt = BuildingTypeExt::ExtMap.Find(pUpgrade);
						int swCount = pUpgradeExt->GetSWCount();

						for (int i = 0; i < swCount; i++)
						{
							int idx = pUpgradeExt->GetSWidx(i, pHouse);
							UpdateStatus(pBuilding, vStatus[idx], idx);
						}
					}
				}

				int swCount = pBuildingTypeExt->GetSWCount();

				for (int i = 0; i < swCount; i++)
				{
					int idx = pBuildingTypeExt->GetSWidx(i, pHouse);
					UpdateStatus(pBuilding, vStatus[idx], idx);
				}
			}
		}

		bool hasPower = pHouse->HasFullPower();

		for (SuperClass* pSuper : pHouse->Supers)
		{
			int idx = pSuper->Type->ArrayIndex;
			SWStatus& status = vStatus[idx];
			SuperWeaponTypeClass* pSWType = pSuper->Type;
			SWTypeExt::ExtData* pSWTypeExt = SWTypeExt::ExtMap.Find(pSWType);

			if (SessionClass::Instance->GameMode != GameMode::Campaign && !Unsorted::SWAllowed)
			{
				if (pSuper->Type->DisableableFromShell)
					status.Available = false;
			}

			if (pSWTypeExt->SW_AlwaysGranted && pSWTypeExt->IsAvailable(pHouse))
			{
				status.Available = true;
				status.Charging = true;
				status.PowerSourced = true;
			}

			if (pSuper->IsPowered())
				status.PowerSourced &= hasPower;
		}
	}

	return vStatus;
}

void __forceinline UpdateSuperWeaponsOwned(HouseClass* pHouse, std::vector<SWStatus>& vStatus)
{
	for (SuperClass* pSuper : pHouse->Supers)
	{
		if (pSuper->Granted)
		{
			SuperWeaponTypeClass* pType = pSuper->Type;
			int idx = pType->ArrayIndex;
			SWStatus& status = vStatus[idx];

			// is this a super weapon to be updated?
			// sw is bound to a building and no single-shot => create goody otherwise
			bool isCreateGoody = (!pSuper->CanHold || pSuper->OneTime);

			if (!isCreateGoody || pHouse->Defeated)
			{

				// shut down or power up super weapon and decide whether
				// a sidebar tab update is needed.
				bool update = false;
				if (!status.Available || pHouse->Defeated)
				{
					update = (pSuper->Lose() && HouseClass::Player);
				}
				else if (status.Charging && !pSuper->IsPowered())
				{
					update = pSuper->IsOnHold && pSuper->SetOnHold(false);
				}
				else if (!status.Charging && !pSuper->IsPowered())
				{
					update = !pSuper->IsOnHold && pSuper->SetOnHold(true);
				}
				else if (!status.PowerSourced)
				{
					update = (pSuper->IsPowered() && pSuper->SetOnHold(true));
				}
				else
				{
					update = (status.PowerSourced && pSuper->SetOnHold(false));
				}

				// update only if needed.
				if (update)
				{
					// only the human player can see the sidebar.
					if (pHouse->IsPlayer())
					{
						if (Unsorted::CurrentSWType == idx)
						{
							Unsorted::CurrentSWType = -1;
						}

						int idxTab = SidebarClass::GetObjectTabIdx(SuperClass::AbsID, idx, 0);
						MouseClass::Instance->RepaintSidebar(idxTab);
					}

					pHouse->RecheckTechTree = true;
				}
			}
		}
	}
}

void __forceinline UpdateSuperWeaponsUnavailable(HouseClass* pHouse, std::vector<SWStatus>& vStatus)
{
	if (!pHouse->Defeated)
	{
		// update all super weapons not repeatedly available
		for (SuperClass* pSuper : pHouse->Supers)
		{
			if (!pSuper->Granted || pSuper->OneTime)
			{
				int idx = pSuper->Type->ArrayIndex;
				SWStatus& status = vStatus[idx];

				if (status.Available)
				{
					pSuper->Grant(false, pHouse->IsPlayer(), !status.PowerSourced);

					if (pHouse->IsPlayer())
					{
						// hide the cameo (only if this is an auto-firing SW)
						SWTypeExt::ExtData* pSWTypeExt = SWTypeExt::ExtMap.Find(pSuper->Type);

						if (pSWTypeExt->SW_ShowCameo || !pSWTypeExt->SW_AutoFire)
						{
							MouseClass::Instance->AddCameo(AbstractType::Special, idx);
							int idxTab = SidebarClass::GetObjectTabIdx(SuperClass::AbsID, idx, 0);
							MouseClass::Instance->RepaintSidebar(idxTab);
						}
					}
				}
			}
		}
	}
}


static void __fastcall HouseClass_UpdateSuperWeaponsOwned(HouseClass* pThis)
{
	std::vector<SWStatus> vStatus;
	GetSuperWeaponStatuses(pThis, vStatus);
	UpdateSuperWeaponsOwned(pThis, vStatus);
}

DEFINE_JUMP(CALL, 0x43BEF0, GET_OFFSET(HouseClass_UpdateSuperWeaponsOwned));
DEFINE_JUMP(CALL, 0x451700, GET_OFFSET(HouseClass_UpdateSuperWeaponsOwned));
DEFINE_JUMP(CALL, 0x451739, GET_OFFSET(HouseClass_UpdateSuperWeaponsOwned));
DEFINE_JUMP(CALL, 0x4F92F6, GET_OFFSET(HouseClass_UpdateSuperWeaponsOwned));
DEFINE_JUMP(CALL, 0x508DDB, GET_OFFSET(HouseClass_UpdateSuperWeaponsOwned));

static void __fastcall HouseClass_UpdateSuperWeaponsUnavailable(HouseClass* pThis)
{
	if (!pThis->Defeated)
	{
		std::vector<SWStatus> vStatus;
		GetSuperWeaponStatuses(pThis, vStatus);
		UpdateSuperWeaponsUnavailable(pThis, vStatus);
	}
}

DEFINE_JUMP(CALL, 0x4409EF, GET_OFFSET(HouseClass_UpdateSuperWeaponsUnavailable));
DEFINE_JUMP(CALL, 0x4F92FD, GET_OFFSET(HouseClass_UpdateSuperWeaponsUnavailable));
