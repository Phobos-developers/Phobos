#include "Body.h"
#include "Trajectories\PhobosVirtualTrajectory.h"

#include <Ext/Anim/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/EBolt/Body.h>
#include <Ext/Techno/Body.h>
#include <New/Entity/LaserTrailClass.h>

namespace LaserRT
{
	void SetLaserTrackingData(LaserDrawClass* pLaser, TechnoClass* pShooter, AbstractClass* pTarget, int weaponIdx, PositionFollow mode, bool ignoreShooter);
}

BulletExt::ExtContainer BulletExt::ExtMap;

BulletExt::~BulletExt()
{
	if (this->GroupIndex != -1)
	{
		if (const auto pMap = this->TrajectoryGroup)
		{
			auto& groupData = (*pMap)[this->TypeExtData->OwnerObject()];
			auto& vec = groupData.Bullets;
			vec.erase(std::remove(vec.begin(), vec.end(), this->OwnerObject()->UniqueID), vec.end());
			groupData.ShouldUpdate = true;
		}
	}

	if (const auto pTraj = this->Trajectory.get())
	{
		const auto flag = pTraj->Flag();

		if (flag == TrajectoryFlag::Engrave || flag == TrajectoryFlag::Tracing)
		{
			if (auto& pLaser = static_cast<VirtualTrajectory*>(pTraj)->Laser)
			{
				pLaser->Duration = 0;
				pLaser = nullptr;
			}
		}
	}
}

void BulletExt::InitializeOnUnlimbo()
{
	const auto pBullet = this->OwnerObject();
	const auto pBulletExt = BulletExt::Fetch(pBullet);
	const auto pBulletTypeExt = pBulletExt->TypeExtData;

	// Without a target, the game will inevitably crash before, so no need to check here
	const auto pTarget = pBullet->Target;

	// Due to various ways of firing weapons, the true firer may have already died
	const auto pFirer = pBullet->Owner;

	// Set additional warhead and weapon count
	pBulletExt->ProximityImpact = pBulletTypeExt->ProximityImpact;
	pBulletExt->DisperseCycle = pBulletTypeExt->DisperseCycle;

	// Record the status of the target
	pBulletExt->TargetIsTechno = (pTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None;
	pBulletExt->TargetIsInAir = (pTarget->AbstractFlags & AbstractFlags::Object) ? (static_cast<ObjectClass*>(pTarget)->GetHeight() > Unsorted::CellHeight) : false;
	int damage = pBullet->Health;

	// Record some information of weapon
	if (const auto pWeapon = pBullet->WeaponType)
	{
		pBulletExt->AttenuationRange = pWeapon->Range;

		if (pBulletTypeExt->ApplyRangeModifiers && pFirer)
			pBulletExt->AttenuationRange = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer);

		damage = pWeapon->Damage;
	}

	// Set basic damage
	pBulletExt->ProximityDamage = pBulletTypeExt->ProximityDamage.Get(damage);
	pBulletExt->PassDetonateDamage = pBulletTypeExt->PassDetonateDamage.Get(damage);

	// Record some information of firer
	if (pFirer)
	{
		// Obtain the launch location
		pBulletExt->GetTechnoFLHCoord();

		// Check trajectory capacity
		if (pBulletTypeExt->CreateCapacity >= 0)
			BulletExt::CheckExceededCapacity(pFirer, pBullet->Type, pBulletExt);
	}
	else
	{
		pBulletExt->NotMainWeapon = true;

		if (pBulletTypeExt->CreateCapacity >= 0)
			pBulletExt->Status |= TrajectoryStatus::Vanish;
	}

	// Initialize additional warheads
	if (pBulletTypeExt->PassDetonate)
		pBulletExt->PassDetonateTimer.Start(pBulletTypeExt->PassDetonateInitialDelay);

	// Initialize additional weapons
	if (!pBulletTypeExt->DisperseWeapons.empty() && !pBulletTypeExt->DisperseCounts.empty() && pBulletExt->DisperseCycle)
	{
		pBulletExt->DisperseCount = pBulletTypeExt->DisperseCounts[0];
		pBulletExt->DisperseTimer.Start(pBulletTypeExt->DisperseInitialDelay);
	}
}

bool BulletExt::CheckOnEarlyUpdate()
{
	// Update group index for members by themselves
	if (this->TrajectoryGroup)
		this->UpdateGroupIndex();

	// In the phase of playing PreImpactAnim
	if (this->OwnerObject()->SpawnNextAnim)
		return false;

	// The previous check requires detonation at this time
	if (this->Status & (TrajectoryStatus::Detonate | TrajectoryStatus::Vanish))
		return true;

	// Check the remaining existence time
	if (this->LifeDurationTimer.Completed())
		return true;

	// Check if the firer's target can be synchronized, the target may have been changed here
	if (this->CheckSynchronize())
		return true;

	// Check if the target needs to be changed, the target may have been changed here
	if (this->TypeExtData->RetargetRadius && this->BulletRetargetTechno())
		return true;

	// After the new target is confirmed, check if the tolerance time has ended
	if (this->CheckNoTargetLifeTime())
		return true;

	// Fire weapons or warheads
	if (this->FireAdditionals())
		return true;

	// Detonate extra warhead on the obstacle after the pass through check is completed
	this->DetonateOnObstacle();
	return false;
}

void BulletExt::CheckOnPreDetonate()
{
	const auto pBullet = this->OwnerObject();
	const auto pBulletTypeExt = this->TypeExtData;

	// Special circumstances, similar to airburst behavior
	if (pBulletTypeExt->DisperseEffectiveRange.Get() < 0)
		this->PrepareDisperseWeapon();

	if (!(this->Status & TrajectoryStatus::Vanish))
	{
		if (!pBulletTypeExt->PeacefulVanish.Get(pBulletTypeExt->ProximityImpact || pBulletTypeExt->DisperseCycle))
		{
			// Calculate the current damage
			pBullet->Health = this->GetTrueDamage(pBullet->Health, true);
			return;
		}

		this->Status |= TrajectoryStatus::Vanish;
	}

	// To skip all extra effects, no damage, no anims...
	pBullet->Health = 0;
	pBullet->Limbo();
	pBullet->UnInit();
}

// Launch additional weapons and warheads
bool BulletExt::FireAdditionals()
{
	const auto pType = this->TypeExtData;

	// Detonate the warhead at the current location
	if (pType->PassDetonate)
		this->PassWithDetonateAt();

	// Detonate the warhead on the technos passing through
	if (this->ProximityImpact != 0 && pType->ProximityRadius.Get() > 0)
		this->PrepareForDetonateAt();

	// Launch additional weapons towards the target
	if (!this->DisperseTimer.Completed())
		return false;

	const auto pBullet = this->OwnerObject();
	const double range = (double)pType->DisperseEffectiveRange.Get();

	// Weapons can only be fired when the distance is close enough
	if (range < 0.0 || (range > 0.0 && pBullet->TargetCoords.DistanceFromSquared(pBullet->Location) > range * range))
		return false;

	// Fire after checking the orientation
	const auto pTraj = this->Trajectory.get();
	return (!pTraj || pTraj->OnFacingCheck()) && this->PrepareDisperseWeapon();
}

// Detonate a extra warhead on the obstacle then detonate bullet itself
void BulletExt::DetonateOnObstacle()
{
	const auto pDetonateAt = this->ExtraCheck;

	// Obstacles were detected in the current frame here
	if (!pDetonateAt)
		return;

	// Slow down and reset the target
	this->ExtraCheck = nullptr;
	const auto pBullet = this->OwnerObject();

	// Set the new target so that the snap function can take effect
	pBullet->SetTarget(pDetonateAt);

	if (const auto pTraj = this->Trajectory.get())
	{
		const double speed = pTraj->MovingSpeed;
		const double distanceSq = pDetonateAt->GetCoords().DistanceFromSquared(pBullet->Location);

		// Check whether need to slow down
		if (speed && distanceSq < speed * speed)
			pTraj->MultiplyBulletVelocity(sqrt(distanceSq) / speed, true);
		else
			this->Status |= TrajectoryStatus::Detonate;
	}

	// Need to cause additional damage?
	if (!this->ProximityImpact)
		return;

	// Detonate extra warhead
	const auto pFirer = pBullet->Owner;
	const auto pOwner = pFirer ? pFirer->Owner : BulletExt::Fetch(pBullet)->FirerHouse;
	this->ProximityDetonateAt(pOwner, pDetonateAt);
}

// Synchronization target inspection
bool BulletExt::CheckSynchronize()
{
	const auto pBullet = this->OwnerObject();
	const auto pType = this->TypeExtData;

	// Find the outermost transporter
	const auto pFirer = BulletExt::GetSurfaceFirer(pBullet->Owner);

	// Synchronize to the target of the firer
	if (pType->Synchronize && pFirer)
	{
		auto pTarget = pFirer->Target;

		// Check should detonate when changing target
		if (pBullet->Target != pTarget && !pType->NoTargetLifeTime)
			return true;

		// Check if the target can be synchronized
		if (pTarget && (pTarget->IsInAir() != this->TargetIsInAir))
			pTarget = nullptr;

		// Replace with a new target
		pBullet->SetTarget(pTarget);
	}

	return false;
}

// Tolerance timer inspection
bool BulletExt::CheckNoTargetLifeTime()
{
	const auto pBullet = this->OwnerObject();
	const auto pType = this->TypeExtData;

	// Check should detonate when no target
	if (!pBullet->Target && !pType->NoTargetLifeTime)
		return true;

	// Update timer
	if (pBullet->Target)
	{
		this->NoTargetLifeTimer.Stop();
	}
	else if (pType->NoTargetLifeTime > 0)
	{
		if (this->NoTargetLifeTimer.Completed())
			return true;
		else if (!this->NoTargetLifeTimer.IsTicking())
			this->NoTargetLifeTimer.Start(pType->NoTargetLifeTime);
	}

	return false;
}

// Update trajectory capacity group index
void BulletExt::UpdateGroupIndex()
{
	const auto pBullet = this->OwnerObject();
	auto& groupData = (*this->TrajectoryGroup)[pBullet->Type];

	// Should update group index
	if (groupData.ShouldUpdate)
	{
		if (const int size = static_cast<int>(groupData.Bullets.size()))
		{
			for (int i = 0; i < size; ++i)
			{
				if (groupData.Bullets[i] == pBullet->UniqueID)
				{
					this->GroupIndex = i;
					break;
				}
			}

			// If is the last member, reset flag to false
			if (this->GroupIndex == size - 1)
				groupData.ShouldUpdate = false;
		}
		else
		{
			groupData.ShouldUpdate = false;
		}
	}

	return;
}

// Check and set the group
bool BulletExt::CheckExceededCapacity(TechnoClass* pTechno, BulletTypeClass* pBulletType, BulletExt* pBulletExt)
{
	const auto pTechnoExt = TechnoExt::Fetch(pTechno);

	if (!pTechnoExt->TrajectoryGroup)
		pTechnoExt->TrajectoryGroup = std::make_shared<PhobosMap<BulletTypeClass*, BulletGroupData>>();

	// Get shared container
	auto& group = (*pTechnoExt->TrajectoryGroup)[pBulletType].Bullets;
	const auto size = static_cast<int>(group.size());

	if (!pBulletExt)
		return size >= BulletTypeExt::Fetch(pBulletType)->CreateCapacity;

	pBulletExt->TrajectoryGroup = pTechnoExt->TrajectoryGroup;

	// Check trajectory capacity
	if (size >= pBulletExt->TypeExtData->CreateCapacity)
	{
		// Peaceful vanish
		pBulletExt->Status |= TrajectoryStatus::Vanish;
		return true;
	}
	else
	{
		// Increase trajectory count
		pBulletExt->GroupIndex = size;
		group.push_back(pBulletExt->OwnerObject()->UniqueID);
		return false;
	}
}

void BulletExt::InterceptBullet(TechnoClass* pSource, BulletClass* pInterceptor)
{
	const auto pThis = this->OwnerObject();
	auto pTypeExt = this->TypeExtData;
	const auto pInterceptorType = BulletExt::Fetch(pInterceptor)->InterceptorTechnoType->InterceptorType.get();

	if (!pTypeExt->Armor.isset())
	{
		if (!pInterceptorType->KeepIntact)
			this->InterceptedStatus |= InterceptedStatus::Intercepted;
	}
	else
	{
		const double versus = GeneralUtils::GetWarheadVersusArmor(pInterceptor->WH, pTypeExt->Armor.Get());

		if (versus == 0.0)
			return;

		const int damage = static_cast<int>(pInterceptor->Health * versus);
		this->CurrentStrength -= damage;

		if (Phobos::DisplayDamageNumbers && damage != 0)
			GeneralUtils::DisplayDamageNumberString(damage, DamageDisplayType::Intercept, pThis->GetRenderCoords(), this->DamageNumberOffset);

		if (this->CurrentStrength <= 0)
		{
			this->CurrentStrength = 0;

			if (!pInterceptorType->KeepIntact)
				this->InterceptedStatus |= InterceptedStatus::Intercepted;
		}
	}

	this->DetonateOnInterception = !pInterceptorType->DeleteOnIntercept.Get(pTypeExt->Interceptable_DeleteOnIntercept);

	if (const auto pWeaponOverride = pInterceptorType->WeaponOverride.Get(pTypeExt->Interceptable_WeaponOverride))
	{
		pThis->WeaponType = pWeaponOverride;
		pThis->Health = pInterceptorType->WeaponCumulativeDamage.Get() ? pThis->Health + pWeaponOverride->Damage : pWeaponOverride->Damage;
		pThis->WH = pWeaponOverride->Warhead;
		pThis->Bright = pWeaponOverride->Bright;

		if (pInterceptorType->WeaponReplaceProjectile
			&& pWeaponOverride->Projectile
			&& pWeaponOverride->Projectile != pThis->Type)
		{
			pThis->Speed = pWeaponOverride->Speed;
			pThis->Type = pWeaponOverride->Projectile;
			pTypeExt = BulletTypeExt::Fetch(pThis->Type);
			this->TypeExtData = pTypeExt;

			if (this->LaserTrails.size())
				this->LaserTrails.clear();

			if (!pThis->Type->Inviso)
				this->InitializeLaserTrails();

			// Lose target if the current bullet is no longer interceptable.
			if (pSource && (!pTypeExt->Interceptable || (pTypeExt->Armor.isset() && GeneralUtils::GetWarheadVersusArmor(pInterceptor->WH, pTypeExt->Armor.Get()) == 0.0)))
				pSource->SetTarget(nullptr);
		}
	}
}

void BulletExt::ApplyRadiationToCell(CellStruct cell, int spread, int radLevel)
{
	const auto pCell = MapClass::Instance.TryGetCellAt(cell);

	if (!pCell)
		return;

	const auto pThis = this->OwnerObject();
	const auto pWeapon = pThis->GetWeaponType();
	const auto pWeaponExt = WeaponTypeExt::Fetch(pWeapon);
	const auto pRadType = pWeaponExt->RadType;
	const auto pCellExt = CellExt::Fetch(pCell);

	const auto it = std::find_if(pCellExt->RadSites.cbegin(), pCellExt->RadSites.cend(),
		[=](const auto pSite)
		{
			const auto pRadExt = RadSiteExt::Fetch(pSite);

			if (pRadExt->Type != pRadType || spread != pSite->Spread)
				return false;

			if (pRadExt->RadInvoker && pThis->Owner)
				return pRadExt->RadInvoker == pThis->Owner;

			return true;
		}
	);

	if (it != pCellExt->RadSites.cend())
	{
		const auto pRadExt = RadSiteExt::Fetch(*it);
		// Handle It
		pRadExt->Add(std::min(radLevel, pRadType->GetLevelMax() - (*it)->GetRadLevel()));
		return;
	}

	const auto pThisHouse = pThis->Owner ? pThis->Owner->Owner : this->FirerHouse;
	RadSiteExt::CreateInstance(cell, spread, radLevel, pWeaponExt, pThisHouse, pThis->Owner);
}

void BulletExt::InitializeLaserTrails()
{
	if (this->LaserTrails.size())
		return;

	auto const pThis = this->OwnerObject();
	auto const pTypeExt = BulletTypeExt::Fetch(pThis->Type);
	auto const pOwner = pThis->Owner ? pThis->Owner->Owner : nullptr;
	this->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

	for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
		this->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(LaserTrailTypeClass::Array[idxTrail].get(), pOwner));
}

static inline int SetBuildingFireAnimZAdjust(BuildingClass* pBuilding, int animY)
{
	if (pBuilding->GetOccupantCount() > 0)
		return -200;

	const auto renderCoords = pBuilding->GetRenderCoords();
	const auto zAdj = (animY - renderCoords.Y) / -4;
	return (zAdj >= 0) ? 0 : zAdj;
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringAnim(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach)
{
	const auto pWeapon = pBullet->WeaponType;
	const auto animCounts = pWeapon->Anim.Count;

	if (animCounts <= 0)
		return;

	const auto pTraj = BulletExt::Fetch(pBullet)->Trajectory.get();
	const auto velocityRadian = pTraj ? Math::atan2(pTraj->MovingVelocity.Y, pTraj->MovingVelocity.X) : Math::atan2(pBullet->Velocity.Y, pBullet->Velocity.X);
	const auto pFirer = pBullet->Owner;
	const auto pAnimType = pWeapon->Anim[(animCounts % 8 == 0) // Have direction
		? (static_cast<int>((velocityRadian / Math::TwoPi + 1.5) * animCounts - (animCounts / 8) + 0.5) % animCounts) // Calculate direction
		: ScenarioClass::Instance->Random.RandomRanged(0, animCounts - 1)]; // Simple random;
	/*
		const auto ratioOfRotateAngle = velocityRadian / Math::TwoPi;
		const auto correctRatioOfRotateAngle = ratioOfRotateAngle + 1.5; // Correct the Y-axis in reverse and ensure that the ratio is a positive number
		const auto animIndex = correctRatioOfRotateAngle * animCounts;
		const auto correctAnimIndex = animIndex - (animCounts / 8); // A multiple of 8 greater than 8 will have an additional offset
		const auto trueAnimIndex = static_cast<int>(correctAnimIndex + 0.5) % animCounts; // Round down and prevent exceeding the scope
	*/

	if (!pAnimType)
		return;

	const auto pAnim = GameCreate<AnimClass>(pAnimType, pBullet->SourceCoords);

	AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, nullptr, false, true);
	AnimExt::Fetch(pAnim)->SetInvoker(pFirer, pHouse);

	if (pAttach)
	{
		if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pAttach))
			pAnim->ZAdjust = SetBuildingFireAnimZAdjust(pBuilding, pBullet->SourceCoords.Y);
		else
			pAnim->SetOwnerObject(pAttach);
	}
	else if (const auto pBuilding = abstract_cast<BuildingClass*>(pFirer))
	{
		pAnim->ZAdjust = SetBuildingFireAnimZAdjust(pBuilding, pBullet->SourceCoords.Y);
	}
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringReport(BulletClass* pBullet)
{
	const auto pWeapon = pBullet->WeaponType;

	if (pWeapon->Report.Count <= 0)
		return;

	const auto pFirer = pBullet->Owner;
	const auto reportIndex = pWeapon->Report[(pFirer ? pFirer->unknown_short_3C8 : ScenarioClass::Instance->Random.Random()) % pWeapon->Report.Count];
	VocClass::PlayAt(reportIndex, pBullet->Location, nullptr);
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringLaser(BulletClass* pBullet, HouseClass* pHouse)
{
	// Can not use 0x6FD210 because the firer may die
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon->IsLaser)
		return;

	if (const auto pTrajType = BulletTypeExt::Fetch(pWeapon->Projectile)->TrajectoryType.get())
	{
		const auto flag = pTrajType->Flag();

		if (flag == TrajectoryFlag::Engrave || flag == TrajectoryFlag::Tracing)
			return;
	}

	const auto pWeaponExt = WeaponTypeExt::Fetch(pWeapon);

	LaserDrawClass* pLaser = nullptr;

	if (pWeapon->IsHouseColor || pWeaponExt->Laser_IsSingleColor)
	{
		const auto black = ColorStruct { 0, 0, 0 };
		pLaser = GameCreate<LaserDrawClass>(pBullet->SourceCoords, BulletExt::GetTargetCoordsForFiring(pBullet),
			((pWeapon->IsHouseColor && pHouse) ? pHouse->LaserColor : pWeapon->LaserInnerColor), black, black, pWeapon->LaserDuration);

		pLaser->IsHouseColor = true;
		pLaser->Thickness = pWeaponExt->LaserThickness;
		pLaser->IsSupported = (pLaser->Thickness > 3);
	}
	else
	{
		pLaser = GameCreate<LaserDrawClass>(pBullet->SourceCoords, BulletExt::GetTargetCoordsForFiring(pBullet),
			pWeapon->LaserInnerColor, pWeapon->LaserOuterColor, pWeapon->LaserOuterSpread, pWeapon->LaserDuration);

		pLaser->IsHouseColor = false;
		pLaser->Thickness = 3;
		pLaser->IsSupported = false;
	}

	// LaserPositionUpdate
	if (pLaser)
	{
		auto mode = pWeaponExt->LaserPositionUpdate;
		const bool isSplit = BulletExt::Fetch(pBullet)->IsSplitFromAirburst;

		if (isSplit)
		{
			if (mode == PositionFollow::Firer)
				mode = PositionFollow::None;
			else if (mode == PositionFollow::All)
				mode = PositionFollow::Target;
		}

		if (mode != PositionFollow::None)
		{
			auto const pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
			LaserRT::SetLaserTrackingData(pLaser, pBullet->Owner, pTarget, 0, mode, isSplit);
		}
	}
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringElectricBolt(BulletClass* pBullet)
{
	// Can not use 0x6FD460 because the firer may die
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon->IsElectricBolt)
		return;

	const auto pBolt = EBoltExt::CreateEBolt(pWeapon);
	pBolt->AlternateColor = pWeapon->IsAlternateColor;

	const auto targetCoords = BulletExt::GetTargetCoordsForFiring(pBullet);
	const auto pWeaponExt = WeaponTypeExt::Fetch(pWeapon);
	int zAdjust = pWeaponExt->EBoltZAdjust.Get(RulesExt::Global()->EBoltZAdjust);

	const auto pOwner = pBullet->Owner;
	if (pOwner && pOwner->WhatAmI() == AbstractType::Building)
	{
		const bool clamp = pWeaponExt->EBoltZAdjust_ClampInitialDepthForBuilding.Get(RulesExt::Global()->EBoltZAdjust_ClampInitialDepthForBuilding);
		if (clamp && zAdjust > 0)
			zAdjust = 0;
	}

	pBolt->Fire(pBullet->SourceCoords, targetCoords, zAdjust);

	if (const auto particle = WeaponTypeExt::Fetch(pWeapon)->Bolt_ParticleSystem.Get(RulesClass::Instance->DefaultSparkSystem))
		GameCreate<ParticleSystemClass>(particle, targetCoords, nullptr, nullptr, CoordStruct::Empty, nullptr);
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringRadBeam(BulletClass* pBullet, HouseClass* pHouse)
{
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon->IsRadBeam)
		return;

	const auto pWH = pWeapon->Warhead;
	const bool isTemporal = pWH && pWH->Temporal;
	const auto pRadBeam = RadBeam::Allocate(isTemporal ? RadBeamType::Temporal : RadBeamType::RadBeam);

	pRadBeam->SetCoordsSource(pBullet->SourceCoords);
	pRadBeam->SetCoordsTarget(BulletExt::GetTargetCoordsForFiring(pBullet));

	const auto pWeaponExt = WeaponTypeExt::Fetch(pWeapon);

	pRadBeam->Color = (pWeaponExt->Beam_IsHouseColor && pHouse) ? pHouse->LaserColor
		: pWeaponExt->Beam_Color.Get(isTemporal ? RulesClass::Instance->ChronoBeamColor : RulesClass::Instance->RadColor);

	pRadBeam->Period = pWeaponExt->Beam_Duration;
	pRadBeam->Amplitude = pWeaponExt->Beam_Amplitude;
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringParticleSystem(BulletClass* pBullet, HouseClass* pHouse)
{
	if (const auto pPSType = pBullet->WeaponType->AttachedParticleSystem)
	{
		GameCreate<ParticleSystemClass>(pPSType, pBullet->SourceCoords, pBullet->Target, pBullet->Owner, BulletExt::GetTargetCoordsForFiring(pBullet), pHouse);
	}
}

// Make sure pBullet is not empty before call
void BulletExt::SimulatedFiringUnlimbo(BulletClass* pBullet, HouseClass* pHouse, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords, bool headToTarget, const RadialFireStruct& radialFire)
{
	// Initialize bullet characteristics such as weapon type, range, house etc.
	const auto pType = pBullet->Type;
	const int projectileRange = WeaponTypeExt::Fetch(pWeapon)->ProjectileRange.Get();
	auto velocity = BulletVelocity::Empty;
	pBullet->WeaponType = pWeapon;
	pBullet->Range = projectileRange;
	BulletExt::Fetch(pBullet)->FirerHouse = pHouse;

	if (pType->FirersPalette)
		pBullet->InheritedColor = pHouse->ColorSchemeIndex;

	// If someone asks me, I would say Arcing is just a piece of shit
	// But there are still people who like to use it, so anyway, it has been fixed
	if (pType->Arcing)
	{
		// The target must exist during launch
		const auto targetCoords = pBullet->Target->GetCenterCoords();
		const auto gravity = BulletTypeExt::GetAdjustedGravity(pType);
		const auto distanceCoords = targetCoords - sourceCoords;
		const auto horizontalDistance = Point2D { distanceCoords.X, distanceCoords.Y }.Magnitude();
		const bool lobber = pWeapon->Lobber || static_cast<int>(horizontalDistance) < distanceCoords.Z; // 0x70D590
		// The lower the horizontal velocity, the higher the trajectory
		// WW calculates the launch angle (and limits it) before calculating the velocity
		// Here, some magic numbers are used to directly simulate its calculation
		const auto speedMult = (lobber ? 0.45 : (distanceCoords.Z > 0 ? 0.68 : 1.0)); // Simulated 0x48A9D0
		const auto speed = speedMult * sqrt(horizontalDistance * gravity * 1.2); // 0x48AB90

		// Simulate firing Arcing bullet
		if (horizontalDistance < 1e-10 || speed < 1e-10)
		{
			// No solution
			velocity.Z = speed;
		}
		else
		{
			const auto mult = speed / horizontalDistance;
			velocity.X = static_cast<double>(distanceCoords.X) * mult;
			velocity.Y = static_cast<double>(distanceCoords.Y) * mult;
			velocity.Z = static_cast<double>(distanceCoords.Z) * mult + (gravity * horizontalDistance) / (2 * speed);
		}
	}
	else
	{
		const double speed = pBullet->Speed;

		if (headToTarget) // Home in on target.
		{
			const auto targetCoords = pBullet->Target->GetCenterCoords();
			const auto distanceCoords = targetCoords - sourceCoords;

			Vector3D<double> distanceVector {
				static_cast<double>(distanceCoords.X),
				static_cast<double>(distanceCoords.Y),
				static_cast<double>(distanceCoords.Z) };

			double len = distanceVector.Magnitude();

			if (len > 0.0)
			{
				distanceVector /= len;
				velocity = { distanceVector.X * speed, distanceVector.Y * speed, distanceVector.Z * speed };
			}
		}
		else // Drop down.
		{
			DirStruct dir;
			dir.SetValue<5>(ScenarioClass::Instance->Random.RandomRanged(0, 31));
			const auto cos_factor = -2.44921270764e-16; // cos(1.5 * Math::Pi * 1.00001)
			const auto flatSpeed = cos_factor * speed;
			const auto radians = dir.GetRadian<32>();
			velocity = { Math::cos(radians) * flatSpeed, Math::sin(radians) * flatSpeed, -speed };
		}
	}

	if (radialFire.Segments > 0)
		velocity = ApplyRadialFireVelocityWarp(velocity, radialFire);

	// Unlimbo
	pBullet->MoveTo(sourceCoords, velocity);
}

BulletVelocity BulletExt::ApplyRadialFireVelocityWarp(BulletVelocity velocity, const RadialFireStruct& radialFire)
{
	if (radialFire.Segments <= 0)
		return velocity;

	const double speedXY = std::hypot(velocity.X, velocity.Y);

	if (speedXY <= 0.0)
		return velocity;

	const double offset =
		(Math::Pi / radialFire.Segments)
		* radialFire.Index
		- Math::HalfPi;

	const double baseAngle = radialFire.Direction.GetRadian<32>();
	const double angle = baseAngle + offset;

	velocity.X = std::cos(angle) * speedXY;
	velocity.Y = -std::sin(angle) * speedXY;

	return velocity;
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
void BulletExt::SimulatedFiringEffects(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach, bool firingEffect, bool visualEffect)
{
	if (firingEffect)
	{
		BulletExt::SimulatedFiringAnim(pBullet, pHouse, pAttach);
		BulletExt::SimulatedFiringReport(pBullet);
	}

	if (visualEffect)
	{
		BulletExt::SimulatedFiringLaser(pBullet, pHouse);
		BulletExt::SimulatedFiringElectricBolt(pBullet);
		BulletExt::SimulatedFiringRadBeam(pBullet, pHouse);
		BulletExt::SimulatedFiringParticleSystem(pBullet, pHouse);
	}
}

CoordStruct BulletExt::GetTargetCoordsForFiring(BulletClass* pBullet)
{
	if (pBullet->Type->Inviso && pBullet->Type->FlakScatter)
		return pBullet->Location;
	else if (const auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target))
		return pTarget->GetTargetCoords();

	return pBullet->TargetCoords;
}

void BulletExt::ApplyArcingFix(BulletClass* pThis, const CoordStruct& sourceCoords, const CoordStruct& targetCoords, BulletVelocity& velocity)
{
	const auto distanceCoords = targetCoords - sourceCoords;
	const auto horizontalDistance = Point2D { distanceCoords.X, distanceCoords.Y }.Magnitude();
	const bool lobber = pThis->WeaponType->Lobber || static_cast<int>(horizontalDistance) < distanceCoords.Z; // 0x70D590
	// The lower the horizontal velocity, the higher the trajectory
	// WW calculates the launch angle (and limits it) before calculating the velocity
	// Here, some magic numbers are used to directly simulate its calculation
	const auto speedMult = (lobber ? 0.45 : (distanceCoords.Z > 0 ? 0.68 : 1.0)); // Simulated 0x48A9D0
	const auto gravity = BulletTypeExt::GetAdjustedGravity(pThis->Type);
	const auto speed = speedMult * sqrt(horizontalDistance * gravity * 1.2); // 0x48AB90

	if (horizontalDistance < 1e-10 || speed < 1e-10)
	{
		// No solution
		velocity.Z = speed;
	}
	else
	{
		const auto mult = speed / horizontalDistance;
		velocity.X = static_cast<double>(distanceCoords.X) * mult;
		velocity.Y = static_cast<double>(distanceCoords.Y) * mult;
		velocity.Z = (static_cast<double>(distanceCoords.Z) + velocity.Z) * mult + (gravity * horizontalDistance) / (2 * speed);
	}
}

// Detonate weapon/warhead using a bullet.
void BulletExt::Detonate(const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse, AbstractClass* pTarget, bool isBright, WeaponTypeClass* pWeapon, WarheadTypeClass* pWarhead)
{
	auto const pType = pWeapon ? pWeapon->Projectile : BulletTypeExt::GetDefaultBulletType();
	auto const pBullet = pType->CreateBullet(pTarget, pOwner, damage, pWarhead, 100, isBright);
	pBullet->WeaponType = pWeapon;

	auto const pBulletExt = BulletExt::Fetch(pBullet);
	pBulletExt->IsInstantDetonation = true;

	if (pFiringHouse)
		pBulletExt->FirerHouse = pFiringHouse;

	pBullet->SetLocation(coords);
	pBullet->Explode(true);
	pBullet->UnInit();
}

// =============================
// load / save

template <typename T>
void BulletExt::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeExtData)
		.Process(this->FirerHouse)
		.Process(this->CurrentStrength)
		.Process(this->InterceptorTechnoType)
		.Process(this->InterceptedStatus)
		.Process(this->DetonateOnInterception)
		.Process(this->LaserTrails)
		.Process(this->SnappedToTarget)
		.Process(this->DamageNumberOffset)
		.Process(this->ParabombFallRate)
		.Process(this->IsInstantDetonation)
		.Process(this->FirepowerMult)
		.Process(this->IsSplitFromAirburst)

		.Process(this->Trajectory)
		.Process(this->DispersedTrajectory)
		.Process(this->LifeDurationTimer)
		.Process(this->NoTargetLifeTimer)
		.Process(this->RetargetTimer)
		.Process(this->AttenuationRange)
		.Process(this->TargetIsInAir)
		.Process(this->TargetIsTechno)
		.Process(this->NotMainWeapon)
		.Process(this->Status)
		.Process(this->FLHCoord)
		.Process(this->TrajectoryGroup)
		.Process(this->GroupIndex)
		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateTimer)
		.Process(this->ProximityImpact)
		.Process(this->ProximityDamage)
		.Process(this->ExtraCheck)
		.Process(this->Casualty)
		.Process(this->DisperseIndex)
		.Process(this->DisperseCount)
		.Process(this->DisperseCycle)
		.Process(this->DisperseTimer)
		;
}

void BulletExt::LoadFromStream(PhobosStreamReader& Stm)
{
	ObjectExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BulletExt::SaveToStream(PhobosStreamWriter& Stm)
{
	ObjectExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BulletGroupData::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool BulletGroupData::Save(PhobosStreamWriter& stm) const
{
	return const_cast<BulletGroupData*>(this)->Serialize(stm);
}

template <typename T>
bool BulletGroupData::Serialize(T& stm)
{
	return stm
		.Process(this->Bullets)
		.Process(this->Angle)
		.Process(this->ShouldUpdate)
		.Success();
}

// =============================
// container

BulletExt::ExtContainer::ExtContainer() : Container("BulletClass") {}

BulletExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.Remove(pItem);

	return 0;
}

