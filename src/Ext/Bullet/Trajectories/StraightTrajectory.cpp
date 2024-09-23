#include "StraightTrajectory.h"
#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Techno/Body.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <AircraftTrackerClass.h>

PhobosTrajectory* StraightTrajectoryType::CreateInstance() const
{
	return new StraightTrajectory(this);
}

template<typename T>
void StraightTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->ApplyRangeModifiers)
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
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
}

template<typename T>
void StraightTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
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
		.Process(this->RemainingDistance)
		.Process(this->ExtraCheck)
		.Process(this->LastCasualty)
		.Process(this->FirepowerMult)
		.Process(this->LastTargetCoord)
		.Process(this->CurrentBurst)
		.Process(this->CountOfBurst)
		.Process(this->WaitOneFrame)
		;
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool StraightTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);
	const_cast<StraightTrajectory*>(this)->Serialize(Stm);
	return true;
}

void StraightTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	this->LastCasualty.reserve(1);
	this->LastTargetCoord = pBullet->TargetCoords;
	pBullet->Velocity = BulletVelocity::Empty;

	TechnoClass* const pOwner = pBullet->Owner;
	WeaponTypeClass* const pWeapon = pBullet->WeaponType;

	if (pWeapon)
	{
		this->AttenuationRange = pWeapon->Range;
		this->CountOfBurst = pWeapon->Burst;

		if (this->ApplyRangeModifiers && pOwner)
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
		const CasualtyData TheOwner {pOwner, 20};
		this->LastCasualty.push_back(TheOwner);
		this->FirepowerMult = pOwner->FirepowerMultiplier;
		this->CurrentBurst = pOwner->CurrentBurstIndex;

		if (this->MirrorCoord && pOwner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	if (!this->LeadTimeCalculate || !abstract_cast<FootClass*>(pBullet->Target))
		this->PrepareForOpenFire(pBullet);
	else
		this->WaitOneFrame.Start(1);
}

bool StraightTrajectory::OnAI(BulletClass* pBullet)
{
	if (this->WaitOneFrame.IsTicking() && this->BulletPrepareCheck(pBullet))
		return false;

	const double straightSpeed = this->GetTrajectorySpeed(pBullet);
	HouseClass* const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

	if (this->BulletDetonatePreCheck(pBullet, straightSpeed))
		return true;

	if (this->PassDetonate)
		this->PassWithDetonateAt(pBullet, pOwner);

	if (this->ProximityImpact != 0 && this->ProximityRadius > 0)
		this->PrepareForDetonateAt(pBullet, pOwner);

	if (straightSpeed < 256.0 && this->ConfineAtHeight > 0 && this->PassAndConfineAtHeight(pBullet, straightSpeed))
		return true;

	this->BulletDetonateLastCheck(pBullet, pOwner, straightSpeed);

	return false;
}

void StraightTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	TechnoClass* pTechno = abstract_cast<TechnoClass*>(pBullet->Target);
	pBullet->Health = this->GetTheTrueDamage(pBullet->Health, pBullet, pTechno, true);

	if (this->PassDetonateLocal)
	{
		CoordStruct detonateCoords = pBullet->Location;
		detonateCoords.Z = MapClass::Instance->GetCellFloorHeight(detonateCoords);
		pBullet->SetLocation(detonateCoords);
	}

	if (this->PassThrough || this->TargetSnapDistance <= 0)
		return;

	const ObjectClass* const pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	const CoordStruct coords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (coords.DistanceFrom(pBullet->Location) <= this->TargetSnapDistance)
	{
		auto const pExt = BulletExt::ExtMap.Find(pBullet);
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
	double rotateAngle = 0.0;
	const double straightSpeed = this->GetTrajectorySpeed(pBullet);
	const AbstractClass* const pTarget = pBullet->Target;
	CoordStruct theTargetCoords = pBullet->TargetCoords;
	CoordStruct theSourceCoords = pBullet->SourceCoords;

	//TODO If I could calculate this before firing, perhaps it can solve the problem of one frame delay and not so correct turret orientation.
	if (this->LeadTimeCalculate && pTarget)
	{
		theTargetCoords = pTarget->GetCoords();
		theSourceCoords = pBullet->Location;

		if (theTargetCoords != this->LastTargetCoord)
		{
			const CoordStruct extraOffsetCoord = theTargetCoords - this->LastTargetCoord;
			const CoordStruct targetSourceCoord = theSourceCoords - theTargetCoords;
			const CoordStruct lastSourceCoord = theSourceCoords - this->LastTargetCoord;

			const double theDistanceSquared = targetSourceCoord.MagnitudeSquared();
			const double targetSpeedSquared = extraOffsetCoord.MagnitudeSquared();
			const double targetSpeed = sqrt(targetSpeedSquared);

			const double crossFactor = lastSourceCoord.CrossProduct(targetSourceCoord).MagnitudeSquared();
			const double verticalDistanceSquared = crossFactor / targetSpeedSquared;

			const double horizonDistanceSquared = theDistanceSquared - verticalDistanceSquared;
			const double horizonDistance = sqrt(horizonDistanceSquared);

			const double straightSpeedSquared = straightSpeed * straightSpeed;
			const double baseFactor = straightSpeedSquared - targetSpeedSquared;
			const double squareFactor = baseFactor * verticalDistanceSquared + straightSpeedSquared * horizonDistanceSquared;

			if (squareFactor > 1e-10)
			{
				const double minusFactor = -(horizonDistance * targetSpeed);
				int travelTime = 0;

				if (abs(baseFactor) < 1e-10)
				{
					travelTime = abs(horizonDistance) > 1e-10 ? (static_cast<int>(theDistanceSquared / (2 * horizonDistance * targetSpeed)) + 1) : 0;
				}
				else
				{
					const int travelTimeM = static_cast<int>((minusFactor - sqrt(squareFactor)) / baseFactor);
					const int travelTimeP = static_cast<int>((minusFactor + sqrt(squareFactor)) / baseFactor);

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

	if (!this->LeadTimeCalculate && theTargetCoords == theSourceCoords && pBullet->Owner) //For disperse.
	{
		const CoordStruct theOwnerCoords = pBullet->Owner->GetCoords();
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
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		const double offsetMult = 0.0004 * theSourceCoords.DistanceFrom(theTargetCoords);
		const int offsetMin = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Min.Get(Leptons(0)));
		const int offsetMax = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter)));
		const int offsetDistance = ScenarioClass::Instance->Random.RandomRanged(offsetMin, offsetMax);
		theTargetCoords = MapClass::GetRandomCoordsNear(theTargetCoords, offsetDistance, false);
	}

	if (this->PassThrough)
	{
		if (this->DetonationDistance > 0)
			this->RemainingDistance += static_cast<int>(this->DetonationDistance + straightSpeed);
		else if (this->DetonationDistance < 0)
			this->RemainingDistance += static_cast<int>(theSourceCoords.DistanceFrom(theTargetCoords) - this->DetonationDistance + straightSpeed);
		else
			this->RemainingDistance = INT_MAX;
	}
	else
	{
		this->RemainingDistance += static_cast<int>(theSourceCoords.DistanceFrom(theTargetCoords) + straightSpeed);
	}

	pBullet->TargetCoords = theTargetCoords;
	pBullet->Velocity.X = static_cast<double>(theTargetCoords.X - theSourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(theTargetCoords.Y - theSourceCoords.Y);

	if (this->ConfineAtHeight > 0 && this->PassDetonateLocal)
		pBullet->Velocity.Z = 0;
	else
		pBullet->Velocity.Z = static_cast<double>(this->GetVelocityZ(pBullet));

	if (!this->UseDisperseBurst && abs(this->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
	{
		BulletVelocity rotationAxis
		{
			this->AxisOfRotation.X * Math::cos(rotateAngle) + this->AxisOfRotation.Y * Math::sin(rotateAngle),
			this->AxisOfRotation.X * Math::sin(rotateAngle) - this->AxisOfRotation.Y * Math::cos(rotateAngle),
			static_cast<double>(this->AxisOfRotation.Z)
		};

		const double rotationAxisLengthSquared = rotationAxis.MagnitudeSquared();

		if (abs(rotationAxisLengthSquared) > 1e-10)
		{
			double extraRotate = 0.0;
			rotationAxis *= 1 / sqrt(rotationAxisLengthSquared);

			if (this->MirrorCoord)
			{
				if (pBullet->Owner && pBullet->Owner->CurrentBurstIndex % 2 == 1)
					rotationAxis *= -1;

				extraRotate = Math::Pi * (this->RotateCoord * ((this->CurrentBurst / 2) / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}
			else
			{
				extraRotate = Math::Pi * (this->RotateCoord * (this->CurrentBurst / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}

			const double cosRotate = Math::cos(extraRotate);
			pBullet->Velocity = (pBullet->Velocity * cosRotate) + (rotationAxis * ((1 - cosRotate) * (pBullet->Velocity * rotationAxis))) + (rotationAxis.CrossProduct(pBullet->Velocity) * Math::sin(extraRotate));
		}
	}

	if (this->CalculateBulletVelocity(pBullet, straightSpeed))
		this->RemainingDistance = 0;
}

int StraightTrajectory::GetVelocityZ(BulletClass* pBullet)
{
	int bulletVelocity = static_cast<int>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);

	if (!this->PassThrough)
		return bulletVelocity;

	if (pBullet->Owner && abs(pBullet->Owner->GetCoords().Z - pBullet->TargetCoords.Z) <= 32)
	{
		const double distanceOfTwo = pBullet->SourceCoords.DistanceFrom(pBullet->TargetCoords);
		double theDistance = this->DetonationDistance;

		if (this->DetonationDistance == 0)
			return 0;

		if (this->DetonationDistance < 0)
			theDistance = distanceOfTwo - this->DetonationDistance;

		if (abs(theDistance) > 1e-10)
			bulletVelocity = static_cast<int>(bulletVelocity * (distanceOfTwo / theDistance));
		else
			return 0;
	}

	return bulletVelocity;
}

bool StraightTrajectory::CalculateBulletVelocity(BulletClass* pBullet, double straightSpeed)
{
	const double velocityLength = pBullet->Velocity.Magnitude();

	if (velocityLength > 1e-10)
		pBullet->Velocity *= straightSpeed / velocityLength;
	else
		return true;

	return false;
}

bool StraightTrajectory::BulletPrepareCheck(BulletClass* pBullet)
{
	if (this->WaitOneFrame.HasTimeLeft())
		return true;

	this->PrepareForOpenFire(pBullet);
	this->WaitOneFrame.Stop();

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

	if (!this->PassThrough && this->DetonationDistance > 0 && pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	if (this->SubjectToGround && MapClass::Instance->GetCellFloorHeight(pBullet->Location) >= (pBullet->Location.Z + 15))
		return true;

	if (CellClass* const pCell = MapClass::Instance->TryGetCellAt(pBullet->Location))
		return false;
	else
		return true;
}

//If the check result here is true, it only needs to be detonated in the next frame, without returning.
void StraightTrajectory::BulletDetonateLastCheck(BulletClass* pBullet, HouseClass* pOwner, double straightSpeed)
{
	bool velocityCheck = false;
	double locationDistance = this->RemainingDistance;

	if (locationDistance < straightSpeed)
		velocityCheck = true;

	if (this->ExtraCheck)
	{
		locationDistance = this->ExtraCheck->GetCoords().DistanceFrom(pBullet->Location);
		velocityCheck = true;
	}

	const bool checkThrough = (!this->ThroughBuilding || !this->ThroughVehicles);
	const bool checkSubject = (this->SubjectToGround || pBullet->Type->SubjectToWalls);

	if (straightSpeed < 256.0) //Low speed with checkSubject was already done well.
	{
		if (checkThrough)
		{
			if (CellClass* const pCell = MapClass::Instance->GetCellAt(pBullet->Location))
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
		const CoordStruct theSourceCoords = pBullet->Location;
		const CoordStruct theTargetCoords
		{
			pBullet->Location.X + static_cast<int>(pBullet->Velocity.X),
			pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y),
			pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z)
		};

		const CellStruct sourceCell = CellClass::Coord2Cell(theSourceCoords);
		const CellStruct targetCell = CellClass::Coord2Cell(theTargetCoords);
		const CellStruct cellDist = sourceCell - targetCell;
		const CellStruct cellPace = CellStruct { static_cast<short>(std::abs(cellDist.X)), static_cast<short>(std::abs(cellDist.Y)) };

		size_t largePace = static_cast<size_t>(std::max(cellPace.X, cellPace.Y));
		const CoordStruct stepCoord = !largePace ? CoordStruct::Empty : (theTargetCoords - theSourceCoords) * (1.0 / largePace);
		CoordStruct curCoord = theSourceCoords;
		CellClass* pCurCell = MapClass::Instance->GetCellAt(sourceCell);
		double cellDistance = locationDistance;

		for (size_t i = 0; i < largePace; ++i)
		{
			if (this->SubjectToGround && (curCoord.Z + 15) < MapClass::Instance->GetCellFloorHeight(curCoord))
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
	ObjectClass* pObject = pCell->FirstObject;
	TechnoClass* pNearest = nullptr;

	while (pObject)
	{
		TechnoClass* const pTechno = abstract_cast<TechnoClass*>(pObject);
		pObject = pObject->NextObject;

		if (!pTechno || (pOwner->IsAlliedWith(pTechno->Owner) && pTechno != abstract_cast<TechnoClass*>(pBullet->Target)))
			continue;

		const AbstractType technoType = pTechno->WhatAmI();

		if (technoType == AbstractType::Building)
		{
			BuildingClass* pBuilding = static_cast<BuildingClass*>(pTechno);

			if (pBuilding->Type->InvisibleInGame)
				continue;

			if (pBuilding->Type->IsVehicle() ? !this->ThroughVehicles : !this->ThroughBuilding)
				pNearest = pTechno;
		}

		if (!this->ThroughVehicles && (technoType == AbstractType::Unit || technoType == AbstractType::Aircraft))
			pNearest = pTechno;
	}

	if (pNearest && this->ProximityImpact != 0)
	{
		WarheadTypeClass* pWH = this->ProximityWarhead;

		if (!pWH)
			return static_cast<bool>(pNearest);

		int damage = this->GetTheTrueDamage(this->ProximityDamage, pBullet, this->ProximityMedial ? nullptr : pNearest, false);

		if (this->ProximityDirect)
			pNearest->ReceiveDamage(&damage, 0, pWH, pBullet->Owner, false, false, pOwner);
		else
			WarheadTypeExt::DetonateAt(pWH, this->ProximityMedial ? pBullet->Location : pNearest->GetCoords(), pBullet->Owner, damage, pOwner, pNearest);
	}

	return static_cast<bool>(pNearest);
}

void StraightTrajectory::PassWithDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->PassDetonateTimer.Completed())
	{
		this->PassDetonateTimer.Start(this->PassDetonateDelay);
		CoordStruct detonateCoords = pBullet->Location;

		if (this->PassDetonateLocal)
			detonateCoords.Z = MapClass::Instance->GetCellFloorHeight(detonateCoords);

		WarheadTypeClass* pWH = this->PassDetonateWarhead;

		if (!pWH)
			return;

		const int damage = this->GetTheTrueDamage(this->PassDetonateDamage, pBullet, nullptr, false);
		WarheadTypeExt::DetonateAt(pWH, detonateCoords, pBullet->Owner, damage, pOwner);
	}
}

//Select suitable targets and choose the closer targets then attack each target only once.
void StraightTrajectory::PrepareForDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
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
	const TechnoClass* const pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target);

	for (auto const& pRecCell : recCellClass)
	{
		ObjectClass* pObject = pRecCell->FirstObject;

		while (pObject)
		{
			TechnoClass* const pTechno = abstract_cast<TechnoClass*>(pObject);
			pObject = pObject->NextObject;

			if (!pTechno || !pTechno->IsAlive || !pTechno->IsOnMap || pTechno->Health <= 0)
				continue;

			const AbstractType technoType = pTechno->WhatAmI();

			if (technoType == AbstractType::Building)
			{
				BuildingClass* const pBuilding = static_cast<BuildingClass*>(pTechno);

				if (pBuilding->Type->InvisibleInGame || (pBuilding->Type->IsVehicle() ? !this->ThroughVehicles : !this->ThroughBuilding))
					continue;
			}

			if (!this->ThroughVehicles && (technoType == AbstractType::Unit || technoType == AbstractType::Aircraft))
				continue;

			if (!this->ProximityAllies && pOwner->IsAlliedWith(pTechno->Owner) && !(pTargetTechno && pTechno == pTargetTechno))
				continue;

			const CoordStruct distanceCrd = pTechno->GetCoords() - pBullet->SourceCoords;
			const CoordStruct locationCrd = (velocityCrd + (pBullet->Location - pBullet->SourceCoords));
			const CoordStruct terminalCrd = distanceCrd - locationCrd;
			double distance = locationCrd.MagnitudeSquared(); //Not true distance yet.

			if (distanceCrd * velocityCrd < 0 || terminalCrd * velocityCrd > 0)
				continue;

			distance = (distance > 1e-10) ? sqrt(distanceCrd.CrossProduct(terminalCrd).MagnitudeSquared() / distance) : distanceCrd.Magnitude();

			if (technoType != AbstractType::Building && distance > this->ProximityRadius)
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
	if (this->ProximityFlight)
	{
		AircraftTrackerClass* const airTracker = &AircraftTrackerClass::Instance.get();
		airTracker->FillCurrentVector(MapClass::Instance->GetCellAt(pBullet->Location + velocityCrd * 0.5), static_cast<int>((this->ProximityRadius + this->GetTrajectorySpeed(pBullet) / 2) / Unsorted::LeptonsPerCell));

		for (TechnoClass* pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
		{
			if (!pTechno->IsAlive || !pTechno->IsOnMap || pTechno->Health <= 0)
				continue;

			if (!this->ProximityAllies && pOwner->IsAlliedWith(pTechno->Owner) && !(pTargetTechno && pTechno == pTargetTechno))
				continue;

			const AbstractType technoType = pTechno->WhatAmI();

			if (!this->ThroughVehicles && (technoType == AbstractType::Unit || technoType == AbstractType::Aircraft))
				continue;

			const CoordStruct distanceCrd = pTechno->GetCoords() - pBullet->Location;
			const CoordStruct terminalCrd = distanceCrd - velocityCrd;
			double distance = velocityCrd.MagnitudeSquared(); //Not true distance yet.

			if (distanceCrd * velocityCrd < 0 || terminalCrd * velocityCrd > 0)
				continue;

			distance = (distance > 1e-10) ? sqrt(distanceCrd.CrossProduct(terminalCrd).MagnitudeSquared() / distance) : distanceCrd.Magnitude();

			if (distance > this->ProximityRadius)
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
	const size_t iMax = this->LastCasualty.size();
	const size_t jMax = validTechnos.size();
	const size_t capacity = iMax + jMax;

	std::sort(&validTechnos[0], &validTechnos[jMax]);
	std::vector<CasualtyData> casualty;
	casualty.reserve(capacity);
	std::vector<TechnoClass*> casualtyChecked;
	casualtyChecked.reserve(capacity);

	size_t i = 0;
	size_t j = 0;
	TechnoClass* pThis = nullptr;
	TechnoClass* pLast = nullptr;
	int thisTime = 0;
	bool check = false;

	for (size_t k = 0; k < capacity; ++k) //Merge, and avoid using wild pointers
	{
		if (i < iMax && j < jMax)
		{
			if (this->LastCasualty[i].pCasualty < validTechnos[j])
			{
				check = false; // Don't know whether wild
				pThis = this->LastCasualty[i].pCasualty;
				thisTime = this->LastCasualty[i].RemainTime;
				++i;
			}
			else if (this->LastCasualty[i].pCasualty > validTechnos[j])
			{
				check = true; // Not duplicated and not wild
				pThis = validTechnos[j];
				thisTime = 20;
				++j;
			}
			else // this->LastCasualty[i].pCasualty == validTechnos[j]
			{
				check = false; // Duplicated and not wild
				pThis = validTechnos[j];
				thisTime = 20;
				++i;
				++j;
			}
		}
		else if (i < iMax)
		{
			check = false; // Don't know whether wild
			pThis = this->LastCasualty[i].pCasualty;
			thisTime = this->LastCasualty[i].RemainTime;
			++i;
		}
		else if (j < jMax)
		{
			check = true; // Not duplicated and not wild
			pThis = validTechnos[j];
			thisTime = 20;
			++j;
		}
		else
		{
			break; // No technos left
		}

		if (pThis && pThis != pLast)
		{
			if (check) // Not duplicated pointer, and not wild pointer
				casualtyChecked.push_back(pThis);

			if (--thisTime > 0) // Record 20 frames
			{
				const CasualtyData thisCasualty {pThis, thisTime};
				casualty.push_back(thisCasualty);
			}

			pLast = pThis;
		}
	}

	this->LastCasualty = casualty; // Record vector for next check

	//Step 4: Detonate warheads in sequence based on distance.
	const size_t casualtySize = casualtyChecked.size();

	if (this->ProximityImpact > 0 && static_cast<int>(casualtySize) > this->ProximityImpact)
	{
		std::sort(&casualtyChecked[0], &casualtyChecked[casualtySize],[pBullet](TechnoClass* pTechnoA, TechnoClass* pTechnoB){
			return pTechnoA->GetCoords().DistanceFromSquared(pBullet->SourceCoords) < pTechnoB->GetCoords().DistanceFromSquared(pBullet->SourceCoords);
		});
	}

	WarheadTypeClass* pWH = this->ProximityWarhead;

	if (!pWH)
		return;

	for (auto const& pTechno : casualtyChecked)
	{
		int damage = this->GetTheTrueDamage(this->ProximityDamage, pBullet, this->ProximityMedial ? nullptr : pTechno, false);

		if (this->ProximityDirect)
			pTechno->ReceiveDamage(&damage, 0, pWH, pBullet->Owner, false, false, pOwner);
		else
			WarheadTypeExt::DetonateAt(pWH, this->ProximityMedial ? pBullet->Location : pTechno->GetCoords(), pBullet->Owner, damage, pOwner, pTechno);

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
	const double sideMult = this->ProximityRadius / walkCoord.Magnitude();

	const CoordStruct cor1Coord { static_cast<int>(walkCoord.Y * sideMult), static_cast<int>((-walkCoord.X) * sideMult), 0 };
	const CoordStruct cor4Coord { static_cast<int>((-walkCoord.Y) * sideMult), static_cast<int>(walkCoord.X * sideMult), 0 };
	const CellStruct thisCell = CellClass::Coord2Cell(pBullet->Location);

	CellStruct cor1Cell = CellClass::Coord2Cell((pBullet->Location + cor1Coord));
	CellStruct cor4Cell = CellClass::Coord2Cell((pBullet->Location + cor4Coord));

	const CellStruct off1Cell = cor1Cell - thisCell;
	const CellStruct off4Cell = cor4Cell - thisCell;
	const CellStruct nextCell = CellClass::Coord2Cell((pBullet->Location + walkCoord));

	CellStruct cor2Cell = nextCell + off1Cell;
	CellStruct cor3Cell = nextCell + off4Cell;

	//Arrange the vertices of the rectangle in order from bottom to top.
	int cornerIndex = 0;
	CellStruct corner[4] = {cor1Cell, cor2Cell, cor3Cell, cor4Cell};

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

	for (auto const& pCells : recCells)
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
	const int cellNums = (abs(topEndCell.Y - bottomStaCell.Y) + 1) * (abs(rightMidCell.X - leftMidCell.X) + 1);
	recCells.reserve(cellNums);
	recCells.push_back(bottomStaCell);

	if (bottomStaCell == leftMidCell || bottomStaCell == rightMidCell)
	{
		CellStruct middleCurCell = bottomStaCell;

		const CellStruct middleTheDist = topEndCell - bottomStaCell;
		const CellStruct middleTheUnit = { static_cast<short>(Math::sgn(middleTheDist.X)), static_cast<short>(Math::sgn(middleTheDist.Y)) };
		const CellStruct middleThePace = { static_cast<short>(middleTheDist.X * middleTheUnit.X), static_cast<short>(middleTheDist.Y * middleTheUnit.Y) };
		float mTheCurN = static_cast<float>((middleThePace.Y - middleThePace.X) / 2.0);

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
		CellStruct leftCurCell = bottomStaCell;
		CellStruct rightCurCell = bottomStaCell;
		CellStruct middleCurCell = bottomStaCell;

		bool leftNext = false;
		bool rightNext = false;
		bool leftSkip = false;
		bool rightSkip = false;
		bool leftContinue = false;
		bool rightContinue = false;

		const CellStruct left1stDist = leftMidCell - bottomStaCell;
		const CellStruct left1stUnit = { static_cast<short>(Math::sgn(left1stDist.X)), static_cast<short>(Math::sgn(left1stDist.Y)) };
		const CellStruct left1stPace = { static_cast<short>(left1stDist.X * left1stUnit.X), static_cast<short>(left1stDist.Y * left1stUnit.Y) };
		float left1stCurN = static_cast<float>((left1stPace.Y - left1stPace.X) / 2.0);

		const CellStruct left2ndDist = topEndCell - leftMidCell;
		const CellStruct left2ndUnit = { static_cast<short>(Math::sgn(left2ndDist.X)), static_cast<short>(Math::sgn(left2ndDist.Y)) };
		const CellStruct left2ndPace = { static_cast<short>(left2ndDist.X * left2ndUnit.X), static_cast<short>(left2ndDist.Y * left2ndUnit.Y) };
		float left2ndCurN = static_cast<float>((left2ndPace.Y - left2ndPace.X) / 2.0);

		const CellStruct right1stDist = rightMidCell - bottomStaCell;
		const CellStruct right1stUnit = { static_cast<short>(Math::sgn(right1stDist.X)), static_cast<short>(Math::sgn(right1stDist.Y)) };
		const CellStruct right1stPace = { static_cast<short>(right1stDist.X * right1stUnit.X), static_cast<short>(right1stDist.Y * right1stUnit.Y) };
		float right1stCurN = static_cast<float>((right1stPace.Y - right1stPace.X) / 2.0);

		const CellStruct right2ndDist = topEndCell - rightMidCell;
		const CellStruct right2ndUnit = { static_cast<short>(Math::sgn(right2ndDist.X)), static_cast<short>(Math::sgn(right2ndDist.Y)) };
		const CellStruct right2ndPace = { static_cast<short>(right2ndDist.X * right2ndUnit.X), static_cast<short>(right2ndDist.Y * right2ndUnit.Y) };
		float right2ndCurN = static_cast<float>((right2ndPace.Y - right2ndPace.X) / 2.0);

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

	if (this->EdgeAttenuation != 1.0)
	{
		const double damageMultiplier = this->GetExtraDamageMultiplier(pBullet, pTechno);
		const double calculatedDamage = self ? damage * damageMultiplier : damage * this->FirepowerMult * damageMultiplier;
		const int signal = Math::sgn(calculatedDamage);
		damage = static_cast<int>(calculatedDamage);

		if (damage == 0 && this->EdgeAttenuation != 0)
			damage = signal;
	}

	return damage;
}

double StraightTrajectory::GetExtraDamageMultiplier(BulletClass* pBullet, TechnoClass* pTechno)
{
	double distance = 0.0;
	double damageMult = 1.0;

	if (pTechno)
		distance = pTechno->GetCoords().DistanceFrom(pBullet->SourceCoords);
	else
		distance = pBullet->Location.DistanceFrom(pBullet->SourceCoords);

	if (this->AttenuationRange < static_cast<int>(distance))
		return this->EdgeAttenuation;

	if (distance > 256.0)
		damageMult += (this->EdgeAttenuation - 1.0) * ((distance - 256.0) / (static_cast<double>(this->AttenuationRange - 256)));

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

	if (const CellClass* const pCell = MapClass::Instance->GetCellAt(futureCoords))
	{
		int checkDifference = MapClass::Instance->GetCellFloorHeight(futureCoords) - futureCoords.Z;
		const CoordStruct cellCoords = pCell->GetCoordsWithBridge();
		const int differenceOnBridge = cellCoords.Z - futureCoords.Z;

		if (abs(differenceOnBridge) < abs(checkDifference))
			checkDifference = differenceOnBridge;

		if (abs(checkDifference) < 384 || !pBullet->Type->SubjectToCliffs)
		{
			pBullet->Velocity.Z += static_cast<double>(checkDifference + this->ConfineAtHeight);

			if (!this->PassDetonateLocal && this->CalculateBulletVelocity(pBullet, straightSpeed))
				return true;
		}
		else
		{
			return true;
		}
	}

	return false;
}
