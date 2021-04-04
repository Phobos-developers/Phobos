#include "CaptureManager.h"

#include <Helpers/Macro.h>
#include <HouseClass.h>
#include <BuildingClass.h>
#include <AnimClass.h>

#include "../Ext/TechnoType/Body.h"

// I hate Ares' completely rewritten things
bool CaptureManager::CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget)
{
    if (pManager->MaxControlNodes == 1)
        return pManager->CanCapture(pTarget);

    pManager->MaxControlNodes += 1;
    bool result = pManager->CanCapture(pTarget);
    pManager->MaxControlNodes -= 1;
    return result;
}

bool CaptureManager::FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bSilent)
{
    if (!bSilent)
        return pManager->FreeUnit(pTarget);

    if (pTarget)
    {
        for (int i = 0; i < pManager->ControlNodes.Count; ++i)
        {
            auto const pControlNode = pManager->ControlNodes[i];
            if (pControlNode->Unit == pTarget)
            {
                if (auto& pRingAnim = pTarget->MindControlRingAnim)
                {
                    pRingAnim->UnInit();
                    pRingAnim = nullptr;
                }
                
                auto const pSettingHouse = pControlNode->OriginalOwner->Defeated ?
                    HouseClass::FindNeutral() : pControlNode->OriginalOwner;
                if (!pSettingHouse)
                    return false;
                pTarget->SetOwningHouse(pSettingHouse);

                pManager->ControlNodes.RemoveItem(i);
                return true;
            }
        }
    }

    return false;
}

bool CaptureManager::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bRemoveFirst)
{
    if (CaptureManager::CanCapture(pManager, pTarget))
    {
        // issue #59
        // An improvement of Multiple MindControl
        if (pManager->MaxControlNodes == 1 && pManager->ControlNodes.Count == 1)
        {
            CaptureManager::FreeUnit(pManager, pManager->ControlNodes[0]->Unit, true);
            GameDelete(pManager->ControlNodes[0]);
        }
        if (pManager->MaxControlNodes > 1 && pManager->ControlNodes.Count == pManager->MaxControlNodes)
        {
            if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pManager->Owner->GetTechnoType()))
            {
                if (!pTechnoTypeExt->MultipleMCReleaseVictim)
                    return false;

                if (pManager->ControlNodes.Count > 0)
                {
                    CaptureManager::FreeUnit(pManager, pManager->ControlNodes[0]->Unit, true);
                    GameDelete(pManager->ControlNodes[0]);
                }
            }
        }

        auto const pOriginHouse = pTarget->Owner;
        auto pControlNode = GameCreate<ControlNode>();
        if (pControlNode)
        {
            pControlNode->OriginalOwner = pOriginHouse;
            pControlNode->Unit = pTarget;

            pManager->ControlNodes.AddItem(pControlNode);
            pControlNode->LinkDrawTimer.Start(RulesClass::Instance->MindControlAttackLineFrames);

            if (pTarget->SetOwningHouse(pManager->Owner->Owner))
            {
                pManager->DecideUnitFate(pTarget);
                
                auto const pBld = abstract_cast<BuildingClass*>(pTarget);
                auto const pType = pTarget->GetTechnoType();

                CoordStruct location = pTarget->GetCoords();
                if (pBld) {
                    location.Z += pBld->Type->Height * Unsorted::LevelHeight;
                }
                else {
                    location.Z += pType->MindControlRingOffset;
                }
                if (auto const pAnimType = RulesClass::Instance->ControlledAnimationType) {
                    if (auto const pAnim = GameCreate<AnimClass>(pAnimType, location)) {
                        pTarget->MindControlRingAnim = pAnim;
                        pAnim->SetOwnerObject(pTarget);
                        if (pBld) {
                            pAnim->ZAdjust = -1024;
                        }
                    }
                }

                return true;
            }

        }
    }
    return false;
}

DEFINE_HOOK(471D40, CaptureManagerClass_CaptureUnit, 7)
{
    GET(CaptureManagerClass*, pThis, ECX);
    GET_STACK(TechnoClass*, pTechno, -0x4);

    auto const pTarget = pTechno->AbsDerivateID & AbstractFlags::Techno ? pTechno : nullptr;

    bool bRemoveFirst = false;
    if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType()))
        bRemoveFirst = pTechnoTypeExt->MultipleMCReleaseVictim;

    if (pTarget)
        R->EAX(CaptureManager::CaptureUnit(pThis, pTarget, bRemoveFirst));
    else
        R->EAX<bool>(false);

    return 0x471D5A;
}

DEFINE_HOOK(6FCB34, TechnoClass_CanFire_CanCapture, 6)
{
    GET(TechnoClass*, pThis, ESI);
    GET(TechnoClass*, pTarget, EBP);
    R->EAX(CaptureManager::CanCapture(pThis->CaptureManager, pTarget));
    return 0x6FCB40;
}