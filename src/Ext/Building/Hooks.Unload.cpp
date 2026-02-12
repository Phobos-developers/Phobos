#include "Body.h"

DEFINE_HOOK(0x447358, BuildingClass_MouseOverObject_DeployFire, 0x6)
{
	GET(BuildingTypeClass* const, pType, EAX);

	return pType->DeployFire ? 0x4472EC : 0;
}

DEFINE_HOOK(0x4434FF, BuildingClass_ObjectClickedAction_DeployFire, 0x6)
{
	GET(BuildingTypeClass* const, pType, EAX);

	return pType->DeployFire ? 0x443509 : 0;
}

DEFINE_HOOK(0x44E29D, BuildingClass_Mission_Unload_DeployFire, 0x6)
{
	GET(BuildingClass* const, pThis, EBP);
	GET(BuildingTypeClass* const, pType, EAX);
	enum { SkipGameCode = 0x44E37F, ReturnGuard = 0x44E371, Continue = 0x44E2BE };

	if (!pType->GapGenerator || !pType->SuperGapRadiusInCells)
	{
		if (pType->DeployFire)
		{
			auto const pCell = pThis->GetCell();

			if (pThis->Target != pCell)
				pThis->SetTarget(pCell);

			const int deployFireWeapon = pType->DeployFireWeapon;
			const int weaponIndex = deployFireWeapon >= 0 ? deployFireWeapon : pThis->SelectWeapon(pCell);

			if (pThis->GetFireError(pCell, weaponIndex, true) == FireError::OK &&
				pThis->Fire(pCell, weaponIndex))
			{
				auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

				if (pWeapon->FireOnce)
					return ReturnGuard;
			}

			R->EBX(ScenarioClass::Instance->Random.RandomRanged(0, 2) + 14);
			return SkipGameCode;
		}

		return ReturnGuard;
	}

	return Continue;
}

DEFINE_HOOK(0x730B09, DeployCommandClass_Execute_BuildingDeploy, 0x5)
{
	for (const auto pObject : ObjectClass::CurrentObjects)
	{
		const AbstractFlags flags = pObject->AbstractFlags;

		if (!(flags & AbstractFlags::Techno) || (flags & AbstractFlags::Foot))
			continue;

		const auto pBuilding = static_cast<BuildingClass*>(pObject);
		const auto pHouse = pBuilding->Owner;

		if (!pHouse->IsControlledByCurrentPlayer() || !pBuilding->Type->DeployFire)
			continue;

		const Mission currentMission = pBuilding->CurrentMission;

		if (currentMission == Mission::Construction || currentMission == Mission::Selling)
			continue;

		if (pBuilding->EMPLockRemaining > 0 || !pBuilding->WasOnline || pBuilding->BunkerLinkedItem)
			continue;

		pBuilding->ClickedMission(Mission::Unload, nullptr, nullptr, nullptr);
	}

	return 0;
}
