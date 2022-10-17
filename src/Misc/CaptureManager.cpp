#include "CaptureManager.h"

#include <Helpers/Macro.h>
#include <HouseClass.h>
#include <BuildingClass.h>
#include <AnimClass.h>

#include <Ext/TechnoType/Body.h>

bool CaptureManager::CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget)
{
	if (pManager->MaxControlNodes == 1)
		return pManager->CanCapture(pTarget);

	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pManager->Owner->GetTechnoType());
	if (pTechnoTypeExt && pTechnoTypeExt->MultiMindControl_ReleaseVictim)
	{
		// I hate Ares' completely rewritten things - secsome
		pManager->MaxControlNodes += 1;
		bool result = pManager->CanCapture(pTarget);
		pManager->MaxControlNodes -= 1;
		return result;
	}

	return pManager->CanCapture(pTarget);
}

bool CaptureManager::FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bSilent)
{
	if (pTarget)
	{
		for (int i = pManager->ControlNodes.Count - 1; i >= 0; --i)
		{
			const auto pNode = pManager->ControlNodes[i];
			if (pTarget == pNode->Unit)
			{
				if (pTarget->MindControlRingAnim)
				{
					pTarget->MindControlRingAnim->UnInit();
					pTarget->MindControlRingAnim = nullptr;
				}

				if (!bSilent)
				{
					int nSound = pTarget->GetTechnoType()->MindClearedSound;

					if (nSound == -1)
						nSound = RulesClass::Instance->MindClearedSound;
					if (nSound != -1)
						VocClass::PlayIndexAtPos(nSound, pTarget->GetCoords());
				}

				// Fix : Player defeated should not get this unit.
				auto pOriginOwner = pNode->OriginalOwner->Defeated ?
					HouseClass::FindNeutral() : pNode->OriginalOwner;

				pTarget->SetOwningHouse(pOriginOwner);
				pManager->DecideUnitFate(pTarget);
				pTarget->MindControlledBy = nullptr;

				if (pNode)
					GameDelete(pNode);

				pManager->ControlNodes.RemoveItem(i);

				return true;
			}
		}
	}

	return false;
}

bool CaptureManager::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget,
	bool bRemoveFirst, AnimTypeClass* pControlledAnimType)
{
	if (CaptureManager::CanCapture(pManager, pTarget))
	{
		if (pManager->MaxControlNodes <= 0)
			return false;

		if (!pManager->InfiniteMindControl)
		{
			if (pManager->MaxControlNodes == 1 && pManager->ControlNodes.Count == 1)
				CaptureManager::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
			else if (pManager->ControlNodes.Count == pManager->MaxControlNodes)
				if (bRemoveFirst)
					CaptureManager::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
		}

		auto pControlNode = GameCreate<ControlNode>();
		if (pControlNode)
		{
			pControlNode->OriginalOwner = pTarget->Owner;
			pControlNode->Unit = pTarget;

			pManager->ControlNodes.AddItem(pControlNode);
			pControlNode->LinkDrawTimer.Start(RulesClass::Instance->MindControlAttackLineFrames);

			if (pTarget->SetOwningHouse(pManager->Owner->Owner))
			{
				pTarget->MindControlledBy = pManager->Owner;

				pManager->DecideUnitFate(pTarget);

				auto const pBld = abstract_cast<BuildingClass*>(pTarget);
				auto const pType = pTarget->GetTechnoType();
				CoordStruct location = pTarget->GetCoords();

				if (pBld)
					location.Z += pBld->Type->Height * Unsorted::LevelHeight;
				else
					location.Z += pType->MindControlRingOffset;

				if (auto const pAnimType = pControlledAnimType)
				{
					if (auto const pAnim = GameCreate<AnimClass>(pAnimType, location))
					{
						pTarget->MindControlRingAnim = pAnim;
						pAnim->SetOwnerObject(pTarget);

						if (pBld)
							pAnim->ZAdjust = -1024;
					}
				}

				pTarget->SetTarget(nullptr);

				if (pManager->Owner->Owner->IsControlledByHuman())
				{
					if (auto pTargetFoot = abstract_cast<FootClass*>(pTarget))
					{
						pTargetFoot->QueueMission(Mission::Guard, false);
						pTargetFoot->SetDestination(pTarget->GetCell(), true);
						pTargetFoot->NextMission();
					}
				}

				return true;
			}

		}
	}

	return false;
}

bool CaptureManager::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTechno, AnimTypeClass* pControlledAnimType)
{
	if (pTechno)
	{
		const auto pTarget = pTechno->AbsDerivateID & AbstractFlags::Techno ? pTechno : nullptr;

		bool bRemoveFirst = false;
		if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pManager->Owner->GetTechnoType()))
			bRemoveFirst = pTechnoTypeExt->MultiMindControl_ReleaseVictim;

		return CaptureManager::CaptureUnit(pManager, pTarget, bRemoveFirst, pControlledAnimType);
	}

	return false;
}

void CaptureManager::DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot)
{
	// to be implemented (if needed). - secsome
}

DEFINE_HOOK(0x471D40, CaptureManagerClass_CaptureUnit, 0x7)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureManager::CaptureUnit(pThis, pTechno));

	return 0x471D5A;
}

DEFINE_HOOK(0x471FF0, CaptureManagerClass_FreeUnit, 0x8)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureManager::FreeUnit(pThis, pTechno));

	return 0x472006;
}

DEFINE_HOOK(0x6FCB34, TechnoClass_CanFire_CanCapture, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EBP);

	R->AL(CaptureManager::CanCapture(pThis->CaptureManager, pTarget));

	return 0x6FCB40;
}
