#include <Helpers/Macro.h>
#include <FootClass.h>
#include <HouseClass.h>
#include <TriggerClass.h>
#include <TActionClass.h>

DEFINE_HOOK(6E0D60, TActionClass_Text_Trigger, 6)
{
    GET(TActionClass*, pThis, ECX);
    GET_STACK(HouseClass*, pHouse, 0x4);
    GET_STACK(TriggerClass*, pTrigger, 0xC);

    int nNewOwner = pThis->Value;
    HouseClass* pNewOwner = nullptr;

    if (nNewOwner == 0x2325)
        pNewOwner = pTrigger->GetHouse();
    else
    {
        if (nNewOwner == -1)
            return 0;
        if (pHouse->Index_IsMP(nNewOwner))
            pNewOwner = pHouse->FindByIndex(nNewOwner);
        else
            pNewOwner = pHouse->FindByCountryIndex(nNewOwner);
    }
    if (!pNewOwner)
        return 0;
    if (HouseClass::Player == pNewOwner)
        return 0;

    R->AL(1);
    return 0x6E0DE5;
}