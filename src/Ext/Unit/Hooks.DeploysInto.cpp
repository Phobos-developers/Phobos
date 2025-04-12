#include <TerrainClass.h>
#include <IsometricTileTypeClass.h>

#include <Ext/TerrainType/Body.h>
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
		TechnoExt::ExtMap.Find(pStructure)->UpdateMindControlAnim();

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

	if (!TechnoExt::CanDeployIntoBuilding(pThis))
		action = Action::NoDeploy;

	return SkipGameCode;
}

// Exclude the specific unit who want to deploy
// Allow placing buildings on top of TerrainType with CanBeBuiltOn
DEFINE_HOOK(0x47C640, CellClass_CanThisExistHere_IgnoreSomething, 0x6)
{
	enum { CanNotExistHere = 0x47C6D1, CanExistHere = 0x47C6A0 };

	GET(const CellClass* const, pCell, EDI);
	GET(const BuildingTypeClass* const, pBuildingType, EAX);
	GET_STACK(HouseClass* const, pOwner, STACK_OFFSET(0x18, 0xC));

	if (!Game::IsActive)
		return CanExistHere;

	if (pBuildingType->LaserFence)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (pObject->WhatAmI() == AbstractType::Building)
			{
				return CanNotExistHere;
			}
			else if (const auto pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				if (!TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn)
					return CanNotExistHere;
			}
		}
	}
	else if (pBuildingType->LaserFencePost || pBuildingType->Gate)
	{
		bool skipFlag = TechnoExt::Deployer ? TechnoExt::Deployer->CurrentMapCoords == pCell->MapCoords : false;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (const auto pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				if (!TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn)
					return CanNotExistHere;
			}
			else if (pObject->AbstractFlags & AbstractFlags::Techno)
			{
				if (pObject == TechnoExt::Deployer)
				{
					skipFlag = true;
				}
				else if (pObject->WhatAmI() != AbstractType::Building
					|| pOwner != static_cast<BuildingClass*>(pObject)->Owner
					|| !static_cast<BuildingClass*>(pObject)->Type->LaserFence)
				{
					return CanNotExistHere;
				}
			}
		}

		if (pCell->OccupationFlags & (skipFlag ? 0x1F : 0x3F))
			return CanNotExistHere;
	}
	else if (pBuildingType->ToTile)
	{
		const auto isoTileTypeIndex = pCell->IsoTileTypeIndex;

		if (isoTileTypeIndex >= 0 && isoTileTypeIndex < IsometricTileTypeClass::Array.Count
			&& !IsometricTileTypeClass::Array.Items[isoTileTypeIndex]->Morphable)
		{
			return CanNotExistHere;
		}

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (pObject->WhatAmI() == AbstractType::Building)
				return CanNotExistHere;
		}
	}
	else
	{
		bool skipFlag = TechnoExt::Deployer ? TechnoExt::Deployer->CurrentMapCoords == pCell->MapCoords : false;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (pObject->AbstractFlags & AbstractFlags::Techno)
			{
				if (pObject == TechnoExt::Deployer)
					skipFlag = true;
				else
					return CanNotExistHere;
			}
			else if (const auto pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				if (!TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn)
					return CanNotExistHere;
			}
		}

		if (pCell->OccupationFlags & (skipFlag ? 0x1F : 0x3F))
			return CanNotExistHere;
	}

	return CanExistHere; // Continue check the overlays .etc
}

#pragma endregion
