#include "Body.h"
#include <BuildingClass.h>

// Guard building disguise detector activation against multiple additions and offline states
DEFINE_HOOK(0x455A88, BuildingClass_DisguiseDetectorActivate, 0x8)
{
	enum { ReturnActivate = 0x455A98, ReturnExit = 0x455B84 };

	GET(BuildingClass*, pBld, ESI);
	const auto pExt = BuildingExt::Fetch(pBld);

	if (pBld->IsPowerOnline() && !pBld->Deactivated && !pExt->DetectDisguiseActiveCounter++)
		return ReturnActivate;

	return ReturnExit;
}

// Guard building disguise detector deactivation against multiple removals and cell counter underflow
DEFINE_HOOK(0x455991, BuildingClass_DisguiseDetectorDeactivate, 0x6)
{
	enum { ReturnDeactivate = 0, ReturnExit = 0x455A71 };

	GET(BuildingClass*, pBld, ECX);
	const auto pExt = BuildingExt::Fetch(pBld);

	if (pExt->DetectDisguiseActiveCounter > 0)
	{
		pExt->DetectDisguiseActiveCounter = 0;
		return ReturnDeactivate;
	}

	return ReturnExit;
}

// Update disguise detection when building power state changes
DEFINE_HOOK_AGAIN(0x454B5F, BuildingClass_UpdatePowered_DetectDisguise, 0x6)
DEFINE_HOOK(0x4549F8, BuildingClass_UpdatePowered_DetectDisguise, 0x6)
{
	GET(BuildingClass*, pBld, ESI);
	const auto pExt = BuildingExt::Fetch(pBld);
	pExt->UpdateDetectDisguise();

	return 0;
}

// Update disguise detection when building is disabled by EMP or toggled off
DEFINE_HOOK(0x4524A3, BuildingClass_DisableThings_DetectDisguise, 0x6)
{
	GET(BuildingClass*, pBld, EDI);
	const auto pExt = BuildingExt::Fetch(pBld);
	pExt->UpdateDetectDisguise();

	return 0;
}

// Deactivate disguise detector for previous owner on capture or mind control
DEFINE_HOOK(0x448B70, BuildingClass_ChangeOwnership_DetectDisguiseA, 0x6)
{
	GET(BuildingClass*, pBld, ESI);

	if (pBld->Type->DetectDisguise)
		pBld->DisguiseDetectorDeactivate();

	return 0;
}

// Activate disguise detector for new owner on capture or mind control
DEFINE_HOOK(0x448C3E, BuildingClass_ChangeOwnership_DetectDisguiseB, 0x6)
{
	GET(BuildingClass*, pBld, ESI);

	if (pBld->Type->DetectDisguise)
		pBld->DisguiseDetectorActivate();

	return 0;
}

// Deactivate disguise detector on building destruction
DEFINE_HOOK(0x4416A2, BuildingClass_Destroy_DetectDisguise, 0x6)
{
	GET(BuildingClass*, pBld, ESI);

	if (pBld->Type->DetectDisguise)
		pBld->DisguiseDetectorDeactivate();

	return 0;
}

// Display DetectDisguiseRange on radial indicator if structure has no valid weapon
DEFINE_HOOK(0x45671E, BuildingClass_GetRangeOfRadial_DetectDisguise_NoWeapon, 0x6)
{
	enum { ReturnSet = 0x45674B };

	GET(bool, isValid, EAX);
	GET(BuildingClass*, pThis, ESI);
	auto const pType = pThis->Type;

	if (!isValid && pType->DetectDisguise && pType->DetectDisguiseRange > 0)
	{
		R->EAX(pType->DetectDisguiseRange);
		return ReturnSet;
	}

	return 0;
}

// Display DetectDisguiseRange on radial indicator if structure has no valid weapon range
DEFINE_HOOK(0x45672A, BuildingClass_GetRangeOfRadial_DetectDisguise_NoRange, 0x5)
{
	enum { ReturnSet = 0x45674B };

	GET(int, range, EAX);
	GET(BuildingClass*, pThis, ESI);
	auto const pType = pThis->Type;

	if (range <= 0 && pType->DetectDisguise && pType->DetectDisguiseRange > 0)
	{
		R->EAX(pType->DetectDisguiseRange);
		return ReturnSet;
	}

	return 0;
}
