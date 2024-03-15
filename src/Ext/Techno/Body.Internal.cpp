#include "Body.h"

#include <Utilities/EnumFunctions.h>
#include <Ext/Script/Body.h>

// Unsorted methods

void TechnoExt::ExtData::InitializeLaserTrails()
{
	if (this->LaserTrails.size())
		return;

	if (auto pTypeExt = this->TypeExtData)
	{
		for (auto const& entry : pTypeExt->LaserTrailData)
		{
			if (auto const pLaserType = LaserTrailTypeClass::Array[entry.idxType].get())
			{
				this->LaserTrails.push_back(
					LaserTrailClass { pLaserType, this->OwnerObject()->Owner, entry.FLH, entry.IsOnTurret });
			}
		}
	}
}

void TechnoExt::ObjectKilledBy(TechnoClass* pVictim, TechnoClass* pKiller)
{
	if (!pVictim || !pKiller)
		return;

	TechnoClass* pRealKiller = ((pKiller->GetTechnoType()->Spawned || pKiller->GetTechnoType()->MissileSpawn) && pKiller->SpawnOwner) ?
		pKiller->SpawnOwner : pKiller;

	if (!pRealKiller->BelongsToATeam())
		return;

	auto pKillerExt = TechnoExt::ExtMap.Find(pRealKiller);
	if (!pKillerExt)
		return;

	auto const pFootKiller = static_cast<FootClass*>(pRealKiller);
	TechnoClass* pFocus = nullptr;

	if (pFootKiller->Team->Focus
		&& (pFootKiller->Team->Focus->WhatAmI() == AbstractType::Unit
			|| pFootKiller->Team->Focus->WhatAmI() == AbstractType::Aircraft
			|| pFootKiller->Team->Focus->WhatAmI() == AbstractType::Infantry
			|| pFootKiller->Team->Focus->WhatAmI() == AbstractType::Building)
		)
	{
		pFocus = static_cast<TechnoClass*>(pFootKiller->Team->Focus);
	}

	/*Debug::Log("ObjectKilledBy() -> [%s] [%s](UID: %d) registered a kill of the type [%s](UID: %d)\n",
		pFootKiller->Team->Type->ID, pRealKiller->GetTechnoType()->ID, pRealKiller->UniqueID, pVictim->GetTechnoType()->ID, pVictim->UniqueID);*/

	pKillerExt->LastKillWasTeamTarget = false;

	if (pFocus && (pFocus->GetTechnoType() == pVictim->GetTechnoType()))
		pKillerExt->LastKillWasTeamTarget = true;
	
	// Conditional Jump Script Action stuff
	auto pKillerTeamExt = TeamExt::ExtMap.Find(pFootKiller->Team);
	if (!pKillerTeamExt)
		return;

	if (pKillerTeamExt->ConditionalJump_EnabledKillsCount)
	{
		bool isValidKill = pKillerTeamExt->ConditionalJump_Index < 0 ? false : ScriptExt::EvaluateObjectWithMask(pVictim, pKillerTeamExt->ConditionalJump_Index, -1, -1, pKiller);

		if (isValidKill || pKillerExt->LastKillWasTeamTarget)
			pKillerTeamExt->ConditionalJump_Counter++;
	}
	
	// Special case for interrupting current action
	if (pKillerTeamExt->AbortActionAfterKilling
		&& pKillerExt->LastKillWasTeamTarget)
	{
		pKillerTeamExt->AbortActionAfterKilling = false;
		auto pTeam = pFootKiller->Team;

		Debug::Log("[%s] [%s] %d = %d,%d - Force next script action (AbortActionAfterKilling=true): %d = %d,%d\n"
			, pTeam->Type->ID
			, pTeam->CurrentScript->Type->ID
			, pTeam->CurrentScript->CurrentMission
			, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action
			, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument
			, pTeam->CurrentScript->CurrentMission + 1
			, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 1].Action
			, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 1].Argument);

		// Jumping to the next line of the script list
		pTeam->StepCompleted = true;
	}
}

// reversed from 6F3D60
CoordStruct TechnoExt::GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct pCoord, bool isOnTurret)
{
	auto const pType = pThis->GetTechnoType();
	auto const pFoot = abstract_cast<FootClass*>(pThis);
	Matrix3D mtx;

	// Step 1: get body transform matrix
	if (pFoot && pFoot->Locomotor)
		mtx = pFoot->Locomotor->Draw_Matrix(nullptr);
	else // no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter
		mtx.MakeIdentity();

	// Steps 2-3: turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		TechnoTypeExt::ApplyTurretOffset(pType, &mtx);

		double turretRad = pThis->TurretFacing().GetRadian<32>();
		double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
		float angle = (float)(turretRad - bodyRad);

		mtx.RotateZ(angle);
	}

	// Step 4: apply FLH offset
	mtx.Translate((float)pCoord.X, (float)pCoord.Y, (float)pCoord.Z);

	auto result = mtx * Vector3D<float>::Empty;

	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// Step 5: apply as an offset to global object coords
	CoordStruct location = pThis->GetCoords();
	location += { (int)result.X, (int)result.Y, (int)result.Z };

	return location;
}

CoordStruct TechnoExt::GetBurstFLH(TechnoClass* pThis, int weaponIndex, bool& FLHFound)
{
	FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	auto pInf = abstract_cast<InfantryClass*>(pThis);
	auto pickedFLHs = pExt->WeaponBurstFLHs;

	if (pThis->Veterancy.IsElite())
	{
		if (pInf && pInf->IsDeployed())
			pickedFLHs = pExt->EliteDeployedWeaponBurstFLHs;
		else if (pInf && pInf->Crawling)
			pickedFLHs = pExt->EliteCrouchedWeaponBurstFLHs;
		else
			pickedFLHs = pExt->EliteWeaponBurstFLHs;
	}
	else
	{
		if (pInf && pInf->IsDeployed())
			pickedFLHs = pExt->DeployedWeaponBurstFLHs;
		else if (pInf && pInf->Crawling)
			pickedFLHs = pExt->CrouchedWeaponBurstFLHs;
	}
	if ((int)pickedFLHs[weaponIndex].size() > pThis->CurrentBurstIndex)
	{
		FLHFound = true;
		FLH = pickedFLHs[weaponIndex][pThis->CurrentBurstIndex];
	}

	return FLH;
}

CoordStruct TechnoExt::GetSimpleFLH(InfantryClass* pThis, int weaponIndex, bool& FLHFound)
{
	FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type))
	{
		Nullable<CoordStruct> pickedFLH;

		if (pThis->IsDeployed())
		{
			if (weaponIndex == 0)
				pickedFLH = pTypeExt->DeployedPrimaryFireFLH;
			else if (weaponIndex == 1)
				pickedFLH = pTypeExt->DeployedSecondaryFireFLH;
		}
		else
		{
			if (pThis->Crawling)
			{
				if (weaponIndex == 0)
					pickedFLH = pTypeExt->PronePrimaryFireFLH;
				else if (weaponIndex == 1)
					pickedFLH = pTypeExt->ProneSecondaryFireFLH;
			}
		}

		if (pickedFLH.isset())
		{
			FLH = pickedFLH.Get();
			FLHFound = true;
		}
	}

	return FLH;
}

// This is still not even correct, but let's see how far this can help us
void TechnoExt::ChangeOwnerMissionFix(FootClass* pThis)
{
	pThis->ShouldScanForTarget = false;
	pThis->ShouldEnterAbsorber = false;
	pThis->ShouldEnterOccupiable = false;
	pThis->ShouldGarrisonStructure = false;

	if (pThis->HasAnyLink() || pThis->GetTechnoType()->ResourceGatherer) // Don't want miners to stop
		return;

	switch (pThis->GetCurrentMission())
	{
	case Mission::Harvest:
	case Mission::Sleep:
	case Mission::Harmless:
	case Mission::Repair:
		return;
	}

	pThis->Override_Mission(Mission::Guard, nullptr, nullptr); // I don't even know what this is
	pThis->ShouldLoseTargetNow = TRUE;
	pThis->QueueMission(pThis->GetTechnoType()->DefaultToGuardArea ? Mission::Area_Guard : Mission::Guard, true);
}

// Updates layers of all animations attached to the given techno.
void TechnoExt::UpdateAttachedAnimLayers(TechnoClass* pThis)
{
	// Skip if has no attached animations.
	if (!pThis || !pThis->HasParachute)
		return;

	// Could possibly be faster to track the attached anims in TechnoExt but the profiler doesn't show this as a performance hog so whatever.
	for (auto pAnim : *AnimClass::Array)
	{
		if (pAnim->OwnerObject != pThis)
			continue;

		DisplayClass::Instance->Submit(pAnim);
	}
}
