#include "BombardTrajectory.h"

#include <LineTrail.h>
#include <AnimClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>

std::unique_ptr<PhobosTrajectory> BombardTrajectoryType::CreateInstance(BulletClass* pBullet) const
{
	return std::make_unique<BombardTrajectory>(this, pBullet);
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
		.Process(this->FreeFallOnTarget)
		.Process(this->NoLaunch)
		.Process(this->TurningPointAnims)
		;
}

bool BombardTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->ActualTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool BombardTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->ActualTrajectoryType::Save(Stm);
	const_cast<BombardTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void BombardTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->PhobosTrajectoryType::Read(pINI, pSection);
	INI_EX exINI(pINI);
	// LiveShell
	this->RotateCoord.Read(exINI, pSection, "Trajectory.RotateCoord");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.OffsetCoord");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.AxisOfRotation");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.LeadTimeCalculate");
	this->EarlyDetonation.Read(exINI, pSection, "Trajectory.EarlyDetonation");
	this->DetonationHeight.Read(exINI, pSection, "Trajectory.DetonationHeight");
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.TargetSnapDistance");
	// Bombard
	this->Height.Read(exINI, pSection, "Trajectory.Bombard.Height");
	this->Height = Math::max(0.0, this->Height);
	this->FallPercent.Read(exINI, pSection, "Trajectory.Bombard.FallPercent");
	this->FallPercentShift.Read(exINI, pSection, "Trajectory.Bombard.FallPercentShift");
	this->FallScatter_Max.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Max");
	this->FallScatter_Min.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Min");
	this->FallScatter_Linear.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Linear");
	this->FallSpeed.Read(exINI, pSection, "Trajectory.Bombard.FallSpeed");
	this->FreeFallOnTarget.Read(exINI, pSection, "Trajectory.Bombard.FreeFallOnTarget");
	this->NoLaunch.Read(exINI, pSection, "Trajectory.Bombard.NoLaunch");
	this->TurningPointAnims.Read(exINI, pSection, "Trajectory.Bombard.TurningPointAnims");
}

template<typename T>
void BombardTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->Height)
		.Process(this->FallPercent)
		.Process(this->IsFalling)
		.Process(this->ToFalling)
		.Process(this->InitialTargetCoord)
		.Process(this->RotateRadian)
		;
}

bool BombardTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->ActualTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool BombardTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->ActualTrajectory::Save(Stm);
	const_cast<BombardTrajectory*>(this)->Serialize(Stm);
	return true;
}

void BombardTrajectory::OnUnlimbo()
{
	this->ActualTrajectory::OnUnlimbo();
	// Bombard
	const auto pBullet = this->Bullet;
	this->Height += pBullet->TargetCoords.Z;
	// use scaling since RandomRanged only support int
	this->FallPercent += ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(200 * this->Type->FallPercentShift)) / 100.0;
	// Record the initial target coordinates without offset
	this->InitialTargetCoord = pBullet->TargetCoords;
	// Special case: Set the target to the ground
	if (this->Type->DetonationDistance.Get() <= -1e-10)
	{
		const auto pTarget = pBullet->Target;

		if (pTarget->AbstractFlags & AbstractFlags::Foot)
		{
			if (const auto pCell = MapClass::Instance.TryGetCellAt(pTarget->GetCoords()))
			{
				pBullet->Target = pCell;
				pBullet->TargetCoords = pCell->GetCoords();
			}
		}
	}
	// Waiting for launch trigger
	if (!BulletExt::ExtMap.Find(pBullet)->DispersedTrajectory)
		this->OpenFire();
}

bool BombardTrajectory::OnVelocityCheck()
{
	return this->BulletVelocityChange() || this->PhobosTrajectory::OnVelocityCheck();
}

TrajectoryCheckReturnType BombardTrajectory::OnDetonateUpdate(const CoordStruct& position)
{
	if (this->WaitOneFrame)
		return TrajectoryCheckReturnType::SkipGameCheck;
	else if (this->PhobosTrajectory::OnDetonateUpdate(position) == TrajectoryCheckReturnType::Detonate)
		return TrajectoryCheckReturnType::Detonate;

	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(position) < pType->DetonationDistance.Get())
		return TrajectoryCheckReturnType::Detonate;
	// Height
	if (pType->DetonationHeight >= 0 && (pType->EarlyDetonation
		? ((position.Z - pBullet->SourceCoords.Z) > pType->DetonationHeight)
		: (this->IsFalling && (position.Z - pBullet->SourceCoords.Z) < pType->DetonationHeight)))
	{
		return TrajectoryCheckReturnType::Detonate;
	}

	return TrajectoryCheckReturnType::SkipGameCheck;
}

void BombardTrajectory::OpenFire()
{
	const auto pType = this->Type;
	// Wait, or launch immediately?
	if (!pType->NoLaunch || !pType->LeadTimeCalculate.Get(false) || !abstract_cast<FootClass*>(this->Bullet->Target))
		this->FireTrajectory();
	else
		this->WaitOneFrame = 2;

	this->PhobosTrajectory::OpenFire();
}

void BombardTrajectory::FireTrajectory()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	this->CalculateTargetCoords();

	if (!pType->NoLaunch)
	{
		const auto middleLocation = this->CalculateMiddleCoords();
		this->RemainingDistance += static_cast<int>(middleLocation.DistanceFrom(pBullet->SourceCoords));
		this->MovingVelocity = PhobosTrajectory::Coord2Vector(middleLocation - pBullet->SourceCoords);

		if (this->CalculateBulletVelocity(pType->Speed))
			this->ShouldDetonate = true;
		// Rotate the selected angle
		if (std::abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
			this->DisperseBurstSubstitution(this->RotateRadian);
	}
	else
	{
		this->ToFalling = true;
		this->IsFalling = true;
		auto middleLocation = CoordStruct::Empty;

		if (!pType->FreeFallOnTarget)
		{
			middleLocation = this->CalculateMiddleCoords();
			const auto fallSpeed = pType->FallSpeed.Get(pType->Speed);
			this->RemainingDistance += static_cast<int>(pBullet->TargetCoords.DistanceFrom(middleLocation));
			this->MovingVelocity = PhobosTrajectory::Coord2Vector(pBullet->TargetCoords - middleLocation);

			if (this->CalculateBulletVelocity(fallSpeed))
				this->ShouldDetonate = true;
			// Rotate the selected angle
			if (std::abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
				this->DisperseBurstSubstitution(this->RotateRadian);
		}
		else
		{
			middleLocation = CoordStruct { pBullet->TargetCoords.X, pBullet->TargetCoords.Y, static_cast<int>(this->Height) };
			this->RemainingDistance += (middleLocation.Z - pBullet->TargetCoords.Z);
		}

		const auto pExt = BulletExt::ExtMap.Find(pBullet);

		if (pExt->LaserTrails.size())
		{
			for (auto& trail : pExt->LaserTrails)
				trail.LastLocation = middleLocation;
		}
		this->RefreshBulletLineTrail();

		pBullet->SetLocation(middleLocation);
		const auto pTechno = pBullet->Owner;
		const auto pOwner = pTechno ? pTechno->Owner : pExt->FirerHouse;
		AnimExt::CreateRandomAnim(pType->TurningPointAnims, middleLocation, pTechno, pOwner, true);
	}
}

void BombardTrajectory::SetBulletNewTarget(AbstractClass* const pTarget)
{
	const auto pBullet = this->Bullet;
	pBullet->SetTarget(pTarget);
	pBullet->TargetCoords = pTarget->GetCoords();

	if (this->Type->LeadTimeCalculate.Get(false) && !this->IsFalling)
		this->LastTargetCoord = pBullet->TargetCoords;
}

bool BombardTrajectory::CalculateBulletVelocity(const double speed)
{
	if (this->IsFalling && this->Type->FreeFallOnTarget)
	{
		this->MovingSpeed = speed;
		this->MovingVelocity.X = 0;
		this->MovingVelocity.Y = 0;
		this->MovingVelocity.Z = -speed;

		return false;
	}

	return this->PhobosTrajectory::CalculateBulletVelocity(speed);
}

void BombardTrajectory::MultiplyBulletVelocity(const double ratio, const bool shouldDetonate)
{
	this->MovingVelocity *= ratio;
	this->MovingSpeed = this->MovingSpeed * ratio;
	// Only be truly detonated during the descent phase
	if (shouldDetonate && this->IsFalling)
		this->ShouldDetonate = true;
}

CoordStruct BombardTrajectory::CalculateMiddleCoords()
{
	const auto pBullet = this->Bullet;
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

void BombardTrajectory::CalculateTargetCoords()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	auto& target = pBullet->TargetCoords;
	const auto& source = pBullet->SourceCoords;

	if (pType->NoLaunch)
		target += this->CalculateBulletLeadTime();
	// Calculate the orientation of the coordinate system
	this->RotateRadian = this->Get2DOpRadian(((target == source && pBullet->Owner) ? pBullet->Owner->GetCoords() : source), target);
	// Add the fixed offset value
	if (pType->OffsetCoord != CoordStruct::Empty)
		target += this->GetOnlyStableOffsetCoords(this->RotateRadian);
	// Add random offset value
	if (pBullet->Type->Inaccurate)
		target = this->GetInaccurateTargetCoords(target, source.DistanceFrom(target));
}

CoordStruct BombardTrajectory::CalculateBulletLeadTime()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	auto coords = CoordStruct::Empty;

	if (pType->LeadTimeCalculate.Get(false))
	{
		if (const auto pTarget = pBullet->Target)
		{
			const auto target = pTarget->GetCoords();
			const auto& source = pBullet->Location;
			// Solving trigonometric functions
			if (target != this->LastTargetCoord)
			{
				int travelTime = 0;
				const auto extraOffsetCoord = target - this->LastTargetCoord;
				const auto targetSourceCoord = source - target;
				const auto lastSourceCoord = source - this->LastTargetCoord;

				if (pType->FreeFallOnTarget)
				{
					travelTime += static_cast<int>(sqrt(2 * (this->Height - target.Z) / BulletTypeExt::GetAdjustedGravity(pBullet->Type)));
					coords += extraOffsetCoord * (travelTime + 1);
				}
				else if (pType->NoLaunch)
				{
					const auto fallSpeed = pType->FallSpeed.Get(pType->Speed);
					travelTime += static_cast<int>((this->Height - target.Z) / fallSpeed);
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

					const auto fallSpeed = pType->FallSpeed.Get(pType->Speed);
					const auto straightSpeedSquared = fallSpeed * fallSpeed;
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

bool BombardTrajectory::BulletVelocityChange()
{
	const auto pType = this->Type;

	if (!this->IsFalling)
	{
		this->RemainingDistance -= static_cast<int>(this->MovingSpeed);

		if (this->RemainingDistance < this->MovingSpeed)
		{
			const auto pBullet = this->Bullet;

			if (this->ToFalling)
			{
				this->IsFalling = true;
				this->RemainingDistance = 1;
				const auto pTarget = pBullet->Target;
				auto middleLocation = CoordStruct::Empty;

				if (!pType->FreeFallOnTarget)
				{
					if (pType->LeadTimeCalculate.Get(false) && pTarget)
						pBullet->TargetCoords += pTarget->GetCoords() - this->InitialTargetCoord + this->CalculateBulletLeadTime();

					middleLocation = pBullet->Location;
					const auto fallSpeed = pType->FallSpeed.Get(pType->Speed);
					this->MovingVelocity = PhobosTrajectory::Coord2Vector(pBullet->TargetCoords - middleLocation);

					if (this->CalculateBulletVelocity(fallSpeed))
						return true;

					// Rotate the selected angle
					if (std::abs(pType->RotateCoord) > 1e-10 && this->CountOfBurst > 1)
						this->DisperseBurstSubstitution(this->RotateRadian);

					this->RemainingDistance += static_cast<int>(pBullet->TargetCoords.DistanceFrom(middleLocation));
				}
				else
				{
					if (pType->LeadTimeCalculate.Get(false) && pTarget)
						pBullet->TargetCoords += pTarget->GetCoords() - this->InitialTargetCoord + this->CalculateBulletLeadTime();

					middleLocation = pBullet->TargetCoords;
					middleLocation.Z = pBullet->Location.Z;

					this->MovingSpeed = 0;
					this->CalculateBulletVelocity(0);
					this->RemainingDistance += pBullet->Location.Z - MapClass::Instance.GetCellFloorHeight(middleLocation);
				}

				const auto pExt = BulletExt::ExtMap.Find(pBullet);

				if (pExt->LaserTrails.size())
				{
					for (auto& trail : pExt->LaserTrails)
						trail.LastLocation = middleLocation;
				}
				this->RefreshBulletLineTrail();

				pBullet->SetLocation(middleLocation);
				const auto pTechno = pBullet->Owner;
				const auto pOwner = pTechno ? pTechno->Owner : pExt->FirerHouse;
				AnimExt::CreateRandomAnim(pType->TurningPointAnims, middleLocation, pTechno, pOwner, true);
			}
			else
			{
				this->ToFalling = true;
				const auto pTarget = pBullet->Target;

				if (pType->LeadTimeCalculate.Get(false) && pTarget)
					this->LastTargetCoord = pTarget->GetCoords();
			}
		}
	}
	else
	{
		if (pType->FreeFallOnTarget)
			this->CalculateBulletVelocity(-this->MovingVelocity.Z + BulletTypeExt::GetAdjustedGravity(this->Bullet->Type));

		this->RemainingDistance -= static_cast<int>(this->MovingSpeed);
		// Check the remaining travel distance of the bullet
		if (this->RemainingDistance < 0)
			return true;
	}

	return false;
}

void BombardTrajectory::RefreshBulletLineTrail()
{
	const auto pBullet = this->Bullet;

	if (const auto pLineTrailer = pBullet->LineTrailer)
	{
		pLineTrailer->~LineTrail();
		pBullet->LineTrailer = nullptr;
	}

	const auto pType = pBullet->Type;

	if (pType->UseLineTrail)
	{
		if (const auto pLineTrailer = GameCreate<LineTrail>())
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
