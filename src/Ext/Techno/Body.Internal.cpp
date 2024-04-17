#include "Body.h"

#include <BitFont.h>

#include <Ext/WeaponType/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/EnumFunctions.h>

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
	TechnoClass* pObjectKiller = ((pKiller->GetTechnoType()->Spawned || pKiller->GetTechnoType()->MissileSpawn) && pKiller->SpawnOwner) ?
		pKiller->SpawnOwner : pKiller;

	if (pObjectKiller && pObjectKiller->BelongsToATeam())
	{
		if (auto const pFootKiller = generic_cast<FootClass*>(pObjectKiller))
		{
			auto pKillerTechnoData = TechnoExt::ExtMap.Find(pObjectKiller);
			pKillerTechnoData->LastKillWasTeamTarget = pFootKiller->Team->Focus == pVictim;
		}
	}
}

Matrix3D TechnoExt::GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey, bool isShadow)
{
	Matrix3D mtx;

	if ((pThis->AbstractFlags & AbstractFlags::Foot) && ((FootClass*)pThis)->Locomotor)
		mtx = isShadow ? ((FootClass*)pThis)->Locomotor->Shadow_Matrix(pKey) : ((FootClass*)pThis)->Locomotor->Draw_Matrix(pKey);
	else // no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter
		mtx.MakeIdentity();

	return mtx;
}

Matrix3D TechnoExt::TransformFLHForTurret(TechnoClass* pThis, Matrix3D mtx, bool isOnTurret, double factor)
{
	auto const pType = pThis->GetTechnoType();

	// turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		TechnoTypeExt::ApplyTurretOffset(pType, &mtx, factor);

		double turretRad = pThis->TurretFacing().GetRadian<32>();
		double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
		float angle = (float)(turretRad - bodyRad);

		mtx.RotateZ(angle);
	}

	return mtx;
}

Matrix3D TechnoExt::GetFLHMatrix(TechnoClass* pThis, CoordStruct pCoord, bool isOnTurret, double factor, bool isShadow)
{
	Matrix3D transform = TechnoExt::GetTransform(pThis, nullptr, isShadow);
	Matrix3D mtx = TechnoExt::TransformFLHForTurret(pThis, transform, isOnTurret, factor);

	CoordStruct scaledCoord = pCoord * factor;
	// apply FLH offset
	mtx.Translate((float)scaledCoord.X, (float)scaledCoord.Y, (float)scaledCoord.Z);

	return mtx;
}

// reversed from 6F3D60
CoordStruct TechnoExt::GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct pCoord, bool isOnTurret)
{
	auto result = TechnoExt::GetFLHMatrix(pThis, pCoord, isOnTurret) * Vector3D<float>::Empty;

	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// apply as an offset to global object coords
	CoordStruct location = pThis->GetCoords();
	location += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

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
