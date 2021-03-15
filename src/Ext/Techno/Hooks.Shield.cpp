#include "Body.h"

#include "../TechnoType/Body.h"

// #issue 88 : shield logic
DEFINE_HOOK(701900, TechnoClass_ReceiveDamage_Shield, 6)
{
    GET(TechnoClass*, pThis, ECX);
    GET_STACK(int*, pDamage, 0x4);
    GET_STACK(WarheadTypeClass*, pWH, 0xC);
    //GET_STACK(HouseClass*, pSourceHouse, -0x1C);

    auto pExt = TechnoExt::ExtMap.Find(pThis);

    if (auto pShieldData = pExt->ShieldData.get())
    {
        auto nDamageLeft = pShieldData->ReceiveDamage(*pDamage, pWH);
        if (nDamageLeft >= 0)
            *pDamage = nDamageLeft;
    }

    return 0;

}

DEFINE_HOOK(6F9E50, TechnoClass_Update_Shield, 5)
{
    GET(TechnoClass*, pThis, ECX);
    auto pExt = TechnoExt::ExtMap.Find(pThis);
    auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

    if (pTypeData->Shield_Strength)
    {
        if (!pExt->ShieldData)
            pExt->ShieldData = std::make_unique<ShieldTechnoClass>(pThis);
        pExt->ShieldData->Update();
    }
    return 0;
}