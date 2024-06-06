#include "Body.h"

namespace CloakTemp
{
	bool IsInReadyToCloak = false;
}

bool __fastcall TechnoClass_IsReadyToCloak_Wrapper(TechnoClass* pThis)
{
	bool cloak = pThis->Cloakable;
	TechnoExt::ExtData* pExt = nullptr;

	if (!pThis->Cloakable)
	{
		pExt = TechnoExt::ExtMap.Find(pThis);
		pThis->Cloakable = pExt->AE_Cloakable;
	}

	CloakTemp::IsInReadyToCloak = true;
	bool retVal = pThis->TechnoClass::IsReadyToCloak();
	CloakTemp::IsInReadyToCloak = false;
	pThis->Cloakable = cloak;

	if (retVal)
	{
		if (!pExt)
			pExt = TechnoExt::ExtMap.Find(pThis);

		if (pExt->AE_ForceDecloak)
			return false;
	}

	return retVal;
}

bool __fastcall TechnoClass_ShouldNotCloak_Wrapper(TechnoClass* pThis)
{
	bool cloak = pThis->Cloakable;
	TechnoExt::ExtData* pExt = nullptr;

	if (!pThis->Cloakable)
	{
		pExt = TechnoExt::ExtMap.Find(pThis);
		pThis->Cloakable = pExt->AE_Cloakable;
	}

	bool retVal = pThis->TechnoClass::ShouldNotBeCloaked();
	pThis->Cloakable = cloak;

	if (!retVal)
	{
		if (!pExt)
			pExt = TechnoExt::ExtMap.Find(pThis);

		if (pExt->AE_ForceDecloak)
			return true;
	}

	return retVal;
}

// Replaces the vanilla vtable calls & calls function from wrapper to retain compatibility with Ares.
// The vanilla functions are completely overwritten by Ares.
DEFINE_JUMP(VTABLE, 0x7E2544, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // AircraftClass
DEFINE_JUMP(VTABLE, 0x7E8F34, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // FootClass
DEFINE_JUMP(VTABLE, 0x7EB2F8, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // InfantryClass
DEFINE_JUMP(VTABLE, 0x7F4C00, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // TechnoClass
DEFINE_JUMP(VTABLE, 0x7F5F10, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // UnitClass
DEFINE_JUMP(CALL, 0x457779, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper))    // BuildingClass

DEFINE_JUMP(VTABLE, 0x7E2548, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper)); // AircraftClass
DEFINE_JUMP(VTABLE, 0x7E8F38, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper)); // FootClass
DEFINE_JUMP(VTABLE, 0x7EB2FC, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper)); // InfantryClass
DEFINE_JUMP(VTABLE, 0x7F4C04, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper)); // TechnoClass
DEFINE_JUMP(VTABLE, 0x7F5F14, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper)); // UnitClass
DEFINE_JUMP(CALL, 0x4578C9, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper));   // BuildingClass

// Allow units with DecloakToFire=no weapons to cloak even when about to fire on target.
DEFINE_HOOK(0x6F7792, TechnoClass_InWeaponRange_DecloakToFire, 0xA)
{
	enum { SkipGameCode = 0x6F779C };

	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EBX);
	GET(int, weaponIndex, EAX);

	if (CloakTemp::IsInReadyToCloak && !pThis->GetWeapon(weaponIndex)->WeaponType->DecloakToFire)
		R->EAX(0);
	else
		R->EAX(pThis->IsCloseEnough(pTarget, weaponIndex));

	return SkipGameCode;
}

DEFINE_HOOK_AGAIN(0x703789, TechnoClass_CloakUpdateMCAnim, 0x6) // TechnoClass_Do_Cloak
DEFINE_HOOK(0x6FB9D7, TechnoClass_CloakUpdateMCAnim, 0x6)       // TechnoClass_Cloaking_AI
{
	GET(TechnoClass*, pThis, ESI);

	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
		pExt->UpdateMindControlAnim();

	return 0;
}

DEFINE_HOOK(0x703A09, TechnoClass_VisualCharacter_CloakVisibility, 0x7)
{
	enum { UseShadowyVisual = 0x703A5A, CheckMutualAlliance = 0x703A16 };

	// Allow observers to always see cloaked objects.
	// Skip IsCampaign check (confirmed being useless from Mental Omega mappers)
	if (HouseClass::IsCurrentPlayerObserver())
		return UseShadowyVisual;

	return CheckMutualAlliance;
}

DEFINE_HOOK(0x45455B, BuildingClass_VisualCharacter_CloakVisibility, 0x5)
{
	enum { UseShadowyVisual = 0x45452D, CheckMutualAlliance = 0x454564 };

	if (HouseClass::IsCurrentPlayerObserver())
		return UseShadowyVisual;

	return CheckMutualAlliance;
}
