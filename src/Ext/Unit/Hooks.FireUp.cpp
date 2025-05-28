#include <Helpers/Macro.h>
#include <Utilities/Macro.h>

#include <UnitClass.h>
#include <Ext/Techno/Body.h>

DEFINE_JUMP(LJMP, 0x741406, 0x741427)

DEFINE_HOOK(0x736F61, UnitClass_UpdateFiring_FireUp, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	enum { SkipGameCode = 0x736F73 };

	if (pThis->HasTurret() || pThis->Type->Voxel)
		return 0;

	const auto pExt = TechnoExt::ExtMap.Find(static_cast<TechnoClass*>(pThis));
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
