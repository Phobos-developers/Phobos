#include "StraightTrajectory.h"
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Techno/Body.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);

	Stm
		.Process(this->DetonationDistance, false)
		.Process(this->TargetSnapDistance, false)
		.Process(this->PassThrough, false)
		.Process(this->PassDetonate, false)
		.Process(this->PassDetonateDamage, false)
		.Process(this->PassDetonateDelay, false)
		.Process(this->PassDetonateTimer, false)
		.Process(this->PassDetonateLocal, false)
		.Process(this->LeadTimeCalculate, false)
		.Process(this->OffsetCoord, false)
		.Process(this->RotateCoord, false)
		.Process(this->MirrorCoord, false)
		.Process(this->UseDisperseBurst, false)
		.Process(this->AxisOfRotation, false)
		.Process(this->ProximityImpact, false)
		.Process(this->ProximityDamage, false)
		.Process(this->ProximityRadius, false)
		.Process(this->ProximityAllies, false)
		.Process(this->ProximityFlight, false)
		.Process(this->ThroughVehicles, false)
		.Process(this->ThroughBuilding, false)
		.Process(this->SubjectToGround, false)
		.Process(this->ConfineAtHeight, false)
		.Process(this->EdgeAttenuation, false)
		;

	return true;
}

bool StraightTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);

	Stm
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
		.Process(this->PassThrough)
		.Process(this->PassDetonate)
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
		.Process(this->ProximityDamage)
		.Process(this->ProximityRadius)
		.Process(this->ProximityAllies)
		.Process(this->ProximityFlight)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->SubjectToGround)
		.Process(this->ConfineAtHeight)
		.Process(this->EdgeAttenuation)
		;

	return true;
}


void StraightTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Straight.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Straight.TargetSnapDistance");
	this->PassThrough.Read(exINI, pSection, "Trajectory.Straight.PassThrough");
	this->PassDetonate.Read(exINI, pSection, "Trajectory.Straight.PassDetonate");
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
	this->ProximityDamage.Read(exINI, pSection, "Trajectory.Straight.ProximityDamage");
	this->ProximityRadius.Read(exINI, pSection, "Trajectory.Straight.ProximityRadius");
	this->ProximityAllies.Read(exINI, pSection, "Trajectory.Straight.ProximityAllies");
	this->ProximityFlight.Read(exINI, pSection, "Trajectory.Straight.ProximityFlight");
	this->ThroughVehicles.Read(exINI, pSection, "Trajectory.Straight.ThroughVehicles");
	this->ThroughBuilding.Read(exINI, pSection, "Trajectory.Straight.ThroughBuilding");
	this->SubjectToGround.Read(exINI, pSection, "Trajectory.Straight.SubjectToGround");
	this->ConfineAtHeight.Read(exINI, pSection, "Trajectory.Straight.ConfineAtHeight");
	this->EdgeAttenuation.Read(exINI, pSection, "Trajectory.Straight.EdgeAttenuation");
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
		.Process(this->PassThrough)
		.Process(this->PassDetonate)
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
		.Process(this->ProximityDamage)
		.Process(this->ProximityRadius)
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

	return true;
}

bool StraightTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
		.Process(this->PassThrough)
		.Process(this->PassDetonate)
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
		.Process(this->ProximityDamage)
		.Process(this->ProximityRadius)
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

	return true;
}

void StraightTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto const pType = this->GetTrajectoryType<StraightTrajectoryType>(pBullet);

	this->DetonationDistance = pType->DetonationDistance;
	this->TargetSnapDistance = pType->TargetSnapDistance;
	this->PassThrough = pType->PassThrough;
	this->PassDetonate = pType->PassDetonate;
	this->PassDetonateDamage = pType->PassDetonateDamage;
	this->PassDetonateDelay = pType->PassDetonateDelay > 0 ? pType->PassDetonateDelay : 1;
	this->PassDetonateTimer.Start(pType->PassDetonateTimer);
	this->PassDetonateLocal = pType->PassDetonateLocal;
	this->LeadTimeCalculate = pType->LeadTimeCalculate;
	this->OffsetCoord = pType->OffsetCoord;
	this->RotateCoord = pType->RotateCoord;
	this->MirrorCoord = pType->MirrorCoord;
	this->UseDisperseBurst = pType->UseDisperseBurst;
	this->AxisOfRotation = pType->AxisOfRotation;
	this->ProximityImpact = pType->ProximityImpact;
	this->ProximityDamage = pType->ProximityDamage;
	this->ProximityRadius = pType->ProximityRadius;
	this->ProximityAllies = pType->ProximityAllies;
	this->ProximityFlight = pType->ProximityFlight;
	this->ThroughVehicles = pType->ThroughVehicles;
	this->ThroughBuilding = pType->ThroughBuilding;
	this->SubjectToGround = pType->SubjectToGround;
	this->ConfineAtHeight = pType->ConfineAtHeight;
	this->EdgeAttenuation = pType->EdgeAttenuation > 0.0 ? pType->EdgeAttenuation : 1.0;
	this->RemainingDistance = 1;
	this->ExtraCheck = nullptr;
	this->LastCasualty.reserve(1);
	this->FirepowerMult = 1.0;
	this->LastTargetCoord = pBullet->TargetCoords;
	this->CurrentBurst = 0;
	this->CountOfBurst = pBullet->WeaponType ? pBullet->WeaponType->Burst : 0;

	if (pBullet->Owner)
	{
		const CasualtyData TheOwner {pBullet->Owner, 20};
		this->LastCasualty.push_back(TheOwner);
		this->FirepowerMult = pBullet->Owner->FirepowerMultiplier;
		this->CurrentBurst = pBullet->Owner->CurrentBurstIndex;

		if (this->MirrorCoord && pBullet->Owner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	if (!this->LeadTimeCalculate || (pBullet->Target && pBullet->Target->WhatAmI() == AbstractType::Building))
		PrepareForOpenFire(pBullet);
	else
		this->WaitOneFrame.Start(1);
}

bool StraightTrajectory::OnAI(BulletClass* pBullet)
{
	if (this->WaitOneFrame.IsTicking() && BulletPrepareCheck(pBullet))
		return false;

	const double StraightSpeed = this->GetTrajectorySpeed(pBullet);
	HouseClass* const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

	if (BulletDetonatePreCheck(pBullet, pOwner, StraightSpeed))
		return true;

	if (this->PassDetonate)
		PassWithDetonateAt(pBullet, pOwner);

	if (this->ProximityImpact != 0 && this->ProximityRadius > 0)
		PrepareForDetonateAt(pBullet, pOwner);

	if (StraightSpeed < 256.0 && this->ConfineAtHeight > 0 && PassAndConfineAtHeight(pBullet, StraightSpeed))
		return true;

	BulletDetonateLastCheck(pBullet, pOwner, StraightSpeed);

	return false;
}

void StraightTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	HouseClass* const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

	if (pBullet->WeaponType && this->EdgeAttenuation != 1.0)
	{
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pBullet->Target);
		const int Damage = GetTheTrueDamage(pBullet->WeaponType->Damage, pBullet, pTechno, pOwner, true);
		pBullet->Construct(pBullet->Type, pBullet->Target, pBullet->Owner, Damage, pBullet->WH, pBullet->Speed, pBullet->Bright);
	}

	if (this->PassDetonateLocal)
	{
		CoordStruct DetonateCoords = pBullet->Location;
		DetonateCoords.Z = MapClass::Instance->GetCellFloorHeight(DetonateCoords);
		pBullet->SetLocation(DetonateCoords);
	}

	if (this->PassThrough || this->TargetSnapDistance <= 0)
		return;

	const ObjectClass* const pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	const CoordStruct pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= this->TargetSnapDistance)
	{
		auto const pExt = BulletExt::ExtMap.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
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
	double RotateAngle = 0.0;
	const double StraightSpeed = this->GetTrajectorySpeed(pBullet);
	ObjectClass* const pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	CoordStruct TheTargetCoords = pBullet->TargetCoords;
	CoordStruct TheSourceCoords = pBullet->SourceCoords;

	if (this->LeadTimeCalculate && pTarget)
	{
		TheTargetCoords = pTarget->GetCoords();
		TheSourceCoords = pBullet->Location;

		if (TheTargetCoords != this->LastTargetCoord)
		{
			const CoordStruct ExtraOffsetCoord = TheTargetCoords - this->LastTargetCoord;
			const CoordStruct TargetSourceCoord = TheSourceCoords - TheTargetCoords;
			const CoordStruct LastSourceCoord = TheSourceCoords - this->LastTargetCoord;

			const double TheDistanceSquared = TargetSourceCoord.MagnitudeSquared();
			const double TargetSpeedSquared = ExtraOffsetCoord.MagnitudeSquared();
			const double TargetSpeed = sqrt(TargetSpeedSquared);

			const double CrossFactor = LastSourceCoord.CrossProduct(TargetSourceCoord).MagnitudeSquared();
			const double VerticalDistanceSquared = CrossFactor / TargetSpeedSquared;

			const double HorizonDistanceSquared = TheDistanceSquared - VerticalDistanceSquared;
			const double HorizonDistance = sqrt(HorizonDistanceSquared);

			const double StraightSpeedSquared = StraightSpeed * StraightSpeed;
			const double BaseFactor = StraightSpeedSquared - TargetSpeedSquared;
			const double SquareFactor = BaseFactor * VerticalDistanceSquared + StraightSpeedSquared * HorizonDistanceSquared;

			if (SquareFactor > 0)
			{
				const double MinusFactor = -(HorizonDistance * TargetSpeed);
				int TravelTime = 0;

				if (BaseFactor == 0)
				{
					TravelTime = (HorizonDistance != 0) ? (static_cast<int>(TheDistanceSquared / (2 * HorizonDistance * TargetSpeed)) + 1) : 0;
				}
				else
				{
					const int TravelTimeM = static_cast<int>((MinusFactor - sqrt(SquareFactor)) / BaseFactor);
					const int TravelTimeP = static_cast<int>((MinusFactor + sqrt(SquareFactor)) / BaseFactor);

					if (TravelTimeM > 0 && TravelTimeP > 0)
						TravelTime = TravelTimeM < TravelTimeP ? TravelTimeM : TravelTimeP;
					else if (TravelTimeM > 0)
						TravelTime = TravelTimeM;
					else if (TravelTimeP > 0)
						TravelTime = TravelTimeP;

					if (TargetSourceCoord.MagnitudeSquared() < LastSourceCoord.MagnitudeSquared())
						TravelTime += 1;
					else
						TravelTime += 2;
				}

				TheTargetCoords += ExtraOffsetCoord * TravelTime;
			}
		}
	}

	if (!this->LeadTimeCalculate && TheTargetCoords == TheSourceCoords && pBullet->Owner)
	{
		const CoordStruct TheOwnerCoords = pBullet->Owner->GetCoords();
		RotateAngle = Math::atan2(TheTargetCoords.Y - TheOwnerCoords.Y , TheTargetCoords.X - TheOwnerCoords.X);
	}
	else
	{
		RotateAngle = Math::atan2(TheTargetCoords.Y - TheSourceCoords.Y , TheTargetCoords.X - TheSourceCoords.X);
	}

	if (this->OffsetCoord.X != 0 || this->OffsetCoord.Y != 0 || this->OffsetCoord.Z != 0)
	{
		TheTargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(RotateAngle) + this->OffsetCoord.Y * Math::sin(RotateAngle));
		TheTargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(RotateAngle) - this->OffsetCoord.Y * Math::cos(RotateAngle));
		TheTargetCoords.Z += this->OffsetCoord.Z;
	}

	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		const double OffsetMult = 0.0004 * TheSourceCoords.DistanceFrom(TheTargetCoords);
		const int OffsetMin = static_cast<int>(OffsetMult * pTypeExt->BallisticScatter_Min.Get(Leptons(0)));
		const int OffsetMax = static_cast<int>(OffsetMult * pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter)));
		const int OffsetDistance = ScenarioClass::Instance->Random.RandomRanged(OffsetMin, OffsetMax);
		TheTargetCoords = MapClass::GetRandomCoordsNear(TheTargetCoords, OffsetDistance, false);
	}

	if (this->PassThrough)
	{
		if (this->DetonationDistance > 0)
			this->RemainingDistance = static_cast<int>(this->DetonationDistance + StraightSpeed);
		else if (this->DetonationDistance < 0)
			this->RemainingDistance = static_cast<int>(TheSourceCoords.DistanceFrom(TheTargetCoords) - this->DetonationDistance + StraightSpeed);
		else
			this->RemainingDistance = INT_MAX;
	}
	else
	{
		this->RemainingDistance = static_cast<int>(TheSourceCoords.DistanceFrom(TheTargetCoords) + StraightSpeed);
	}

	pBullet->TargetCoords = TheTargetCoords;

	pBullet->Velocity.X = static_cast<double>(TheTargetCoords.X - TheSourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(TheTargetCoords.Y - TheSourceCoords.Y);

	if (this->ConfineAtHeight > 0 && this->PassDetonateLocal)
		pBullet->Velocity.Z = 0;
	else
		pBullet->Velocity.Z = static_cast<double>(this->GetVelocityZ(pBullet));

	if (!this->UseDisperseBurst && this->RotateCoord != 0 && this->CountOfBurst > 1)
	{
		BulletVelocity RotationAxis
		{
			this->AxisOfRotation.X * Math::cos(RotateAngle) + this->AxisOfRotation.Y * Math::sin(RotateAngle),
			this->AxisOfRotation.X * Math::sin(RotateAngle) - this->AxisOfRotation.Y * Math::cos(RotateAngle),
			static_cast<double>(this->AxisOfRotation.Z)
		};

		const double RotationAxisLengthSquared = RotationAxis.MagnitudeSquared();

		if (RotationAxisLengthSquared != 0)
		{
			double ExtraRotate = 0;
			RotationAxis *= 1 / sqrt(RotationAxisLengthSquared);

			if (this->MirrorCoord)
			{
				if (pBullet->Owner && pBullet->Owner->CurrentBurstIndex % 2 == 1)
					RotationAxis *= -1;

				ExtraRotate = Math::Pi * (this->RotateCoord * ((this->CurrentBurst / 2) / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}
			else
			{
				ExtraRotate = Math::Pi * (this->RotateCoord * (this->CurrentBurst / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}

			const double CosRotate = Math::cos(ExtraRotate);
			pBullet->Velocity = (pBullet->Velocity * CosRotate) + (RotationAxis * ((1 - CosRotate) * (pBullet->Velocity * RotationAxis))) + (RotationAxis.CrossProduct(pBullet->Velocity) * Math::sin(ExtraRotate));
		}
	}

	if (CalculateBulletVelocity(pBullet, StraightSpeed))
		this->RemainingDistance = 0;
}

int StraightTrajectory::GetVelocityZ(BulletClass* pBullet)
{
	int BulletVelocity = static_cast<int>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);

	if (!this->PassThrough)
		return BulletVelocity;

	if (pBullet->Owner && abs(pBullet->Owner->GetCoords().Z - pBullet->TargetCoords.Z) <= 32)
	{
		const double DistanceOfTwo = pBullet->SourceCoords.DistanceFrom(pBullet->TargetCoords);
		double TheDistance = this->DetonationDistance;

		if (this->DetonationDistance == 0)
			return 0;

		if (this->DetonationDistance < 0)
			TheDistance = DistanceOfTwo - this->DetonationDistance;

		if (TheDistance != 0)
			BulletVelocity = static_cast<int>(BulletVelocity * (DistanceOfTwo / TheDistance));
		else
			return 0;
	}

	return BulletVelocity;
}

bool StraightTrajectory::CalculateBulletVelocity(BulletClass* pBullet, double StraightSpeed)
{
	const double VelocityLength = pBullet->Velocity.Magnitude();

	if (VelocityLength > 0)
		pBullet->Velocity *= StraightSpeed / VelocityLength;
	else
		return true;

	return false;
}

bool StraightTrajectory::BulletPrepareCheck(BulletClass* pBullet)
{
	if (this->WaitOneFrame.HasTimeLeft())
		return true;

	PrepareForOpenFire(pBullet);
	this->WaitOneFrame.Stop();

	return false;
}

bool StraightTrajectory::BulletDetonatePreCheck(BulletClass* pBullet, HouseClass* pOwner, double StraightSpeed)
{
	if (this->ExtraCheck)
	{
		pBullet->SetTarget(this->ExtraCheck);
		pBullet->TargetCoords = this->ExtraCheck->GetCoords();

		return true;
	}

	this->RemainingDistance -= static_cast<int>(StraightSpeed);

	if (this->RemainingDistance < 0)
		return true;

	if (!this->PassThrough && this->DetonationDistance > 0 && pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	if (this->SubjectToGround && MapClass::Instance->GetCellFloorHeight(pBullet->Location) >= pBullet->Location.Z)
		return true;

	if (CellClass* const pCell = MapClass::Instance->TryGetCellAt(pBullet->Location))
		return false;
	else
		return true;
}

//If the check result here is true, it only needs to be detonated in the next frame, without returning.
void StraightTrajectory::BulletDetonateLastCheck(BulletClass* pBullet, HouseClass* pOwner, double StraightSpeed)
{
	bool VelocityCheck = false;
	double LocationDistance = this->RemainingDistance;

	if (LocationDistance < StraightSpeed)
		VelocityCheck = true;

	if (this->ExtraCheck)
	{
		LocationDistance = this->ExtraCheck->GetCoords().DistanceFrom(pBullet->Location);
		VelocityCheck = true;
	}

	const bool CheckThrough = (!this->ThroughBuilding || !this->ThroughVehicles);
	const bool CheckSubject = (this->SubjectToGround || pBullet->Type->SubjectToWalls);
	const bool LowSpeedMode = (StraightSpeed < 256.0);

	if (CheckThrough || CheckSubject)
	{
		if (LowSpeedMode && CheckThrough) //LowSpeedMode with CheckSubject was already done well.
		{
			if (CellClass* const pCell = MapClass::Instance->GetCellAt(pBullet->Location))
			{
				if (CheckThroughAndSubjectInCell(pBullet, pCell, 0, LocationDistance, pOwner))
				{
					LocationDistance = 0;
					VelocityCheck = true;
				}
			}
		}
		else if (!LowSpeedMode)
		{
			std::vector<CellClass*> StrCellClass = GetCellsInPassThrough(pBullet);

			for (auto const& pStrCell : StrCellClass)
			{
				const double CellDistance = pStrCell->GetCoords().DistanceFrom(pBullet->Location);
				const double BulletHeight = pBullet->Location.Z + CellDistance / StraightSpeed * pBullet->Velocity.Z;

				if (this->SubjectToGround && CellDistance < LocationDistance && MapClass::Instance->GetCellFloorHeight(pStrCell->GetCoords()) >= BulletHeight)
				{
					LocationDistance = CellDistance;
					VelocityCheck = true;
				}

				if (pBullet->Type->SubjectToWalls && CellDistance < LocationDistance && pStrCell->OverlayTypeIndex != -1 && OverlayTypeClass::Array->GetItem(pStrCell->OverlayTypeIndex)->Wall)
				{
					LocationDistance = CellDistance;
					VelocityCheck = true;
				}

				if (!CheckThrough)
					continue;

				if (CheckThroughAndSubjectInCell(pBullet, pStrCell, CellDistance, LocationDistance, pOwner))
				{
					LocationDistance = CellDistance;
					VelocityCheck = true;
				}
			}
		}
	}

	if (VelocityCheck)
	{
		this->RemainingDistance = 0;
		pBullet->Velocity *= LocationDistance / StraightSpeed;
	}
}

bool StraightTrajectory::CheckThroughAndSubjectInCell(BulletClass* pBullet, CellClass* pCell,
	double CellDistance, double ThisDistance, HouseClass* pOwner)
{
	ObjectClass* pObject = pCell->FirstObject;
	TechnoClass* pNearest = nullptr;

	while (pObject)
	{
		TechnoClass* const pTechno = abstract_cast<TechnoClass*>(pObject);
		pObject = pObject->NextObject;

		if (!pTechno || pOwner->IsAlliedWith(pTechno->Owner))
			continue;

		const AbstractType TechnoType = pTechno->WhatAmI();

		if (TechnoType == AbstractType::Building)
		{
			BuildingClass* pBuilding = static_cast<BuildingClass*>(pTechno);

			if (pBuilding->Type->InvisibleInGame)
				continue;

			if (CellDistance < ThisDistance && (pBuilding->Type->IsVehicle() ? !this->ThroughVehicles : !this->ThroughBuilding))
			{
				ThisDistance = CellDistance;
				pNearest = pTechno;
			}
		}

		if (CellDistance < ThisDistance && !this->ThroughVehicles && (TechnoType == AbstractType::Unit || TechnoType == AbstractType::Aircraft))
		{
			ThisDistance = CellDistance;
			pNearest = pTechno;
		}
	}

	if (pNearest && this->ProximityImpact != 0)
	{
		auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		WarheadTypeClass* pWH = pBulletTypeExt->Straight_ProximityWarhead;

		if (!pWH)
			return static_cast<bool>(pNearest);

		const int Damage = GetTheTrueDamage(this->ProximityDamage, pBullet, pNearest, pOwner, false);
		WarheadTypeExt::DetonateAt(pWH, pNearest->GetCoords(), pBullet->Owner, Damage, pOwner);
	}

	return static_cast<bool>(pNearest);
}

void StraightTrajectory::PassWithDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->PassDetonateTimer.Completed())
	{
		this->PassDetonateTimer.Start(this->PassDetonateDelay);
		CoordStruct DetonateCoords = pBullet->Location;

		if (this->PassDetonateLocal)
			DetonateCoords.Z = MapClass::Instance->GetCellFloorHeight(DetonateCoords);

		auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		WarheadTypeClass* pWH = pBulletTypeExt->Straight_PassDetonateWarhead;

		if (!pWH)
			return;

		const int Damage = GetTheTrueDamage(this->PassDetonateDamage, pBullet, nullptr, nullptr, false);
		WarheadTypeExt::DetonateAt(pWH, DetonateCoords, pBullet->Owner, Damage, pOwner);
	}
}

//Select suitable targets and choose the closer targets then attack each target only once.
void StraightTrajectory::PrepareForDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
	//Step 1: Find valid targets on the ground within range.
	std::vector<CellClass*> RecCellClass = GetCellsInProximityRadius(pBullet);
	const size_t CellSize = RecCellClass.size() * 2;
	size_t VectSize = CellSize;
	size_t ThisSize = 0;

	const int CheckHeight = this->ProximityFlight ? 0 : 200;
	const CoordStruct VelocityCrd
	{
		static_cast<int>(pBullet->Velocity.X),
		static_cast<int>(pBullet->Velocity.Y),
		static_cast<int>(pBullet->Velocity.Z)
	};

	std::vector<TechnoClass*> ValidTechnos;
	ValidTechnos.reserve(VectSize);
	const TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target);

	for (auto const& pRecCell : RecCellClass)
	{
		ObjectClass* pObject = pRecCell->FirstObject;

		while (pObject)
		{
			TechnoClass* const pTechno = abstract_cast<TechnoClass*>(pObject);
			pObject = pObject->NextObject;

			if (!pTechno || pTechno->GetHeight() > CheckHeight)
				continue;

			const AbstractType TechnoType = pTechno->WhatAmI();

			if (TechnoType == AbstractType::Building)
			{
				BuildingClass* const pBuilding = static_cast<BuildingClass*>(pTechno);

				if (pBuilding->Type->InvisibleInGame || (pBuilding->Type->IsVehicle() ? !this->ThroughVehicles : !this->ThroughBuilding))
					continue;
			}

			if (!this->ThroughVehicles && (TechnoType == AbstractType::Unit || TechnoType == AbstractType::Aircraft))
				continue;

			if (this->ProximityAllies == 0 && pOwner->IsAlliedWith(pTechno->Owner) && !(pTargetTechno && pTechno == pTargetTechno))
				continue;

			const CoordStruct DistanceCrd = pTechno->GetCoords() - pBullet->SourceCoords;
			const CoordStruct LocationCrd = (VelocityCrd + (pBullet->Location - pBullet->SourceCoords));
			const CoordStruct TerminalCrd = DistanceCrd - LocationCrd;
			double Distance = LocationCrd.MagnitudeSquared(); //Not true distance yet.

			if (DistanceCrd * VelocityCrd < 0 || TerminalCrd * VelocityCrd > 0)
				continue;

			Distance = (Distance > 0) ? sqrt(DistanceCrd.CrossProduct(TerminalCrd).MagnitudeSquared() / Distance) : DistanceCrd.Magnitude();

			if (TechnoType != AbstractType::Building && Distance > this->ProximityRadius)
				continue;

			if (ThisSize < VectSize)
			{
				ValidTechnos.push_back(pTechno);
			}
			else
			{
				std::vector<TechnoClass*> ValidTechnosBuffer;
				VectSize += CellSize;
				ValidTechnosBuffer.reserve(VectSize);

				for (auto const& pTechnoBuffer : ValidTechnos)
					ValidTechnosBuffer.push_back(pTechnoBuffer);

				ValidTechnos = ValidTechnosBuffer;
				ValidTechnos.push_back(pTechno);
			}

			ThisSize += 1;
		}
	}

	//Step 2: Find valid targets in the air within range if necessary.
	if (this->ProximityFlight)
	{
		for (auto const& pTechno : *TechnoClass::Array)
		{
			if (pTechno->GetHeight() <= 0)
				continue;

			const AbstractType TechnoType = pTechno->WhatAmI();

			if (!this->ThroughVehicles && (TechnoType == AbstractType::Unit || TechnoType == AbstractType::Aircraft))
				continue;

			const CoordStruct DistanceCrd = pTechno->GetCoords() - pBullet->Location;
			const CoordStruct TerminalCrd = DistanceCrd - VelocityCrd;
			double Distance = VelocityCrd.MagnitudeSquared(); //Not true distance yet.

			if (DistanceCrd * VelocityCrd < 0 || TerminalCrd * VelocityCrd > 0)
				continue;

			Distance = (Distance > 0) ? sqrt(DistanceCrd.CrossProduct(TerminalCrd).MagnitudeSquared() / Distance) : DistanceCrd.Magnitude();

			if (Distance > this->ProximityRadius)
				continue;

			if (ThisSize < VectSize)
			{
				ValidTechnos.push_back(pTechno);
			}
			else
			{
				std::vector<TechnoClass*> ValidTechnosBuffer;
				VectSize += CellSize;
				ValidTechnosBuffer.reserve(VectSize);

				for (auto const& pTechnoBuffer : ValidTechnos)
					ValidTechnosBuffer.push_back(pTechnoBuffer);

				ValidTechnos = ValidTechnosBuffer;
				ValidTechnos.push_back(pTechno);
			}

			ThisSize += 1;
		}
	}

	//Step 3: Record each target without repetition.
	const size_t iMax = this->LastCasualty.size();
	const size_t jMax = ValidTechnos.size();
	const size_t Capacity = iMax + jMax;

	std::sort(&ValidTechnos[0], &ValidTechnos[jMax]);
	std::vector<CasualtyData> Casualty;
	Casualty.reserve(Capacity);
	std::vector<TechnoClass*> CasualtyChecked;
	CasualtyChecked.reserve(Capacity);

	size_t i = 0;
	size_t j = 0;
	TechnoClass* pThis = nullptr;
	TechnoClass* pLast = nullptr;
	int ThisTime = 0;
	bool Check = false;

	for (size_t k = 0; k < Capacity; k++) //Merge
	{
		if (i < iMax && j < jMax)
		{
			if (this->LastCasualty[i].pCasualty < ValidTechnos[j])
			{
				Check = false;
				pThis = this->LastCasualty[i].pCasualty;
				ThisTime = this->LastCasualty[i].RemainTime;
				i += 1;
			}
			else if (this->LastCasualty[i].pCasualty > ValidTechnos[j])
			{
				Check = true;
				pThis = ValidTechnos[j];
				ThisTime = 20;
				j += 1;
			}
			else
			{
				Check = false;
				pThis = this->LastCasualty[i].pCasualty;
				ThisTime = 20;
				i += 1;
				j += 1;
			}
		}
		else if (i < iMax)
		{
			Check = false;
			pThis = this->LastCasualty[i].pCasualty;
			ThisTime = this->LastCasualty[i].RemainTime;
			i += 1;
		}
		else if (j < jMax)
		{
			Check = true;
			pThis = ValidTechnos[j];
			ThisTime = 20;
			j += 1;
		}
		else
		{
			break;
		}

		if (pThis && pThis != pLast)
		{
			if (Check)
				CasualtyChecked.push_back(pThis);

			if (--ThisTime > 0)
			{
				const CasualtyData ThisCasualty {pThis, ThisTime};
				Casualty.push_back(ThisCasualty);
			}

			pLast = pThis;
		}
	}

	this->LastCasualty = Casualty;

	//Step 4: Detonate warheads in sequence based on distance.
	const size_t CasualtySize = CasualtyChecked.size();

	if (this->ProximityImpact > 0 && static_cast<int>(CasualtySize) > this->ProximityImpact)
	{
		std::sort(&CasualtyChecked[0], &CasualtyChecked[CasualtySize],[pBullet](TechnoClass* pTechnoA, TechnoClass* pTechnoB){
			return pTechnoA->GetCoords().DistanceFromSquared(pBullet->SourceCoords) < pTechnoB->GetCoords().DistanceFromSquared(pBullet->SourceCoords);
		});
	}

	auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
	WarheadTypeClass* pWH = pBulletTypeExt->Straight_ProximityWarhead;

	if (!pWH)
		return;

	for (auto const& pTechno : CasualtyChecked)
	{
		const int Damage = GetTheTrueDamage(this->ProximityDamage, pBullet, pTechno, pOwner, false);
		WarheadTypeExt::DetonateAt(pWH, pTechno->GetCoords(), pBullet->Owner, Damage, pOwner);

		if (this->ProximityImpact == 1)
		{
			this->ExtraCheck = pTechno;
			this->ProximityImpact = 0;
			break;
		}
		else if (this->ProximityImpact > 0)
		{
			this->ProximityImpact--;
		}
	}
}

std::vector<CellClass*> StraightTrajectory::GetCellsInPassThrough(BulletClass* pBullet)
{
	std::vector<CellClass*> StaCellClass;

	if (pBullet->Velocity.X != 0 || pBullet->Velocity.Y != 0)
	{
		const CoordStruct WalkCoord { static_cast<int>(pBullet->Velocity.X), static_cast<int>(pBullet->Velocity.Y), 0 };
		const CellStruct ThisCell = CellClass::Coord2Cell(pBullet->Location);
		const CellStruct NextCell = CellClass::Coord2Cell((pBullet->Location + WalkCoord));

		std::vector<CellStruct> StaCells = GetCellsInRectangle(ThisCell, ThisCell, NextCell, NextCell);
		StaCellClass.reserve(StaCells.size());

		for (auto const& pCells : StaCells)
		{
			if (CellClass* const pStaCell = MapClass::Instance->TryGetCellAt(pCells))
				StaCellClass.push_back(pStaCell);
		}
	}

	return StaCellClass;
}

//A rectangular shape with a custom width from the current frame to the next frame in length.
std::vector<CellClass*> StraightTrajectory::GetCellsInProximityRadius(BulletClass* pBullet)
{
	//Seems like the y-axis is reversed, but it's okay.
	const CoordStruct WalkCoord { static_cast<int>(pBullet->Velocity.X), static_cast<int>(pBullet->Velocity.Y), 0 };
	const double SideMult = this->ProximityRadius / WalkCoord.Magnitude();

	const CoordStruct Cor1Coord { static_cast<int>(WalkCoord.Y * SideMult), static_cast<int>((-WalkCoord.X) * SideMult), 0 };
	const CoordStruct Cor4Coord { static_cast<int>((-WalkCoord.Y) * SideMult), static_cast<int>(WalkCoord.X * SideMult), 0 };
	const CellStruct ThisCell = CellClass::Coord2Cell(pBullet->Location);

	CellStruct Cor1Cell = CellClass::Coord2Cell((pBullet->Location + Cor1Coord));
	CellStruct Cor4Cell = CellClass::Coord2Cell((pBullet->Location + Cor4Coord));

	const CellStruct Off1Cell = Cor1Cell - ThisCell;
	const CellStruct Off4Cell = Cor4Cell - ThisCell;
	const CellStruct NextCell = CellClass::Coord2Cell((pBullet->Location + WalkCoord));

	CellStruct Cor2Cell = NextCell + Off1Cell;
	CellStruct Cor3Cell = NextCell + Off4Cell;

	//Arrange the vertices of the rectangle in order from bottom to top.
	int CornerIndex = 0;
	CellStruct Corner[4] = {Cor1Cell, Cor2Cell, Cor3Cell, Cor4Cell};

	for (int i = 1; i < 4; i++)
	{
		if (Corner[CornerIndex].Y > Corner[i].Y)
			CornerIndex = i;
	}

	Cor1Cell = Corner[CornerIndex++];
	CornerIndex %= 4;
	Cor2Cell = Corner[CornerIndex++];
	CornerIndex %= 4;
	Cor3Cell = Corner[CornerIndex++];
	CornerIndex %= 4;
	Cor4Cell = Corner[CornerIndex];

	std::vector<CellStruct> RecCells = GetCellsInRectangle(Cor1Cell, Cor4Cell, Cor2Cell, Cor3Cell);
	std::vector<CellClass*> RecCellClass;
	RecCellClass.reserve(RecCells.size());

	for (auto const& pCells : RecCells)
	{
		if (CellClass* pRecCell = MapClass::Instance->TryGetCellAt(pCells))
			RecCellClass.push_back(pRecCell);
	}

	return RecCellClass;
}

//Record cells in the order of "draw left boundary, draw right boundary, fill middle, and move up one level".
std::vector<CellStruct> StraightTrajectory::GetCellsInRectangle(CellStruct bStaCell, CellStruct lMidCell, CellStruct rMidCell, CellStruct tEndCell)
{
	std::vector<CellStruct> RecCells;
	const int CellNums = (abs(tEndCell.Y - bStaCell.Y) + 1) * (abs(rMidCell.X - lMidCell.X) + 1);
	RecCells.reserve(CellNums);
	RecCells.push_back(bStaCell);

	if (bStaCell == lMidCell || bStaCell == rMidCell)
	{
		CellStruct mCurCell = bStaCell;

		const CellStruct mTheDist = tEndCell - bStaCell;
		const CellStruct mTheUnit = { static_cast<short>(Math::sgn(mTheDist.X)), static_cast<short>(Math::sgn(mTheDist.Y)) };
		const CellStruct mThePace = { static_cast<short>(mTheDist.X * mTheUnit.X), static_cast<short>(mTheDist.Y * mTheUnit.Y) };
		float mTheCurN = static_cast<float>((mThePace.Y - mThePace.X) / 2.0);

		while (mCurCell != tEndCell)
		{
			if (mTheCurN > 0)
			{
				mTheCurN -= mThePace.X;
				mCurCell.Y += mTheUnit.Y;
				RecCells.push_back(mCurCell);
			}
			else if (mTheCurN < 0)
			{
				mTheCurN += mThePace.Y;
				mCurCell.X += mTheUnit.X;
				RecCells.push_back(mCurCell);
			}
			else
			{
				mTheCurN += mThePace.Y - mThePace.X;
				mCurCell.X += mTheUnit.X;
				RecCells.push_back(mCurCell);
				mCurCell.X -= mTheUnit.X;
				mCurCell.Y += mTheUnit.Y;
				RecCells.push_back(mCurCell);
				mCurCell.X += mTheUnit.X;
				RecCells.push_back(mCurCell);
			}
		}
	}
	else
	{
		CellStruct lCurCell = bStaCell;
		CellStruct rCurCell = bStaCell;
		CellStruct mCurCell = bStaCell;

		bool lNext = false;
		bool rNext = false;
		bool lSkip = false;
		bool rSkip = false;
		bool lContinue = false;
		bool rContinue = false;

		const CellStruct l1stDist = lMidCell - bStaCell;
		const CellStruct l1stUnit = { static_cast<short>(Math::sgn(l1stDist.X)), static_cast<short>(Math::sgn(l1stDist.Y)) };
		const CellStruct l1stPace = { static_cast<short>(l1stDist.X * l1stUnit.X), static_cast<short>(l1stDist.Y * l1stUnit.Y) };
		float l1stCurN = static_cast<float>((l1stPace.Y - l1stPace.X) / 2.0);

		const CellStruct l2ndDist = tEndCell - lMidCell;
		const CellStruct l2ndUnit = { static_cast<short>(Math::sgn(l2ndDist.X)), static_cast<short>(Math::sgn(l2ndDist.Y)) };
		const CellStruct l2ndPace = { static_cast<short>(l2ndDist.X * l2ndUnit.X), static_cast<short>(l2ndDist.Y * l2ndUnit.Y) };
		float l2ndCurN = static_cast<float>((l2ndPace.Y - l2ndPace.X) / 2.0);

		const CellStruct r1stDist = rMidCell - bStaCell;
		const CellStruct r1stUnit = { static_cast<short>(Math::sgn(r1stDist.X)), static_cast<short>(Math::sgn(r1stDist.Y)) };
		const CellStruct r1stPace = { static_cast<short>(r1stDist.X * r1stUnit.X), static_cast<short>(r1stDist.Y * r1stUnit.Y) };
		float r1stCurN = static_cast<float>((r1stPace.Y - r1stPace.X) / 2.0);

		const CellStruct r2ndDist = tEndCell - rMidCell;
		const CellStruct r2ndUnit = { static_cast<short>(Math::sgn(r2ndDist.X)), static_cast<short>(Math::sgn(r2ndDist.Y)) };
		const CellStruct r2ndPace = { static_cast<short>(r2ndDist.X * r2ndUnit.X), static_cast<short>(r2ndDist.Y * r2ndUnit.Y) };
		float r2ndCurN = static_cast<float>((r2ndPace.Y - r2ndPace.X) / 2.0);

		while (lCurCell != tEndCell || rCurCell != tEndCell)
		{
			while (lCurCell != tEndCell) //Left
			{
				if (!lNext) //Bottom Left Side
				{
					if (l1stCurN > 0)
					{
						l1stCurN -= l1stPace.X;
						lCurCell.Y += l1stUnit.Y;

						if (lCurCell == lMidCell)
						{
							lNext = true;
						}
						else
						{
							RecCells.push_back(lCurCell);
							break;
						}
					}
					else
					{
						l1stCurN += l1stPace.Y;
						lCurCell.X += l1stUnit.X;

						if (lCurCell == lMidCell)
						{
							lNext = true;
							lSkip = true;
						}
					}
				}
				else //Top Left Side
				{
					if (l2ndCurN >= 0)
					{
						if (lSkip)
						{
							lSkip = false;
							l2ndCurN -= l2ndPace.X;
							lCurCell.Y += l2ndUnit.Y;
						}
						else
						{
							lContinue = true;
							break;
						}
					}
					else
					{
						l2ndCurN += l2ndPace.Y;
						lCurCell.X += l2ndUnit.X;
					}
				}

				if (lCurCell != rCurCell) //Avoid double counting cells.
					RecCells.push_back(lCurCell);
			}

			while (rCurCell != tEndCell) //Right
			{
				if (!rNext) //Bottom Right Side
				{
					if (r1stCurN > 0)
					{
						r1stCurN -= r1stPace.X;
						rCurCell.Y += r1stUnit.Y;

						if (rCurCell == rMidCell)
						{
							rNext = true;
						}
						else
						{
							RecCells.push_back(rCurCell);
							break;
						}
					}
					else
					{
						r1stCurN += r1stPace.Y;
						rCurCell.X += r1stUnit.X;

						if (rCurCell == rMidCell)
						{
							rNext = true;
							rSkip = true;
						}
					}
				}
				else //Top Right Side
				{
					if (r2ndCurN >= 0)
					{
						if (rSkip)
						{
							rSkip = false;
							r2ndCurN -= r2ndPace.X;
							rCurCell.Y += r2ndUnit.Y;
						}
						else
						{
							rContinue = true;
							break;
						}
					}
					else
					{
						r2ndCurN += r2ndPace.Y;
						rCurCell.X += r2ndUnit.X;
					}
				}

				if (rCurCell != lCurCell) //Avoid double counting cells.
					RecCells.push_back(rCurCell);
			}

			mCurCell = lCurCell;
			mCurCell.X += 1;

			while (mCurCell.X < rCurCell.X) //Center
			{
				RecCells.push_back(mCurCell);
				mCurCell.X += 1;
			}

			if (lContinue) //Continue Top Left Side
			{
				lContinue = false;
				l2ndCurN -= l2ndPace.X;
				lCurCell.Y += l2ndUnit.Y;
				RecCells.push_back(lCurCell);
			}

			if (rContinue) //Continue Top Right Side
			{
				rContinue = false;
				r2ndCurN -= r2ndPace.X;
				rCurCell.Y += r2ndUnit.Y;
				RecCells.push_back(rCurCell);
			}
		}
	}

	return RecCells;
}

int StraightTrajectory::GetTheTrueDamage(int Damage, BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner, bool Self)
{
	if (Damage == 0)
		return 0;

	int TrueDamage = Damage;

	if (this->EdgeAttenuation != 1.0 || this->ProximityAllies != 1.0)
	{
		const double CalculatedDamage = Damage * this->FirepowerMult * GetExtraDamageMultiplier(pBullet, pTechno, pOwner, Self);
		TrueDamage = static_cast<int>(CalculatedDamage + 0.5);

		if (TrueDamage == 0 && Damage != 0)
			TrueDamage = Math::sgn(CalculatedDamage);
	}

	return TrueDamage;
}

double StraightTrajectory::GetExtraDamageMultiplier(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner, bool Self)
{
	bool CheckAllies = false;
	double Distance = 0;
	double DamageMult = 1.0;
	const double MaxDistance = pBullet->WeaponType ? static_cast<double>(pBullet->WeaponType->Range) : 0;

	if (pTechno)
	{
		Distance = pTechno->GetCoords().DistanceFrom(pBullet->SourceCoords);

		if (pOwner && pOwner->IsAlliedWith(pTechno->Owner) && this->ProximityAllies != 0)
			CheckAllies = true;
	}
	else
	{
		Distance = pBullet->Location.DistanceFrom(pBullet->SourceCoords);
	}

	if (MaxDistance < Distance)
		return this->EdgeAttenuation;

	if (Distance > 256.0)
		DamageMult += (this->EdgeAttenuation - 1.0) * ((Distance - 256.0) / (MaxDistance - 256.0));

	if (!Self && CheckAllies)
		DamageMult *= this->ProximityAllies;

	return DamageMult;
}

bool StraightTrajectory::PassAndConfineAtHeight(BulletClass* pBullet, double StraightSpeed)
{
	const CoordStruct FutureCoords
	{
		pBullet->Location.X + static_cast<int>(pBullet->Velocity.X),
		pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y),
		pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z)
	};

	if (CellClass* const pCell = MapClass::Instance->GetCellAt(FutureCoords))
	{
		int CheckDifference = MapClass::Instance->GetCellFloorHeight(FutureCoords) - FutureCoords.Z;
		const CoordStruct CellCoords = pCell->GetCoordsWithBridge();
		const int DifferenceOnBridge = CellCoords.Z - FutureCoords.Z;

		if (abs(DifferenceOnBridge) < abs(CheckDifference))
			CheckDifference = DifferenceOnBridge;

		if (abs(CheckDifference) < 384 || !pBullet->Type->SubjectToCliffs)
		{
			pBullet->Velocity.Z += static_cast<double>(CheckDifference + this->ConfineAtHeight);

			if (!this->PassDetonateLocal && CalculateBulletVelocity(pBullet, StraightSpeed))
				return true;
		}
		else
		{
			return true;
		}
	}

	return false;
}
