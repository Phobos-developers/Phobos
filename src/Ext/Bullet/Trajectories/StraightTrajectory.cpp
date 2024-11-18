#include "StraightTrajectory.h"
#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Techno/Body.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <AircraftTrackerClass.h>

std::unique_ptr<PhobosTrajectory> StraightTrajectoryType::CreateInstance() const
{
	return std::make_unique<StraightTrajectory>(this);
}

template<typename T>
void StraightTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
		.Process(this->ApplyRangeModifiers)
		.Process(this->PassThrough)
		.Process(this->PassDetonate)
		.Process(this->PassDetonateWarhead)
		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateDelay)
		.Process(this->PassDetonateTimer)
		.Process(this->PassDetonateLocal)
		.Process(this->LeadTimeCalculate)
		.Process(this->OffsetCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
		.Process(this->ProximityImpact)
		.Process(this->ProximityWarhead)
		.Process(this->ProximityDamage)
		.Process(this->ProximityRadius)
		.Process(this->ProximityDirect)
		.Process(this->ProximityMedial)
		.Process(this->ProximityAllies)
		.Process(this->ProximityFlight)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->SubjectToGround)
		.Process(this->ConfineAtHeight)
		.Process(this->EdgeAttenuation)
		.Process(this->CountAttenuation)
		;
}

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool StraightTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<StraightTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void StraightTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->ApplyRangeModifiers.Read(exINI, pSection, "Trajectory.Straight.ApplyRangeModifiers");
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Straight.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Straight.TargetSnapDistance");
	this->PassThrough.Read(exINI, pSection, "Trajectory.Straight.PassThrough");
	this->PassDetonate.Read(exINI, pSection, "Trajectory.Straight.PassDetonate");
	this->PassDetonateWarhead.Read<true>(exINI, pSection, "Trajectory.Straight.PassDetonateWarhead");
	this->PassDetonateDamage.Read(exINI, pSection, "Trajectory.Straight.PassDetonateDamage");
	this->PassDetonateDelay.Read(exINI, pSection, "Trajectory.Straight.PassDetonateDelay");
	this->PassDetonateTimer.Read(exINI, pSection, "Trajectory.Straight.PassDetonateTimer");
	this->PassDetonateLocal.Read(exINI, pSection, "Trajectory.Straight.PassDetonateLocal");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.Straight.LeadTimeCalculate");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.Straight.OffsetCoord");
	this->RotateCoord.Read(exINI, pSection, "Trajectory.Straight.RotateCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Straight.MirrorCoord");
	this->UseDisperseBurst.Read(exINI, pSection, "Trajectory.Straight.UseDisperseBurst");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.Straight.AxisOfRotation");
	this->ProximityImpact.Read(exINI, pSection, "Trajectory.Straight.ProximityImpact");
	this->ProximityWarhead.Read<true>(exINI, pSection, "Trajectory.Straight.ProximityWarhead");
	this->ProximityDamage.Read(exINI, pSection, "Trajectory.Straight.ProximityDamage");
	this->ProximityRadius.Read(exINI, pSection, "Trajectory.Straight.ProximityRadius");
	this->ProximityDirect.Read(exINI, pSection, "Trajectory.Straight.ProximityDirect");
	this->ProximityMedial.Read(exINI, pSection, "Trajectory.Straight.ProximityMedial");
	this->ProximityAllies.Read(exINI, pSection, "Trajectory.Straight.ProximityAllies");
	this->ProximityFlight.Read(exINI, pSection, "Trajectory.Straight.ProximityFlight");
	this->ThroughVehicles.Read(exINI, pSection, "Trajectory.Straight.ThroughVehicles");
	this->ThroughBuilding.Read(exINI, pSection, "Trajectory.Straight.ThroughBuilding");
	this->SubjectToGround.Read(exINI, pSection, "Trajectory.Straight.SubjectToGround");
	this->ConfineAtHeight.Read(exINI, pSection, "Trajectory.Straight.ConfineAtHeight");
	this->EdgeAttenuation.Read(exINI, pSection, "Trajectory.Straight.EdgeAttenuation");

	if (this->EdgeAttenuation < 0.0)
		this->EdgeAttenuation = 0.0;

	this->CountAttenuation.Read(exINI, pSection, "Trajectory.Straight.CountAttenuation");

	if (this->CountAttenuation < 0.0)
		this->CountAttenuation = 0.0;
}

template<typename T>
void StraightTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->DetonationDistance)
		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateTimer)
		.Process(this->OffsetCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->ProximityImpact)
		.Process(this->ProximityDamage)
		.Process(this->RemainingDistance)
		.Process(this->ExtraCheck)
		.Process(this->TheCasualty)
		.Process(this->FirepowerMult)
		.Process(this->AttenuationRange)
		.Process(this->LastTargetCoord)
		.Process(this->CurrentBurst)
		.Process(this->CountOfBurst)
		.Process(this->WaitOneFrame)
		;
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Serialize(Stm);
	return true;
}

bool StraightTrajectory::Save(PhobosStreamWriter& Stm) const
{
	const_cast<StraightTrajectory*>(this)->Serialize(Stm);
	return true;
}

void StraightTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	const auto pType = this->Type;
	this->PassDetonateTimer.Start(pType->PassDetonateTimer > 0 ? pType->PassDetonateTimer : 0);
	this->LastTargetCoord = pBullet->TargetCoords;
	pBullet->Velocity = BulletVelocity::Empty;
	const auto pOwner = pBullet->Owner;

	if (const auto pWeapon = pBullet->WeaponType)
	{
		this->AttenuationRange = pWeapon->Range;
		this->CountOfBurst = pWeapon->Burst;

		if (pType->ApplyRangeModifiers && pOwner)
		{
			if (this->DetonationDistance >= 0)
				this->DetonationDistance = Leptons(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pOwner, this->DetonationDistance));
			else
				this->DetonationDistance = Leptons(-WeaponTypeExt::GetRangeWithModifiers(pWeapon, pOwner, -this->DetonationDistance));

			this->AttenuationRange = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pOwner);
		}
	}

	if (pOwner)
	{
		this->FirepowerMult = pOwner->FirepowerMultiplier;
		this->CurrentBurst = pOwner->CurrentBurstIndex;

		if (pType->MirrorCoord && pOwner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	if (!pType->LeadTimeCalculate || !abstract_cast<FootClass*>(pBullet->Target))
		this->PrepareForOpenFire(pBullet);
	else
		this->WaitOneFrame = 2;
}

bool StraightTrajectory::OnAI(BulletClass* pBullet)
{
	if (this->WaitOneFrame && this->BulletPrepareCheck(pBullet))
		return false;

	const auto pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
	const auto pType = this->Type;

	if (this->BulletDetonatePreCheck(pBullet, pType->Trajectory_Speed))
		return true;

	if (pType->PassDetonate)
		this->PassWithDetonateAt(pBullet, pOwner);

	if (this->ProximityImpact != 0 && pType->ProximityRadius.Get() > 0)
		this->PrepareForDetonateAt(pBullet, pOwner);

	if (pType->Trajectory_Speed < 256.0 && pType->ConfineAtHeight > 0 && this->PassAndConfineAtHeight(pBullet, pType->Trajectory_Speed))
		return true;

	this->BulletDetonateLastCheck(pBullet, pOwner, pType->Trajectory_Speed);

	return false;
}

void StraightTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	const auto pType = this->Type;
	const auto pTechno = abstract_cast<TechnoClass*>(pBullet->Target);
	pBullet->Health = this->GetTheTrueDamage(pBullet->Health, pBullet, pTechno, true);

	if (pType->PassDetonateLocal)
	{
		CoordStruct detonateCoords = pBullet->Location;
		detonateCoords.Z = MapClass::Instance->GetCellFloorHeight(detonateCoords);
		pBullet->SetLocation(detonateCoords);
	}

	const auto targetSnapDistance = pType->TargetSnapDistance.Get();

	if (pType->PassThrough || targetSnapDistance <= 0)
		return;

	const auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	const auto coords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (coords.DistanceFrom(pBullet->Location) <= targetSnapDistance)
	{
		const auto pExt = BulletExt::ExtMap.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(coords);
	}
}

void StraightTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType StraightTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // No longer needed.
}

TrajectoryCheckReturnType StraightTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

void StraightTrajectory::PrepareForOpenFire(BulletClass* pBullet)
{
	const auto pType = this->Type;
	double rotateAngle = 0.0;
	const auto pTarget = pBullet->Target;
	auto theTargetCoords = pBullet->TargetCoords;
	auto theSourceCoords = pBullet->SourceCoords;

	//TODO If I could calculate this before firing, perhaps it can solve the problem of one frame delay and not so correct turret orientation.
	if (pType->LeadTimeCalculate && pTarget)
	{
		theTargetCoords = pTarget->GetCoords();
		theSourceCoords = pBullet->Location;

		if (theTargetCoords != this->LastTargetCoord)
		{
			const auto extraOffsetCoord = theTargetCoords - this->LastTargetCoord;
			const auto targetSourceCoord = theSourceCoords - theTargetCoords;
			const auto lastSourceCoord = theSourceCoords - this->LastTargetCoord;

			const auto theDistanceSquared = targetSourceCoord.MagnitudeSquared();
			const auto targetSpeedSquared = extraOffsetCoord.MagnitudeSquared();
			const auto targetSpeed = sqrt(targetSpeedSquared);

			const auto crossFactor = lastSourceCoord.CrossProduct(targetSourceCoord).MagnitudeSquared();
			const auto verticalDistanceSquared = crossFactor / targetSpeedSquared;

			const auto horizonDistanceSquared = theDistanceSquared - verticalDistanceSquared;
			const auto horizonDistance = sqrt(horizonDistanceSquared);

			const auto straightSpeedSquared = pType->Trajectory_Speed * pType->Trajectory_Speed;
			const auto baseFactor = straightSpeedSquared - targetSpeedSquared;
			const auto squareFactor = baseFactor * verticalDistanceSquared + straightSpeedSquared * horizonDistanceSquared;

			if (squareFactor > 1e-10)
			{
				const auto minusFactor = -(horizonDistance * targetSpeed);
				int travelTime = 0;

				if (abs(baseFactor) < 1e-10)
				{
					travelTime = abs(horizonDistance) > 1e-10 ? (static_cast<int>(theDistanceSquared / (2 * horizonDistance * targetSpeed)) + 1) : 0;
				}
				else
				{
					const auto travelTimeM = static_cast<int>((minusFactor - sqrt(squareFactor)) / baseFactor);
					const auto travelTimeP = static_cast<int>((minusFactor + sqrt(squareFactor)) / baseFactor);

					if (travelTimeM > 0 && travelTimeP > 0)
						travelTime = travelTimeM < travelTimeP ? travelTimeM : travelTimeP;
					else if (travelTimeM > 0)
						travelTime = travelTimeM;
					else if (travelTimeP > 0)
						travelTime = travelTimeP;

					if (targetSourceCoord.MagnitudeSquared() < lastSourceCoord.MagnitudeSquared())
						travelTime += 1;
					else
						travelTime += 2;
				}

				theTargetCoords += extraOffsetCoord * travelTime;
			}
		}
	}

	if (!pType->LeadTimeCalculate && theTargetCoords == theSourceCoords && pBullet->Owner) //For disperse.
	{
		const auto theOwnerCoords = pBullet->Owner->GetCoords();
		rotateAngle = Math::atan2(theTargetCoords.Y - theOwnerCoords.Y , theTargetCoords.X - theOwnerCoords.X);
	}
	else
	{
		rotateAngle = Math::atan2(theTargetCoords.Y - theSourceCoords.Y , theTargetCoords.X - theSourceCoords.X);
	}

	if (this->OffsetCoord != CoordStruct::Empty)
	{
		theTargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(rotateAngle) + this->OffsetCoord.Y * Math::sin(rotateAngle));
		theTargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(rotateAngle) - this->OffsetCoord.Y * Math::cos(rotateAngle));
		theTargetCoords.Z += this->OffsetCoord.Z;
	}

	if (pBullet->Type->Inaccurate)
	{
		const auto pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		const auto offsetMult = 0.0004 * theSourceCoords.DistanceFrom(theTargetCoords);
		const auto offsetMin = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Min.Get(Leptons(0)));
		const auto offsetMax = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter)));
		const auto offsetDistance = ScenarioClass::Instance->Random.RandomRanged(offsetMin, offsetMax);
		theTargetCoords = MapClass::GetRandomCoordsNear(theTargetCoords, offsetDistance, false);
	}

	if (pType->PassThrough)
	{
		if (this->DetonationDistance > 0)
			this->RemainingDistance += static_cast<int>(this->DetonationDistance + pType->Trajectory_Speed);
		else if (this->DetonationDistance < 0)
			this->RemainingDistance += static_cast<int>(theSourceCoords.DistanceFrom(theTargetCoords) - this->DetonationDistance + pType->Trajectory_Speed);
		else
			this->RemainingDistance = INT_MAX;
	}
	else
	{
		this->RemainingDistance += static_cast<int>(theSourceCoords.DistanceFrom(theTargetCoords) + pType->Trajectory_Speed);
	}

	pBullet->TargetCoords = theTargetCoords;
	pBullet->Velocity.X = static_cast<double>(theTargetCoords.X - theSourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(theTargetCoords.Y - theSourceCoords.Y);

	if (pType->ConfineAtHeight > 0 && pType->PassDetonateLocal)
		pBullet->Velocity.Z = 0;
	else
		pBullet->Velocity.Z = static_cast<double>(this->GetVelocityZ(pBullet));

	if (!this->UseDisperseBurst && abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
	{
		const auto axis = pType->AxisOfRotation.Get();

		BulletVelocity rotationAxis
		{
			axis.X * Math::cos(rotateAngle) + axis.Y * Math::sin(rotateAngle),
			axis.X * Math::sin(rotateAngle) - axis.Y * Math::cos(rotateAngle),
			static_cast<double>(axis.Z)
		};

		const auto rotationAxisLengthSquared = rotationAxis.MagnitudeSquared();

		if (abs(rotationAxisLengthSquared) > 1e-10)
		{
			double extraRotate = 0.0;
			rotationAxis *= 1 / sqrt(rotationAxisLengthSquared);

			if (pType->MirrorCoord)
			{
				if (this->CurrentBurst % 2 == 1)
					rotationAxis *= -1;

				extraRotate = Math::Pi * (pType->RotateCoord * ((this->CurrentBurst / 2) / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}
			else
			{
				extraRotate = Math::Pi * (pType->RotateCoord * (this->CurrentBurst / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}

			const auto cosRotate = Math::cos(extraRotate);
			pBullet->Velocity = (pBullet->Velocity * cosRotate) + (rotationAxis * ((1 - cosRotate) * (pBullet->Velocity * rotationAxis))) + (rotationAxis.CrossProduct(pBullet->Velocity) * Math::sin(extraRotate));
		}
	}

	if (this->CalculateBulletVelocity(pBullet, pType->Trajectory_Speed))
		this->RemainingDistance = 0;
}

int StraightTrajectory::GetVelocityZ(BulletClass* pBullet)
{
	const auto pType = this->Type;
	auto sourceCellZ = pBullet->SourceCoords.Z;
	auto targetCellZ = pBullet->TargetCoords.Z;
	auto bulletVelocityZ = static_cast<int>(targetCellZ - sourceCellZ);

	if (!pType->PassThrough)
		return bulletVelocityZ;

	if (const auto pTechno = pBullet->Owner)
	{
		const auto pCell = pTechno->GetCell();
		sourceCellZ = pCell->Level * Unsorted::LevelHeight;

		if (pCell->ContainsBridge())
			sourceCellZ += CellClass::BridgeHeight;
	}

	if (const auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target))
	{
		const auto pCell = pTarget->GetCell();
		targetCellZ = pCell->Level * Unsorted::LevelHeight;

		if (pCell->ContainsBridge())
			targetCellZ += CellClass::BridgeHeight;
	}

	if (sourceCellZ == targetCellZ || abs(bulletVelocityZ) <= 32)
	{
		if (!this->DetonationDistance)
			return 0;

		const CoordStruct sourceCoords { pBullet->SourceCoords.X, pBullet->SourceCoords.Y, 0 };
		const CoordStruct targetCoords { pBullet->TargetCoords.X, pBullet->TargetCoords.Y, 0 };
		const auto distanceOfTwo = sourceCoords.DistanceFrom(targetCoords);
		const auto theDistance = (this->DetonationDistance < 0) ? (distanceOfTwo - this->DetonationDistance) : this->DetonationDistance;

		if (abs(theDistance) > 1e-10)
			bulletVelocityZ = static_cast<int>(bulletVelocityZ * (distanceOfTwo / theDistance));
		else
			return 0;
	}

	return bulletVelocityZ;
}

bool StraightTrajectory::CalculateBulletVelocity(BulletClass* pBullet, double straightSpeed)
{
	const auto velocityLength = pBullet->Velocity.Magnitude();

	if (velocityLength > 1e-10)
		pBullet->Velocity *= straightSpeed / velocityLength;
	else
		return true;

	return false;
}

bool StraightTrajectory::BulletPrepareCheck(BulletClass* pBullet)
{
	// The time between bullets' Unlimbo() and Update() is completely uncertain.
	// Technos will update its location after firing, which may result in inaccurate
	// target position recorded by the LastTargetCoord in Unlimbo(). Therefore, it's
	// necessary to record the position during the first Update(). - CrimRecya
	if (this->WaitOneFrame == 2)
	{
		if (const auto pTarget = pBullet->Target)
		{
			this->LastTargetCoord = pTarget->GetCoords();
			this->WaitOneFrame = 1;
			return true;
		}
	}

	this->WaitOneFrame = 0;
	this->PrepareForOpenFire(pBullet);

	return false;
}

bool StraightTrajectory::BulletDetonatePreCheck(BulletClass* pBullet, double straightSpeed)
{
	if (this->ExtraCheck)
	{
		pBullet->SetTarget(this->ExtraCheck);
		pBullet->TargetCoords = this->ExtraCheck->GetCoords();
		return true;
	}

	this->RemainingDistance -= static_cast<int>(straightSpeed);

	if (this->RemainingDistance < 0)
		return true;

	const auto pType = this->Type;

	if (!pType->PassThrough && pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	if (pType->SubjectToGround && MapClass::Instance->GetCellFloorHeight(pBullet->Location) >= (pBullet->Location.Z + 15))
		return true;

	if (const auto pCell = MapClass::Instance->TryGetCellAt(pBullet->Location))
		return false;
	else
		return true;
}

//If the check result here is true, it only needs to be detonated in the next frame, without returning.
void StraightTrajectory::BulletDetonateLastCheck(BulletClass* pBullet, HouseClass* pOwner, double straightSpeed)
{
	const auto pType = this->Type;
	bool velocityCheck = false;
	double locationDistance = this->RemainingDistance;

	if (locationDistance < straightSpeed)
		velocityCheck = true;

	if (this->ExtraCheck)
	{
		locationDistance = this->ExtraCheck->GetCoords().DistanceFrom(pBullet->Location);
		velocityCheck = true;
	}

	const bool checkThrough = (!pType->ThroughBuilding || !pType->ThroughVehicles);
	const bool checkSubject = (pType->SubjectToGround || pBullet->Type->SubjectToWalls);

	if (straightSpeed < 256.0) //Low speed with checkSubject was already done well.
	{
		if (checkThrough)
		{
			if (const auto pCell = MapClass::Instance->GetCellAt(pBullet->Location))
			{
				if (this->CheckThroughAndSubjectInCell(pBullet, pCell, pOwner))
				{
					locationDistance = 0.0;
					velocityCheck = true;
				}
			}
		}
	}
	else if (checkThrough || checkSubject)
	{
		const auto theSourceCoords = pBullet->Location;
		const CoordStruct theTargetCoords
		{
			pBullet->Location.X + static_cast<int>(pBullet->Velocity.X),
			pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y),
			pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z)
		};

		const auto sourceCell = CellClass::Coord2Cell(theSourceCoords);
		const auto targetCell = CellClass::Coord2Cell(theTargetCoords);
		const auto cellDist = sourceCell - targetCell;
		const auto cellPace = CellStruct { static_cast<short>(std::abs(cellDist.X)), static_cast<short>(std::abs(cellDist.Y)) };

		auto largePace = static_cast<size_t>(std::max(cellPace.X, cellPace.Y));
		const auto stepCoord = !largePace ? CoordStruct::Empty : (theTargetCoords - theSourceCoords) * (1.0 / largePace);
		auto curCoord = theSourceCoords;
		auto pCurCell = MapClass::Instance->GetCellAt(sourceCell);
		double cellDistance = locationDistance;

		for (size_t i = 0; i < largePace; ++i)
		{
			if (pType->SubjectToGround && (curCoord.Z + 15) < MapClass::Instance->GetCellFloorHeight(curCoord))
			{
				velocityCheck = true;
				cellDistance = curCoord.DistanceFrom(theSourceCoords);
				break;
			}

			if (pBullet->Type->SubjectToWalls && pCurCell->OverlayTypeIndex != -1 && OverlayTypeClass::Array->GetItem(pCurCell->OverlayTypeIndex)->Wall)
			{
				velocityCheck = true;
				cellDistance = curCoord.DistanceFrom(theSourceCoords);
				break;
			}

			if (checkThrough && this->CheckThroughAndSubjectInCell(pBullet, pCurCell, pOwner))
			{
				velocityCheck = true;
				cellDistance = curCoord.DistanceFrom(theSourceCoords);
				break;
			}

			curCoord += stepCoord;
			pCurCell = MapClass::Instance->GetCellAt(curCoord);
		}

		locationDistance = cellDistance;
	}

	if (velocityCheck)
	{
		this->RemainingDistance = 0;
		pBullet->Velocity *= locationDistance / straightSpeed;
	}
}

bool StraightTrajectory::CheckThroughAndSubjectInCell(BulletClass* pBullet, CellClass* pCell, HouseClass* pOwner)
{
	const auto pType = this->Type;
	auto pObject = pCell->GetContent();
	TechnoClass* pNearest = nullptr;

	while (pObject)
	{
		const auto pTechno = abstract_cast<TechnoClass*>(pObject);
		pObject = pObject->NextObject;

		if (!pTechno || (pOwner->IsAlliedWith(pTechno->Owner) && pTechno != abstract_cast<TechnoClass*>(pBullet->Target)))
			continue;

		const auto technoType = pTechno->WhatAmI();

		if (technoType == AbstractType::Building)
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);

			if (pBuilding->Type->InvisibleInGame)
				continue;

			if (pBuilding->Type->IsVehicle() ? !pType->ThroughVehicles : !pType->ThroughBuilding)
				pNearest = pTechno;
		}

		if (!pType->ThroughVehicles && (technoType == AbstractType::Unit || technoType == AbstractType::Aircraft))
			pNearest = pTechno;
	}

	if (pNearest && this->ProximityImpact != 0)
	{
		const auto pWH = pType->ProximityWarhead;

		if (!pWH)
			return static_cast<bool>(pNearest);

		auto damage = this->GetTheTrueDamage(this->ProximityDamage, pBullet, pType->ProximityMedial ? nullptr : pNearest, false);

		if (pType->ProximityDirect)
			pNearest->ReceiveDamage(&damage, 0, pWH, pBullet->Owner, false, false, pOwner);
		else
			WarheadTypeExt::DetonateAt(pWH, pType->ProximityMedial ? pBullet->Location : pNearest->GetCoords(), pBullet->Owner, damage, pOwner, pNearest);

		this->CalculateNewDamage(pBullet);
	}

	return static_cast<bool>(pNearest);
}

void StraightTrajectory::CalculateNewDamage(BulletClass* pBullet)
{
	const auto pType = this->Type;
	const auto ratio = pType->CountAttenuation.Get();

	if (ratio != 1.0)
	{
		if (ratio)
		{
			if (pBullet->Health)
			{
				if (const auto newDamage = static_cast<int>(pBullet->Health * ratio))
					pBullet->Health = newDamage;
				else
					pBullet->Health = Math::sgn(pBullet->Health);
			}

			if (this->ProximityDamage)
			{
				if (const auto newDamage = static_cast<int>(this->ProximityDamage * ratio))
					this->ProximityDamage = newDamage;
				else
					this->ProximityDamage = Math::sgn(this->ProximityDamage);
			}

			if (this->PassDetonateDamage)
			{
				if (const auto newDamage = static_cast<int>(this->PassDetonateDamage * ratio))
					this->PassDetonateDamage = newDamage;
				else
					this->PassDetonateDamage = Math::sgn(this->PassDetonateDamage);
			}
		}
		else
		{
			pBullet->Health = 0;
			this->ProximityDamage = 0;
			this->PassDetonateDamage = 0;
		}
	}
}

void StraightTrajectory::PassWithDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->PassDetonateTimer.Completed())
	{
		const auto pType = this->Type;
		const auto pWH = pType->PassDetonateWarhead;

		if (!pWH)
			return;

		this->PassDetonateTimer.Start(pType->PassDetonateDelay > 0 ? pType->PassDetonateDelay : 1);
		auto detonateCoords = pBullet->Location;

		if (pType->PassDetonateLocal)
			detonateCoords.Z = MapClass::Instance->GetCellFloorHeight(detonateCoords);

		const auto damage = this->GetTheTrueDamage(this->PassDetonateDamage, pBullet, nullptr, false);
		WarheadTypeExt::DetonateAt(pWH, detonateCoords, pBullet->Owner, damage, pOwner);
		this->CalculateNewDamage(pBullet);
	}
}

//Select suitable targets and choose the closer targets then attack each target only once.
void StraightTrajectory::PrepareForDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
	const auto pType = this->Type;
	const auto pWH = pType->ProximityWarhead;

	if (!pWH)
		return;

	//Step 1: Find valid targets on the ground within range.
	std::vector<CellClass*> recCellClass = this->GetCellsInProximityRadius(pBullet);
	const size_t cellSize = recCellClass.size() * 2;
	size_t vectSize = cellSize;
	size_t thisSize = 0;

	const CoordStruct velocityCrd
	{
		static_cast<int>(pBullet->Velocity.X),
		static_cast<int>(pBullet->Velocity.Y),
		static_cast<int>(pBullet->Velocity.Z)
	};

	std::vector<TechnoClass*> validTechnos;
	validTechnos.reserve(vectSize);
	const auto pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target);

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

			if (technoType == AbstractType::Building)
			{
				const auto pBuilding = static_cast<BuildingClass*>(pTechno);

				if (pBuilding->Type->InvisibleInGame || (pBuilding->Type->IsVehicle() ? !pType->ThroughVehicles : !pType->ThroughBuilding))
					continue;
			}

			if (!pType->ThroughVehicles && (technoType == AbstractType::Unit || technoType == AbstractType::Aircraft))
				continue;

			if (!pType->ProximityAllies && pOwner->IsAlliedWith(pTechno->Owner) && !(pTargetTechno && pTechno == pTargetTechno))
				continue;

			const auto distanceCrd = pTechno->GetCoords() - pBullet->SourceCoords;
			const auto locationCrd = (velocityCrd + (pBullet->Location - pBullet->SourceCoords));
			const auto terminalCrd = distanceCrd - locationCrd;
			auto distance = locationCrd.MagnitudeSquared(); //Not true distance yet.

			if (distanceCrd * velocityCrd < 0 || terminalCrd * velocityCrd > 0)
				continue;

			distance = (distance > 1e-10) ? sqrt(distanceCrd.CrossProduct(terminalCrd).MagnitudeSquared() / distance) : distanceCrd.Magnitude();

			if (technoType != AbstractType::Building && distance > pType->ProximityRadius.Get())
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

	//Step 2: Find valid targets in the air within range if necessary.
	if (pType->ProximityFlight)
	{
		const auto airTracker = &AircraftTrackerClass::Instance;
		airTracker->FillCurrentVector(MapClass::Instance->GetCellAt(pBullet->Location + velocityCrd * 0.5), static_cast<int>((pType->ProximityRadius.Get() + pType->Trajectory_Speed / 2) / Unsorted::LeptonsPerCell));

		for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
		{
			if (!pTechno->IsAlive || !pTechno->IsOnMap || pTechno->Health <= 0 || pTechno->InLimbo || pTechno->IsSinking)
				continue;

			if (!pType->ProximityAllies && pOwner->IsAlliedWith(pTechno->Owner) && !(pTargetTechno && pTechno == pTargetTechno))
				continue;

			const auto technoType = pTechno->WhatAmI();

			if (!pType->ThroughVehicles && (technoType == AbstractType::Unit || technoType == AbstractType::Aircraft))
				continue;

			const auto distanceCrd = pTechno->GetCoords() - pBullet->Location;
			const auto terminalCrd = distanceCrd - velocityCrd;
			auto distance = velocityCrd.MagnitudeSquared(); //Not true distance yet.

			if (distanceCrd * velocityCrd < 0 || terminalCrd * velocityCrd > 0)
				continue;

			distance = (distance > 1e-10) ? sqrt(distanceCrd.CrossProduct(terminalCrd).MagnitudeSquared() / distance) : distanceCrd.Magnitude();

			if (distance > pType->ProximityRadius.Get())
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

	//Step 3: Record each target without repetition.
	std::vector<TechnoClass*> casualtyChecked;
	casualtyChecked.reserve(std::max(validTechnos.size(), this->TheCasualty.size()));

	if (pBullet->Owner)
		this->TheCasualty[pBullet->Owner] = 20;

	for (const auto& [pTechno, remainTime] : this->TheCasualty)
	{
		if (pTechno->IsAlive && pTechno->IsOnMap && pTechno->Health > 0 && !pTechno->InLimbo && !pTechno->IsSinking && remainTime > 0)
			this->TheCasualty[pTechno] = remainTime - 1;
		else
			casualtyChecked.push_back(pTechno);
	}

	for (const auto& pTechno : casualtyChecked)
	{
		this->TheCasualty.erase(pTechno);
	}

	casualtyChecked.clear();

	for (const auto& pTechno : validTechnos)
	{
		if (!this->TheCasualty.contains(pTechno))
			casualtyChecked.push_back(pTechno);

		this->TheCasualty[pTechno] = 20;
	}

	//Step 4: Detonate warheads in sequence based on distance.
	const auto casualtySize = casualtyChecked.size();

	if (this->ProximityImpact > 0 && static_cast<int>(casualtySize) > this->ProximityImpact)
	{
		std::sort(&casualtyChecked[0], &casualtyChecked[casualtySize],[pBullet](TechnoClass* pTechnoA, TechnoClass* pTechnoB){
			return pTechnoA->GetCoords().DistanceFromSquared(pBullet->SourceCoords) < pTechnoB->GetCoords().DistanceFromSquared(pBullet->SourceCoords);
		});
	}

	for (const auto& pTechno : casualtyChecked)
	{
		auto damage = this->GetTheTrueDamage(this->ProximityDamage, pBullet, pType->ProximityMedial ? nullptr : pTechno, false);

		if (pType->ProximityDirect)
			pTechno->ReceiveDamage(&damage, 0, pWH, pBullet->Owner, false, false, pOwner);
		else
			WarheadTypeExt::DetonateAt(pWH, pType->ProximityMedial ? pBullet->Location : pTechno->GetCoords(), pBullet->Owner, damage, pOwner, pTechno);

		this->CalculateNewDamage(pBullet);

		if (this->ProximityImpact == 1)
		{
			this->ExtraCheck = pTechno;
			this->ProximityImpact = 0;
			break;
		}
		else if (this->ProximityImpact > 0)
		{
			--this->ProximityImpact;
		}
	}
}

//A rectangular shape with a custom width from the current frame to the next frame in length.
std::vector<CellClass*> StraightTrajectory::GetCellsInProximityRadius(BulletClass* pBullet)
{
	//Seems like the y-axis is reversed, but it's okay.
	const CoordStruct walkCoord { static_cast<int>(pBullet->Velocity.X), static_cast<int>(pBullet->Velocity.Y), 0 };
	const auto sideMult = this->Type->ProximityRadius.Get() / walkCoord.Magnitude();

	const CoordStruct cor1Coord { static_cast<int>(walkCoord.Y * sideMult), static_cast<int>((-walkCoord.X) * sideMult), 0 };
	const CoordStruct cor4Coord { static_cast<int>((-walkCoord.Y) * sideMult), static_cast<int>(walkCoord.X * sideMult), 0 };
	const auto thisCell = CellClass::Coord2Cell(pBullet->Location);

	auto cor1Cell = CellClass::Coord2Cell((pBullet->Location + cor1Coord));
	auto cor4Cell = CellClass::Coord2Cell((pBullet->Location + cor4Coord));

	const auto off1Cell = cor1Cell - thisCell;
	const auto off4Cell = cor4Cell - thisCell;
	const auto nextCell = CellClass::Coord2Cell((pBullet->Location + walkCoord));

	auto cor2Cell = nextCell + off1Cell;
	auto cor3Cell = nextCell + off4Cell;

	//Arrange the vertices of the rectangle in order from bottom to top.
	int cornerIndex = 0;
	CellStruct corner[4] = { cor1Cell, cor2Cell, cor3Cell, cor4Cell };

	for (int i = 1; i < 4; ++i)
	{
		if (corner[cornerIndex].Y > corner[i].Y)
			cornerIndex = i;
	}

	cor1Cell = corner[cornerIndex];
	++cornerIndex %= 4;
	cor2Cell = corner[cornerIndex];
	++cornerIndex %= 4;
	cor3Cell = corner[cornerIndex];
	++cornerIndex %= 4;
	cor4Cell = corner[cornerIndex];

	std::vector<CellStruct> recCells = this->GetCellsInRectangle(cor1Cell, cor4Cell, cor2Cell, cor3Cell);
	std::vector<CellClass*> recCellClass;
	recCellClass.reserve(recCells.size());

	for (const auto& pCells : recCells)
	{
		if (CellClass* pRecCell = MapClass::Instance->TryGetCellAt(pCells))
			recCellClass.push_back(pRecCell);
	}

	return recCellClass;
}

//Record cells in the order of "draw left boundary, draw right boundary, fill middle, and move up one level".
std::vector<CellStruct> StraightTrajectory::GetCellsInRectangle(CellStruct bottomStaCell, CellStruct leftMidCell, CellStruct rightMidCell, CellStruct topEndCell)
{
	std::vector<CellStruct> recCells;
	const auto cellNums = (abs(topEndCell.Y - bottomStaCell.Y) + 1) * (abs(rightMidCell.X - leftMidCell.X) + 1);
	recCells.reserve(cellNums);
	recCells.push_back(bottomStaCell);

	if (bottomStaCell == leftMidCell || bottomStaCell == rightMidCell)
	{
		auto middleCurCell = bottomStaCell;

		const auto middleTheDist = topEndCell - bottomStaCell;
		const CellStruct middleTheUnit { static_cast<short>(Math::sgn(middleTheDist.X)), static_cast<short>(Math::sgn(middleTheDist.Y)) };
		const CellStruct middleThePace { static_cast<short>(middleTheDist.X * middleTheUnit.X), static_cast<short>(middleTheDist.Y * middleTheUnit.Y) };
		auto mTheCurN = static_cast<float>((middleThePace.Y - middleThePace.X) / 2.0);

		while (middleCurCell != topEndCell)
		{
			if (mTheCurN > 0)
			{
				mTheCurN -= middleThePace.X;
				middleCurCell.Y += middleTheUnit.Y;
				recCells.push_back(middleCurCell);
			}
			else if (mTheCurN < 0)
			{
				mTheCurN += middleThePace.Y;
				middleCurCell.X += middleTheUnit.X;
				recCells.push_back(middleCurCell);
			}
			else
			{
				mTheCurN += middleThePace.Y - middleThePace.X;
				middleCurCell.X += middleTheUnit.X;
				recCells.push_back(middleCurCell);
				middleCurCell.X -= middleTheUnit.X;
				middleCurCell.Y += middleTheUnit.Y;
				recCells.push_back(middleCurCell);
				middleCurCell.X += middleTheUnit.X;
				recCells.push_back(middleCurCell);
			}
		}
	}
	else
	{
		auto leftCurCell = bottomStaCell;
		auto rightCurCell = bottomStaCell;
		auto middleCurCell = bottomStaCell;

		bool leftNext = false;
		bool rightNext = false;
		bool leftSkip = false;
		bool rightSkip = false;
		bool leftContinue = false;
		bool rightContinue = false;

		const auto left1stDist = leftMidCell - bottomStaCell;
		const CellStruct left1stUnit { static_cast<short>(Math::sgn(left1stDist.X)), static_cast<short>(Math::sgn(left1stDist.Y)) };
		const CellStruct left1stPace { static_cast<short>(left1stDist.X * left1stUnit.X), static_cast<short>(left1stDist.Y * left1stUnit.Y) };
		auto left1stCurN = static_cast<float>((left1stPace.Y - left1stPace.X) / 2.0);

		const auto left2ndDist = topEndCell - leftMidCell;
		const CellStruct left2ndUnit { static_cast<short>(Math::sgn(left2ndDist.X)), static_cast<short>(Math::sgn(left2ndDist.Y)) };
		const CellStruct left2ndPace { static_cast<short>(left2ndDist.X * left2ndUnit.X), static_cast<short>(left2ndDist.Y * left2ndUnit.Y) };
		auto left2ndCurN = static_cast<float>((left2ndPace.Y - left2ndPace.X) / 2.0);

		const auto right1stDist = rightMidCell - bottomStaCell;
		const CellStruct right1stUnit { static_cast<short>(Math::sgn(right1stDist.X)), static_cast<short>(Math::sgn(right1stDist.Y)) };
		const CellStruct right1stPace { static_cast<short>(right1stDist.X * right1stUnit.X), static_cast<short>(right1stDist.Y * right1stUnit.Y) };
		auto right1stCurN = static_cast<float>((right1stPace.Y - right1stPace.X) / 2.0);

		const auto right2ndDist = topEndCell - rightMidCell;
		const CellStruct right2ndUnit { static_cast<short>(Math::sgn(right2ndDist.X)), static_cast<short>(Math::sgn(right2ndDist.Y)) };
		const CellStruct right2ndPace { static_cast<short>(right2ndDist.X * right2ndUnit.X), static_cast<short>(right2ndDist.Y * right2ndUnit.Y) };
		auto right2ndCurN = static_cast<float>((right2ndPace.Y - right2ndPace.X) / 2.0);

		while (leftCurCell != topEndCell || rightCurCell != topEndCell)
		{
			while (leftCurCell != topEndCell) //Left
			{
				if (!leftNext) //Bottom Left Side
				{
					if (left1stCurN > 0)
					{
						left1stCurN -= left1stPace.X;
						leftCurCell.Y += left1stUnit.Y;

						if (leftCurCell == leftMidCell)
						{
							leftNext = true;
						}
						else
						{
							recCells.push_back(leftCurCell);
							break;
						}
					}
					else
					{
						left1stCurN += left1stPace.Y;
						leftCurCell.X += left1stUnit.X;

						if (leftCurCell == leftMidCell)
						{
							leftNext = true;
							leftSkip = true;
						}
					}
				}
				else //Top Left Side
				{
					if (left2ndCurN >= 0)
					{
						if (leftSkip)
						{
							leftSkip = false;
							left2ndCurN -= left2ndPace.X;
							leftCurCell.Y += left2ndUnit.Y;
						}
						else
						{
							leftContinue = true;
							break;
						}
					}
					else
					{
						left2ndCurN += left2ndPace.Y;
						leftCurCell.X += left2ndUnit.X;
					}
				}

				if (leftCurCell != rightCurCell) //Avoid double counting cells.
					recCells.push_back(leftCurCell);
			}

			while (rightCurCell != topEndCell) //Right
			{
				if (!rightNext) //Bottom Right Side
				{
					if (right1stCurN > 0)
					{
						right1stCurN -= right1stPace.X;
						rightCurCell.Y += right1stUnit.Y;

						if (rightCurCell == rightMidCell)
						{
							rightNext = true;
						}
						else
						{
							recCells.push_back(rightCurCell);
							break;
						}
					}
					else
					{
						right1stCurN += right1stPace.Y;
						rightCurCell.X += right1stUnit.X;

						if (rightCurCell == rightMidCell)
						{
							rightNext = true;
							rightSkip = true;
						}
					}
				}
				else //Top Right Side
				{
					if (right2ndCurN >= 0)
					{
						if (rightSkip)
						{
							rightSkip = false;
							right2ndCurN -= right2ndPace.X;
							rightCurCell.Y += right2ndUnit.Y;
						}
						else
						{
							rightContinue = true;
							break;
						}
					}
					else
					{
						right2ndCurN += right2ndPace.Y;
						rightCurCell.X += right2ndUnit.X;
					}
				}

				if (rightCurCell != leftCurCell) //Avoid double counting cells.
					recCells.push_back(rightCurCell);
			}

			middleCurCell = leftCurCell;
			middleCurCell.X += 1;

			while (middleCurCell.X < rightCurCell.X) //Center
			{
				recCells.push_back(middleCurCell);
				middleCurCell.X += 1;
			}

			if (leftContinue) //Continue Top Left Side
			{
				leftContinue = false;
				left2ndCurN -= left2ndPace.X;
				leftCurCell.Y += left2ndUnit.Y;
				recCells.push_back(leftCurCell);
			}

			if (rightContinue) //Continue Top Right Side
			{
				rightContinue = false;
				right2ndCurN -= right2ndPace.X;
				rightCurCell.Y += right2ndUnit.Y;
				recCells.push_back(rightCurCell);
			}
		}
	}

	return recCells;
}

int StraightTrajectory::GetTheTrueDamage(int damage, BulletClass* pBullet, TechnoClass* pTechno, bool self)
{
	if (damage == 0)
		return 0;

	const auto pType = this->Type;

	if (pType->EdgeAttenuation != 1.0)
	{
		const auto damageMultiplier = this->GetExtraDamageMultiplier(pBullet, pTechno, pType->EdgeAttenuation);
		const auto calculatedDamage = self ? damage * damageMultiplier : damage * this->FirepowerMult * damageMultiplier;
		const auto signal = Math::sgn(calculatedDamage);
		damage = static_cast<int>(calculatedDamage);

		if (!damage && pType->EdgeAttenuation > 0.0)
			damage = signal;
	}

	return damage;
}

double StraightTrajectory::GetExtraDamageMultiplier(BulletClass* pBullet, TechnoClass* pTechno, double edgeAttenuation)
{
	double distance = 0.0;
	double damageMult = 1.0;

	if (pTechno)
		distance = pTechno->GetCoords().DistanceFrom(pBullet->SourceCoords);
	else
		distance = pBullet->Location.DistanceFrom(pBullet->SourceCoords);

	if (this->AttenuationRange < static_cast<int>(distance))
		return edgeAttenuation;

	if (distance > 256.0)
		damageMult += (edgeAttenuation - 1.0) * ((distance - 256.0) / (static_cast<double>(this->AttenuationRange - 256)));

	return damageMult;
}

bool StraightTrajectory::PassAndConfineAtHeight(BulletClass* pBullet, double straightSpeed)
{
	const CoordStruct futureCoords
	{
		pBullet->Location.X + static_cast<int>(pBullet->Velocity.X),
		pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y),
		pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z)
	};

	if (const auto pCell = MapClass::Instance->GetCellAt(futureCoords))
	{
		auto checkDifference = MapClass::Instance->GetCellFloorHeight(futureCoords) - futureCoords.Z;
		const auto cellCoords = pCell->GetCoordsWithBridge();
		const auto differenceOnBridge = cellCoords.Z - futureCoords.Z;

		if (abs(differenceOnBridge) < abs(checkDifference))
			checkDifference = differenceOnBridge;

		if (abs(checkDifference) < 384 || !pBullet->Type->SubjectToCliffs)
		{
			const auto pType = this->Type;
			pBullet->Velocity.Z += static_cast<double>(checkDifference + pType->ConfineAtHeight);

			if (!pType->PassDetonateLocal && this->CalculateBulletVelocity(pBullet, straightSpeed))
				return true;
		}
		else
		{
			return true;
		}
	}

	return false;
}
