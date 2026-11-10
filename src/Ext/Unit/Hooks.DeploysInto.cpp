#include <IsometricTileTypeClass.h>
#include <SessionClass.h>

#include "Body.h"

#include <Ext/TerrainType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/Building/Body.h>

#pragma region AllowDeployControlledMCV

DEFINE_HOOK_AGAIN(0x443770, TechnoClass_AllowDeployControlledMCV, 0x6)// BuildingClass::CellClickedAction
DEFINE_HOOK_AGAIN(0x443AB0, TechnoClass_AllowDeployControlledMCV, 0x6)// BuildingClass::SetRallyPoint
DEFINE_HOOK_AGAIN(0x44F614, TechnoClass_AllowDeployControlledMCV, 0x6)// BuildingClass::IsControllable
DEFINE_HOOK(0x700ED0, TechnoClass_AllowDeployControlledMCV, 0x6)// UnitClass::CanDeploySlashUnload
{
	return RulesExt::Global()->AllowDeployControlledMCV ? R->Origin() + 0xE : 0;
}

#pragma endregion

static inline void TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo)
{
	const auto pAnimType = pTechnoFrom->MindControlRingAnim
		? pTechnoFrom->MindControlRingAnim->Type
		: TechnoExt::Fetch(pTechnoFrom)->MindControlRingAnimType;

	if (const auto Controller = pTechnoFrom->MindControlledBy)
	{
		if (const auto Manager = Controller->CaptureManager)
		{
			CaptureManagerExt::FreeUnit(Manager, pTechnoFrom, true);

			if (CaptureManagerExt::CaptureUnit(Manager, pTechnoTo, false, pAnimType, true))
			{
				if (const auto pBld = abstract_cast<BuildingClass*, true>(pTechnoTo))
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
	else if (const auto MCHouse = pTechnoFrom->MindControlledByHouse)
	{
		pTechnoTo->MindControlledByHouse = MCHouse;
		pTechnoFrom->MindControlledByHouse = nullptr;
	}
	else if (pTechnoFrom->MindControlledByAUnit) // Perma MC
	{
		pTechnoTo->MindControlledByAUnit = true;

		const auto pBuilding = abstract_cast<BuildingClass*, true>(pTechnoTo);
		CoordStruct location = pTechnoTo->GetCoords();

		location.Z += pBuilding
			? pBuilding->Type->Height * Unsorted::LevelHeight
			: pTechnoTo->GetTechnoType()->MindControlRingOffset;

		const auto pAnim = pAnimType
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

	if (pUnit->Tiberium.GetTotalAmount() > 0.0f)
	{
		if (auto const pHouse = pStructure->Owner)
		{
			auto const pBldTypeExt = BuildingTypeExt::Fetch(pStructure->Type);
			int const storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

			if (pBldTypeExt->Refinery_UseStorage && (pHouse->IsControlledByHuman() || SessionClass::Instance.GameMode == GameMode::Campaign))
			{
				int const lastStorage = static_cast<int>(pHouse->OwnedTiberium.GetTotalAmount());
				int const lastTotalStorage = pHouse->TotalStorage;

				for (int i = 0; i < 4; ++i)
				{
					float const amount = pUnit->Tiberium.GetAmount(i);
					if (amount > 0.0f)
					{
						pUnit->Tiberium.RemoveAmount(amount, i);
						int const idxStorage = storageTiberiumIndex >= 0 ? storageTiberiumIndex : i;
						auto const pTiberium = TiberiumClass::Array.GetItem(i);
						auto const pDepositableTiberium = TiberiumClass::Array.GetItem(idxStorage);
						float remaining = (amount * pTiberium->Value) / pDepositableTiberium->Value;

						if (pStructure->Type->Storage > 0)
						{
							float const freeSpace = static_cast<float>(pStructure->Type->Storage) - pStructure->Tiberium.GetTotalAmount();
							if (freeSpace > 0.0f)
							{
								float const toStore = std::min(remaining, freeSpace);
								pStructure->Tiberium.AddAmount(toStore, idxStorage);
								pHouse->OwnedTiberium.AddAmount(toStore, idxStorage);
								remaining -= toStore;
							}
						}

						if (remaining > 0.0f)
							pHouse->GiveTiberium(remaining, idxStorage);
					}
				}

				pHouse->UpdateAllSilos(lastStorage, lastTotalStorage);
			}
			else
			{
				for (int i = 0; i < 4; ++i)
				{
					float const amount = pUnit->Tiberium.GetAmount(i);
					if (amount > 0.0f)
					{
						pUnit->Tiberium.RemoveAmount(amount, i);
						pHouse->GiveTiberium(amount, i);
					}
				}
			}
		}
	}

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

	if (pStructure->Tiberium.GetTotalAmount() > 0.0f)
	{
		int const maxStorage = pUnit->Type->Storage > 0 ? pUnit->Type->Storage : pStructure->Type->Storage;

		for (int i = 0; i < 4; ++i)
		{
			float const amount = pStructure->Tiberium.GetAmount(i);
			if (amount > 0.0f)
			{
				pStructure->Tiberium.RemoveAmount(amount, i);

				if (maxStorage > 0)
				{
					float const freeSpace = static_cast<float>(maxStorage) - pUnit->Tiberium.GetTotalAmount();
					if (freeSpace > 0.0f)
					{
						float const toStore = std::min(amount, freeSpace);
						pUnit->Tiberium.AddAmount(toStore, i);
					}
				}
			}
		}

		if (auto const pHouse = pStructure->Owner)
			pHouse->UpdateAllSilos(0, 0);
	}

	// This line will break the bahavior of UnDeploysInto buildings. However, it might serve a purpose that no one knows yet
	// Comment out the line instead of removing it for now, so we can turn to it if something related goes wrong in the future
	// pUnit->QueueMission(Mission::Hunt, true);
	return 0;
}

DEFINE_HOOK(0x449E2E, BuildingClass_Mi_Selling_CreateUnit, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	R->ECX<HouseClass*>(pStructure->GetOriginalOwner());

	// Remember MC ring animation.
	if (pStructure->IsMindControlled())
	{
		auto const pTechnoExt = TechnoExt::Fetch(pStructure);
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
// This can cause desyncs if there are certain types of units around the deploying
// unit because the OccupationFlags may be accidentally cleared, or the order of
// the objects linked list may be scrambled.
#pragma region DeploysIntoDesyncFix

DEFINE_HOOK(0x73FEC1, UnitClass_WhatAction_DeploysIntoDesyncFix, 0x6)
{
	enum { SkipGameCode = 0x73FFDF };

	GET(UnitClass* const, pThis, ESI);
	REF_STACK(Action, action, STACK_OFFSET(0x20, 0x8));

	if (!UnitExt::CanDeployIntoBuilding(pThis))
		action = Action::NoDeploy;

	return SkipGameCode;
}

DEFINE_HOOK(0x7396D2, UnitClass_TryToDeploy_Transfer, 0x5)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	if (pUnit->Type->DeployToFire && pUnit->Target)
		pStructure->LastTarget = pUnit->Target;

	const auto pStructureExt = BuildingExt::Fetch(pStructure);
	pStructureExt->DeployedTechno = true;

	return 0;
}

#pragma endregion
