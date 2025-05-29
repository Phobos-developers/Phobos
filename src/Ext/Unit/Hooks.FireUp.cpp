#include <Helpers/Macro.h>
#include <Utilities/Macro.h>

#include <UnitClass.h>
#include <Ext/Techno/Body.h>

DEFINE_JUMP(LJMP, 0x741406, 0x741427)

namespace UnitUpdateFiringTemp
{
	TechnoExt::ExtData* TechnoExtData = nullptr;
}

DEFINE_HOOK(0x736E34, UnitClass_UpdateFiring_ResetFireUp, 0x5)
{
	GET(UnitClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EAX);
	GET(int, nWeaponIndex, EDI);

	const auto pExt = TechnoExt::ExtMap.Find(static_cast<TechnoClass*>(pThis));
	UnitUpdateFiringTemp::TechnoExtData = pExt;

	FireError fireError = pThis->GetFireError(pTarget, nWeaponIndex, true);

	if (fireError != FireError::OK &&
		fireError != FireError::FACING &&
		fireError != FireError::MOVING)
	{
		pExt->FireUp = 0;
	}

	R->EAX(fireError);
	return 0x736E40;
}

DEFINE_HOOK(0x736F61, UnitClass_UpdateFiring_FireUp, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	enum { SkipGameCode = 0x736F73 };

	if (pThis->HasTurret() || pThis->Type->Voxel)
		return 0;

	const auto pExt = UnitUpdateFiringTemp::TechnoExtData;
	UnitUpdateFiringTemp::TechnoExtData = nullptr;
	const auto pTypeExt = pExt->TypeExtData;

	// SHP vehicles have no secondary action frames, so it does not need SecondaryFire.
	int fireUp = pTypeExt->FireUp.Get();

	if (fireUp > 0 && !pThis->Type->OpportunityFire &&
		pThis->Locomotor->Is_Really_Moving_Now())
	{
		if (pExt->FireUp > 0)
			pExt->FireUp = 0;

		return SkipGameCode;
	}

	if (pExt->FireUp == 0)
	{
		int frames = pThis->Type->FiringFrames;

		if (frames >= 0)
			pThis->unknown_int_6C0 = 2 * frames - 1;
	}

	if (fireUp > 0)
	{
		if (pExt->FireUp < fireUp)
		{
			pExt->FireUp++;
			return SkipGameCode;
		}
		else
		{
			pExt->FireUp = 0;
		}
	}

	return 0;
}
