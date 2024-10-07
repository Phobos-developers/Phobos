#include "BombardTrajectory.h"
#include "Memory.h"

#include <LineTrail.h>
#include <AnimClass.h>
#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>

std::unique_ptr<PhobosTrajectory> BombardTrajectoryType::CreateInstance() const
{
	return std::make_unique<BombardTrajectory>(this);
}

template<typename T>
void BombardTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->Height)
		.Process(this->FallPercent)
		.Process(this->FallPercentShift)
		.Process(this->FallScatter_Max)
		.Process(this->FallScatter_Min)
		.Process(this->FallSpeed)
		.Process(this->DetonationDistance)
		.Process(this->DetonationHeight)
		.Process(this->EarlyDetonation)
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
		.Process(this->SubjectToGround)
		;
}

bool BombardTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool BombardTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<BombardTrajectoryType*>(this)->Serialize(Stm);
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

	if (abs(this->FallSpeed.Get()) < 1e-10)
		this->FallSpeed = this->Trajectory_Speed;

	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Bombard.DetonationDistance");
	this->DetonationHeight.Read(exINI, pSection, "Trajectory.Bombard.DetonationHeight");
	this->EarlyDetonation.Read(exINI, pSection, "Trajectory.Bombard.EarlyDetonation");
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
	this->SubjectToGround.Read(exINI, pSection, "Trajectory.Bombard.SubjectToGround");
}

template<typename T>
void BombardTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->Height)
		.Process(this->FallPercent)
		.Process(this->OffsetCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->IsFalling)
		.Process(this->ToFalling)
		.Process(this->RemainingDistance)
		.Process(this->LastTargetCoord)
		.Process(this->InitialTargetCoord)
		.Process(this->CountOfBurst)
		.Process(this->CurrentBurst)
		.Process(this->RotateAngle)
		.Process(this->WaitOneFrame)
		;
}

bool BombardTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Serialize(Stm);
	return true;
}

bool BombardTrajectory::Save(PhobosStreamWriter& Stm) const
{
	const_cast<BombardTrajectory*>(this)->Serialize(Stm);
	return true;
}

void BombardTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	const BombardTrajectoryType* const pType = this->Type;
	this->Height += pBullet->TargetCoords.Z;
	// use scaling since RandomRanged only support int
	this->FallPercent += ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(200 * pType->FallPercentShift)) / 100.0;
	this->InitialTargetCoord = pBullet->TargetCoords;
	this->LastTargetCoord = pBullet->TargetCoords;
	pBullet->Velocity = BulletVelocity::Empty;

	if (WeaponTypeClass* const pWeapon = pBullet->WeaponType)
		this->CountOfBurst = pWeapon->Burst;

	if (TechnoClass* const pOwner = pBullet->Owner)
	{
		this->CurrentBurst = pOwner->CurrentBurstIndex;

		if (pType->MirrorCoord && pOwner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	if (!pType->NoLaunch || !pType->LeadTimeCalculate || !abstract_cast<FootClass*>(pBullet->Target))
		this->PrepareForOpenFire(pBullet);
	else
		this->WaitOneFrame = 2;
}

bool BombardTrajectory::OnAI(BulletClass* pBullet)
{
	if (this->WaitOneFrame && this->BulletPrepareCheck(pBullet))
		return false;

	if (this->BulletDetonatePreCheck(pBullet))
		return true;

	// Extra check for trajectory falling
	auto const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

	if (this->IsFalling && !this->Type->FreeFallOnTarget && this->BulletDetonateRemainCheck(pBullet, pOwner))
		return true;

	this->BulletVelocityChange(pBullet);

	return false;
}

void BombardTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	const BombardTrajectoryType* const pType = this->Type;
	auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= pType->TargetSnapDistance.Get())
	{
		auto const pExt = BulletExt::ExtMap.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

void BombardTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType BombardTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

TrajectoryCheckReturnType BombardTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

void BombardTrajectory::PrepareForOpenFire(BulletClass* pBullet)
{
	const BombardTrajectoryType* const pType = this->Type;
	this->CalculateTargetCoords(pBullet);

	if (!pType->NoLaunch)
	{
		const CoordStruct middleLocation = this->CalculateMiddleCoords(pBullet);

		pBullet->Velocity.X = static_cast<double>(middleLocation.X - pBullet->SourceCoords.X);
		pBullet->Velocity.Y = static_cast<double>(middleLocation.Y - pBullet->SourceCoords.Y);
		pBullet->Velocity.Z = static_cast<double>(middleLocation.Z - pBullet->SourceCoords.Z);
		pBullet->Velocity *= pType->Trajectory_Speed / pBullet->Velocity.Magnitude();

		this->CalculateDisperseBurst(pBullet);
	}
	else
	{
		this->IsFalling = true;
		CoordStruct middleLocation = CoordStruct::Empty;

		if (!pType->FreeFallOnTarget)
		{
			middleLocation = this->CalculateMiddleCoords(pBullet);

			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - middleLocation.X);
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - middleLocation.Y);
			pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - middleLocation.Z);
			pBullet->Velocity *= pType->FallSpeed / pBullet->Velocity.Magnitude();

			this->CalculateDisperseBurst(pBullet);
			this->RemainingDistance += static_cast<int>(pBullet->TargetCoords.DistanceFrom(middleLocation) + pType->FallSpeed);
		}
		else
		{
			middleLocation = CoordStruct { pBullet->TargetCoords.X, pBullet->TargetCoords.Y, static_cast<int>(this->Height) };
		}

		auto const pExt = BulletExt::ExtMap.Find(pBullet);

		if (pExt->LaserTrails.size())
		{
			for (auto& trail : pExt->LaserTrails)
				trail.LastLocation = middleLocation;
		}
		this->RefreshBulletLineTrail(pBullet);

		pBullet->SetLocation(middleLocation);
		HouseClass* const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		AnimExt::CreateRandomAnim(pType->TurningPointAnims, middleLocation, pBullet->Owner, pOwner, true);
	}
}

CoordStruct BombardTrajectory::CalculateMiddleCoords(BulletClass* pBullet)
{
	const BombardTrajectoryType* const pType = this->Type;
	const double angel = ScenarioClass::Instance->Random.RandomDouble() * Math::TwoPi;
	const double length = ScenarioClass::Instance->Random.RandomRanged(pType->FallScatter_Min.Get(), pType->FallScatter_Max.Get());
	const int scatterX = static_cast<int>(length * Math::cos(angel));
	const int scatterY = static_cast<int>(length * Math::sin(angel));

	return CoordStruct
	{
		pBullet->SourceCoords.X + static_cast<int>((pBullet->TargetCoords.X - pBullet->SourceCoords.X) * this->FallPercent) + scatterX,
		pBullet->SourceCoords.Y + static_cast<int>((pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * this->FallPercent) + scatterY,
		static_cast<int>(this->Height)
	};
}

void BombardTrajectory::CalculateTargetCoords(BulletClass* pBullet)
{
	const BombardTrajectoryType* const pType = this->Type;
	CoordStruct theTargetCoords = pBullet->TargetCoords;
	CoordStruct theSourceCoords = pBullet->SourceCoords;

	if (pType->NoLaunch)
		theTargetCoords += this->CalculateBulletLeadTime(pBullet);

	pBullet->TargetCoords = theTargetCoords;

	if (!pType->LeadTimeCalculate && theTargetCoords == theSourceCoords && pBullet->Owner) //For disperse.
	{
		const CoordStruct theOwnerCoords = pBullet->Owner->GetCoords();
		this->RotateAngle = Math::atan2(theTargetCoords.Y - theOwnerCoords.Y, theTargetCoords.X - theOwnerCoords.X);
	}
	else
	{
		this->RotateAngle = Math::atan2(theTargetCoords.Y - theSourceCoords.Y, theTargetCoords.X - theSourceCoords.X);
	}

	if (this->OffsetCoord != CoordStruct::Empty)
	{
		pBullet->TargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(this->RotateAngle) + this->OffsetCoord.Y * Math::sin(this->RotateAngle));
		pBullet->TargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(this->RotateAngle) - this->OffsetCoord.Y * Math::cos(this->RotateAngle));
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

CoordStruct BombardTrajectory::CalculateBulletLeadTime(BulletClass* pBullet)
{
	const BombardTrajectoryType* const pType = this->Type;
	CoordStruct coords = CoordStruct::Empty;

	if (pType->LeadTimeCalculate)
	{
		if (const AbstractClass* const pTarget = pBullet->Target)
		{
			const CoordStruct theTargetCoords = pTarget->GetCoords();
			const CoordStruct theSourceCoords = pBullet->Location;

			if (theTargetCoords != this->LastTargetCoord)
			{
				int travelTime = 0;
				const CoordStruct extraOffsetCoord = theTargetCoords - this->LastTargetCoord;
				const CoordStruct targetSourceCoord = theSourceCoords - theTargetCoords;
				const CoordStruct lastSourceCoord = theSourceCoords - this->LastTargetCoord;

				if (pType->FreeFallOnTarget)
				{
					travelTime += static_cast<int>(sqrt(2 * (this->Height - theTargetCoords.Z) / BulletTypeExt::GetAdjustedGravity(pBullet->Type)));
					coords += extraOffsetCoord * (travelTime + 1);
				}
				else
				{
					const double theDistanceSquared = targetSourceCoord.MagnitudeSquared();
					const double targetSpeedSquared = extraOffsetCoord.MagnitudeSquared();
					const double targetSpeed = sqrt(targetSpeedSquared);

					const double crossFactor = lastSourceCoord.CrossProduct(targetSourceCoord).MagnitudeSquared();
					const double verticalDistanceSquared = crossFactor / targetSpeedSquared;

					const double horizonDistanceSquared = theDistanceSquared - verticalDistanceSquared;
					const double horizonDistance = sqrt(horizonDistanceSquared);

					const double straightSpeed = pType->FreeFallOnTarget ? pType->Trajectory_Speed : pType->FallSpeed;
					const double straightSpeedSquared = straightSpeed * straightSpeed;

					const double baseFactor = straightSpeedSquared - targetSpeedSquared;
					const double squareFactor = baseFactor * verticalDistanceSquared + straightSpeedSquared * horizonDistanceSquared;

					if (squareFactor > 1e-10)
					{
						const double minusFactor = -(horizonDistance * targetSpeed);

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

						coords += extraOffsetCoord * travelTime;
					}
				}
			}
		}
	}

	return coords;
}

void BombardTrajectory::CalculateDisperseBurst(BulletClass* pBullet)
{
	const BombardTrajectoryType* const pType = this->Type;

	if (!this->UseDisperseBurst && abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
	{
		const CoordStruct axis = pType->AxisOfRotation;

		BulletVelocity rotationAxis
		{
			axis.X * Math::cos(this->RotateAngle) + axis.Y * Math::sin(this->RotateAngle),
			axis.X * Math::sin(this->RotateAngle) - axis.Y * Math::cos(this->RotateAngle),
			static_cast<double>(axis.Z)
		};

		const double rotationAxisLengthSquared = rotationAxis.MagnitudeSquared();

		if (abs(rotationAxisLengthSquared) > 1e-10)
		{
			double extraRotate = 0.0;
			rotationAxis *= 1 / sqrt(rotationAxisLengthSquared);

			if (pType->MirrorCoord)
			{
				if (this->CurrentBurst % 2 == 1)
					rotationAxis *= -1;

				extraRotate = Math::Pi * (pType->RotateCoord * ((this->CurrentBurst / 2) / (this->CountOfBurst - 1.0) - 0.5)) / (this->IsFalling ? 90 : 180);
			}
			else
			{
				extraRotate = Math::Pi * (pType->RotateCoord * (this->CurrentBurst / (this->CountOfBurst - 1.0) - 0.5)) / (this->IsFalling ? 90 : 180);
			}

			const double cosRotate = Math::cos(extraRotate);
			pBullet->Velocity = (pBullet->Velocity * cosRotate) + (rotationAxis * ((1 - cosRotate) * (pBullet->Velocity * rotationAxis))) + (rotationAxis.CrossProduct(pBullet->Velocity) * Math::sin(extraRotate));
		}
	}
}

bool BombardTrajectory::BulletPrepareCheck(BulletClass* pBullet)
{
	// The time between bullets' Unlimbo() and Update() is completely uncertain.
	// Technos will update its location after firing, which may result in inaccurate
	// target position recorded by the LastTargetCoord in Unlimbo(). Therefore, it's
	// necessary to record the position during the first Update(). - CrimRecya
	if (this->WaitOneFrame == 2)
	{
		if (const AbstractClass* const pTarget = pBullet->Target)
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

bool BombardTrajectory::BulletDetonatePreCheck(BulletClass* pBullet)
{
	const BombardTrajectoryType* const pType = this->Type;

	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < pType->DetonationDistance.Get())
		return true;

	// Height
	if (pType->DetonationHeight >= 0)
	{
		if (pType->EarlyDetonation && (pBullet->Location.Z - pBullet->SourceCoords.Z) > pType->DetonationHeight)
			return true;
		else if (this->IsFalling && (pBullet->Location.Z - pBullet->SourceCoords.Z) < pType->DetonationHeight)
			return true;
	}

	// Ground, must be checked when free fall
	if (pType->SubjectToGround || (this->IsFalling && pType->FreeFallOnTarget))
	{
		if (MapClass::Instance->GetCellFloorHeight(pBullet->Location) >= (pBullet->Location.Z + 15))
			return true;
	}

	return false;
}

bool BombardTrajectory::BulletDetonateRemainCheck(BulletClass* pBullet, HouseClass* pOwner)
{
	const BombardTrajectoryType* const pType = this->Type;
	this->RemainingDistance -= static_cast<int>(pType->FallSpeed);

	if (this->RemainingDistance < 0)
		return true;

	if (this->RemainingDistance < pType->FallSpeed)
	{
		pBullet->Velocity *= this->RemainingDistance / pType->FallSpeed;
		this->RemainingDistance = 0;
	}

	return false;
}

void BombardTrajectory::BulletVelocityChange(BulletClass* pBullet)
{
	const BombardTrajectoryType* const pType = this->Type;

	if (!this->IsFalling)
	{
		if (pBullet->Location.Z + pBullet->Velocity.Z >= this->Height)
		{
			if (this->ToFalling)
			{
				this->IsFalling = true;
				const AbstractClass* const pTarget = pBullet->Target;
				CoordStruct middleLocation = CoordStruct::Empty;

				if (!pType->FreeFallOnTarget)
				{
					middleLocation = CoordStruct
					{
						static_cast<int>(pBullet->Location.X + pBullet->Velocity.X),
						static_cast<int>(pBullet->Location.Y + pBullet->Velocity.Y),
						static_cast<int>(pBullet->Location.Z + pBullet->Velocity.Z)
					};

					if (pType->LeadTimeCalculate && pTarget)
						pBullet->TargetCoords += pTarget->GetCoords() - this->InitialTargetCoord + this->CalculateBulletLeadTime(pBullet);

					pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - middleLocation.X);
					pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - middleLocation.Y);
					pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - middleLocation.Z);
					pBullet->Velocity *= pType->FallSpeed / pBullet->Velocity.Magnitude();

					this->CalculateDisperseBurst(pBullet);
					this->RemainingDistance += static_cast<int>(pBullet->TargetCoords.DistanceFrom(middleLocation) + pType->FallSpeed);
				}
				else
				{
					if (pType->LeadTimeCalculate && pTarget)
						pBullet->TargetCoords += pTarget->GetCoords() - this->InitialTargetCoord + this->CalculateBulletLeadTime(pBullet);

					middleLocation = pBullet->TargetCoords;
					middleLocation.Z = pBullet->Location.Z;

					pBullet->Velocity = BulletVelocity::Empty;
				}

				auto const pExt = BulletExt::ExtMap.Find(pBullet);

				if (pExt->LaserTrails.size())
				{
					for (auto& trail : pExt->LaserTrails)
						trail.LastLocation = middleLocation;
				}

				this->RefreshBulletLineTrail(pBullet);
				pBullet->SetLocation(middleLocation);
				TechnoClass* const pTechno = pBullet->Owner;
				AnimExt::CreateRandomAnim(pType->TurningPointAnims, middleLocation, pTechno, pTechno ? pTechno->Owner : pExt->FirerHouse, true);
			}
			else
			{
				this->ToFalling = true;
				const AbstractClass* const pTarget = pBullet->Target;

				if (pType->LeadTimeCalculate && pTarget)
					this->LastTargetCoord = pTarget->GetCoords();

				pBullet->Velocity *= abs((this->Height - pBullet->Location.Z) / pBullet->Velocity.Z);
			}
		}
	}
	else if (pType->FreeFallOnTarget)
	{
		pBullet->Velocity.Z -= BulletTypeExt::GetAdjustedGravity(pBullet->Type);
	}
}

void BombardTrajectory::RefreshBulletLineTrail(BulletClass* pBullet)
{
	if (LineTrail* const pLineTrailer = pBullet->LineTrailer)
	{
		pLineTrailer->~LineTrail();
		pBullet->LineTrailer = nullptr;
	}

	BulletTypeClass* const pType = pBullet->Type;

	if (pType->UseLineTrail)
	{
		if (LineTrail* const pLineTrailer = GameCreate<LineTrail>())
		{
			pBullet->LineTrailer = pLineTrailer;

			if (RulesClass::Instance->LineTrailColorOverride != ColorStruct { 0, 0, 0 })
				pLineTrailer->Color = RulesClass::Instance->LineTrailColorOverride;
			else
				pLineTrailer->Color = pType->LineTrailColor;

			pLineTrailer->SetDecrement(pType->LineTrailColorDecrement);
			pLineTrailer->Owner = pBullet;
		}
	}
}
