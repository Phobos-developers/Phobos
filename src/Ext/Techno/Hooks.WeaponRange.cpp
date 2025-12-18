#include "Body.h"

#include <AircraftClass.h>

#include <Ext/WeaponType/Body.h>

// Reimplements the game function with few changes / optimizations
DEFINE_HOOK(0x7012C2, TechnoClass_WeaponRange, 0x8)
{
	enum { ReturnResult = 0x70138F };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(const int, weaponIndex, STACK_OFFSET(0x8, 0x4));

	int result = 0;
	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	if (pWeapon)
	{
		result = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis);
		auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

		if (!pTypeExt->OpenTopped_IgnoreRangefinding && pTypeExt->OwnerObject()->OpenTopped)
		{
			int smallestRange = INT32_MAX;
			auto pPassenger = abstract_cast<FootClass*>(pThis->Passengers.GetFirstPassenger());

			while (pPassenger)
			{
				const int openTWeaponIndex = pPassenger->GetTechnoType()->OpenTransportWeapon;
				const int tWeaponIndex = openTWeaponIndex == -1 ? pPassenger->SelectWeapon(pThis->Target) : openTWeaponIndex;
				auto const pTWeapon = pPassenger->GetWeapon(tWeaponIndex)->WeaponType;

				if (pTWeapon && pTWeapon->FireInTransport)
				{
					const int range = WeaponTypeExt::GetRangeWithModifiers(pTWeapon, pPassenger);

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

	int range = 0;

	if (const auto keepRange = WeaponTypeExt::GetTechnoKeepRange(pWeapon, pThis, false))
		range = keepRange;
	else
		range = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis);

	R->EDI(range);

	return SkipGameCode;
}

DEFINE_HOOK(0x6F7294, TechnoClass_InRange_OccupyRange, 0x5)
{
	enum { SkipGameCode = 0x6F729F };

	GET(TechnoClass*, pThis, ESI);
	GET(const int, range, EDI);

	const int occupyRange = WeaponTypeExt::GetRangeWithModifiers(nullptr, pThis) / Unsorted::LeptonsPerCell;

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
	GET(const int, distance, EAX);

	const int range = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis);

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

	if (auto const pTechno = abstract_cast<TechnoClass*, true>(pObject))
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

namespace ApproachTargetTemp
{
	bool FromMaximumRange = true;
	int SearchRange = 0;
}

DEFINE_HOOK(0x4D5FBD, FootClass_ApproachTarget_BeforeSearching, 0xA)
{
	enum { WantAggressiveCrush = 0x4D6892, StartSearching = 0x4D5FE0 };

	GET(TechnoTypeClass*, pType, EAX);
	R->ESI(pType->MovementZone);

	GET_STACK(int, searchRange, STACK_OFFSET(0x158, -0x120));
	ApproachTargetTemp::FromMaximumRange = true;
	ApproachTargetTemp::SearchRange = searchRange;

	if (searchRange <= 204)
		return WantAggressiveCrush;

	GET_STACK(const bool, inRange, STACK_OFFSET(0x158, -0x146));

	if (!inRange)
	{
		GET(FootClass*, pThis, EBX);
		GET_STACK(const int, weaponIdx, STACK_OFFSET(0x158, -0xAC));
		const auto pWeapon = pThis->GetWeapon(weaponIdx)->WeaponType;

		if (pWeapon && pWeapon->Range != -512)
		{
			const int distance = (pThis->IsInAir() || pWeapon->Projectile->Arcing || pThis->WhatAmI() == AircraftClass::AbsID)
				? pThis->DistanceFrom(pThis->Target)
				: pThis->DistanceFrom3D(pThis->Target);
			ApproachTargetTemp::FromMaximumRange = distance >= pWeapon->MinimumRange;

			if (!ApproachTargetTemp::FromMaximumRange)
				searchRange = 204;
		}
	}

	R->ECX(searchRange);
	R->Stack(STACK_OFFSET(0x158, -0xF4), searchRange);
	return StartSearching;
}

DEFINE_HOOK(0x4D6874, FootClass_ApproachTarget_NextRadius, 0xC)
{
	enum { ContinueNextRadius = 0x4D5FE0, BreakOut = 0x4D68E7 };

	GET_STACK(int, searchRadius, STACK_OFFSET(0x158, -0xF4));

	if (ApproachTargetTemp::FromMaximumRange)
	{
		searchRadius -= Unsorted::LeptonsPerCell;
		R->Stack(STACK_OFFSET(0x158, -0xF4), searchRadius);
		return searchRadius > 204 ? ContinueNextRadius : BreakOut;
	}

	searchRadius += Unsorted::LeptonsPerCell;
	R->Stack(STACK_OFFSET(0x158, -0xF4), searchRadius);
	return searchRadius <= ApproachTargetTemp::SearchRange ? ContinueNextRadius : BreakOut;
}
