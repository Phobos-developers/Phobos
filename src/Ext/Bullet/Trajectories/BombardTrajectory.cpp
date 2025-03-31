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
		.Process(this->FallScatter_Linear)
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
	this->Height = Math::max(0.0, this->Height);
	this->FallPercent.Read(exINI, pSection, "Trajectory.Bombard.FallPercent");
	this->FallPercentShift.Read(exINI, pSection, "Trajectory.Bombard.FallPercentShift");
	this->FallScatter_Max.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Max");
	this->FallScatter_Min.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Min");
	this->FallScatter_Linear.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Linear");
	this->FallSpeed.Read(exINI, pSection, "Trajectory.Bombard.FallSpeed");
	this->FallSpeed = std::abs(this->FallSpeed.Get()) < 1e-10 ? this->Trajectory_Speed.Get() : this->FallSpeed.Get();
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
	const auto pType = this->Type;
	this->Height += pBullet->TargetCoords.Z;
	// use scaling since RandomRanged only support int
	this->FallPercent += ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(200 * pType->FallPercentShift)) / 100.0;

	// Record the initial target coordinates without offset
	this->InitialTargetCoord = pBullet->TargetCoords;
	this->LastTargetCoord = pBullet->TargetCoords;
	pBullet->Velocity = BulletVelocity::Empty;

	// Record some information
	if (const auto pWeapon = pBullet->WeaponType)
		this->CountOfBurst = pWeapon->Burst;

	if (const auto pOwner = pBullet->Owner)
	{
		this->CurrentBurst = pOwner->CurrentBurstIndex;

		if (pType->MirrorCoord && pOwner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	// Wait, or launch immediately?
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

	this->BulletVelocityChange(pBullet);

	// Extra check for trajectory falling
	if (this->IsFalling && !this->Type->FreeFallOnTarget && this->BulletDetonateRemainCheck(pBullet))
		return true;

	return false;
}

void BombardTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	const auto pType = this->Type;
	const auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	const auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= pType->TargetSnapDistance.Get())
	{
		const auto pExt = BulletExt::ExtMap.Find(pBullet);
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
	const auto pType = this->Type;
	this->CalculateTargetCoords(pBullet);

	if (!pType->NoLaunch)
	{
		const auto middleLocation = this->CalculateMiddleCoords(pBullet);

		pBullet->Velocity.X = static_cast<double>(middleLocation.X - pBullet->SourceCoords.X);
		pBullet->Velocity.Y = static_cast<double>(middleLocation.Y - pBullet->SourceCoords.Y);
		pBullet->Velocity.Z = static_cast<double>(middleLocation.Z - pBullet->SourceCoords.Z);
		pBullet->Velocity *= pType->Trajectory_Speed / pBullet->Velocity.Magnitude();

		this->CalculateDisperseBurst(pBullet);
		this->RemainingDistance += static_cast<int>(middleLocation.DistanceFrom(pBullet->SourceCoords) + pType->Trajectory_Speed);
	}
	else
	{
		this->IsFalling = true;
		auto middleLocation = CoordStruct::Empty;

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

		const auto pExt = BulletExt::ExtMap.Find(pBullet);

		if (pExt->LaserTrails.size())
		{
			for (auto& trail : pExt->LaserTrails)
				trail.LastLocation = middleLocation;
		}
		this->RefreshBulletLineTrail(pBullet);

		pBullet->SetLocation(middleLocation);
		const auto pTechno = pBullet->Owner;
		const auto pOwner = pTechno ? pTechno->Owner : pExt->FirerHouse;
		AnimExt::CreateRandomAnim(pType->TurningPointAnims, middleLocation, pTechno, pOwner, true);
	}
}

CoordStruct BombardTrajectory::CalculateMiddleCoords(BulletClass* pBullet)
{
	const auto pType = this->Type;
	const auto length = ScenarioClass::Instance->Random.RandomRanged(pType->FallScatter_Min.Get(), pType->FallScatter_Max.Get());
	const auto vectorX = (pBullet->TargetCoords.X - pBullet->SourceCoords.X) * this->FallPercent;
	const auto vectorY = (pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * this->FallPercent;
	double scatterX = 0.0;
	double scatterY = 0.0;

	if (!pType->FallScatter_Linear)
	{
		const auto angel = ScenarioClass::Instance->Random.RandomDouble() * Math::TwoPi;
		scatterX = length * Math::cos(angel);
		scatterY = length * Math::sin(angel);
	}
	else
	{
		const auto vectorModule = sqrt(vectorX * vectorX + vectorY * vectorY);
		scatterX = vectorY / vectorModule * length;
		scatterY = -(vectorX / vectorModule * length);

		if (ScenarioClass::Instance->Random.RandomRanged(0, 1))
		{
			scatterX = -scatterX;
			scatterY = -scatterY;
		}
	}

	return CoordStruct
	{
		pBullet->SourceCoords.X + static_cast<int>(vectorX + scatterX),
		pBullet->SourceCoords.Y + static_cast<int>(vectorY + scatterY),
		static_cast<int>(this->Height)
	};
}

void BombardTrajectory::CalculateTargetCoords(BulletClass* pBullet)
{
	const auto pType = this->Type;
	auto theTargetCoords = pBullet->TargetCoords;
	const auto theSourceCoords = pBullet->SourceCoords;

	if (pType->NoLaunch)
		theTargetCoords += this->CalculateBulletLeadTime(pBullet);

	pBullet->TargetCoords = theTargetCoords;

	// Calculate the orientation of the coordinate system
	if (!pType->LeadTimeCalculate && theTargetCoords == theSourceCoords && pBullet->Owner) //For disperse.
	{
		const auto theOwnerCoords = pBullet->Owner->GetCoords();
		this->RotateAngle = Math::atan2(theTargetCoords.Y - theOwnerCoords.Y , theTargetCoords.X - theOwnerCoords.X);
	}
	else
	{
		this->RotateAngle = Math::atan2(theTargetCoords.Y - theSourceCoords.Y , theTargetCoords.X - theSourceCoords.X);
	}

	// Add the fixed offset value
	if (this->OffsetCoord != CoordStruct::Empty)
	{
		pBullet->TargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(this->RotateAngle) + this->OffsetCoord.Y * Math::sin(this->RotateAngle));
		pBullet->TargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(this->RotateAngle) - this->OffsetCoord.Y * Math::cos(this->RotateAngle));
		pBullet->TargetCoords.Z += this->OffsetCoord.Z;
	}

	// Add random offset value
	if (pBullet->Type->Inaccurate)
	{
		const auto pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		const auto offsetMult = 0.0004 * pBullet->SourceCoords.DistanceFrom(pBullet->TargetCoords);
		const auto offsetMin = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Min.Get(Leptons(0)));
		const auto offsetMax = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter)));
		const auto offsetDistance = ScenarioClass::Instance->Random.RandomRanged(offsetMin, offsetMax);
		pBullet->TargetCoords = MapClass::GetRandomCoordsNear(pBullet->TargetCoords, offsetDistance, false);
	}
}

CoordStruct BombardTrajectory::CalculateBulletLeadTime(BulletClass* pBullet)
{
	const auto pType = this->Type;
	auto coords = CoordStruct::Empty;

	if (pType->LeadTimeCalculate)
	{
		if (const auto pTarget = pBullet->Target)
		{
			const auto theTargetCoords = pTarget->GetCoords();
			const auto theSourceCoords = pBullet->Location;

			// Solving trigonometric functions
			if (theTargetCoords != this->LastTargetCoord)
			{
				int travelTime = 0;
				const auto extraOffsetCoord = theTargetCoords - this->LastTargetCoord;
				const auto targetSourceCoord = theSourceCoords - theTargetCoords;
				const auto lastSourceCoord = theSourceCoords - this->LastTargetCoord;

				if (pType->FreeFallOnTarget)
				{
					travelTime += static_cast<int>(sqrt(2 * (this->Height - theTargetCoords.Z) / BulletTypeExt::GetAdjustedGravity(pBullet->Type)));
					coords += extraOffsetCoord * (travelTime + 1);
				}
				else if (pType->NoLaunch)
				{
					travelTime += static_cast<int>((this->Height - theTargetCoords.Z) / pType->FallSpeed);
					coords += extraOffsetCoord * (travelTime + 1);
				}
				else
				{
					const auto theDistanceSquared = targetSourceCoord.MagnitudeSquared();
					const auto targetSpeedSquared = extraOffsetCoord.MagnitudeSquared();
					const auto targetSpeed = sqrt(targetSpeedSquared);

					const auto crossFactor = lastSourceCoord.CrossProduct(targetSourceCoord).MagnitudeSquared();
					const auto verticalDistanceSquared = crossFactor / targetSpeedSquared;

					const auto horizonDistanceSquared = theDistanceSquared - verticalDistanceSquared;
					const auto horizonDistance = sqrt(horizonDistanceSquared);

					const auto straightSpeedSquared = pType->FallSpeed * pType->FallSpeed;
					const auto baseFactor = straightSpeedSquared - targetSpeedSquared;
					const auto squareFactor = baseFactor * verticalDistanceSquared + straightSpeedSquared * horizonDistanceSquared;

					// Is there a solution?
					if (squareFactor > 1e-10)
					{
						const auto minusFactor = -(horizonDistance * targetSpeed);

						if (std::abs(baseFactor) < 1e-10)
						{
							travelTime = std::abs(horizonDistance) > 1e-10 ? (static_cast<int>(theDistanceSquared / (2 * horizonDistance * targetSpeed)) + 1) : 0;
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
	const auto pType = this->Type;

	if (!this->UseDisperseBurst && std::abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
	{
		const auto axis = pType->AxisOfRotation.Get();

		BulletVelocity rotationAxis
		{
			axis.X * Math::cos(this->RotateAngle) + axis.Y * Math::sin(this->RotateAngle),
			axis.X * Math::sin(this->RotateAngle) - axis.Y * Math::cos(this->RotateAngle),
			static_cast<double>(axis.Z)
		};

		const auto rotationAxisLengthSquared = rotationAxis.MagnitudeSquared();

		if (std::abs(rotationAxisLengthSquared) > 1e-10)
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

			const auto cosRotate = Math::cos(extraRotate);
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

bool BombardTrajectory::BulletDetonatePreCheck(BulletClass* pBullet)
{
	const auto pType = this->Type;

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
		if (MapClass::Instance.GetCellFloorHeight(pBullet->Location) >= (pBullet->Location.Z + 15))
			return true;
	}

	return false;
}

bool BombardTrajectory::BulletDetonateRemainCheck(BulletClass* pBullet)
{
	const auto pType = this->Type;
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
	const auto pType = this->Type;

	if (!this->IsFalling)
	{
		this->RemainingDistance -= static_cast<int>(pType->Trajectory_Speed);

		if (this->RemainingDistance < static_cast<int>(pType->Trajectory_Speed))
		{
			if (this->ToFalling)
			{
				this->IsFalling = true;
				this->RemainingDistance = 1;
				const auto pTarget = pBullet->Target;
				auto middleLocation = CoordStruct::Empty;

				if (!pType->FreeFallOnTarget)
				{
					if (pType->LeadTimeCalculate && pTarget)
						pBullet->TargetCoords += pTarget->GetCoords() - this->InitialTargetCoord + this->CalculateBulletLeadTime(pBullet);

					middleLocation = pBullet->Location;
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

				const auto pExt = BulletExt::ExtMap.Find(pBullet);

				if (pExt->LaserTrails.size())
				{
					for (auto& trail : pExt->LaserTrails)
						trail.LastLocation = middleLocation;
				}
				this->RefreshBulletLineTrail(pBullet);

				pBullet->SetLocation(middleLocation);
				const auto pTechno = pBullet->Owner;
				const auto pOwner = pTechno ? pTechno->Owner : pExt->FirerHouse;
				AnimExt::CreateRandomAnim(pType->TurningPointAnims, middleLocation, pTechno, pOwner, true);
			}
			else
			{
				this->ToFalling = true;
				const auto pTarget = pBullet->Target;

				if (pType->LeadTimeCalculate && pTarget)
					this->LastTargetCoord = pTarget->GetCoords();

				pBullet->Velocity *= this->RemainingDistance / pType->Trajectory_Speed;
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
	if (const auto pLineTrailer = pBullet->LineTrailer)
	{
		pLineTrailer->~LineTrail();
		pBullet->LineTrailer = nullptr;
	}

	const auto pType = pBullet->Type;

	if (pType->UseLineTrail)
	{
		const auto pLineTrailer = GameCreate<LineTrail>();
		pBullet->LineTrailer = pLineTrailer;

		if (RulesClass::Instance->LineTrailColorOverride != ColorStruct { 0, 0, 0 })
			pLineTrailer->Color = RulesClass::Instance->LineTrailColorOverride;
		else
			pLineTrailer->Color = pType->LineTrailColor;

		pLineTrailer->SetDecrement(pType->LineTrailColorDecrement);
		pLineTrailer->Owner = pBullet;
	}
}
