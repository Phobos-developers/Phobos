#include "Body.h"
#include <LocomotionClass.h>
#include <TeleportLocomotionClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <TacticalClass.h>

#define GET_LOCO(reg_Loco) \
	GET(ILocomotion *, Loco, reg_Loco); \
	__assume(Loco!=nullptr);\
	TeleportLocomotionClass *pLocomotor = static_cast<TeleportLocomotionClass*>(Loco); \
	FootClass* pLinked = pLocomotor->LinkedTo;\
	TechnoTypeClass const*pType = pLinked->GetTechnoType(); \
	TechnoTypeExt::ExtData const*pExt = TechnoTypeExt::ExtMap.Find(pType);

DEFINE_HOOK(0x7193F6, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 0x6)
{
	GET_LOCO(ESI);

	if (auto pWarpOut = pExt->WarpOut.Get(RulesClass::Instance->WarpOut))
		GameCreate<AnimClass>(pWarpOut, pLinked->Location)->Owner = pLinked->Owner;

	if (pExt->WarpOutWeapon)
		WeaponTypeExt::DetonateAt(pExt->WarpOutWeapon, pLinked, pLinked);

	const int distance = (int)Math::sqrt(pLinked->Location.DistanceFromSquared(pLocomotor->LastCoords));
	auto linkedExt = TechnoExt::ExtMap.Find(pLinked);
	linkedExt->LastWarpDistance = distance;

	if (auto pImage = pType->AlphaImage)
	{
		auto [xy, _] = TacticalClass::Instance->CoordsToClient(pLinked->Location);
		RectangleStruct Dirty = {
			xy.X - (pImage->Width / 2),
			xy.Y - (pImage->Height / 2),
			pImage->Width,
			pImage->Height
		};
		TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
	}

	int duree = pExt->ChronoMinimumDelay.Get(RulesClass::Instance->ChronoMinimumDelay);

	if (distance >= pExt->ChronoRangeMinimum.Get(RulesClass::Instance->ChronoRangeMinimum)
		&& pExt->ChronoTrigger.Get(RulesClass::Instance->ChronoTrigger))
	{
		int factor = std::max(pExt->ChronoDistanceFactor.Get(RulesClass::Instance->ChronoDistanceFactor), 1);
		duree = std::max(distance / factor, duree);

	}

	pLinked->WarpingOut = true;

	if (auto pUnit = specific_cast<UnitClass*>(pLinked))
	{
		if (pUnit->Type->Harvester || pUnit->Type->Weeder)
		{
			duree = 0;
			pLinked->WarpingOut = false;
		}
	}

	pLocomotor->Timer.Start(duree);
	linkedExt->LastWarpInDelay = std::max(duree, linkedExt->LastWarpInDelay);
	return 0x7195BC;
}

DEFINE_HOOK(0x719742, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 0x5)
{
	GET_LOCO(ESI);

	if (auto pWarpIn = pExt->WarpIn.Get(RulesClass::Instance->WarpIn))
		GameCreate<AnimClass>(pWarpIn, pLinked->Location)->Owner
		= pLinked->Owner;

	auto const lastWarpDistance = TechnoExt::ExtMap.Find(pLinked)->LastWarpDistance;
	bool isInMinRange = lastWarpDistance < pExt->ChronoRangeMinimum.Get(RulesClass::Instance->ChronoRangeMinimum);

	if (auto const weaponType = isInMinRange ? pExt->WarpInMinRangeWeapon.Get(pExt->WarpInWeapon) : pExt->WarpInWeapon)
	{
		int damage = pExt->WarpInWeapon_UseDistanceAsDamage ? lastWarpDistance / Unsorted::LeptonsPerCell : weaponType->Damage;
		WeaponTypeExt::DetonateAt(weaponType, pLinked, pLinked, damage);
	}

	return 0x719796;
}

DEFINE_HOOK(0x719827, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 0x5)
{
	GET_LOCO(ESI);

	if (auto pWarpAway = pExt->WarpAway.Get(RulesClass::Instance->WarpOut))
		GameCreate<AnimClass>(pWarpAway, pLinked->Location)->Owner = pLinked->Owner;

	return 0x719878;
}

DEFINE_HOOK(0x719973, TeleportLocomotionClass_ILocomotion_Process_ChronoDelay, 0x5)
{
	GET_LOCO(ESI);

	pLinked->ChronoLockRemaining = pExt->ChronoDelay.Get(RulesClass::Instance->ChronoDelay);
	R->EAX(0);

	return 0x719989;
}

#undef GET_LOCO

DEFINE_HOOK(0x7197E4, TeleportLocomotionClass_Process_ChronospherePreDelay, 0x6)
{
	GET(TeleportLocomotionClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis->Owner);
	pExt->IsBeingChronoSphered = true;
	R->ECX(pExt->TypeExtData->ChronoSpherePreDelay.Get(RulesExt::Global()->ChronoSpherePreDelay));

	return 0;
}

DEFINE_HOOK(0x719BD9, TeleportLocomotionClass_Process_ChronosphereDelay2, 0x6)
{
	GET(TeleportLocomotionClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis->Owner);

	if (!pExt->IsBeingChronoSphered)
		return 0;

	int delay = pExt->TypeExtData->ChronoSphereDelay.Get(RulesExt::Global()->ChronoSphereDelay);

	if (delay > 0)
	{
		pThis->Owner->WarpingOut = true;
		pExt->HasRemainingWarpInDelay = true;
		pExt->LastWarpInDelay = Math::max(delay, pExt->LastWarpInDelay);
	}
	else
	{
		pExt->IsBeingChronoSphered = false;
	}

	return 0;
}

DEFINE_HOOK(0x4DA53E, FootClass_Update_WarpInDelay, 0x6)
{
	GET(FootClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->HasRemainingWarpInDelay)
	{
		if (pExt->LastWarpInDelay)
		{
			pExt->LastWarpInDelay--;
		}
		else
		{
			pExt->HasRemainingWarpInDelay = false;
			pExt->IsBeingChronoSphered = false;
			pThis->WarpingOut = false;
		}
	}

	return 0;
}
