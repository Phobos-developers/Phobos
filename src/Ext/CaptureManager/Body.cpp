#include "Body.h"

bool CaptureManagerExt::CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget)
{
	if (pManager->MaxControlNodes == 1)
		return pManager->CanCapture(pTarget);

	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pManager->Owner->GetTechnoType());
	if (pTechnoTypeExt->MultiMindControl_ReleaseVictim)
	{
		// I hate Ares' completely rewritten things - secsome
		pManager->MaxControlNodes += 1;
		bool result = pManager->CanCapture(pTarget);
		pManager->MaxControlNodes -= 1;
		return result;
	}

	return pManager->CanCapture(pTarget);
}

bool CaptureManagerExt::FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool silent)
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

				if (!silent)
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

				pTarget->SetOwningHouse(pOriginOwner, !silent);
				pManager->DecideUnitFate(pTarget);
				pTarget->MindControlledBy = nullptr;

				GameDelete(pNode);

				pManager->ControlNodes.RemoveItem(i);

				return true;
			}
		}
	}

	return false;
}

bool CaptureManagerExt::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget,
	bool bRemoveFirst, AnimTypeClass* pControlledAnimType, bool silent)
{
	if (CaptureManagerExt::CanCapture(pManager, pTarget))
	{
		if (pManager->MaxControlNodes <= 0)
			return false;

		if (!pManager->InfiniteMindControl)
		{
			if (pManager->MaxControlNodes == 1 && pManager->ControlNodes.Count == 1)
				CaptureManagerExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
			else if (pManager->ControlNodes.Count == pManager->MaxControlNodes)
				if (bRemoveFirst)
					CaptureManagerExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
		}

		auto pControlNode = GameCreate<ControlNode>();
		if (pControlNode)
		{
			pControlNode->OriginalOwner = pTarget->Owner;
			pControlNode->Unit = pTarget;

			pManager->ControlNodes.AddItem(pControlNode);
			pControlNode->LinkDrawTimer.Start(RulesClass::Instance->MindControlAttackLineFrames);

			if (pTarget->SetOwningHouse(pManager->Owner->Owner, !silent))
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
					auto const pAnim = GameCreate<AnimClass>(pAnimType, location);

					pTarget->MindControlRingAnim = pAnim;
					pAnim->SetOwnerObject(pTarget);

					if (pBld)
						pAnim->ZAdjust = -1024;

				}

				return true;
			}

		}
	}

	return false;
}

bool CaptureManagerExt::CaptureUnit(CaptureManagerClass* pManager, AbstractClass* pTechno, AnimTypeClass* pControlledAnimType)
{
	if (const auto pTarget = generic_cast<TechnoClass*>(pTechno))
	{
		bool bRemoveFirst = false;
		if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pManager->Owner->GetTechnoType()))
			bRemoveFirst = pTechnoTypeExt->MultiMindControl_ReleaseVictim;

		return CaptureManagerExt::CaptureUnit(pManager, pTarget, bRemoveFirst, pControlledAnimType);
	}

	return false;
}

void CaptureManagerExt::DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot)
{
	// to be implemented (if needed). - secsome
}
