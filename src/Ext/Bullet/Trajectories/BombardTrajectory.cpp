#include "BombardTrajectory.h"
#include <AnimClass.h>
#include <Ext/Bullet/Body.h>

PhobosTrajectory* BombardTrajectoryType::CreateInstance() const
{
	return new BombardTrajectory(this);
}

bool BombardTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	Stm
		.Process(this->Height, false)
		.Process(this->FallPercent, false)
		.Process(this->FallPercentShift, false)
		.Process(this->FallScatter_Max, false)
		.Process(this->FallScatter_Min, false)
		.Process(this->FallSpeed, false)
		.Process(this->DetonationDistance, false)
		.Process(this->ApplyRangeModifiers, false)
		.Process(this->TargetSnapDistance, false)
		.Process(this->FreeFallOnTarget, false)
		.Process(this->LeadTimeCalculate, false)
		.Process(this->NoLaunch, false)
		.Process(this->TurningPointAnims, false)
		.Process(this->OffsetCoord, false)
		.Process(this->RotateCoord, false)
		.Process(this->MirrorCoord, false)
		.Process(this->UseDisperseBurst, false)
		.Process(this->AxisOfRotation, false)
		;

	return true;
}

bool BombardTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	Stm
		.Process(this->Height)
		.Process(this->FallPercent)
		.Process(this->FallPercentShift)
		.Process(this->FallScatter_Max)
		.Process(this->FallScatter_Min)
		.Process(this->FallSpeed)
		.Process(this->DetonationDistance)
		.Process(this->ApplyRangeModifiers)
		.Process(this->TargetSnapDistance)
		.Process(this->FreeFallOnTarget)
		.Process(this->LeadTimeCalculate)
		.Process(this->NoLaunch)
		.Process(this->TurningPointAnims)
		.Process(this->OffsetCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
		;

	return true;
}

void BombardTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->Height.Read(exINI, pSection, "Trajectory.Bombard.Height");
	this->FallPercent.Read(exINI, pSection, "Trajectory.Bombard.FallPercent");
	this->FallPercentShift.Read(exINI, pSection, "Trajectory.Bombard.FallPercentShift");
	this->FallScatter_Max.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Max");
	this->FallScatter_Min.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Min");
	this->FallSpeed.Read(exINI, pSection, "Trajectory.Bombard.FallSpeed");
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Bombard.DetonationDistance");
	this->ApplyRangeModifiers.Read(exINI, pSection, "Trajectory.Straight.ApplyRangeModifiers");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Bombard.TargetSnapDistance");
	this->FreeFallOnTarget.Read(exINI, pSection, "Trajectory.Bombard.FreeFallOnTarget");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.Bombard.LeadTimeCalculate");
	this->NoLaunch.Read(exINI, pSection, "Trajectory.Bombard.NoLaunch");
	this->TurningPointAnims.Read(exINI, pSection, "Trajectory.Bombard.TurningPointAnims");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.Bombard.OffsetCoord");
	this->RotateCoord.Read(exINI, pSection, "Trajectory.Bombard.RotateCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Bombard.MirrorCoord");
	this->UseDisperseBurst.Read(exINI, pSection, "Trajectory.Bombard.UseDisperseBurst");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.Bombard.AxisOfRotation");
}

bool BombardTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->IsFalling)
		.Process(this->Height)
		.Process(this->RemainingDistance)
		.Process(this->FallPercent)
		.Process(this->FallSpeed)
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
		.Process(this->FreeFallOnTarget)
		.Process(this->LeadTimeCalculate)
		.Process(this->OffsetCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
		.Process(this->LastTargetCoord)
		.Process(this->CountOfBurst)
		.Process(this->CurrentBurst)
		;

	return true;
}

bool BombardTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->IsFalling)
		.Process(this->Height)
		.Process(this->RemainingDistance)
		.Process(this->FallPercent)
		.Process(this->FallSpeed)
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
		.Process(this->FreeFallOnTarget)
		.Process(this->LeadTimeCalculate)
		.Process(this->OffsetCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
		.Process(this->LastTargetCoord)
		.Process(this->CountOfBurst)
		.Process(this->CurrentBurst)
		;

	return true;
}

void BombardTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto const pType = this->GetTrajectoryType<BombardTrajectoryType>(pBullet);
	this->Height = pType->Height + pBullet->TargetCoords.Z;
	// use scaling since RandomRanged only support int
	double fallPercentShift = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(200 * pType->FallPercentShift)) / 100.0;
	this->FallPercent = pType->FallPercent - pType->FallPercentShift + fallPercentShift;
	this->FallSpeed = pType->FallSpeed ? pType->FallSpeed : this->GetTrajectorySpeed(pBullet);
	this->DetonationDistance = pType->DetonationDistance;

	if (pType->ApplyRangeModifiers)
		this->DetonationDistance = Leptons(WeaponTypeExt::GetRangeWithModifiers(pBullet->WeaponType, pBullet->Owner, this->DetonationDistance));

	this->TargetSnapDistance = pType->TargetSnapDistance;
	this->FreeFallOnTarget = pType->FreeFallOnTarget;
	this->LeadTimeCalculate = pType->LeadTimeCalculate;
	this->OffsetCoord = pType->OffsetCoord;
	this->RotateCoord = pType->RotateCoord;
	this->MirrorCoord = pType->MirrorCoord;
	this->UseDisperseBurst = pType->UseDisperseBurst;
	this->AxisOfRotation = pType->AxisOfRotation;
	this->LastTargetCoord = pBullet->TargetCoords;

	if (WeaponTypeClass* const pWeapon = pBullet->WeaponType)
		this->CountOfBurst = pWeapon->Burst;

	if (TechnoClass* const pOwner = pBullet->Owner)
	{
		this->CurrentBurst = pOwner->CurrentBurstIndex;

		if (pType->MirrorCoord && pOwner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	if (!pType->NoLaunch)
	{
		if (this->FreeFallOnTarget)
			BombardTrajectory::CalculateLeadTime(pBullet);

		pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * this->FallPercent;
		pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * this->FallPercent;
		pBullet->Velocity.Z = static_cast<double>(this->Height - pBullet->SourceCoords.Z);
		pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
	}
	else
	{
		this->IsFalling = true;
		CoordStruct SourceLocation;
		SourceLocation.Z = static_cast<int>(this->Height - pBullet->SourceCoords.Z);
		double angel = ScenarioClass::Instance->Random.RandomDouble() * Math::TwoPi;
		double length = ScenarioClass::Instance->Random.RandomRanged(Leptons { pType->FallScatter_Min }, Leptons { pType->FallScatter_Max });
		int scatterX = static_cast<int>(length * Math::cos(angel));
		int scatterY = static_cast<int>(length * Math::sin(angel));

		if (!this->FreeFallOnTarget)
		{
			BombardTrajectory::CalculateLeadTime(pBullet);

			SourceLocation.X = pBullet->SourceCoords.X + static_cast<int>((pBullet->TargetCoords.X - pBullet->SourceCoords.X) * this->FallPercent) + scatterX;
			SourceLocation.Y = pBullet->SourceCoords.Y + static_cast<int>((pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * this->FallPercent) + scatterY;
			pBullet->Limbo();
			pBullet->Unlimbo(SourceLocation, DirType::North);
			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - SourceLocation.X);
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - SourceLocation.Y);
			pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - SourceLocation.Z);
			pBullet->Velocity *= this->FallSpeed / pBullet->Velocity.Magnitude();
		}
		else
		{
			const CoordStruct extraOffsetCoord = pBullet->TargetCoords - this->LastTargetCoord;
			const int travelTime = static_cast<int>(sqrt(2 * (this->Height - pBullet->TargetCoords.Z) / BulletTypeExt::GetAdjustedGravity(pBullet->Type)));
			pBullet->TargetCoords += extraOffsetCoord * travelTime;

			SourceLocation.X = pBullet->TargetCoords.X + scatterX;
			SourceLocation.Y = pBullet->TargetCoords.Y + scatterY;
			pBullet->Limbo();
			pBullet->Unlimbo(SourceLocation, DirType::North);
			pBullet->Velocity.X = 0.0;
			pBullet->Velocity.Y = 0.0;
			pBullet->Velocity.Z = 0.0;

			BombardTrajectory::CalculateDisperseBurst(pBullet);
		}

		this->RemainingDistance = static_cast<int>(pBullet->TargetCoords.DistanceFrom(SourceLocation) + this->FallSpeed);

		if (!pType->TurningPointAnims.empty())
		{
			if (auto const pAnim = GameCreate<AnimClass>(pType->TurningPointAnims[ScenarioClass::Instance->Random.RandomRanged(0, pType->TurningPointAnims.size() - 1)], SourceLocation))
			{
				pAnim->SetOwnerObject(pBullet->Owner);
				pAnim->Owner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
			}
		}
	}
}

void BombardTrajectory::CalculateLeadTime(BulletClass* pBullet)
{
	double rotateAngle = 0.0;
	const double straightSpeed = this->FreeFallOnTarget ? this->GetTrajectorySpeed(pBullet) : this->FallSpeed;
	const AbstractClass* const pTarget = pBullet->Target;
	CoordStruct theTargetCoords = pBullet->TargetCoords;
	CoordStruct theSourceCoords = pBullet->SourceCoords;

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

				if (this->FreeFallOnTarget)
					travelTime += static_cast<int>(sqrt(2 * (this->Height - theTargetCoords.Z) / BulletTypeExt::GetAdjustedGravity(pBullet->Type)));

				theTargetCoords += extraOffsetCoord * travelTime;
			}
		}
	}

	pBullet->TargetCoords = theTargetCoords;

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
		pBullet->TargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(rotateAngle) + this->OffsetCoord.Y * Math::sin(rotateAngle));
		pBullet->TargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(rotateAngle) - this->OffsetCoord.Y * Math::cos(rotateAngle));
		pBullet->TargetCoords.Z += this->OffsetCoord.Z;
	}

	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		const double offsetMult = 0.0004 * pBullet->SourceCoords.DistanceFrom(pBullet->TargetCoords);
		const int offsetMin = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Min.Get(Leptons(0)));
		const int offsetMax = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter)));
		const int offsetDistance = ScenarioClass::Instance->Random.RandomRanged(offsetMin, offsetMax);
		pBullet->TargetCoords = MapClass::GetRandomCoordsNear(pBullet->TargetCoords, offsetDistance, false);
	}
}

void BombardTrajectory::CalculateDisperseBurst(BulletClass* pBullet)
{
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
}

bool BombardTrajectory::OnAI(BulletClass* pBullet)
{
	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	// Extra check for trajectory falling
	auto const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

	if (this->IsFalling && !this->FreeFallOnTarget && BulletDetonatePreCheck(pBullet, pOwner, this->FallSpeed))
		return true;

	return false;
}

void BombardTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= this->TargetSnapDistance)
	{
		auto const pExt = BulletExt::ExtMap.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

void BombardTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	auto const trajType = this->GetTrajectoryType<BombardTrajectoryType>(pBullet);
	if (!this->IsFalling)
	{
		pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
		if (pBullet->Location.Z + pBullet->Velocity.Z >= this->Height)
		{
			this->IsFalling = true;
			if (!this->FreeFallOnTarget)
			{
				BombardTrajectory::CalculateLeadTime(pBullet);

				pSpeed->X = static_cast<double>(pBullet->TargetCoords.X - pBullet->Location.X - pBullet->Velocity.X);
				pSpeed->Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->Location.Y - pBullet->Velocity.Y);
				pSpeed->Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->Location.Z - pBullet->Velocity.Z);
				(*pSpeed) *= this->FallSpeed / pSpeed->Magnitude();
				pPosition->X = pBullet->Location.X + pBullet->Velocity.X;
				pPosition->Y = pBullet->Location.Y + pBullet->Velocity.Y;
				pPosition->Z = pBullet->Location.Z + pBullet->Velocity.Z;

				BombardTrajectory::CalculateDisperseBurst(pBullet);
			}
			else
			{
				pSpeed->X = 0.0;
				pSpeed->Y = 0.0;
				pSpeed->Z = 0.0;

				if (this->FallPercent != 1.0) // change position and recreate laser trail
				{
					auto pExt = BulletExt::ExtMap.Find(pBullet);
					pExt->LaserTrails.clear();
					CoordStruct target = pBullet->TargetCoords;
					target.Z += static_cast<int>(trajType->Height);
					pBullet->Limbo();
					pBullet->Unlimbo(target, DirType::North);
					pPosition->X = pBullet->TargetCoords.X;
					pPosition->Y = pBullet->TargetCoords.Y;
					pPosition->Z = pBullet->TargetCoords.Z + this->GetTrajectoryType<BombardTrajectoryType>(pBullet)->Height;

					if (auto pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type))
					{
						auto pThis = pExt->OwnerObject();
						auto pOwner = pThis->Owner ? pThis->Owner->Owner : nullptr;

						for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
						{
							if (auto const pLaserType = LaserTrailTypeClass::Array[idxTrail].get())
								pExt->LaserTrails.push_back(LaserTrailClass { pLaserType, pOwner });
						}
					}
				}
				else
				{
					pPosition->X = pBullet->TargetCoords.X;
					pPosition->Y = pBullet->TargetCoords.Y;
				}
			}

			CoordStruct BulletLocation {
				static_cast<int>(pPosition->X),
				static_cast<int>(pPosition->Y),
				static_cast<int>(pPosition->Z)
			};

			this->RemainingDistance = static_cast<int>(pBullet->TargetCoords.DistanceFrom(BulletLocation) + this->FallSpeed);

			if (!trajType->TurningPointAnims.empty())
			{
				if (auto const pAnim = GameCreate<AnimClass>(trajType->TurningPointAnims[ScenarioClass::Instance->Random.RandomRanged(0, trajType->TurningPointAnims.size() - 1)], BulletLocation))
				{
					pAnim->SetOwnerObject(pBullet->Owner);
					pAnim->Owner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
				}
			}
		}
	}
	else if (!this->FreeFallOnTarget)
	{
		pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
	}
}

bool BombardTrajectory::BulletDetonatePreCheck(BulletClass* pBullet, HouseClass* pOwner, double StraightSpeed)
{
	this->RemainingDistance -= static_cast<int>(StraightSpeed);

	if (this->RemainingDistance < 0)
		return true;

	if (this->RemainingDistance < StraightSpeed)
	{
		pBullet->Velocity *= this->RemainingDistance / StraightSpeed;
		this->RemainingDistance = 0;
	}

	return false;
}

TrajectoryCheckReturnType BombardTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType BombardTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}
