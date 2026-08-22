#include "Body.h"

DEFINE_HOOK(0x447358, BuildingClass_MouseOverObject_DeployFire, 0x6)
{
	GET(BuildingTypeClass* const, pType, EAX);

	return pType->DeployFire ? 0x4472EC : 0;
}

DEFINE_HOOK(0x443459, BuildingClass_ObjectClickedAction_DeployFire, 0x6)
{
	GET(BuildingClass*, pThis, EBX);
	enum { SkipGameCode = 0x443568, SkipFactory = 0x4434F2 };

	auto const pType = pThis->Type;

	// Perhaps Factory should not allow the use of DeployFire.
	if (pType->Factory != AbstractType::None)
	{
		if (!pThis->IsPrimaryFactory)
		{
			// Do not enter unloading tasks simultaneously, as this may prevent buildings from producing units in a timely manner.
			pThis->ClickedEvent(EventType::Primary);
			return SkipGameCode;
		}
	}
	else if (pType->DeployFire)
	{
		pThis->ClickedMission(Mission::Unload, pThis, nullptr, nullptr);
		return SkipGameCode;
	}

	return SkipFactory;
}

DEFINE_HOOK(0x44E371, BuildingClass_Mission_Unload_DeployFire, 0x6)
{
	GET(BuildingClass* const, pThis, EBP);
	enum { SkipGameCode = 0x44E37F };

	auto const pType = pThis->Type;

	if (!pType->GapGenerator && pType->DeployFire)
	{
		auto const pCell = pThis->GetCell();

		if (pThis->Target != pCell)
			pThis->SetTarget(pCell);

		const int deployFireWeapon = pType->DeployFireWeapon;
		const int weaponIndex = deployFireWeapon >= 0 ? deployFireWeapon : pThis->SelectWeapon(pCell);
		const FireError fireError = pThis->GetFireError(pCell, weaponIndex, true);

		if (fireError == FireError::ILLEGAL)
		{
			// Do not allow the building to remain in the Unload task indefinitely.
			pThis->ForceMission(Mission::Guard);
			return SkipGameCode;
		}
		else if (fireError == FireError::OK && pThis->Fire(pCell, weaponIndex))
		{
			auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

			if (pWeapon->FireOnce)
			{
				// When Turret=yes, the Unload task may not be canceled, so this handling is performed.
				pThis->ForceMission(Mission::Guard);
				return SkipGameCode;
			}
		}

		auto const pTypeExt = BuildingTypeExt::Fetch(pType);
		const int result = pTypeExt->DeployFireDelay.isset()
			? pTypeExt->DeployFireDelay.Get() : (ScenarioClass::Instance->Random.RandomRanged(0, 2) + 14);

		R->EBX(result);
		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x730B09, DeployCommandClass_Execute_BuildingDeploy, 0x5)
{
	for (const auto pObject : ObjectClass::CurrentObjects)
	{
		const AbstractFlags flags = pObject->AbstractFlags;

		if (!(flags & AbstractFlags::Techno) || (flags & AbstractFlags::Foot))
			continue;

		const auto pBuilding = static_cast<BuildingClass*>(pObject);
		auto const pType = pBuilding->Type;
		const auto pHouse = pBuilding->Owner;

		if (!pHouse->IsControlledByCurrentPlayer()
			|| !pType->DeployFire || pType->Factory != AbstractType::None || pType->GapGenerator)
		{
			continue;
		}

		if (!BuildingExt::BuildingOnline(pBuilding))
			continue;

		pBuilding->ClickedMission(Mission::Unload, nullptr, nullptr, nullptr);
	}

	return 0;
}
