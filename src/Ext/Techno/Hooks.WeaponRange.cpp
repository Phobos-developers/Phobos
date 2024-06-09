#include "Body.h"

#include <AircraftClass.h>

#include <Ext/WeaponType/Body.h>

// Reimplements the game function with few changes / optimizations
DEFINE_HOOK(0x7012C2, TechnoClass_WeaponRange, 0x8)
{
	enum { ReturnResult = 0x70138F };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, STACK_OFFSET(0x8, 0x4));

	int result = 0;
	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	if (pWeapon)
	{
		result = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis);
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pThis->GetTechnoType()->OpenTopped && !pTypeExt->OpenTopped_IgnoreRangefinding)
		{
			int smallestRange = INT32_MAX;
			auto pPassenger = abstract_cast<FootClass*>(pThis->Passengers.FirstPassenger);

			while (pPassenger)
			{
				int openTWeaponIndex = pPassenger->GetTechnoType()->OpenTransportWeapon;
				int tWeaponIndex = 0;

				if (openTWeaponIndex != -1)
					tWeaponIndex = openTWeaponIndex;
				else
					tWeaponIndex = pPassenger->SelectWeapon(pThis->Target);

				WeaponTypeClass* pTWeapon = pPassenger->GetWeapon(tWeaponIndex)->WeaponType;

				if (pTWeapon && pTWeapon->FireInTransport)
				{
					int range = WeaponTypeExt::GetRangeWithModifiers(pTWeapon, pPassenger);

					if (range < smallestRange)
						smallestRange = range;
				}

				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
			}

			if (result > smallestRange)
				result = smallestRange;
		}
	}

	R->EBX(result);
	return ReturnResult;
}

DEFINE_HOOK(0x6F7248, TechnoClass_InRange_WeaponRange, 0x6)
{
	enum { SkipGameCode = 0x6F724E };

	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	R->EDI(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis));

	return SkipGameCode;
}

DEFINE_HOOK(0x6F7294, TechnoClass_InRange_OccupyRange, 0x5)
{
	enum { SkipGameCode = 0x6F729F };

	GET(TechnoClass*, pThis, ESI);
	GET(int, range, EDI);

	int occupyRange = WeaponTypeExt::GetRangeWithModifiers(nullptr, pThis);
	occupyRange /= Unsorted::LeptonsPerCell;

	R->EDI(range + occupyRange);

	return SkipGameCode;
}

DEFINE_HOOK(0x6FC3A1, TechnoClass_CanFire_InBunkerRangeCheck, 0x5)
{
	enum { ContinueChecks = 0x6FC3C5, CannotFire = 0x6FC86A };

	GET(TechnoClass*, pThis, EBP);
	GET(WeaponTypeClass*, pWeapon, EDI);

	if (pThis->WhatAmI() == AbstractType::Unit && WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis) < 384.0)
		return CannotFire;

	return ContinueChecks;
}

DEFINE_HOOK(0x70CF6F, TechnoClass_ThreatCoefficients_WeaponRange, 0x6)
{
	enum { SkipGameCode = 0x70CF75 };

	GET(TechnoClass*, pThis, EDI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	R->EAX(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis));

	return SkipGameCode;
}

DEFINE_HOOK(0x41810F, AircraftClass_MissionAttack_WeaponRangeCheck1, 0x6)
{
	enum { WithinDistance = 0x418117, NotWithinDistance = 0x418131 };

	GET(AircraftClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(int, distance, EAX);

	int range = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis);

	if (distance < range)
		return WithinDistance;

	return NotWithinDistance;
}

DEFINE_HOOK(0x418BA8, AircraftClass_MissionAttack_WeaponRangeCheck2, 0x6)
{
	enum { SkipGameCode = 0x418BAE };

	GET(AircraftClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EAX);

	R->EAX(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis));

	return SkipGameCode;
}

DEFINE_HOOK_AGAIN(0x6DC18E, TacticalClass_DrawRadialIndicators_WeaponRange, 0x6)
DEFINE_HOOK(0x6DBE63, TacticalClass_DrawRadialIndicators_WeaponRange, 0x6)
{
	enum { SkipGameCode1 = 0x6DBE6F, SkipGameCode2 = 0x6DC19A };

	GET(ObjectClass*, pObject, ESI);

	int* range = nullptr;
	int originalRange = 0;

	if (auto const pTechno = abstract_cast<TechnoClass*>(pObject))
	{
		auto const pWeapon = pTechno->GetPrimaryWeapon()->WeaponType;

		if (pWeapon)
		{
			range = &pWeapon->Range;
			originalRange = *range;
			pWeapon->Range = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pTechno);
		}
	}

	pObject->DrawRadialIndicator(1);

	if (range)
		*range = originalRange;

	return R->Origin() == 0x6DBE63 ? SkipGameCode1 : SkipGameCode2;
}
