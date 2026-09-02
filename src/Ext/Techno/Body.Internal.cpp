#include <Ext/Foot/Body.h>
#include <Ext/InfantryType/Body.h>

// Unsorted methods

void TechnoExt::InitializeLaserTrails()
{
	if (this->LaserTrails.size())
		return;

	auto const pTypeExt = this->TypeExtData;
	this->LaserTrails.reserve(pTypeExt->LaserTrailData.size());

	for (auto const& entry : pTypeExt->LaserTrailData)
		this->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(entry.GetType(), this->OwnerObject()->Owner, entry.FLH, entry.IsOnTurret));
}

void TechnoExt::ObjectKilledBy(TechnoClass* pVictim, TechnoClass* pKiller)
{
	auto const pKillerType = pKiller->GetTechnoType();
	auto const pObjectKiller = ((pKillerType->Spawned || pKillerType->MissileSpawn) && pKiller->SpawnOwner)
		? pKiller->SpawnOwner : pKiller;

	if (pObjectKiller && pObjectKiller->BelongsToATeam())
	{
		if (auto const pFootKiller = generic_cast<FootClass*, true>(pObjectKiller))
		{
			auto const pKillerTechnoData = FootExt::Fetch(pFootKiller);
			pKillerTechnoData->LastKillWasTeamTarget = pFootKiller->Team->Focus == pVictim;
		}
	}
}

Matrix3D TechnoExt::GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey, bool isShadow)
{
	Matrix3D mtx;
	auto const pFoot = abstract_cast<FootClass*, true>(pThis);

	if (pFoot && pFoot->Locomotor)
		mtx = isShadow ? pFoot->Locomotor->Shadow_Matrix(pKey) : pFoot->Locomotor->Draw_Matrix(pKey);
	else // no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter
		mtx.MakeIdentity();

	return mtx;
}

Matrix3D TechnoExt::TransformFLHForTurret(TechnoClass* pThis, Matrix3D mtx, bool isOnTurret, double factor, int turIdx)
{
	auto const pType = pThis->GetTechnoType();
	const bool isFoot = (pThis->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None;

	// turret offset and rotation
	if (isOnTurret && (pType->Turret || !isFoot)) // If building has no turret, it's TurretFacing is TargetDirection
	{
		TechnoTypeExt::ApplyTurretOffset(pType, &mtx, factor, turIdx);

		const double turretRad = pThis->TurretFacing().GetRadian<32>();
		// For BuildingClass turret facing is equal to primary facing
		const float angle = isFoot ? (float)(turretRad - pThis->PrimaryFacing.Current().GetRadian<32>()) : (float)(turretRad);

		mtx.RotateZ(angle);
	}

	return mtx;
}

Matrix3D TechnoExt::GetFLHMatrix(TechnoClass* pThis, const CoordStruct& flh, bool isOnTurret, double factor, bool isShadow, int turIdx)
{
	Matrix3D transform = TechnoExt::GetTransform(pThis, nullptr, isShadow);
	Matrix3D mtx = TechnoExt::TransformFLHForTurret(pThis, transform, isOnTurret, factor, turIdx);

	// apply FLH offset
	mtx.Translate((float)(flh.X * factor), (float)(flh.Y * factor), (float)(flh.Z * factor));

	return mtx;
}

// reversed from 6F3D60
CoordStruct TechnoExt::GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& flh, bool isOnTurret, int turIdx)
{
	auto result = TechnoExt::GetFLHMatrix(pThis, flh, isOnTurret, 1.0, false, turIdx).GetTranslation();

	// apply as an offset to global object coords
	// Resulting coords are mirrored along X axis, so we mirror it back
	auto const location = pThis->GetRenderCoords() + CoordStruct { (int)result.X, -(int)result.Y, (int)result.Z };

	return location;
}

CoordStruct TechnoExt::GetBurstFLH(TechnoClass* pThis, int weaponIndex, bool& FLHFound)
{
	FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	auto const pExt = TechnoExt::Fetch(pThis)->TypeExtData;

	auto const pInf = abstract_cast<InfantryClass*, true>(pThis);
	std::span<std::vector<CoordStruct>> pickedFLHs = pExt->WeaponBurstFLHs;

	if (pThis->Veterancy.IsElite())
	{
		if (pInf)
		{
			auto const pInfTypeExt = InfantryTypeExt::Fetch(pInf->Type);

			if (pInf->IsDeployed() && pInfTypeExt->EliteDeployedWeaponBurstFLHs.size() > 0)
				pickedFLHs = pInfTypeExt->EliteDeployedWeaponBurstFLHs;
			else if (pInf->Crawling && pInfTypeExt->EliteCrouchedWeaponBurstFLHs.size() > 0)
				pickedFLHs = pInfTypeExt->EliteCrouchedWeaponBurstFLHs;
			else
				pickedFLHs = pExt->EliteWeaponBurstFLHs;
		}
		else
		{
			pickedFLHs = pExt->EliteWeaponBurstFLHs;
		}
	}
	else if (pInf)
	{
		auto const pInfTypeExt = InfantryTypeExt::Fetch(pInf->Type);

		if (pInf->IsDeployed() && pInfTypeExt->DeployedWeaponBurstFLHs.size() > 0)
			pickedFLHs = pInfTypeExt->DeployedWeaponBurstFLHs;
		else if (pInf->Crawling && pInfTypeExt->CrouchedWeaponBurstFLHs.size() > 0)
			pickedFLHs = pInfTypeExt->CrouchedWeaponBurstFLHs;
	}
	if ((int)pickedFLHs[weaponIndex].size() > pThis->CurrentBurstIndex)
	{
		FLHFound = true;
		FLH = pickedFLHs[weaponIndex][pThis->CurrentBurstIndex];
	}

	return FLH;
}

void TechnoExt::InitializeDisplayInfo(TechnoTypeClass* pType)
{
	const auto pThis = this->OwnerObject();
	const auto pPrimary = pThis->GetWeapon(0)->WeaponType;

	if (pPrimary && pType->LandTargeting != LandTargetingType::Land_Not_OK)
		pThis->RearmTimer.TimeLeft = pPrimary->ROF;
	else if (const auto pSecondary = pThis->GetWeapon(1)->WeaponType)
		pThis->RearmTimer.TimeLeft = pSecondary->ROF;

	pThis->RearmTimer.StartTime = Math::min(-2, -pThis->RearmTimer.TimeLeft);
}

void TechnoExt::InitializeAttachEffects()
{
	auto const pTypeExt = this->TypeExtData;

	if (pTypeExt->AttachEffects.AttachTypes.size() < 1)
		return;

	auto const pThis = this->OwnerObject();
	AttachEffectClass::Attach(pThis, pThis->Owner, pThis, pThis, pTypeExt->AttachEffects, true);
}

// Gets tint colors for invulnerability, airstrike laser target and berserk, depending on parameters.
int TechnoExt::GetTintColor(TechnoClass* pThis, bool invulnerability, bool airstrike, bool berserk)
{
	int tintColor = 0;

	if (pThis)
	{
		if (invulnerability && pThis->IsIronCurtained())
		{
			tintColor |= pThis->ForceShielded ? RulesExt::Global()->TintColorForceShield : RulesExt::Global()->TintColorIronCurtain;
		}

		if (airstrike)
		{
			auto const pExt =  TechnoExt::Fetch(pThis);

			if (auto const pAirstrike = pExt->AirstrikeTargetingMe)
			{
				auto const pTypeExt = TechnoExt::Fetch(pAirstrike->Owner)->TypeExtData;
				tintColor |= pTypeExt->TintColorAirstrike;
			}
		}

		if (berserk && pThis->Berzerk)
		{
			tintColor |= RulesExt::Global()->TintColorBerserk;
		}
	}

	return tintColor;
}

// Gets custom tint color from TechnoTypes & Warheads.
int TechnoExt::GetCustomTintColor(TechnoClass* pThis)
{
	int dummy = 0;
	int color = 0;
	TechnoExt::ApplyCustomTintValues(pThis, color, dummy);
	return color;
}

// Gets custom tint intensity from TechnoTypes & Warheads.
int TechnoExt::GetCustomTintIntensity(TechnoClass* pThis)
{
	int dummy = 0;
	int intensity = 0;
	TechnoExt::ApplyCustomTintValues(pThis, dummy, intensity);
	return intensity;
}

// Applies custom tint color and intensity from TechnoTypes and any AttachEffects and shields it might have on provided values.
void TechnoExt::ApplyCustomTintValues(TechnoClass* pThis, int& color, int& intensity)
{
	auto const pExt = TechnoExt::Fetch(pThis);
	auto const pOwner = pThis->Owner;

	if (pOwner == HouseClass::CurrentPlayer)
	{
		color |= pExt->TintColorOwner;
		intensity += pExt->TintIntensityOwner;
	}
	else if (pOwner->IsAlliedWith(HouseClass::CurrentPlayer))
	{
		color |= pExt->TintColorAllies;
		intensity += pExt->TintIntensityAllies;
	}
	else
	{
		color |= pExt->TintColorEnemies;
		intensity += pExt->TintIntensityEnemies;
	}
}

// This is still not even correct, but let's see how far this can help us
void TechnoExt::ChangeOwnerMissionFix(FootClass* pThis, TechnoTypeClass* pType)
{
	pThis->ShouldScanForTarget = false;
	pThis->ShouldEnterAbsorber = false;
	pThis->ShouldEnterOccupiable = false;
	pThis->ShouldGarrisonStructure = false;

	if (pThis->HasAnyLink() || pType->ResourceGatherer) // Don't want miners to stop
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
	pThis->QueueMission(pType->DefaultToGuardArea ? Mission::Area_Guard : Mission::Guard, true);
}

// Updates layers of all animations attached to the given techno.
void TechnoExt::UpdateAttachedAnimLayers(TechnoClass* pThis)
{
	// Skip if has no attached animations.
	if (!pThis->HasParachute)
		return;

	// Could possibly be faster to track the attached anims in TechnoExt but the profiler doesn't show this as a performance hog so whatever.
	for (auto const pAnim : AnimClass::Array)
	{
		if (pAnim->OwnerObject != pThis)
			continue;

		DisplayClass::Instance.Submit(pAnim);
	}
}

int TechnoExt::GetDropCrateIndex(TechnoClass* pThis)
{
	if (!pThis)
		return -1;

	const auto pExt = TechnoExt::Fetch(pThis);
	const auto pTypeExt = pExt->TypeExtData;

	if (pExt->DropCrate >= 0 || pTypeExt->DropCrate.isset())
	{
		int nSelectedPowerup = -1;

		if (pExt->DropCrate >= 0)
		{
			if (pExt->DropCrate == 1)
				nSelectedPowerup = static_cast<int>(pExt->DropCrateType);
		}
		else if (pTypeExt->DropCrate.isset())
		{
			nSelectedPowerup = static_cast<int>(pTypeExt->DropCrate.Get());
		}

		if (nSelectedPowerup >= 0)
			return nSelectedPowerup;
	}

	return -1;
}
