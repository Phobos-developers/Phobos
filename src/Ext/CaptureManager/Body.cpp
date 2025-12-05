#include "Body.h"

#include <Ext/Techno/Body.h>

bool CaptureManagerExt::CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget)
{
	if (pManager->MaxControlNodes == 1)
		return pManager->CanCapture(pTarget);

	const auto pTechnoTypeExt = TechnoExt::ExtMap.Find(pManager->Owner)->TypeExtData;
	if (pTechnoTypeExt->MultiMindControl_ReleaseVictim)
	{
		// I hate Ares' completely rewritten things - secsome
		pManager->MaxControlNodes += 1;
		const bool result = pManager->CanCapture(pTarget);
		pManager->MaxControlNodes -= 1;
		return result;
	}

	return pManager->CanCapture(pTarget);
}

bool CaptureManagerExt::FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool silent)
{
	if (pTarget)
	{
		auto& mindControlRingAnim = pTarget->MindControlRingAnim;
		int nSound = pTarget->GetTechnoType()->MindClearedSound;
		auto const coord = pTarget->GetCoords();

		if (nSound == -1)
			nSound = RulesClass::Instance->MindClearedSound;

		for (int i = pManager->ControlNodes.Count - 1; i >= 0; --i)
		{
			const auto pNode = pManager->ControlNodes[i];
			if (pTarget == pNode->Unit)
			{
				if (mindControlRingAnim)
				{
					mindControlRingAnim->UnInit();
					mindControlRingAnim = nullptr;
				}

				if (!silent && nSound != -1)
					VocClass::PlayIndexAtPos(nSound, coord);

				// Fix : Player defeated should not get this unit.
				const auto pOriginOwner = pNode->OriginalOwner->Defeated
					? HouseClass::FindNeutral() : pNode->OriginalOwner;

				TechnoExt::ExtMap.Find(pTarget)->BeControlledThreatFrame = 0;

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
	bool bRemoveFirst, AnimTypeClass* pControlledAnimType, bool silent, int threatDelay)
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

		auto const pControlNode = GameCreate<ControlNode>();
		pControlNode->OriginalOwner = pTarget->Owner;
		pControlNode->Unit = pTarget;

		pManager->ControlNodes.AddItem(pControlNode);
		pControlNode->LinkDrawTimer.Start(RulesClass::Instance->MindControlAttackLineFrames);

		if (threatDelay > 0)
			TechnoExt::ExtMap.Find(pTarget)->BeControlledThreatFrame = Unsorted::CurrentFrame + threatDelay;

		auto const pOwner = pManager->Owner;

		if (pTarget->SetOwningHouse(pOwner->Owner, !silent))
		{
			pTarget->MindControlledBy = pOwner;

			pManager->DecideUnitFate(pTarget);

			auto const pBld = abstract_cast<BuildingClass*, true>(pTarget);
			auto const pType = pTarget->GetTechnoType();
			CoordStruct location = pTarget->GetCoords();

			if (pBld)
				location.Z += pBld->Type->Height * Unsorted::LevelHeight;
			else
				location.Z += pType->MindControlRingOffset;

			if (pControlledAnimType)
			{
				auto const pAnim = GameCreate<AnimClass>(pControlledAnimType, location);

				pTarget->MindControlRingAnim = pAnim;
				pAnim->SetOwnerObject(pTarget);

				if (pBld)
					pAnim->ZAdjust = -1024;
			}

			return true;
		}
	}

	return false;
}

bool CaptureManagerExt::CaptureUnit(CaptureManagerClass* pManager, AbstractClass* pTechno, AnimTypeClass* pControlledAnimType, int threatDelay)
{
	if (const auto pTarget = generic_cast<TechnoClass*>(pTechno))
	{
		const auto pTechnoTypeExt = TechnoExt::ExtMap.Find(pManager->Owner)->TypeExtData;
		return CaptureManagerExt::CaptureUnit(pManager, pTarget, pTechnoTypeExt->MultiMindControl_ReleaseVictim, pControlledAnimType, false, threatDelay);
	}

	return false;
}

void CaptureManagerExt::DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot)
{
	// to be implemented (if needed). - secsome
}
