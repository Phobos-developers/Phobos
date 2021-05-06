#include <InfantryClass.h>

#include "Body.h"

#include "../TechnoType/Body.h"
#include "../../Utilities/GeneralUtils.h"

DEFINE_HOOK(6F9E50, TechnoClass_AI, 5)
{
    GET(TechnoClass*, pThis, ECX);

    // MindControlRangeLimit
    TechnoExt::ApplyMindControlRangeLimit(pThis);
    // Interceptor
    TechnoExt::ApplyInterceptor(pThis);
    // Powered.KillSpawns
    TechnoExt::ApplyPowered_KillSpawns(pThis);
    // Spawner.LimitRange & Spawner.ExtraLimitRange
    TechnoExt::ApplySpawn_LimitRange(pThis);
    //
    TechnoExt::ApplyCloak_Undeployed(pThis);

    return 0;
}

DEFINE_HOOK(6F36DB, TechnoClass_WhatWeaponShouldIUse, 8)
{
    enum { UsePrimary = 0x6F37AD, UseSecondary = 0x6F3745, ReturnToBeginning = 0x6F36E3, ContinueWithVanillaChecks = 0x6F3754 };

    GET(TechnoClass*, pThis, ESI);
    GET(TechnoClass*, pTarget, EBP);

    if (!pTarget)
        return UsePrimary;

    if (auto pExtTarget = TechnoExt::ExtMap.Find(pTarget))
    {
        auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
        auto pWeaponPrimary = pThis->GetWeapon(0)->WeaponType;
        auto pWeaponSecondary = pThis->GetWeapon(1)->WeaponType;

        // Shield checks.
        if (auto pShieldData = pExtTarget->ShieldData.get())
        {
            if (pShieldData->Available() && pShieldData->GetShieldHP())
            {
                if (pWeaponSecondary)
                {
                    if (!pShieldData->CanBeTargeted(pWeaponPrimary) && !pExt->NoSecondaryWeaponFallback)
                    {
                        return UseSecondary;
                    }
                }

                return UsePrimary;
            }
        }

        // No secondary weapon fallback checks.
        if (pExt->NoSecondaryWeaponFallback)
        {
            // Make an exception for NoAmmoWeapon set explicitly to secondary.
            if (pThis->GetTechnoType()->Ammo > 0 && pThis->Ammo <= pExt->NoAmmoAmount && pExt->NoAmmoWeapon == 1)
            {
                return UseSecondary;
            }

            bool canSecondaryTarget = pWeaponSecondary ? GeneralUtils::GetWarheadVersusArmor(pWeaponSecondary->Warhead,
                static_cast<int>(pTarget->GetTechnoType()->Armor)) != 0.0 : false;

            if (canSecondaryTarget)
            {
                return UsePrimary;
            }

            return ContinueWithVanillaChecks;
        }
    }

    return ReturnToBeginning;
}
