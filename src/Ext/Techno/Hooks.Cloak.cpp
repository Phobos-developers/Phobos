#include "Body.h"

namespace CloakTemp
{
	bool IsInReadyToCloak = false;
}

bool __fastcall TechnoClass_IsReadyToCloak_Wrapper(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	bool cloakable = pThis->Cloakable;
	int rearm = -1;
	pThis->Cloakable |= pExt->AE.Cloakable;

	if (pExt->CanCloakDuringRearm)
	{
		rearm = pThis->RearmTimer.GetTimeLeft();
		pThis->RearmTimer.Stop();
	}

	CloakTemp::IsInReadyToCloak = true;
	bool retVal = pThis->TechnoClass::IsReadyToCloak();
	CloakTemp::IsInReadyToCloak = false;

	pThis->Cloakable = cloakable;

	if (rearm != -1)
		pThis->RearmTimer.Start(rearm);

	if (retVal && pExt->AE.ForceDecloak)
		return false;

	return retVal;
}

bool __fastcall TechnoClass_ShouldNotCloak_Wrapper(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	bool cloakable = pThis->Cloakable;
	pThis->Cloakable |= pExt->AE.Cloakable;

	bool retVal = pThis->TechnoClass::ShouldNotBeCloaked();

	pThis->Cloakable = cloakable;

	if (!retVal && pExt->AE.ForceDecloak)
		return true;

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

	if (CloakTemp::IsInReadyToCloak)
	{
		auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

		if (pWeapon && !pWeapon->DecloakToFire)
		{
			R->EAX(0);
			return SkipGameCode;
		}
	}

	R->EAX(pThis->IsCloseEnough(pTarget, weaponIndex));
	return SkipGameCode;
}

DEFINE_HOOK_AGAIN(0x6FBBC3, TechnoClass_Cloak_BeforeDetach, 0x5)  // TechnoClass_Cloaking_AI
DEFINE_HOOK(0x703789, TechnoClass_Cloak_BeforeDetach, 0x6)        // TechnoClass_Do_Cloak
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pExt = TechnoExt::ExtMap.Find(pThis))
	{
		if (!pExt->MindControlRingAnimType)
			pExt->UpdateMindControlAnim();

		pExt->IsDetachingForCloak = true;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6FBBCE, TechnoClass_Cloak_AfterDetach, 0x7)  // TechnoClass_Cloaking_AI
DEFINE_HOOK(0x703799, TechnoClass_Cloak_AfterDetach, 0xA)        // TechnoClass_Do_Cloak
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pExt = TechnoExt::ExtMap.Find(pThis))
		pExt->IsDetachingForCloak = false;

	return 0;
}

DEFINE_HOOK(0x6FB9D7, TechnoClass_Cloak_RestoreMCAnim, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pExt = TechnoExt::ExtMap.Find(pThis))
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

DEFINE_HOOK(0x457855, BuildingClass_IsReadyToCloak_Sensors, 0x6)
{
	enum { Skip = 0x457879, Continue = 0x457865 };

	GET(BuildingClass*, pThis, EBP);
	GET(TechnoClass*, pTechno, EAX);

	if (pThis->Owner->IsAlliedWith(pTechno))
		return Skip;

	return Continue;
}

DEFINE_HOOK(0x4579A5, BuildingClass_ShouldNotCloak_Sensors, 0x6)
{
	enum { Skip = 0x4579C9, Continue = 0x4579B5 };

	GET(BuildingClass*, pThis, EBP);
	GET(TechnoClass*, pTechno, EAX);

	if (pThis->Owner->IsAlliedWith(pTechno))
		return Skip;

	return Continue;
}
