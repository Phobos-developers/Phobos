#include "Body.h"

#include <Ext/Script/Body.h>
/*
DEFINE_HOOK(0x730B8F, DeployCommand_UniversalDeploy, 0x6)
{
	GET(int, index, EDI);

	TechnoClass* pThis = static_cast<TechnoClass*>(ObjectClass::CurrentObjects->GetItem(index));

	if (!pThis || !ScriptExt::IsUnitAvailable(pThis, false))
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt || pExt->Convert_UniversalDeploy_InProgress)
		return 0;

	// Infantry checks are done in another hook
	if (pThis->WhatAmI() == AbstractType::Infantry)
		return 0;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (!pTypeExt || !pTypeExt->Convert_UniversalDeploy.isset())
		return 0;

	// Building case, send the undeploy signal
	if (pThis->WhatAmI() == AbstractType::Building)
	{
		// Is the building in the middle of a deployment?
		auto const pBuilding = static_cast<BuildingClass*>(pThis);
		if (pBuilding->BState == (int)BStateType::Construction)
			return 0;

		pExt->Convert_UniversalDeploy_InProgress = true;
		pExt->Convert_UniversalDeploy_IsOriginalDeployer = true;

		if (pThis->Target)
			pExt->Convert_UniversalDeploy_RememberTarget = pThis->Target;

		return 0x730C10;
	}

	// Unit case, send the undeploy signal only if this object meets the all the requisites
	if (pThis->WhatAmI() == AbstractType::Unit)
	{
		// Is the unit in the middle of a deployment?
		auto const pUnit = static_cast<UnitClass*>(pThis);
		if (pUnit->Undeploying || pUnit->Deploying || pUnit->IsDeploying)
			return 0;

		auto const pIntoBuildingType = pTypeExt->Convert_UniversalDeploy.Get()->WhatAmI() == AbstractType::Building ? abstract_cast<BuildingTypeClass*>(pTypeExt->Convert_UniversalDeploy.Get()) : nullptr;
		bool canDeployIntoStructure = pIntoBuildingType ? pIntoBuildingType->CanCreateHere(CellClass::Coord2Cell(pThis->GetCoords()), pThis->Owner) : false;
		auto pCell = pThis->GetCell();
		int nObjectsInCell = 0;

		// Count objects located in the desired cell
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			auto const pItem = static_cast<TechnoClass*>(pObject);

			if (pItem && pItem != pThis)
				nObjectsInCell++;
		}

		// Abort if the cell is occupied with objects or can not be deployed into structure. And move the unit to a different nearby location.
		if ((nObjectsInCell > 0 && !pIntoBuildingType) || pIntoBuildingType && !canDeployIntoStructure)
		{
			pExt->Convert_UniversalDeploy_RememberTarget = nullptr;
			pExt->Convert_UniversalDeploy_InProgress = false;
			pThis->IsFallingDown = false;
			pThis->Scatter(CoordStruct::Empty, true, false);

			return 0;
		}

		auto const pFoot = static_cast<FootClass*>(pThis);

		// Stop the deployable unit, can not be converted if the object is moving
		if (!pThis->IsFallingDown && pThis->CurrentMission != Mission::Guard)
		{
			pExt->Convert_UniversalDeploy_RememberTarget = pThis->Target;

			// Reset previous command
			pFoot->SetTarget(nullptr);
			pFoot->SetDestination(nullptr, false);
			pFoot->ForceMission(Mission::Guard);
		}

		// If is set DeployToLand then is probably a flying unit that only can be deployed in ground
		if (pTypeExt->Convert_DeployToLand)
		{
			bool isFlyingUnit = pThis->GetTechnoType()->MovementZone == MovementZone::Fly || pThis->GetTechnoType()->ConsideredAircraft;

			// If the cell is occupied abort operation
			if (isFlyingUnit && pThis->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false) != Move::OK)
			{
				pExt->Convert_UniversalDeploy_RememberTarget = nullptr;
				pExt->Convert_UniversalDeploy_InProgress = false;
				pThis->IsFallingDown = false;
				pThis->Scatter(CoordStruct::Empty, true, false);

				return 0;
			}

			// I don't know if is the right action to force air units to land, including units with BalloonHover=yes
			pThis->IsFallingDown = true;
		}

		// Set the deployment signal, indicating the process hasn't finished
		pExt->Convert_UniversalDeploy_IsOriginalDeployer = true;
		pExt->Convert_UniversalDeploy_InProgress = true;

		if (pThis->Target)
			pExt->Convert_UniversalDeploy_RememberTarget = pThis->Target;

		return 0x730C10;
	}

	return 0;
}

DEFINE_HOOK(0x522510, InfantryClass_UniversalDeploy_DoingDeploy, 0x6)
{
	GET(InfantryClass*, pThis, ECX);

	if (!pThis)
		return 0;

	auto pOld = static_cast<TechnoClass*>(pThis);
	if (!pOld)
		return 0;

	auto const pOldExt = TechnoExt::ExtMap.Find(pOld);
	if (!pOldExt || pOldExt->Convert_UniversalDeploy_InProgress)
		return 0;

	auto const pOldTypeExt = TechnoTypeExt::ExtMap.Find(pOld->GetTechnoType());
	if (!pOldTypeExt)
		return 0;

	// Is the infantry in the middle of a deployment?
	if (pThis->IsDeploying)
		return 0;

	auto const pNewType = pOldTypeExt->Convert_UniversalDeploy.Get();
	bool oldIsFlyingUnit = pThis->GetTechnoType()->MovementZone == MovementZone::Fly || pThis->GetTechnoType()->ConsideredAircraft;
	auto pCell = pOld->GetCell();
	bool cellIsOnWater = (pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach);
	bool isNewTechnoAmphibious = pNewType->MovementZone == MovementZone::Amphibious || pNewType->MovementZone == MovementZone::AmphibiousCrusher || pNewType->MovementZone == MovementZone::AmphibiousDestroyer;
	bool isNewTechnoAircraft = pNewType->ConsideredAircraft || pNewType->MovementZone == MovementZone::Fly;

	// If the deployer is over a water cell and the future deployed object doesn't support water cells abort the operation
	if (cellIsOnWater && !(isNewTechnoAmphibious || pNewType->Naval || isNewTechnoAircraft))
	{
		pOldExt->Convert_UniversalDeploy_RememberTarget = nullptr;
		pOldExt->Convert_UniversalDeploy_InProgress = false;
		pOld->IsFallingDown = false;
		pOld->Scatter(CoordStruct::Empty, true, false);

		return 0;
	}

	// Preparing UniversalDeploy logic
	bool inf2inf = pNewType->WhatAmI() == AbstractType::InfantryType;

	int nObjectsInCell = 0;

	for (auto pObject = pOld->GetCell()->FirstObject; pObject; pObject = pObject->NextObject)
	{
		auto const pItem = static_cast<TechnoClass*>(pObject);

		if (pItem && pItem != pOld)
			nObjectsInCell++;
	}

	// It the cell is occupied and isn't a infantry to infantry conversion the operation is aborted
	if (nObjectsInCell > 0 && !inf2inf)
	{
		pOldExt->Convert_UniversalDeploy_RememberTarget = nullptr;
		pOldExt->Convert_UniversalDeploy_InProgress = false;
		pOld->IsFallingDown = false;
		pOld->Scatter(CoordStruct::Empty, true, false);

		return 0;
	}

	if (pOldTypeExt->Convert_DeployToLand)
	{
		auto newCell = MapClass::Instance->GetCellAt(pThis->Location);

		// If the cell is occupied abort operation
		if (oldIsFlyingUnit && pThis->IsCellOccupied(newCell, FacingType::None, -1, nullptr, false) != Move::OK)
		{
			pOldExt->Convert_UniversalDeploy_RememberTarget = nullptr;
			pOldExt->Convert_UniversalDeploy_InProgress = false;
			pOld->IsFallingDown = false;
			pOld->Scatter(CoordStruct::Empty, true, false);

			return 0;
		}

		pThis->IsFallingDown = true;
	}

	pOldExt->Convert_UniversalDeploy_IsOriginalDeployer = true;
	pOldExt->Convert_UniversalDeploy_InProgress = true;

	if (pThis->Target)
		pOldExt->Convert_UniversalDeploy_RememberTarget = pThis->Target;

	return 0;
}

DEFINE_HOOK(0x449C47, BuildingClass_MissionDeconstruction_UniversalDeploy, 0x6)
{
	enum { Skip = 0x449E00 };

	GET(BuildingClass*, pBuilding, ECX);

	if (!pBuilding)
		return 0;

	// Check if is the UniversalDeploy or a standard deploy
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType());
	if (!pTypeExt || !pTypeExt->Convert_UniversalDeploy.isset())
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pBuilding);
	if (!pExt)
		return 0;

	// Skip code for this UniversalDeploy logic
	if (pExt->Convert_UniversalDeploy_InProgress)
		return Skip;

	return 0;
}

DEFINE_HOOK(0x44725F, BuildingClass_WhatAction_UniversalDeploy_EnableDeployIcon, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(FootClass*, pFoot, 0x1C);

	if (!pThis || !pFoot || pFoot->Location != pThis->Location)
		return 0;

	auto pFootTechno = static_cast<TechnoClass*>(pFoot);
	if (!pFootTechno)
		return 0;

	auto pBldExt = TechnoExt::ExtMap.Find(pThis);
	if (!pBldExt)
		return 0;

	if (pBldExt->Convert_UniversalDeploy_InProgress)
	{
		R->EAX(Action::NoDeploy);
		return 0x447273;
	}

	auto pBldTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (!pBldTypeExt || !pBldTypeExt->Convert_UniversalDeploy.isset())
		return 0;

	auto const pNewType = pBldTypeExt->Convert_UniversalDeploy.Get();

	// If target deploy is a structure we can show the "No Deploy" icon if the foundation has obstacles
	if (pNewType->WhatAmI() == AbstractType::BuildingType)
	{
		auto pNewBldType = static_cast<BuildingTypeClass*>(pNewType);
		bool canDeployIntoStructure = TechnoExt::CanDeployIntoBuilding(pThis, false, pNewBldType);

		if (!canDeployIntoStructure)
		{
			R->EAX(Action::NoDeploy);
			return 0x447273;
		}
	}

	R->EAX(Action::Self_Deploy);

	return 0x447273;
}

// Probably obsolete since I added a new hook
DEFINE_HOOK(0x457DE9, BuildingClass_EvictOccupiers_UniversalDeploy_DontEjectOccupiers, 0xC)
{
	GET(BuildingClass*, pBuilding, ECX);

	if (!pBuilding)
		return 0;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->Type);
	if (!pTypeExt || !pTypeExt->Convert_UniversalDeploy.isset())
		return 0;

	// Don't eject the infantry if the UniversalDeploy is being used. UniversalDeploy Manages that operation
	return 0x4581DB;
}

DEFINE_HOOK(0x4ABEE9, BuildingClass_MouseLeftRelease_UniversalDeploy_ExecuteDeploy, 0x7)
{
	GET(TechnoClass* const, pTechno, ESI);
	GET(Action const, actionType, EBX);

	if (actionType != Action::Self_Deploy || !pTechno || !pTechno->IsSelected || !ScriptExt::IsUnitAvailable(pTechno, false))
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pTechno);
	if (!pExt || pExt->Convert_UniversalDeploy_InProgress)
		return 0;

	if (pTechno->WhatAmI() == AbstractType::Building)
	{
		// Is the building in the middle of a deployment?
		auto const pBuilding = static_cast<BuildingClass*>(pTechno);
		if (pBuilding->BState == (int)BStateType::Construction)
			return 0;
	}

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
	if (!pTypeExt || !pTypeExt->Convert_UniversalDeploy.isset())
		return 0;

	auto const pNewTechnoType = pTypeExt->Convert_UniversalDeploy.Get();
	bool oldIsFlyingUnit = pTechno->GetTechnoType()->MovementZone == MovementZone::Fly || pTechno->GetTechnoType()->ConsideredAircraft;
	auto pCell = pTechno->GetCell();
	bool cellIsOnWater = (pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach);
	bool isNewTechnoAmphibious = pNewTechnoType->MovementZone == MovementZone::Amphibious || pNewTechnoType->MovementZone == MovementZone::AmphibiousCrusher ||
		pNewTechnoType->MovementZone == MovementZone::AmphibiousDestroyer;
	bool isNewTechnoAircraft = pNewTechnoType->ConsideredAircraft || pNewTechnoType->MovementZone == MovementZone::Fly;

	if (pTechno->WhatAmI() == AbstractType::Infantry)
		return 0; // Checked in another hook. Or should I prevent the execution of that hook and put the code here?

	// If the deployer is over a water cell and the future deployed object doesn't support water cells abort the operation
	if (cellIsOnWater && !(isNewTechnoAmphibious || pNewTechnoType->Naval || isNewTechnoAircraft))
	{
		pExt->Convert_UniversalDeploy_RememberTarget = nullptr;
		pExt->Convert_UniversalDeploy_InProgress = false;
		pTechno->IsFallingDown = false;
		pTechno->Scatter(CoordStruct::Empty, true, false);

		return 0;
	}

	// Building case, send the undeploy signal
	if (pTechno->WhatAmI() == AbstractType::Building)
	{
		auto pBld = static_cast<BuildingClass*>(pTechno);
		auto pBldType = static_cast<BuildingTypeClass*>(pNewTechnoType);
		bool canDeployIntoStructure = TechnoExt::CanDeployIntoBuilding(pBld, false, pBldType);

		if (!canDeployIntoStructure)
		{
			pExt->Convert_UniversalDeploy_RememberTarget = nullptr;
			pExt->Convert_UniversalDeploy_InProgress = false;
			pTechno->IsFallingDown = false;
			pTechno->Scatter(CoordStruct::Empty, true, false);

			return 0;
		}

		R->EBX(Action::None);
		pExt->Convert_UniversalDeploy_InProgress = true;
		pExt->Convert_UniversalDeploy_IsOriginalDeployer = true;

		if (pTechno->Target)
			pExt->Convert_UniversalDeploy_RememberTarget = pTechno->Target;

		return 0;
	}

	// vehicles case, send the undeploy signal only if meets the all the requisites
	if (pTechno->WhatAmI() == AbstractType::Unit)
	{
		const auto pUnit = abstract_cast<UnitClass*>(pTechno);
		const auto pIntoBuildingType = pTypeExt->Convert_UniversalDeploy.Get()->WhatAmI() == AbstractType::Building ? abstract_cast<BuildingTypeClass*>(pTypeExt->Convert_UniversalDeploy.Get()) : nullptr;
		bool canDeployIntoStructure = pIntoBuildingType ? pIntoBuildingType->CanCreateHere(CellClass::Coord2Cell(pUnit->GetCoords()), pUnit->Owner) : false;
		int nObjectsInCell = 0;

		// Count objects located in the desired cell
		for (auto pObject = pTechno->GetCell()->FirstObject; pObject; pObject = pObject->NextObject)
		{
			auto const pItem = static_cast<TechnoClass*>(pObject);

			if (pItem && pItem != pTechno)
				nObjectsInCell++;
		}

		// Abort if the cell is occupied with objects or can not be deployed into structure. And move the unit to a different nearby location.
		if ((nObjectsInCell > 0 && !pIntoBuildingType) || pIntoBuildingType && !canDeployIntoStructure)
		{
			pExt->Convert_UniversalDeploy_RememberTarget = nullptr;
			pExt->Convert_UniversalDeploy_InProgress = false;
			pTechno->IsFallingDown = false;
			pTechno->Scatter(CoordStruct::Empty, true, false);

			return 0;
		}

		auto pFoot = static_cast<FootClass*>(pTechno);

		// Stop the deployable unit, can not be converted if the object is moving
		if (!pTechno->IsFallingDown && pTechno->CurrentMission != Mission::Guard)
		{
			if (pTechno->Target)
				pExt->Convert_UniversalDeploy_RememberTarget = pTechno->Target;

			// Reset previous command
			pExt->Convert_UniversalDeploy_RememberTarget = nullptr;
			pFoot->SetTarget(nullptr);
			pFoot->SetDestination(nullptr, false);
			pFoot->ForceMission(Mission::Guard);
		}

		if (pTypeExt->Convert_DeployToLand)
		{
			// If the cell is occupied abort operation
			if (oldIsFlyingUnit && pTechno->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false) != Move::OK)
			{
				pExt->Convert_UniversalDeploy_RememberTarget = nullptr;
				pExt->Convert_UniversalDeploy_InProgress = false;
				pTechno->IsFallingDown = false;
				pTechno->Scatter(CoordStruct::Empty, true, false);

				return 0;
			}

			// I don't know if is the right action to force air units to land, including units with BalloonHover=yes
			pTechno->IsFallingDown = true;
			//pFoot->ParalysisTimer.Start(15);
		}

		// Set the deployment signal, indicating the process hasn't finished
		pExt->Convert_UniversalDeploy_IsOriginalDeployer = true;
		pExt->Convert_UniversalDeploy_InProgress = true;

		if (pTechno->Target)
			pExt->Convert_UniversalDeploy_RememberTarget = pTechno->Target;
	}

	return 0;
}

// Avoid the sell cursor while the structure is being deployed with the UniversalDeploy
DEFINE_HOOK(0x4494DC, BuildingClass_CanDemolish_UniversalDeploy, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt && pExt->Convert_UniversalDeploy_InProgress)
		return 0x449536;

	return 0;
}

// Skip voxel turret drawing while the structure is being deployed with the UniversalDeploy
DEFINE_HOOK(0x43DF6E, BuildingClass_Draw_UniversalDeploy, 0x6)
{
	GET(BuildingClass*, pThis, EBP);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt && pExt->Convert_UniversalDeploy_InProgress)
		return 0x43E795;

	return 0;
}

DEFINE_HOOK(0x73B4DA, UnitClass_DrawVoxel_UniversalDeploy_DontRenderObject, 0x6)
{
	enum { Skip = 0x73C5DC };

	GET(UnitClass*, pThis, EBP);

	if (!pThis)
		return 0;

	auto pTechno = static_cast<TechnoClass*>(pThis);
	if (!pTechno)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pTechno);
	if (!pExt || !pExt->Convert_UniversalDeploy_MakeInvisible)
		return 0;

	// VXL units won't draw graphics when deploy
	return Skip;
}

// Make object graphics invisible because they aren't rendered
DEFINE_HOOK(0x73C602, TechnoClass_DrawObject_UniversalDeploy_DontRenderObject, 0x6)
{
	enum { Skip = 0x73CE00 };

	GET(TechnoClass*, pThis, ECX);

	if (!pThis)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt || !pExt->Convert_UniversalDeploy_MakeInvisible)
		return 0;

	// SHP units won't draw graphics when deploy
	return Skip;
}

// Make object graphics invisible because they aren't rendered
DEFINE_HOOK(0x518FBC, InfantryClass_DrawIt_UniversalDeploy_DontRenderObject, 0x6)
{
	enum { Skip = 0x5192B5 };

	GET(InfantryClass*, pThis, EBP);

	if (!pThis)
		return 0;

	auto pTechno = static_cast<TechnoClass*>(pThis);
	if (!pTechno)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pTechno);
	if (!pExt || !pExt->Convert_UniversalDeploy_MakeInvisible)
		return 0;

	// Here enters SHP units when deploy
	return Skip;
}

// Make object graphics invisible because shouldn't be rendered in the middle of a deployment
DEFINE_HOOK(0x43D29D, BuildingClass_DrawIt_UniversalDeploy_DontRenderObject, 0xD)
{
	enum { Skip = 0x43D688 };

	GET(BuildingClass*, pThis, ESI);

	if (!pThis)
		return 0;

	auto pTechno = static_cast<TechnoClass*>(pThis);
	if (!pTechno)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pTechno);
	if (!pExt || !pExt->Convert_UniversalDeploy_MakeInvisible)
		return 0;

	// Here enters SHP objects, voxel turrets
	return Skip;
}
*/
