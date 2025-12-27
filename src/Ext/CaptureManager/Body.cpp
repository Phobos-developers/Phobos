#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Utilities/AresHelper.h>

int CaptureManagerExt::GetControlledTotalSize(CaptureManagerClass* pManager)
{
	int totalSize = 0;

	for (const auto pNode : pManager->ControlNodes)
	{
		if (const auto pTechno = pNode->Unit)
			totalSize += TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType())->MindControlSize;
	}

	return totalSize;
}

struct DummyExtHere
{
	char _[0x9C];
	bool DriverKilled;
};

struct DummyTypeExtHere
{
	char _[0x131];
	bool Vet_PsionicsImmune;
	bool __[0x6];
	bool Elite_PsionicsImmune;
};

bool CaptureManagerExt::CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget)
{
	// target exists and doesn't belong to capturing player
	if (!pTarget)
		return false;

	if (pManager->MaxControlNodes <= 0)
		return false;

	const auto pOwner = pManager->Owner;

	if (pTarget->Owner == pOwner->Owner)
		return false;

	const auto pTargetType = pTarget->GetTechnoType();

	// generally not capturable
	if (pTargetType->ImmuneToPsionics)
		return false;

	if (AresHelper::CanUseAres)
	{
		const auto pTargetTypeExt_Ares = reinterpret_cast<DummyTypeExtHere*>(pTargetType->align_2FC);

		switch (pTarget->Veterancy.GetRemainingLevel())
		{
		case Rank::Elite:
			if (pTargetTypeExt_Ares->Elite_PsionicsImmune)
				return false;

		case Rank::Veteran:
			if (pTargetTypeExt_Ares->Vet_PsionicsImmune)
				return false;

		default:
			break;
		}
	}

	// disallow capturing bunkered units
	if (pTarget->BunkerLinkedItem && pTarget->WhatAmI() == AbstractType::Unit)
		return false;

	if (pTarget->IsMindControlled() || pTarget->MindControlledByHouse)
		return false;

	// free slot? (move on if infinite or single slot which will be freed if used)
	if (!pManager->InfiniteMindControl && pManager->MaxControlNodes != 1)
	{
		const auto pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

		if (!pOwnerTypeExt->MindControl_IgnoreSize)
		{
			const int totalSize = CaptureManagerExt::GetControlledTotalSize(pManager);
			const int available = pOwnerTypeExt->MultiMindControl_ReleaseVictim ? pManager->MaxControlNodes : pManager->MaxControlNodes - totalSize;

			if (TechnoTypeExt::ExtMap.Find(pTargetType)->MindControlSize > available)
				return false;
		}
		else
		{
			if (pManager->ControlNodes.Count >= pManager->MaxControlNodes && !pOwnerTypeExt->MultiMindControl_ReleaseVictim)
				return false;
		}
	}

	// currently disallowed
	const auto mission = pTarget->CurrentMission;

	if (pTarget->IsIronCurtained() || mission == Mission::Selling || mission == Mission::Construction)
		return false;

	// driver killed. has no mind.
	if (AresHelper::CanUseAres && reinterpret_cast<DummyExtHere*>(*(uintptr_t*)((char*)pTarget + 0x154))->DriverKilled)
		return false;

	return true;
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
	bool removeFirst, AnimTypeClass* pControlledAnimType, bool silent, int threatDelay)
{
	if (CaptureManagerExt::CanCapture(pManager, pTarget))
	{
		if (pManager->MaxControlNodes <= 0)
			return false;

		if (!pManager->InfiniteMindControl)
		{
			if (pManager->MaxControlNodes == 1 && pManager->ControlNodes.Count == 1)
			{
				CaptureManagerExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
			}
			else if (pManager->ControlNodes.Count > 0 && removeFirst)
			{
				const auto pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pManager->Owner->GetTechnoType());

				if (pOwnerTypeExt->MindControl_IgnoreSize)
				{
					if (pManager->ControlNodes.Count == pManager->MaxControlNodes)
						CaptureManagerExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
				}
				else
				{
					const auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());

					while (pManager->ControlNodes.Count && pTargetTypeExt->MindControlSize > pManager->MaxControlNodes - CaptureManagerExt::GetControlledTotalSize(pManager))
						CaptureManagerExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
				}
			}
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
