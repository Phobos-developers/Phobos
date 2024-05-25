#include "StraightTrajectory.h"
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Techno/Body.h>
#include <ScenarioClass.h>

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);

	Stm
		.Process(this->DetonationDistance, false)
		.Process(this->TargetSnapDistance, false)
		.Process(this->PassThrough, false)
		.Process(this->PassDetonate, false)
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
		.Process(this->ProximityRadius, false)
		.Process(this->ProximityAllies, false)
		.Process(this->ThroughVehicles, false)
		.Process(this->ThroughBuilding, false)
		.Process(this->StraightWarhead, false)
		.Process(this->StraightDamage, false)
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
		.Process(this->ProximityRadius)
		.Process(this->ProximityAllies)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->StraightWarhead)
		.Process(this->StraightDamage)
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
	this->ProximityRadius.Read(exINI, pSection, "Trajectory.Straight.ProximityRadius");
	this->ProximityAllies.Read(exINI, pSection, "Trajectory.Straight.ProximityAllies");
	this->ThroughVehicles.Read(exINI, pSection, "Trajectory.Straight.ThroughVehicles");
	this->ThroughBuilding.Read(exINI, pSection, "Trajectory.Straight.ThroughBuilding");
	this->StraightWarhead.Read(exINI, pSection, "Trajectory.Straight.Warhead");
	this->StraightDamage.Read(exINI, pSection, "Trajectory.Straight.Damage");
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
		.Process(this->ProximityRadius)
		.Process(this->ProximityAllies)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->StraightWarhead)
		.Process(this->StraightDamage)
		.Process(this->SubjectToGround)
		.Process(this->ConfineAtHeight)
		.Process(this->EdgeAttenuation)
		.Process(this->CheckTimesLimit)
		.Process(this->ExtraCheck1)
		.Process(this->ExtraCheck2)
		.Process(this->ExtraCheck3)
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
		.Process(this->ProximityRadius)
		.Process(this->ProximityAllies)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->StraightWarhead)
		.Process(this->StraightDamage)
		.Process(this->SubjectToGround)
		.Process(this->ConfineAtHeight)
		.Process(this->EdgeAttenuation)
		.Process(this->CheckTimesLimit)
		.Process(this->ExtraCheck1)
		.Process(this->ExtraCheck2)
		.Process(this->ExtraCheck3)
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
	this->PassDetonateDelay = pType->PassDetonateDelay > 0 ? pType->PassDetonateDelay : 1;
	this->PassDetonateTimer = pType->PassDetonateTimer < this->PassDetonateDelay ?
		pType->PassDetonateTimer : this->PassDetonateDelay - 1;
	this->PassDetonateLocal = pType->PassDetonateLocal;
	this->LeadTimeCalculate = pType->LeadTimeCalculate;
	this->OffsetCoord = pType->OffsetCoord;
	this->RotateCoord = pType->RotateCoord;
	this->MirrorCoord = pType->MirrorCoord;
	this->UseDisperseBurst = pType->UseDisperseBurst;
	this->AxisOfRotation = pType->AxisOfRotation;
	this->ProximityImpact = pType->ProximityImpact;
	this->ProximityRadius = pType->ProximityRadius > 0.0 ? pType->ProximityRadius : 0.0;
	this->ProximityAllies = pType->ProximityAllies;
	this->ThroughVehicles = pType->ThroughVehicles;
	this->ThroughBuilding = pType->ThroughBuilding;
	this->StraightWarhead = pType->StraightWarhead;
	this->StraightDamage = pType->StraightDamage;
	this->SubjectToGround = pType->SubjectToGround;
	this->ConfineAtHeight = pType->ConfineAtHeight;
	this->EdgeAttenuation = pType->EdgeAttenuation > 0.0 ? pType->EdgeAttenuation : 1.0;
	this->CheckTimesLimit = 1;
	this->ExtraCheck1 = nullptr;
	this->ExtraCheck2 = nullptr;
	this->ExtraCheck3 = nullptr;
	this->LastCasualty.reserve(1);
	this->LastTargetCoord = pBullet->TargetCoords;
	this->CountOfBurst = pBullet->WeaponType ? pBullet->WeaponType->Burst : 0;
	this->WaitOneFrame = 0;

	if (pBullet->Owner)
	{
		this->CurrentBurst = pBullet->Owner->CurrentBurstIndex;
		this->LastCasualty.push_back(pBullet->Owner);
		this->FirepowerMult = pBullet->Owner->FirepowerMultiplier;

		if (this->MirrorCoord && pBullet->Owner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}
	else
	{
		this->CurrentBurst += 1;

		if (this->CountOfBurst > 0)
			this->CurrentBurst %= this->CountOfBurst;
		else
			this->CurrentBurst = 0;

		this->FirepowerMult = 1.0;
	}

	if (!this->LeadTimeCalculate || (pBullet->Target && pBullet->Target->WhatAmI() == AbstractType::Building))
		PrepareForOpenFire(pBullet);
	else
		this->WaitOneFrame = 2; //OnAI() not always check after OnUnlimbo() immediately. And Looking for better method.
}

bool StraightTrajectory::OnAI(BulletClass* pBullet)
{
	if (this->WaitOneFrame > 0)
	{
		this->WaitOneFrame -= 1;

		if (this->WaitOneFrame == 0 || !pBullet->Target || (pBullet->Target && pBullet->Target->GetCoords() != this->LastTargetCoord))
		{
			PrepareForOpenFire(pBullet);
			this->WaitOneFrame = 0;
		}
	}
	else
	{
		double StraightSpeed = this->GetTrajectorySpeed(pBullet);
		auto const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

		if (BulletDetonatePreCheck(pBullet, pOwner))
			return true;

		if (this->PassDetonate && this->StraightWarhead)
			PassWithDetonateAt(pBullet, pOwner);

		if (this->ProximityImpact != 0 && this->ProximityRadius > 0)
			PrepareForDetonateAt(pBullet, pOwner);

		if (StraightSpeed <= 256.0 && this->ConfineAtHeight > 0 && PassAndConfineAtHeight(pBullet, StraightSpeed))
			return true;

		BulletDetonateLastCheck(pBullet, pOwner, StraightSpeed);
	}

	return false;
}

void StraightTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	auto const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

	if ((this->EdgeAttenuation != 1.0 || this->ProximityAllies != 0) && pBullet->WeaponType)
	{
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pBullet->Target);
		int Damage = static_cast<int>(pBullet->WeaponType->Damage * this->FirepowerMult * GetExtraDamageMultiplier(pBullet, pTechno, pOwner, true));
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

	auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

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
	if (pTechno && this->GetTrajectorySpeed(pBullet) <= 256.0)
	{
		auto const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		if (this->ProximityAllies == 0 && pOwner->IsAlliedWith(pTechno->Owner))
			return TrajectoryCheckReturnType::SkipGameCheck;

		if (pTechno->WhatAmI() == AbstractType::Building)
		{
			auto const pBuilding = static_cast<BuildingClass*>(pTechno);

			if (pBuilding->Type)
			{
				if (pBuilding->Type->InvisibleInGame)
					return TrajectoryCheckReturnType::SkipGameCheck;

				if (pBuilding->Type->IsVehicle())
				{
					if (!this->ThroughVehicles)
						this->ExtraCheck1 = pTechno;
				}
				else
				{
					if (!this->ThroughBuilding)
						this->ExtraCheck1 = pTechno;
				}
			}
		}
		else if (!this->ThroughVehicles && (pTechno->WhatAmI() == AbstractType::Unit || pTechno->WhatAmI() == AbstractType::Aircraft))
		{
			this->ExtraCheck1 = pTechno;
		}
	}

	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

void StraightTrajectory::PrepareForOpenFire(BulletClass* pBullet)
{
	double StraightSpeed = this->GetTrajectorySpeed(pBullet);

	ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	CoordStruct TheTargetCoords = pBullet->TargetCoords;
	CoordStruct TheSourceCoords = pBullet->SourceCoords;

	if (this->LeadTimeCalculate && pTarget)
	{
		TheTargetCoords = pTarget->GetCoords();
		TheSourceCoords = pBullet->Location;

		if (TheTargetCoords != this->LastTargetCoord)
		{
			CoordStruct ExtraOffsetCoord = TheTargetCoords - this->LastTargetCoord;
			CoordStruct TargetSourceCoord = TheSourceCoords - TheTargetCoords;
			CoordStruct LastSourceCoord = TheSourceCoords - this->LastTargetCoord;

			double TheDistanceSquared = TheSourceCoords.DistanceFromSquared(TheTargetCoords);

			double TargetSpeedSquared = ExtraOffsetCoord.MagnitudeSquared();
			double TargetSpeed = sqrt(TargetSpeedSquared);

			double CrossFactor = LastSourceCoord.CrossProduct(TargetSourceCoord).MagnitudeSquared();
			double VerticalDistanceSquared = CrossFactor / TargetSpeedSquared;

			double HorizonDistanceSquared = TheDistanceSquared - VerticalDistanceSquared;
			double HorizonDistance = sqrt(HorizonDistanceSquared);

			double StraightSpeedSquared = StraightSpeed * StraightSpeed;
			double BaseFactor = StraightSpeedSquared - TargetSpeedSquared;

			double SquareFactor = BaseFactor * VerticalDistanceSquared + StraightSpeedSquared * HorizonDistanceSquared;
			if (SquareFactor > 0)
			{
				double MinusFactor = -(HorizonDistance * TargetSpeed);
				int TravelTime = 0;

				if (BaseFactor == 0)
				{
					if (HorizonDistance != 0)
						TravelTime = static_cast<int>(TheDistanceSquared / (2 * HorizonDistance * TargetSpeed)) + 1;
				}
				else
				{
					int TravelTimeM = static_cast<int>((MinusFactor - sqrt(SquareFactor)) / BaseFactor);
					int TravelTimeP = static_cast<int>((MinusFactor + sqrt(SquareFactor)) / BaseFactor);

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

	double RotateAngle = Math::atan2(TheTargetCoords.Y - TheSourceCoords.Y , TheTargetCoords.X - TheSourceCoords.X);

	if (this->OffsetCoord.X != 0 || this->OffsetCoord.Y != 0 || this->OffsetCoord.Z != 0)
	{
		TheTargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(RotateAngle) + this->OffsetCoord.Y * Math::sin(RotateAngle));
		TheTargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(RotateAngle) - this->OffsetCoord.Y * Math::cos(RotateAngle));
		TheTargetCoords.Z += this->OffsetCoord.Z;
	}

	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		int ScatterDefault = RulesClass::Instance->BallisticScatter;
		int ScatterMin = pTypeExt->BallisticScatter_Min.Get(Leptons(0));
		int ScatterMax = pTypeExt->BallisticScatter_Max.Get(Leptons(ScatterDefault));
		double OffsetMult = 0.0004 * TheSourceCoords.DistanceFrom(TheTargetCoords);

		int OffsetMin = static_cast<int>(OffsetMult * ScatterMin);
		int OffsetMax = static_cast<int>(OffsetMult * ScatterMax);

		int OffsetDistance = ScenarioClass::Instance->Random.RandomRanged(OffsetMin, OffsetMax);
		TheTargetCoords = MapClass::GetRandomCoordsNear(TheTargetCoords, OffsetDistance, false);
	}

	double DistanceOfTwo = TheSourceCoords.DistanceFrom(TheTargetCoords);

	if (this->PassThrough)
	{
		if (this->DetonationDistance > 0)
			this->CheckTimesLimit = static_cast<int>(this->DetonationDistance / StraightSpeed) + 1;
		else if (this->DetonationDistance < 0)
			this->CheckTimesLimit = static_cast<int>((DistanceOfTwo - this->DetonationDistance) / StraightSpeed) + 1;
		else
			this->CheckTimesLimit = INT_MAX;
	}
	else
	{
		this->CheckTimesLimit = static_cast<int>(DistanceOfTwo / StraightSpeed) + 1;
	}

	pBullet->TargetCoords = TheTargetCoords;

	pBullet->Velocity.X = static_cast<double>(TheTargetCoords.X - TheSourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(TheTargetCoords.Y - TheSourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(this->GetVelocityZ(pBullet));

	if (!this->UseDisperseBurst && this->RotateCoord != 0 && this->CountOfBurst > 1)
	{
		BulletVelocity RotationAxis
		{
			this->AxisOfRotation.X * Math::cos(RotateAngle) + this->AxisOfRotation.Y * Math::sin(RotateAngle),
			this->AxisOfRotation.X * Math::sin(RotateAngle) - this->AxisOfRotation.Y * Math::cos(RotateAngle),
			static_cast<double>(this->AxisOfRotation.Z)
		};

		double ExtraRotate = Math::Pi * (this->RotateCoord * (this->CurrentBurst / (this->CountOfBurst - 1.0) - 0.5)) / 180;
		double RotationAxisLengthSquared = RotationAxis.MagnitudeSquared();

		if (RotationAxisLengthSquared != 0)
		{
			RotationAxis *= 1 / sqrt(RotationAxisLengthSquared);
			double CosRotate = Math::cos(ExtraRotate);

			pBullet->Velocity = (pBullet->Velocity * CosRotate) + (RotationAxis * ((1 - CosRotate) * (pBullet->Velocity * RotationAxis)))
				+ (RotationAxis.CrossProduct(pBullet->Velocity) * Math::sin(ExtraRotate));
		}
	}

	if (CalculateBulletVelocity(pBullet, StraightSpeed))
		this->CheckTimesLimit = 0;
}

int StraightTrajectory::GetVelocityZ(BulletClass* pBullet)
{
	int velocity = static_cast<int>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);

	if (!this->PassThrough)
		return velocity;

	if (pBullet->Owner && abs(pBullet->Owner->GetCoords().Z - pBullet->TargetCoords.Z) <= 32)
	{
		double DistanceOfTwo = pBullet->SourceCoords.DistanceFrom(pBullet->TargetCoords);
		double TheDistance = this->DetonationDistance;

		if (this->DetonationDistance == 0)
			return 0;

		if (this->DetonationDistance < 0)
			TheDistance = DistanceOfTwo - this->DetonationDistance;

		if (TheDistance != 0)
			velocity = static_cast<int>(velocity * (DistanceOfTwo / TheDistance));
		else
			return 0;
	}

	return velocity;
}

bool StraightTrajectory::CalculateBulletVelocity(BulletClass* pBullet, double StraightSpeed)
{
	double VelocityLength = pBullet->Velocity.Magnitude();

	if (VelocityLength > 0)
		pBullet->Velocity *= StraightSpeed / VelocityLength;
	else
		return true;

	return false;
}

bool StraightTrajectory::BulletDetonatePreCheck(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->ExtraCheck1)
	{
		CoordStruct ExtraCheckCoord = this->ExtraCheck1->GetCoords();

		pBullet->Target = this->ExtraCheck1;
		pBullet->TargetCoords = ExtraCheckCoord;

		return true;
	}

	if (this->ExtraCheck2)
	{
		CoordStruct ExtraCheckCoord = this->ExtraCheck2->GetCoords();

		pBullet->SetLocation(ExtraCheckCoord);
		pBullet->Target = this->ExtraCheck2;
		pBullet->TargetCoords = ExtraCheckCoord;

		return true;
	}

	if (this->ExtraCheck3)
	{
		CoordStruct ExtraCheckCoord = this->ExtraCheck3->GetCoords();

		pBullet->SetLocation(ExtraCheckCoord);
		pBullet->Target = this->ExtraCheck3;
		pBullet->TargetCoords = ExtraCheckCoord;

		return true;
	}

	this->CheckTimesLimit -= 1;
	if (this->CheckTimesLimit < 0)
		return true;

	if (!this->PassThrough && this->DetonationDistance > 0
		&& pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
	{
		return true;
	}

	if (this->SubjectToGround && MapClass::Instance->GetCellFloorHeight(pBullet->Location) >= pBullet->Location.Z)
		return true;

	if (CellClass* pCell = MapClass::Instance->TryGetCellAt(pBullet->Location))
		return false;
	else
		return true;
}

//If the check result here is true, it only needs to be detonated in the next frame, without returning.
void StraightTrajectory::BulletDetonateLastCheck(BulletClass* pBullet, HouseClass* pOwner, double StraightSpeed)
{
	bool VelocityCheck = false;
	double LocationDistance = pBullet->Location.DistanceFrom(pBullet->TargetCoords);

	if (this->PassThrough)
	{
		if (this->DetonationDistance < 0)
			LocationDistance -= this->DetonationDistance;
		else
			LocationDistance = DBL_MAX;
	}

	if (LocationDistance < StraightSpeed)
		VelocityCheck = true;

	if (this->ExtraCheck1)
	{
		LocationDistance = this->ExtraCheck1->GetCoords().DistanceFrom(pBullet->Location);
		VelocityCheck = true;
	}
	else if (this->ExtraCheck2)
	{
		LocationDistance = this->ExtraCheck2->GetCoords().DistanceFrom(pBullet->Location);
		VelocityCheck = true;
	}
	else if (this->ExtraCheck3)
	{
		LocationDistance = this->ExtraCheck3->GetCoords().DistanceFrom(pBullet->Location);
		VelocityCheck = true;
	}

	if (StraightSpeed > 256 && (!this->ThroughBuilding || !this->ThroughVehicles || this->SubjectToGround))
	{
		std::vector<CellClass*> StaCellClass = GetCellsInPassThrough(pBullet);

		for (auto const pStaCell : StaCellClass)
		{
			double CellDistance = pStaCell->GetCoords().DistanceFrom(pBullet->Location);
			double BulletHeight = pBullet->Location.Z + CellDistance / StraightSpeed * pBullet->Velocity.Z;

			if (MapClass::Instance->GetCellFloorHeight(pStaCell->GetCoords()) >= BulletHeight
				&& CellDistance < LocationDistance)
			{
				LocationDistance = CellDistance;
				VelocityCheck = true;
			}

			if (this->ThroughBuilding && this->ThroughVehicles)
				continue;

			ObjectClass* pObject = pStaCell->FirstObject;

			while (pObject)
			{
				auto const pTechno = abstract_cast<TechnoClass*>(pObject);
				pObject = pObject->NextObject;

				if (!pTechno)
					continue;

				if (pOwner->IsAlliedWith(pTechno->Owner))
					continue;

				if (pTechno->GetHeight() > 0)
					continue;

				auto const TechnoType = pTechno->WhatAmI();

				if (TechnoType == AbstractType::Building)
				{
					BuildingClass* pBuilding = static_cast<BuildingClass*>(pTechno);

					if (pBuilding->Type)
					{
						if (pBuilding->Type->InvisibleInGame)
							continue;

						if (CellDistance < LocationDistance)
						{
							if (pBuilding->Type->IsVehicle())
							{
								if (!this->ThroughVehicles)
								{
									LocationDistance = CellDistance;
									VelocityCheck = true;
								}
							}
							else
							{
								if (!this->ThroughBuilding)
								{
									LocationDistance = CellDistance;
									VelocityCheck = true;
								}
							}
						}
					}
				}

				if (CellDistance < LocationDistance && !this->ThroughVehicles
					&& (TechnoType == AbstractType::Unit || TechnoType == AbstractType::Aircraft))
				{
					LocationDistance = CellDistance;
					VelocityCheck = true;
				}
			}
		}
	}

	if (VelocityCheck)
	{
		this->CheckTimesLimit = 0;
		pBullet->Velocity *= LocationDistance / StraightSpeed;
	}
}

void StraightTrajectory::PassWithDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->PassDetonateTimer == 0)
	{
		CoordStruct DetonateCoords = pBullet->Location;
		int Damage = this->StraightDamage;

		if (this->PassDetonateLocal)
			DetonateCoords.Z = MapClass::Instance->GetCellFloorHeight(DetonateCoords);

		if (this->EdgeAttenuation != 1.0 || this->ProximityAllies != 0)
			Damage = static_cast<int>(Damage * GetExtraDamageMultiplier(pBullet, nullptr, nullptr, false));

		WarheadTypeExt::DetonateAt(this->StraightWarhead, DetonateCoords, pBullet->Owner, Damage, pOwner);
	}

	this->PassDetonateTimer += 1;
	if (this->PassDetonateTimer > 0)
		this->PassDetonateTimer %= this->PassDetonateDelay;
}

//Select suitable targets and choose the closer targets then attack each target only once.
void StraightTrajectory::PrepareForDetonateAt(BulletClass* pBullet, HouseClass* pOwner)
{
	//Step1 Find valid targets on the ground within range.
	std::vector<CellClass*> RecCellClass = GetCellsInProximityRadius(pBullet);
	size_t CellSize = RecCellClass.size() * 2;
	size_t ThisSize = 0;

	std::vector<TechnoClass*> ValidTechnos;
	ValidTechnos.reserve(CellSize);
	TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target);

	for (auto const pRecCell : RecCellClass)
	{
		ObjectClass* pObject = pRecCell->FirstObject;

		while (pObject)
		{
			auto const pTechno = abstract_cast<TechnoClass*>(pObject);
			pObject = pObject->NextObject;

			if (!pTechno)
				continue;

			if (pTechno == pBullet->Owner)
				continue;

			if (pTechno->GetHeight() > 0)
				continue;

			auto const TechnoType = pTechno->WhatAmI();

			if (TechnoType == AbstractType::Building)
			{
				auto const pBuilding = static_cast<BuildingClass*>(pTechno);

				if (pBuilding->Type)
				{
					if (pBuilding->Type->InvisibleInGame)
						continue;

					if (pBuilding->Type->IsVehicle())
					{
						if (!this->ThroughVehicles)
							continue;
					}
					else
					{
						if (!this->ThroughBuilding)
							continue;
					}
				}
			}

			if (!this->ThroughVehicles && (TechnoType == AbstractType::Unit || TechnoType == AbstractType::Aircraft))
				continue;

			bool TargetCheck = this->ProximityAllies == 0 && pOwner->IsAlliedWith(pTechno->Owner);

			if (pTargetTechno && pTechno == pTargetTechno)
			{
				if (!this->PassThrough)
				{
					continue;
				}
				else if (this->DetonationDistance < 0 && TargetCheck)
				{
					this->ExtraCheck3 = pTargetTechno;
					continue;
				}
			}

			if (TargetCheck)
				continue;

			CoordStruct SourceCrd = pBullet->SourceCoords;
			CoordStruct TechnoCrd = pTechno->GetCoords();
			CoordStruct BulletCrd = pBullet->Location;
			CoordStruct FutureCrd
			{
				BulletCrd.X + static_cast<int>(pBullet->Velocity.X),
				BulletCrd.Y + static_cast<int>(pBullet->Velocity.Y),
				BulletCrd.Z
			};

			if ((TechnoCrd.X - SourceCrd.X) * pBullet->Velocity.X + (TechnoCrd.Y - SourceCrd.Y) * pBullet->Velocity.Y < 0)
				continue;

			double Distance = abs(TechnoCrd.X - BulletCrd.X);

			if (FutureCrd.X != BulletCrd.X)
			{
				double Factor = (FutureCrd.Y - BulletCrd.Y) / (FutureCrd.X - BulletCrd.X);
				Distance = abs(Factor * TechnoCrd.X - TechnoCrd.Y + BulletCrd.Y - Factor * BulletCrd.X) / sqrt(Factor * Factor + 1);
			}

			if (TechnoType != AbstractType::Building && Distance > this->ProximityRadius * 256.0)
				continue;

			if (ThisSize < CellSize)
			{
				ValidTechnos.push_back(pTechno);
			}
			else
			{
				std::vector<TechnoClass*> ValidTechnosBuffer;
				ValidTechnosBuffer.reserve(ThisSize + CellSize);

				for (auto const pTechnoBuffer : ValidTechnos)
					ValidTechnosBuffer.push_back(pTechnoBuffer);

				ValidTechnos = ValidTechnosBuffer;
			}

			ThisSize += 1;
		}
	}

	//Step2 Record each target without repetition.
	size_t i = 0, iMax = this->LastCasualty.size();
	size_t j = 0, jMax = ValidTechnos.size();
	size_t Capacity = iMax + jMax;

	std::sort(&ValidTechnos[0], &ValidTechnos[jMax]);

	std::vector<TechnoClass*> Casualty;
	Casualty.reserve(Capacity);
	std::vector<TechnoClass*> CasualtyChecked;
	CasualtyChecked.reserve(Capacity);

	TechnoClass* pThis = nullptr;
	TechnoClass* pLast = nullptr;
	bool Check = false;

	for (size_t k = 0; k < Capacity; k++) //Merge
	{
		if (i < iMax && j < jMax)
		{
			if (this->LastCasualty[i] < ValidTechnos[j])
			{
				Check = false;
				pThis = nullptr;
				i += 1;
			}
			else if (this->LastCasualty[i] > ValidTechnos[j])
			{
				Check = true;
				pThis = ValidTechnos[j];
				j += 1;
			}
			else
			{
				Check = false;
				pThis = ValidTechnos[j];
				i += 1;
				j += 1;
			}
		}
		else if (i < iMax)
		{
			Check = false;
			pThis = nullptr;
			i += 1;
		}
		else if (j < jMax)
		{
			Check = true;
			pThis = ValidTechnos[j];
			j += 1;
		}
		else
		{
			break;
		}

		if (pThis && pThis != pLast)
		{
			Casualty.push_back(pThis);

			if (Check)
				CasualtyChecked.push_back(pThis);

			pLast = pThis;
		}
	}

	this->LastCasualty = Casualty;

	//Step3 Detonate warheads in sequence based on distance.
	size_t CasualtySize = CasualtyChecked.size();

	if (this->ProximityImpact > 0 && static_cast<int>(CasualtySize) > this->ProximityImpact)
	{
		std::sort(&CasualtyChecked[0], &CasualtyChecked[CasualtySize],[pBullet](TechnoClass* pTechnoA, TechnoClass* pTechnoB){
			return pTechnoA->GetCoords().DistanceFromSquared(pBullet->SourceCoords) < pTechnoB->GetCoords().DistanceFromSquared(pBullet->SourceCoords);
		});
	}

	for (auto const& pTechno : CasualtyChecked)
	{
		if (this->ProximityImpact != 1 && this->StraightWarhead)
		{
			int Damage = this->StraightDamage;

			if (this->EdgeAttenuation != 1.0 || this->ProximityAllies != 0)
				Damage = static_cast<int>(Damage * GetExtraDamageMultiplier(pBullet, pTechno, pOwner, false));

			WarheadTypeExt::DetonateAt(this->StraightWarhead, pTechno->GetCoords(), pBullet->Owner, Damage, pOwner);

			if (this->ProximityImpact > 0)
				this->ProximityImpact -= 1;
		}
		else
		{
			this->ExtraCheck2 = pTechno;
			break;
		}
	}
}

std::vector<CellClass*> StraightTrajectory::GetCellsInPassThrough(BulletClass* pBullet)
{
	std::vector<CellClass*> StaCellClass;

	if (pBullet->Velocity.X != 0 || pBullet->Velocity.Y != 0)
	{
		CoordStruct WalkCoord { static_cast<int>(pBullet->Velocity.X), static_cast<int>(pBullet->Velocity.Y), 0 };

		CellStruct ThisCell = CellClass::Coord2Cell(pBullet->Location);
		CellStruct NextCell = CellClass::Coord2Cell((pBullet->Location + WalkCoord));

		std::vector<CellStruct> StaCells = GetCellsInRectangle(ThisCell, ThisCell, NextCell, NextCell);

		StaCellClass.reserve(StaCells.size());
		for (auto const& pCells : StaCells)
		{
			if (CellClass* pStaCell = MapClass::Instance->TryGetCellAt(pCells))
				StaCellClass.push_back(pStaCell);
		}
	}

	return StaCellClass;
}

//A rectangular shape with a custom width from the current frame to the next frame in length.
std::vector<CellClass*> StraightTrajectory::GetCellsInProximityRadius(BulletClass* pBullet)
{
	std::vector<CellClass*> RecCellClass;

	if (pBullet->Velocity.X != 0 || pBullet->Velocity.Y != 0)
	{
		CoordStruct WalkCoord { static_cast<int>(pBullet->Velocity.X), static_cast<int>(pBullet->Velocity.Y), 0 };
		double SideMult = (this->ProximityRadius * 256.0) / WalkCoord.Magnitude();

		CoordStruct Cor1Coord { static_cast<int>(WalkCoord.Y * SideMult), static_cast<int>((-WalkCoord.X) * SideMult), 0 };
		CoordStruct Cor4Coord { static_cast<int>((-WalkCoord.Y) * SideMult), static_cast<int>(WalkCoord.X * SideMult), 0 };

		CellStruct ThisCell = CellClass::Coord2Cell(pBullet->Location);
		CellStruct Cor1Cell = CellClass::Coord2Cell((pBullet->Location + Cor1Coord));
		CellStruct Cor4Cell = CellClass::Coord2Cell((pBullet->Location + Cor4Coord));

		CellStruct Off1Cell = Cor1Cell - ThisCell;
		CellStruct Off4Cell = Cor4Cell - ThisCell;

		CellStruct NextCell = CellClass::Coord2Cell((pBullet->Location + WalkCoord));
		CellStruct Cor2Cell = NextCell + Off1Cell;
		CellStruct Cor3Cell = NextCell + Off4Cell;

		std::vector<CellStruct> RecCells;

		//Arrange the vertices of the rectangle in order from bottom to top.
		if (Cor1Cell.X > Cor2Cell.X) //Left
		{
			if (Cor1Cell.Y >= Cor2Cell.Y) //↙ and ←
				RecCells = GetCellsInRectangle(Cor3Cell, Cor2Cell, Cor4Cell, Cor1Cell);
			else //↖
				RecCells = GetCellsInRectangle(Cor4Cell, Cor3Cell, Cor1Cell, Cor2Cell);
		}
		else if (Cor1Cell.X == Cor2Cell.X) //Mid
		{
			if (Cor1Cell.Y >= Cor2Cell.Y) //↓ and Center
				RecCells = GetCellsInRectangle(Cor2Cell, Cor1Cell, Cor3Cell, Cor4Cell);
			else //↑
				RecCells = GetCellsInRectangle(Cor4Cell, Cor3Cell, Cor1Cell, Cor2Cell);
		}
		else //Right
		{
			if (Cor1Cell.Y >= Cor2Cell.Y) //↘ and →
				RecCells = GetCellsInRectangle(Cor2Cell, Cor1Cell, Cor3Cell, Cor4Cell);
			else //↗
				RecCells = GetCellsInRectangle(Cor1Cell, Cor4Cell, Cor2Cell, Cor3Cell);
		}

		RecCellClass.reserve(RecCells.size());
		for (auto const& pCells : RecCells)
		{
			if (CellClass* pRecCell = MapClass::Instance->TryGetCellAt(pCells))
				RecCellClass.push_back(pRecCell);
		}
	}

	return RecCellClass;
}

//Record cells in the order of "draw left boundary, draw right boundary, fill middle, and move up one level".
std::vector<CellStruct> StraightTrajectory::GetCellsInRectangle(CellStruct bStaCell, CellStruct lMidCell, CellStruct rMidCell, CellStruct tEndCell)
{
	std::vector<CellStruct> RecCells;
	int CellNums = (abs(tEndCell.Y - bStaCell.Y) + 1) * (abs(rMidCell.X - lMidCell.X) + 1);
	RecCells.reserve(CellNums);
	RecCells.push_back(bStaCell);

	if (bStaCell == lMidCell || bStaCell == rMidCell)
	{
		CellStruct mCurCell = bStaCell;
		CellStruct mTheDist = tEndCell - bStaCell;
		CellStruct mTheUnit = { static_cast<short>(Math::sgn(mTheDist.X)), static_cast<short>(Math::sgn(mTheDist.Y)) };
		mTheDist = { static_cast<short>(mTheDist.X * mTheUnit.X), static_cast<short>(mTheDist.Y * mTheUnit.Y) };
		float mTheCurN = static_cast<float>((mTheDist.Y - mTheDist.X) / 2.0);

		while (mCurCell != tEndCell)
		{
			if (mTheCurN > 0)
			{
				mTheCurN -= mTheDist.X;
				mCurCell.Y += mTheUnit.Y;
				RecCells.push_back(mCurCell);
			}
			else if (mTheCurN < 0)
			{
				mTheCurN += mTheDist.Y;
				mCurCell.X += mTheUnit.X;
				RecCells.push_back(mCurCell);
			}
			else
			{
				mTheCurN += mTheDist.Y - mTheDist.X;
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
		CellStruct lCurCell = bStaCell, rCurCell = bStaCell ,mCurCell = bStaCell;
		bool lNext = false, rNext = false, lSkip = false, rSkip = false, lContinue = false, rContinue = false;

		CellStruct l1stDist = lMidCell - bStaCell;
		CellStruct l1stUnit = { static_cast<short>(Math::sgn(l1stDist.X)), static_cast<short>(Math::sgn(l1stDist.Y)) };
		l1stDist = { static_cast<short>(l1stDist.X * l1stUnit.X), static_cast<short>(l1stDist.Y * l1stUnit.Y) };
		float l1stCurN = static_cast<float>((l1stDist.Y - l1stDist.X) / 2.0);

		CellStruct l2ndDist = tEndCell - lMidCell;
		CellStruct l2ndUnit = { static_cast<short>(Math::sgn(l2ndDist.X)), static_cast<short>(Math::sgn(l2ndDist.Y)) };
		l2ndDist = { static_cast<short>(l2ndDist.X * l2ndUnit.X), static_cast<short>(l2ndDist.Y * l2ndUnit.Y) };
		float l2ndCurN = static_cast<float>((l2ndDist.Y - l2ndDist.X) / 2.0);

		CellStruct r1stDist = rMidCell - bStaCell;
		CellStruct r1stUnit = { static_cast<short>(Math::sgn(r1stDist.X)), static_cast<short>(Math::sgn(r1stDist.Y)) };
		r1stDist = { static_cast<short>(r1stDist.X * r1stUnit.X), static_cast<short>(r1stDist.Y * r1stUnit.Y) };
		float r1stCurN = static_cast<float>((r1stDist.Y - r1stDist.X) / 2.0);

		CellStruct r2ndDist = tEndCell - rMidCell;
		CellStruct r2ndUnit = { static_cast<short>(Math::sgn(r2ndDist.X)), static_cast<short>(Math::sgn(r2ndDist.Y)) };
		r2ndDist = { static_cast<short>(r2ndDist.X * r2ndUnit.X), static_cast<short>(r2ndDist.Y * r2ndUnit.Y) };
		float r2ndCurN = static_cast<float>((r2ndDist.Y - r2ndDist.X) / 2.0);

		while (lCurCell != tEndCell || rCurCell != tEndCell)
		{
			while (lCurCell != tEndCell) //Left
			{
				if (!lNext) //Bottom Left Side
				{
					if (l1stCurN > 0)
					{
						l1stCurN -= l1stDist.X;
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
						l1stCurN += l1stDist.Y;
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
							l2ndCurN -= l2ndDist.X;
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
						l2ndCurN += l2ndDist.Y;
						lCurCell.X += l2ndUnit.X;
					}
				}

				RecCells.push_back(lCurCell);
			}

			while (rCurCell != tEndCell) //Right
			{
				if (!rNext) //Bottom Right Side
				{
					if (r1stCurN > 0)
					{
						r1stCurN -= r1stDist.X;
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
						r1stCurN += r1stDist.Y;
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
							r2ndCurN -= r2ndDist.X;
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
						r2ndCurN += r2ndDist.Y;
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
				l2ndCurN -= l2ndDist.X;
				lCurCell.Y += l2ndUnit.Y;
				RecCells.push_back(lCurCell);
			}

			if (rContinue) //Continue Top Right Side
			{
				rContinue = false;
				r2ndCurN -= r2ndDist.X;
				rCurCell.Y += r2ndUnit.Y;
				RecCells.push_back(rCurCell);
			}
		}
	}

	return RecCells;
}

double StraightTrajectory::GetExtraDamageMultiplier(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner, bool Self)
{
	double Distance = 0;
	double DamageMult = 1.0;
	double AlliesMultiplier = 1.0;
	double MaxDistance = pBullet->WeaponType ? static_cast<double>(pBullet->WeaponType->Range) : 0;

	if (pTechno)
	{
		Distance = pTechno->GetCoords().DistanceFrom(pBullet->SourceCoords);

		if (pOwner && pOwner->IsAlliedWith(pTechno->Owner))
			AlliesMultiplier = this->ProximityAllies;
	}
	else
	{
		Distance = pBullet->Location.DistanceFrom(pBullet->SourceCoords);
	}

	if (MaxDistance < Distance)
		return this->EdgeAttenuation;

	if (Distance > 256.0)
		DamageMult += (this->EdgeAttenuation - 1.0) * ((Distance - 256.0) / (MaxDistance - 256.0));

	if (!Self)
		DamageMult *= AlliesMultiplier;

	return DamageMult;
}

bool StraightTrajectory::PassAndConfineAtHeight(BulletClass* pBullet, double StraightSpeed)
{
	CoordStruct FutureCoords
	{
		pBullet->Location.X + static_cast<int>(pBullet->Velocity.X),
		pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y),
		pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z)
	};

	if (auto const pCell = MapClass::Instance->GetCellAt(FutureCoords))
	{
		int CheckDifference = MapClass::Instance->GetCellFloorHeight(FutureCoords) - FutureCoords.Z;
		CoordStruct CellCoords = pCell->GetCoordsWithBridge();
		int DifferenceOnBridge = CellCoords.Z - FutureCoords.Z;

		if (abs(DifferenceOnBridge) < abs(CheckDifference))
			CheckDifference = DifferenceOnBridge;

		if (abs(CheckDifference) < 384 || !pBullet->Type->SubjectToCliffs)
		{
			pBullet->Velocity.Z += static_cast<double>(CheckDifference + this->ConfineAtHeight);

			if (CalculateBulletVelocity(pBullet, StraightSpeed))
				return true;
		}
		else
		{
			return true;
		}
	}

	return false;
}
