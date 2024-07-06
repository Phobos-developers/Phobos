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
		.Process(this->FallScatterRange, false)
		.Process(this->FallSpeed, false)
		.Process(this->TargetSnapDistance, false)
		.Process(this->FreeFallOnTarget, false)
		.Process(this->NoLaunch, false)
		.Process(this->TurningPointAnim, false)
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
		.Process(this->FallScatterRange)
		.Process(this->FallSpeed)
		.Process(this->TargetSnapDistance)
		.Process(this->FreeFallOnTarget)
		.Process(this->NoLaunch)
		.Process(this->TurningPointAnim)
		;

	return true;
}

void BombardTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->Height.Read(exINI, pSection, "Trajectory.Bombard.Height");
	this->FallPercent.Read(exINI, pSection, "Trajectory.Bombard.FallPercent");
	this->FallPercentShift.Read(exINI, pSection, "Trajectory.Bombard.FallPercentShift");
	this->FallScatterRange.Read(exINI, pSection, "Trajectory.Bombard.FallScatterRange");
	this->FallSpeed.Read(exINI, pSection, "Trajectory.Bombard.FallSpeed");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Bombard.TargetSnapDistance");
	this->FreeFallOnTarget.Read(exINI, pSection, "Trajectory.Bombard.FreeFallOnTarget");
	this->NoLaunch.Read(exINI, pSection, "Trajectory.Bombard.NoLaunch");
	this->TurningPointAnim.Read(exINI, pSection, "Trajectory.Bombard.TurningPointAnim");
}

bool BombardTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->IsFalling)
		.Process(this->Height)
		.Process(this->RemainingDistance)
		.Process(this->FallPercent)
		.Process(this->FallPercentShift)
		.Process(this->FallScatterRange)
		.Process(this->FallSpeed)
		.Process(this->TargetSnapDistance)
		.Process(this->FreeFallOnTarget)
		.Process(this->NoLaunch)
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
		.Process(this->FallPercentShift)
		.Process(this->FallScatterRange)
		.Process(this->FallSpeed)
		.Process(this->TargetSnapDistance)
		.Process(this->FreeFallOnTarget)
		.Process(this->NoLaunch)
		;

	return true;
}

void BombardTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto const pType = this->GetTrajectoryType<BombardTrajectoryType>(pBullet);
	this->Height = pType->Height + pBullet->TargetCoords.Z;

	// use scaling since RandomRanged only support int
	this->FallPercentShift = pType->FallPercentShift;
	double fallPercentShift = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(200 * this->FallPercentShift)) / 100.0;
	this->FallPercent = pType->FallPercent - this->FallPercentShift + fallPercentShift;

	this->FallScatterRange = pType->FallScatterRange;
	this->FallSpeed = pType->FallSpeed ? pType->FallSpeed : this->GetTrajectorySpeed(pBullet);
	this->TargetSnapDistance = pType->TargetSnapDistance;
	this->FreeFallOnTarget = pType->FreeFallOnTarget;
	this->NoLaunch = pType->NoLaunch;

	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

		int ballisticScatter = RulesClass::Instance->BallisticScatter;
		int scatterMax = (int)(pTypeExt->BallisticScatter_Max.Get(Leptons { ballisticScatter }));
		int scatterMin = (int)(pTypeExt->BallisticScatter_Min.Get(Leptons { scatterMax / 2 }));

		double random = ScenarioClass::Instance->Random.RandomRanged(scatterMin, scatterMax);
		double theta = ScenarioClass::Instance->Random.RandomDouble() * Math::TwoPi;

		CoordStruct offset
		{
			static_cast<int>(random * Math::cos(theta)),
			static_cast<int>(random * Math::sin(theta)),
			0
		};
		pBullet->TargetCoords += offset;
	}

	if (!this->NoLaunch)
	{
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
		int scatterRange = static_cast<int>(this->FallScatterRange);
		double angel = ScenarioClass::Instance->Random.RandomDouble() * Math::TwoPi;
		double length = ScenarioClass::Instance->Random.RandomRanged(-scatterRange, scatterRange);
		int scatterX = static_cast<int>(length * Math::cos(angel));
		int scatterY = static_cast<int>(length * Math::sin(angel));

		if (!this->FreeFallOnTarget)
		{
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
			SourceLocation.X = pBullet->TargetCoords.X + scatterX;
			SourceLocation.Y = pBullet->TargetCoords.Y + scatterY;
			pBullet->Limbo();
			pBullet->Unlimbo(SourceLocation, DirType::North);
			pBullet->Velocity.X = 0.0;
			pBullet->Velocity.Y = 0.0;
			pBullet->Velocity.Z = 0.0;
		}

		this->RemainingDistance = static_cast<int>(pBullet->TargetCoords.DistanceFrom(SourceLocation) + this->FallSpeed);
		if (auto turningPointAnim = pType->TurningPointAnim.Get())
		{
			auto const pAnim = GameCreate<AnimClass>(turningPointAnim, SourceLocation);
			pAnim->SetOwnerObject(pBullet->Owner);
			pAnim->Owner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		}
	}
}

bool BombardTrajectory::OnAI(BulletClass* pBullet)
{
	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < 100) // This value maybe adjusted?
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
				pSpeed->X = static_cast<double>(pBullet->TargetCoords.X - pBullet->Location.X - pBullet->Velocity.X);
				pSpeed->Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->Location.Y - pBullet->Velocity.Y);
				pSpeed->Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->Location.Z - pBullet->Velocity.Z);
				(*pSpeed) *= this->FallSpeed / pSpeed->Magnitude();
				pPosition->X = pBullet->Location.X + pBullet->Velocity.X;
				pPosition->Y = pBullet->Location.Y + pBullet->Velocity.Y;
				pPosition->Z = pBullet->Location.Z + pBullet->Velocity.Z;
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
			if (auto const turningPointAnim = trajType->TurningPointAnim.Get())
			{
				auto const pAnim = GameCreate<AnimClass>(turningPointAnim, BulletLocation);
				pAnim->SetOwnerObject(pBullet->Owner);
				pAnim->Owner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
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
