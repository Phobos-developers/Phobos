#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>

static void TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo)
{
	auto pAnimType = pTechnoFrom->MindControlRingAnim ?
		pTechnoFrom->MindControlRingAnim->Type : TechnoExt::ExtMap.Find(pTechnoFrom)->MindControlRingAnimType;

	if (auto Controller = pTechnoFrom->MindControlledBy)
	{
		if (auto Manager = Controller->CaptureManager)
		{
			CaptureManagerExt::FreeUnit(Manager, pTechnoFrom, true);

			if (CaptureManagerExt::CaptureUnit(Manager, pTechnoTo, false, pAnimType, true))
			{
				if (auto pBld = abstract_cast<BuildingClass*>(pTechnoTo))
				{
					// Capturing the building after unlimbo before buildup has finished or even started appears to throw certain things off,
					// Hopefully this is enough to fix most of it like anims playing prematurely etc.
					pBld->ActuallyPlacedOnMap = false;
					pBld->DestroyNthAnim(BuildingAnimSlot::All);

					pBld->BeginMode(BStateType::Construction);
					pBld->QueueMission(Mission::Construction, false);
				}
			}
			else
			{
				int nSound = pTechnoTo->GetTechnoType()->MindClearedSound;
				if (nSound == -1)
					nSound = RulesClass::Instance->MindClearedSound;

				if (nSound != -1)
					VocClass::PlayIndexAtPos(nSound, pTechnoTo->Location);
			}
		}
	}
	else if (auto MCHouse = pTechnoFrom->MindControlledByHouse)
	{
		pTechnoTo->MindControlledByHouse = MCHouse;
		pTechnoFrom->MindControlledByHouse = nullptr;
	}
	else if (pTechnoFrom->MindControlledByAUnit) // Perma MC
	{
		pTechnoTo->MindControlledByAUnit = true;

		auto const pBuilding = abstract_cast<BuildingClass*>(pTechnoTo);
		CoordStruct location = pTechnoTo->GetCoords();

		location.Z += pBuilding
			? pBuilding->Type->Height * Unsorted::LevelHeight
			: pTechnoTo->GetTechnoType()->MindControlRingOffset;

		auto const pAnim = pAnimType
			? GameCreate<AnimClass>(pAnimType, location, 0, 1)
			: nullptr;

		if (pAnim)
		{
			if (pBuilding)
				pAnim->ZAdjust = -1024;

			pTechnoTo->MindControlRingAnim = pAnim;
			pAnim->SetOwnerObject(pTechnoTo);
		}
	}
}

DEFINE_HOOK(0x739956, UnitClass_Deploy_Transfer, 0x6)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	TransferMindControlOnDeploy(pUnit, pStructure);
	ShieldClass::SyncShieldToAnother(pUnit, pStructure);
	TechnoExt::SyncInvulnerability(pUnit, pStructure);
	AttachEffectClass::TransferAttachedEffects(pUnit, pStructure);

	return 0;
}

DEFINE_HOOK(0x44A03C, BuildingClass_Mi_Selling_Transfer, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	GET(UnitClass*, pUnit, EBX);

	TransferMindControlOnDeploy(pStructure, pUnit);
	ShieldClass::SyncShieldToAnother(pStructure, pUnit);
	TechnoExt::SyncInvulnerability(pStructure, pUnit);
	AttachEffectClass::TransferAttachedEffects(pStructure, pUnit);

	pUnit->QueueMission(Mission::Hunt, true);
	//Why?
	return 0;
}

DEFINE_HOOK(0x449E2E, BuildingClass_Mi_Selling_CreateUnit, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	R->ECX<HouseClass*>(pStructure->GetOriginalOwner());

	// Remember MC ring animation.
	if (pStructure->IsMindControlled())
	{
		auto const pTechnoExt = TechnoExt::ExtMap.Find(pStructure);
		pTechnoExt->UpdateMindControlAnim();
	}

	return 0x449E34;
}

DEFINE_HOOK(0x7396AD, UnitClass_Deploy_CreateBuilding, 0x6)
{
	GET(UnitClass*, pUnit, EBP);
	R->EDX<HouseClass*>(pUnit->GetOriginalOwner());

	return 0x7396B3;
}

// Game removes deploying vehicles from map temporarily to check if there's enough
// space to deploy into a building when displaying allow/disallow deploy cursor.
// This can cause desyncs if there are certain types of units around the deploying unit.
// Only reasonable way to solve this is to perform the cell clear check on every client per frame
// and use that result in cursor display which is client-specific. This is now implemented in multiplayer games only.
#pragma region DeploysIntoDesyncFix

DEFINE_HOOK(0x73635B, UnitClass_AI_DeploysIntoDesyncFix, 0x6)
{
	if (!SessionClass::IsMultiplayer())
		return 0;

	GET(UnitClass*, pThis, ESI);

	if (pThis->Type->DeploysInto)
		TechnoExt::ExtMap.Find(pThis)->CanCurrentlyDeployIntoBuilding = TechnoExt::CanDeployIntoBuilding(pThis);

	return 0;
}

DEFINE_HOOK(0x73FEC1, UnitClass_WhatAction_DeploysIntoDesyncFix, 0x6)
{
	if (!SessionClass::IsMultiplayer())
		return 0;

	enum { SkipGameCode = 0x73FFDF };

	GET(UnitClass*, pThis, ESI);
	LEA_STACK(Action*, pAction, STACK_OFFSET(0x20, 0x8));

	if (!TechnoExt::ExtMap.Find(pThis)->CanCurrentlyDeployIntoBuilding)
		*pAction = Action::NoDeploy;

	return SkipGameCode;
}

#pragma endregion
