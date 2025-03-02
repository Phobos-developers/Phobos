#include "EngraveTrajectory.h"

#include <TacticalClass.h>
#include <LaserDrawClass.h>
#include <AircraftTrackerClass.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>

std::unique_ptr<PhobosTrajectory> EngraveTrajectoryType::CreateInstance() const
{
	return std::make_unique<EngraveTrajectory>(this);
}

template<typename T>
void EngraveTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->SourceCoord)
		.Process(this->TargetCoord)
		.Process(this->MirrorCoord)
		.Process(this->UseDisperseCoord)
		.Process(this->ApplyRangeModifiers)
		.Process(this->AllowFirerTurning)
		.Process(this->Duration)
		.Process(this->IsLaser)
		.Process(this->IsIntense)
		.Process(this->IsHouseColor)
		.Process(this->IsSingleColor)
		.Process(this->LaserInnerColor)
		.Process(this->LaserOuterColor)
		.Process(this->LaserOuterSpread)
		.Process(this->LaserThickness)
		.Process(this->LaserDuration)
		.Process(this->LaserDelay)
		.Process(this->DamageDelay)
		.Process(this->ProximityImpact)
		.Process(this->ProximityWarhead)
		.Process(this->ProximityDamage)
		.Process(this->ProximityRadius)
		.Process(this->ProximityDirect)
		.Process(this->ProximityMedial)
		.Process(this->ProximityAllies)
		.Process(this->ProximityFlight)
		.Process(this->ProximitySuicide)
		.Process(this->ConfineOnGround)
		;
}

bool EngraveTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool EngraveTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<EngraveTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void EngraveTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->Trajectory_Speed = Math::min(128.0, this->Trajectory_Speed);
	this->SourceCoord.Read(exINI, pSection, "Trajectory.Engrave.SourceCoord");
	this->TargetCoord.Read(exINI, pSection, "Trajectory.Engrave.TargetCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Engrave.MirrorCoord");
	this->UseDisperseCoord.Read(exINI, pSection, "Trajectory.Engrave.UseDisperseCoord");
	this->ApplyRangeModifiers.Read(exINI, pSection, "Trajectory.Engrave.ApplyRangeModifiers");
	this->AllowFirerTurning.Read(exINI, pSection, "Trajectory.Engrave.AllowFirerTurning");
	this->Duration.Read(exINI, pSection, "Trajectory.Engrave.Duration");
	this->IsLaser.Read(exINI, pSection, "Trajectory.Engrave.IsLaser");
	this->IsIntense.Read(exINI, pSection, "Trajectory.Engrave.IsIntense");
	this->IsHouseColor.Read(exINI, pSection, "Trajectory.Engrave.IsHouseColor");
	this->IsSingleColor.Read(exINI, pSection, "Trajectory.Engrave.IsSingleColor");
	this->LaserInnerColor.Read(exINI, pSection, "Trajectory.Engrave.LaserInnerColor");
	this->LaserOuterColor.Read(exINI, pSection, "Trajectory.Engrave.LaserOuterColor");
	this->LaserOuterSpread.Read(exINI, pSection, "Trajectory.Engrave.LaserOuterSpread");
	this->LaserThickness.Read(exINI, pSection, "Trajectory.Engrave.LaserThickness");
	this->LaserThickness = Math::max(1, this->LaserThickness);
	this->LaserDuration.Read(exINI, pSection, "Trajectory.Engrave.LaserDuration");
	this->LaserDuration = Math::max(1, this->LaserDuration);
	this->LaserDelay.Read(exINI, pSection, "Trajectory.Engrave.LaserDelay");
	this->LaserDelay = Math::max(1, this->LaserDelay);
	this->DamageDelay.Read(exINI, pSection, "Trajectory.Engrave.DamageDelay");
	this->DamageDelay = Math::max(1, this->DamageDelay);
	this->ProximityImpact.Read(exINI, pSection, "Trajectory.Engrave.ProximityImpact");
	this->ProximityWarhead.Read<true>(exINI, pSection, "Trajectory.Engrave.ProximityWarhead");
	this->ProximityDamage.Read(exINI, pSection, "Trajectory.Engrave.ProximityDamage");
	this->ProximityRadius.Read(exINI, pSection, "Trajectory.Engrave.ProximityRadius");
	this->ProximityDirect.Read(exINI, pSection, "Trajectory.Engrave.ProximityDirect");
	this->ProximityMedial.Read(exINI, pSection, "Trajectory.Engrave.ProximityMedial");
	this->ProximityAllies.Read(exINI, pSection, "Trajectory.Engrave.ProximityAllies");
	this->ProximityFlight.Read(exINI, pSection, "Trajectory.Engrave.ProximityFlight");
	this->ProximitySuicide.Read(exINI, pSection, "Trajectory.Engrave.ProximitySuicide");
	this->ConfineOnGround.Read(exINI, pSection, "Trajectory.Engrave.ConfineOnGround");
}

template<typename T>
void EngraveTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->SourceCoord)
		.Process(this->TargetCoord)
		.Process(this->Duration)
		.Process(this->LaserTimer)
		.Process(this->DamageTimer)
		.Process(this->TechnoInTransport)
		.Process(this->NotMainWeapon)
		.Process(this->FLHCoord)
		.Process(this->BuildingCoord)
		.Process(this->StartCoord)
		.Process(this->ProximityImpact)
		.Process(this->TheCasualty)
		;
}

bool EngraveTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Serialize(Stm);
	return true;
}

bool EngraveTrajectory::Save(PhobosStreamWriter& Stm) const
{
	const_cast<EngraveTrajectory*>(this)->Serialize(Stm);
	return true;
}

void EngraveTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	const auto pType = this->Type;
	this->LaserTimer.Start(0);
	this->DamageTimer.Start(0);
	this->FLHCoord = pBullet->SourceCoords;
	const auto pTechno = pBullet->Owner;

	// When the launcher exists, the launcher's location will be used to calculate the direction of the coordinate axis instead of bullet's SourceCoords
	if (pTechno)
	{
		for (auto pTrans = pTechno->Transporter; pTrans; pTrans = pTrans->Transporter)
			this->TechnoInTransport = pTrans->UniqueID;

		this->GetTechnoFLHCoord(pBullet, pTechno);
		this->CheckMirrorCoord(pTechno);

		const auto theSource = pTechno->GetCoords();
		const auto rotateAngle = Math::atan2(pBullet->TargetCoords.Y - theSource.Y , pBullet->TargetCoords.X - theSource.X);
		this->SetEngraveDirection(pBullet, rotateAngle);
	}
	else
	{
		this->NotMainWeapon = true;

		const auto rotateAngle = Math::atan2(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y , pBullet->TargetCoords.X - pBullet->SourceCoords.X);
		this->SetEngraveDirection(pBullet, rotateAngle);
	}

	// Substitute the speed to calculate velocity
	auto coordDistance = pBullet->Velocity.Magnitude();
	pBullet->Velocity *= (coordDistance > 1e-10) ? (pType->Trajectory_Speed / coordDistance) : 0;

	// Calculate additional range
	if (pType->ApplyRangeModifiers && pTechno)
	{
		if (const auto pWeapon = pBullet->WeaponType)
			coordDistance = static_cast<double>(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pTechno, static_cast<int>(coordDistance)));
	}

	// Automatically calculate duration
	if (this->Duration <= 0)
		this->Duration = static_cast<int>(coordDistance / pType->Trajectory_Speed) + 1;
}

bool EngraveTrajectory::OnAI(BulletClass* pBullet)
{
	const auto pTechno = pBullet->Owner;

	if (!this->NotMainWeapon && this->InvalidFireCondition(pBullet, pTechno))
		return true;

	if (--this->Duration < 0 || this->PlaceOnCorrectHeight(pBullet))
		return true;

	const auto pOwner = pTechno ? pTechno->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

	if (this->Type->IsLaser && this->LaserTimer.Completed())
		this->DrawEngraveLaser(pBullet, pTechno, pOwner);

	if (this->DamageTimer.Completed())
		this->DetonateLaserWarhead(pBullet, pTechno, pOwner);

	if (this->ProximityImpact != 0 && this->Type->ProximityRadius.Get() > 0)
		this->PrepareForDetonateAt(pBullet, pOwner);

	return false;
}

void EngraveTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	//Prevent damage again.
	pBullet->Health = 0;
	pBullet->Limbo();
	pBullet->UnInit();
}

void EngraveTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
}

TrajectoryCheckReturnType EngraveTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

TrajectoryCheckReturnType EngraveTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

void EngraveTrajectory::GetTechnoFLHCoord(BulletClass* pBullet, TechnoClass* pTechno)
{
	const auto pExt = TechnoExt::ExtMap.Find(pTechno);

	// Record the launch location, the building has an additional offset
	if (!pExt || !pExt->LastWeaponType || pExt->LastWeaponType->Projectile != pBullet->Type)
	{
		this->NotMainWeapon = true;
		return;
	}
	else if (pTechno->WhatAmI() == AbstractType::Building)
	{
		const auto pBuilding = static_cast<BuildingClass*>(pTechno);
		Matrix3D mtx;
		mtx.MakeIdentity();

		if (pTechno->HasTurret())
		{
			TechnoTypeExt::ApplyTurretOffset(pBuilding->Type, &mtx);
			mtx.RotateZ(static_cast<float>(pTechno->TurretFacing().GetRadian<32>()));
		}

		mtx.Translate(static_cast<float>(pExt->LastWeaponFLH.X), static_cast<float>(pExt->LastWeaponFLH.Y), static_cast<float>(pExt->LastWeaponFLH.Z));
		const auto result = mtx.GetTranslation();
		this->BuildingCoord = pBullet->SourceCoords - pBuilding->GetCoords() - CoordStruct { static_cast<int>(result.X), -static_cast<int>(result.Y), static_cast<int>(result.Z) };
	}

	this->FLHCoord = pExt->LastWeaponFLH;
}

inline void EngraveTrajectory::CheckMirrorCoord(TechnoClass* pTechno)
{
	if (this->NotMainWeapon || !(pTechno->CurrentBurstIndex % 2))
		return;

	if (this->Type->MirrorCoord)
	{
		this->SourceCoord.Y = -(this->SourceCoord.Y);
		this->TargetCoord.Y = -(this->TargetCoord.Y);
	}
}

void EngraveTrajectory::SetEngraveDirection(BulletClass* pBullet, double rotateAngle)
{
	auto theSource = pBullet->SourceCoords;
	auto theTarget = pBullet->TargetCoords;

	// Special case: Starting from the launch position
	if (this->SourceCoord.X != 0 || this->SourceCoord.Y != 0)
	{
		theSource = theTarget;
		theSource.X += static_cast<int>(this->SourceCoord.X * Math::cos(rotateAngle) + this->SourceCoord.Y * Math::sin(rotateAngle));
		theSource.Y += static_cast<int>(this->SourceCoord.X * Math::sin(rotateAngle) - this->SourceCoord.Y * Math::cos(rotateAngle));
	}

	if (this->Type->ConfineOnGround)
		theSource.Z = this->GetFloorCoordHeight(pBullet, theSource);

	this->StartCoord = theSource;
	pBullet->SetLocation(theSource);

	theTarget.X += static_cast<int>(this->TargetCoord.X * Math::cos(rotateAngle) + this->TargetCoord.Y * Math::sin(rotateAngle));
	theTarget.Y += static_cast<int>(this->TargetCoord.X * Math::sin(rotateAngle) - this->TargetCoord.Y * Math::cos(rotateAngle));

	pBullet->Velocity.X = theTarget.X - theSource.X;
	pBullet->Velocity.Y = theTarget.Y - theSource.Y;
	pBullet->Velocity.Z = 0;
}

bool EngraveTrajectory::InvalidFireCondition(BulletClass* pBullet, TechnoClass* pTechno)
{
	if (!pTechno)
		return true;

	for (auto pTrans = pTechno->Transporter; pTrans; pTrans = pTrans->Transporter)
		pTechno = pTrans;

	if (!TechnoExt::IsActive(pTechno) || (this->TechnoInTransport && this->TechnoInTransport != pTechno->UniqueID))
		return true;

	if (this->Type->AllowFirerTurning)
		return false;

	const auto SourceCrd = pTechno->GetCoords();
	const auto TargetCrd = pBullet->TargetCoords;

	const auto rotateAngle = Math::atan2(TargetCrd.Y - SourceCrd.Y , TargetCrd.X - SourceCrd.X);
	const auto tgtDir = DirStruct(-rotateAngle);

	const auto& face = pTechno->HasTurret() ? pTechno->SecondaryFacing : pTechno->PrimaryFacing;
	const auto curDir = face.Current();

	return (std::abs(static_cast<short>(static_cast<short>(tgtDir.Raw) - static_cast<short>(curDir.Raw))) >= 4096);
}

int EngraveTrajectory::GetFloorCoordHeight(BulletClass* pBullet, const CoordStruct& coord)
{
	const auto pCell = MapClass::Instance->GetCellAt(coord);
	const auto onFloor = MapClass::Instance->GetCellFloorHeight(coord);
	const auto onBridge = pCell->ContainsBridge() ? onFloor + CellClass::BridgeHeight : onFloor;

	return (pBullet->SourceCoords.Z >= onBridge || pBullet->TargetCoords.Z >= onBridge) ? onBridge : onFloor;
}

bool EngraveTrajectory::PlaceOnCorrectHeight(BulletClass* pBullet)
{
	if (!this->Type->ConfineOnGround)
		return false;

	auto bulletCoords = pBullet->Location;
	CoordStruct futureCoords
	{
		bulletCoords.X + static_cast<int>(pBullet->Velocity.X),
		bulletCoords.Y + static_cast<int>(pBullet->Velocity.Y),
		bulletCoords.Z + static_cast<int>(pBullet->Velocity.Z)
	};

	// Calculate where will be located in the next frame
	const auto checkDifference = this->GetFloorCoordHeight(pBullet, futureCoords) - futureCoords.Z;

	// When crossing the cliff, directly move the position of the bullet, otherwise change the vertical velocity (384 -> (4 * Unsorted::LevelHeight - 32(error range)))
	if (std::abs(checkDifference) >= 384)
	{
		if (pBullet->Type->SubjectToCliffs)
			return true;

		if (checkDifference > 0)
		{
			bulletCoords.Z += checkDifference;
			pBullet->SetLocation(bulletCoords);
		}
		else
		{
			const auto nowDifference = bulletCoords.Z - this->GetFloorCoordHeight(pBullet, bulletCoords);

			if (nowDifference >= 256)
			{
				bulletCoords.Z -= nowDifference;
				pBullet->SetLocation(bulletCoords);
			}
		}
	}
	else
	{
		pBullet->Velocity.Z += checkDifference;
	}

	return false;
}

void EngraveTrajectory::DrawEngraveLaser(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner)
{
	const auto pType = this->Type;
	this->LaserTimer.Start(pType->LaserDelay);
	auto fireCoord = pBullet->SourceCoords;

	for (auto pTrans = pTechno->Transporter; pTrans; pTrans = pTrans->Transporter)
		pTechno = pTrans;

	// Considering that the CurrentBurstIndex may be different, it is not possible to call existing functions
	if (!this->NotMainWeapon && pTechno && !pTechno->InLimbo)
	{
		if (pTechno->WhatAmI() != AbstractType::Building)
		{
			fireCoord = TechnoExt::GetFLHAbsoluteCoords(pTechno, this->FLHCoord, pTechno->HasTurret());
		}
		else
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			Matrix3D mtx;
			mtx.MakeIdentity();

			if (pTechno->HasTurret())
			{
				TechnoTypeExt::ApplyTurretOffset(pBuilding->Type, &mtx);
				mtx.RotateZ(static_cast<float>(pTechno->TurretFacing().GetRadian<32>()));
			}

			mtx.Translate(static_cast<float>(this->FLHCoord.X), static_cast<float>(this->FLHCoord.Y), static_cast<float>(this->FLHCoord.Z));
			const auto result = mtx.GetTranslation();
			fireCoord = pBuilding->GetCoords() + this->BuildingCoord + CoordStruct { static_cast<int>(result.X), -static_cast<int>(result.Y), static_cast<int>(result.Z) };
		}
	}

	// Draw laser from head to tail
	if (pType->IsHouseColor || pType->IsSingleColor)
	{
		const auto pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, ((pType->IsHouseColor && pOwner) ? pOwner->LaserColor : pType->LaserInnerColor), ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, pType->LaserDuration);
		pLaser->IsHouseColor = true;
		pLaser->Thickness = pType->LaserThickness;
		pLaser->IsSupported = pType->IsIntense;
	}
	else
	{
		const auto pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, pType->LaserInnerColor, pType->LaserOuterColor, pType->LaserOuterSpread, pType->LaserDuration);
		pLaser->IsHouseColor = false;
		pLaser->Thickness = 3;
		pLaser->IsSupported = false;
	}
}

inline void EngraveTrajectory::DetonateLaserWarhead(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner)
{
	this->DamageTimer.Start(this->Type->DamageDelay);
	WarheadTypeExt::DetonateAt(pBullet->WH, pBullet->Location, pTechno, pBullet->Health, pOwner);
}

// Select suitable targets and choose the closer targets then attack each target only once.
void EngraveTrajectory::PrepareForDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
	const auto pType = this->Type;
	const auto pWH = pType->ProximityWarhead;

	if (!pWH)
		return;

	// Step 1: Find valid targets on the ground within range.
	const auto radius = pType->ProximityRadius.Get();
	std::vector<CellClass*> recCellClass = PhobosTrajectoryType::GetCellsInProximityRadius(pBullet, radius);
	const size_t cellSize = recCellClass.size() * 2;
	size_t vectSize = cellSize;
	size_t thisSize = 0;

	const CoordStruct velocityCrd
	{
		static_cast<int>(pBullet->Velocity.X),
		static_cast<int>(pBullet->Velocity.Y),
		static_cast<int>(pBullet->Velocity.Z)
	};
	const auto velocitySq = velocityCrd.MagnitudeSquared();
	const auto pTarget = pBullet->Target;

	std::vector<TechnoClass*> validTechnos;
	validTechnos.reserve(vectSize);

	for (const auto& pRecCell : recCellClass)
	{
		auto pObject = pRecCell->GetContent();

		while (pObject)
		{
			const auto pTechno = abstract_cast<TechnoClass*>(pObject);
			pObject = pObject->NextObject;

			if (!pTechno || !pTechno->IsAlive || !pTechno->IsOnMap || pTechno->Health <= 0 || pTechno->InLimbo || pTechno->IsSinking)
				continue;

			const auto technoType = pTechno->WhatAmI();

			if (technoType == AbstractType::Building && static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame)
				continue;

			// Not directly harming friendly forces
			if (!pType->ProximityAllies && pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pTarget)
				continue;

			// Check distance
			const auto targetCrd = pTechno->GetCoords();
			const auto pathCrd = targetCrd - this->StartCoord;

			if (pathCrd * velocityCrd < 0) // In front of the techno
				continue;

			const auto distanceCrd = targetCrd - pBullet->Location;
			const auto nextDistanceCrd = distanceCrd - velocityCrd;

			if (nextDistanceCrd * velocityCrd > 0) // Behind the bullet
				continue;

			const auto cross = distanceCrd.CrossProduct(nextDistanceCrd).MagnitudeSquared();
			const auto distance = (velocitySq > 1e-10) ? sqrt(cross / velocitySq) : distanceCrd.Magnitude();

			if (technoType != AbstractType::Building && distance > radius) // In the cylinder
				continue;

			if (thisSize >= vectSize)
			{
				vectSize += cellSize;
				validTechnos.reserve(vectSize);
			}

			validTechnos.push_back(pTechno);
			thisSize += 1;
		}
	}

	// Step 2: Find valid targets in the air within range if necessary.
	if (pType->ProximityFlight)
	{
		const auto airTracker = &AircraftTrackerClass::Instance;
		airTracker->FillCurrentVector(MapClass::Instance->GetCellAt(pBullet->Location + velocityCrd * 0.5),
			Game::F2I(sqrt(radius * radius + (velocitySq / 4)) / Unsorted::LeptonsPerCell));

		for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
		{
			if (!pTechno->IsAlive || !pTechno->IsOnMap || pTechno->Health <= 0 || pTechno->InLimbo || pTechno->IsSinking)
				continue;

			// Not directly harming friendly forces
			if (!pType->ProximityAllies && pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pTarget)
				continue;

			// Check distance
			const auto targetCrd = pTechno->GetCoords();
			const auto pathCrd = targetCrd - this->StartCoord;

			if (pathCrd * velocityCrd < 0) // In front of the techno
				continue;

			const auto distanceCrd = targetCrd - pBullet->Location;
			const auto nextDistanceCrd = distanceCrd - velocityCrd;

			if (nextDistanceCrd * velocityCrd > 0) // Behind the bullet
				continue;

			const auto cross = distanceCrd.CrossProduct(nextDistanceCrd).MagnitudeSquared();
			const auto distance = (velocitySq > 1e-10) ? sqrt(cross / velocitySq) : distanceCrd.Magnitude();

			if (distance > radius) // In the cylinder
				continue;

			if (thisSize >= vectSize)
			{
				vectSize += cellSize;
				validTechnos.reserve(vectSize);
			}

			validTechnos.push_back(pTechno);
			thisSize += 1;
		}
	}

	// Step 3: Record each target without repetition.
	std::vector<int> casualtyChecked;
	casualtyChecked.reserve(std::max(validTechnos.size(), this->TheCasualty.size()));

	if (const auto pFirer = pBullet->Owner)
		this->TheCasualty[pFirer->UniqueID] = 20;

	// Update Record
	for (const auto& [ID, remainTime] : this->TheCasualty)
	{
		if (remainTime > 0)
			this->TheCasualty[ID] = remainTime - 1;
		else
			casualtyChecked.push_back(ID);
	}

	for (const auto& ID : casualtyChecked)
		this->TheCasualty.erase(ID);

	std::vector<TechnoClass*> validTargets;

	// checking for duplicate
	for (const auto& pTechno : validTechnos)
	{
		if (!this->TheCasualty.contains(pTechno->UniqueID))
			validTargets.push_back(pTechno);

		this->TheCasualty[pTechno->UniqueID] = 20;
	}

	// Step 4: Detonate warheads in sequence based on distance.
	const auto targetsSize = validTargets.size();

	if (this->ProximityImpact > 0 && static_cast<int>(targetsSize) > this->ProximityImpact)
	{
		std::sort(&validTargets[0], &validTargets[targetsSize],[this](TechnoClass* pTechnoA, TechnoClass* pTechnoB)
		{
			const auto distanceA = pTechnoA->GetCoords().DistanceFromSquared(this->StartCoord);
			const auto distanceB = pTechnoB->GetCoords().DistanceFromSquared(this->StartCoord);

			// Distance priority
			if (distanceA < distanceB)
				return true;

			if (distanceA > distanceB)
				return false;

			return pTechnoA->UniqueID < pTechnoB->UniqueID;
		});
	}

	for (const auto& pTechno : validTargets)
	{
		// Cause damage
		auto damage = pType->ProximityDamage;

		if (pType->ProximityDirect)
			pTechno->ReceiveDamage(&damage, 0, pWH, pBullet->Owner, false, false, pOwner);
		else if (pType->ProximityMedial)
			WarheadTypeExt::DetonateAt(pWH, pBullet->Location, pBullet->Owner, damage, pOwner);
		else
			WarheadTypeExt::DetonateAt(pWH, pTechno->GetCoords(), pBullet->Owner, damage, pOwner, pTechno);

		if (this->ProximityImpact > 0 && --this->ProximityImpact == 0)
		{
			if (pType->ProximitySuicide)
				this->Duration = 0;

			break;
		}
	}
}
