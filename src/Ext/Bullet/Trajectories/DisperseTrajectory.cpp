#include "DisperseTrajectory.h"
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include "Ext/WeaponType/Body.h"
#include <LaserDrawClass.h>
#include <EBolt.h>
#include <RadBeam.h>
#include <ParticleSystemClass.h>
#include <ScenarioClass.h>
#include <Utilities/Helpers.Alex.h>

bool DisperseTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);

	Stm
		.Process(this->UniqueCurve, false)
		.Process(this->PreAimCoord, false)
		.Process(this->LaunchSpeed, false)
		.Process(this->Acceleration, false)
		.Process(this->ROT, false)
		.Process(this->LockDirection, false)
		.Process(this->CruiseEnable, false)
		.Process(this->CruiseUnableRange, false)
		.Process(this->LeadTimeCalculate, false)
		.Process(this->TargetSnapDistance, false)
		.Process(this->RetargetAllies, false)
		.Process(this->RetargetRadius, false)
		.Process(this->SuicideAboveRange, false)
		.Process(this->SuicideIfNoWeapon, false)
		.Process(this->Weapon, false)
		.Process(this->WeaponBurst, false)
		.Process(this->WeaponCount, false)
		.Process(this->WeaponDelay, false)
		.Process(this->WeaponTimer, false)
		.Process(this->WeaponScope, false)
		.Process(this->WeaponRetarget, false)
		.Process(this->WeaponLocation, false)
		.Process(this->WeaponTendency, false)
		.Process(this->WeaponToAllies, false)
		;

	return true;
}

bool DisperseTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);

	Stm
		.Process(this->UniqueCurve)
		.Process(this->PreAimCoord)
		.Process(this->LaunchSpeed)
		.Process(this->Acceleration)
		.Process(this->ROT)
		.Process(this->LockDirection)
		.Process(this->CruiseEnable)
		.Process(this->CruiseUnableRange)
		.Process(this->LeadTimeCalculate)
		.Process(this->TargetSnapDistance)
		.Process(this->RetargetAllies)
		.Process(this->RetargetRadius)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		.Process(this->Weapon)
		.Process(this->WeaponBurst)
		.Process(this->WeaponCount)
		.Process(this->WeaponDelay)
		.Process(this->WeaponTimer)
		.Process(this->WeaponScope)
		.Process(this->WeaponRetarget)
		.Process(this->WeaponLocation)
		.Process(this->WeaponTendency)
		.Process(this->WeaponToAllies)
		;

	return true;
}

void DisperseTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	this->UniqueCurve.Read(exINI, pSection, "Trajectory.Disperse.UniqueCurve");
	this->PreAimCoord.Read(exINI, pSection, "Trajectory.Disperse.PreAimCoord");
	this->LaunchSpeed.Read(exINI, pSection, "Trajectory.Disperse.LaunchSpeed");
	this->Acceleration.Read(exINI, pSection, "Trajectory.Disperse.Acceleration");
	this->ROT.Read(exINI, pSection, "Trajectory.Disperse.ROT");
	this->LockDirection.Read(exINI, pSection, "Trajectory.Disperse.LockDirection");
	this->CruiseEnable.Read(exINI, pSection, "Trajectory.Disperse.CruiseEnable");
	this->CruiseUnableRange.Read(exINI, pSection, "Trajectory.Disperse.CruiseUnableRange");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.Disperse.LeadTimeCalculate");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Disperse.TargetSnapDistance");
	this->RetargetAllies.Read(exINI, pSection, "Trajectory.Disperse.RetargetAllies");
	this->RetargetRadius.Read(exINI, pSection, "Trajectory.Disperse.RetargetRadius");
	this->SuicideAboveRange.Read(exINI, pSection, "Trajectory.Disperse.SuicideAboveRange");
	this->SuicideIfNoWeapon.Read(exINI, pSection, "Trajectory.Disperse.SuicideIfNoWeapon");
	this->Weapon.Read(exINI, pSection, "Trajectory.Disperse.Weapons");
	this->WeaponBurst.Read(exINI, pSection, "Trajectory.Disperse.WeaponBurst");
	this->WeaponCount.Read(exINI, pSection, "Trajectory.Disperse.WeaponCount");
	this->WeaponDelay.Read(exINI, pSection, "Trajectory.Disperse.WeaponDelay");
	this->WeaponTimer.Read(exINI, pSection, "Trajectory.Disperse.WeaponTimer");
	this->WeaponScope.Read(exINI, pSection, "Trajectory.Disperse.WeaponScope");
	this->WeaponRetarget.Read(exINI, pSection, "Trajectory.Disperse.WeaponRetarget");
	this->WeaponLocation.Read(exINI, pSection, "Trajectory.Disperse.WeaponLocation");
	this->WeaponTendency.Read(exINI, pSection, "Trajectory.Disperse.WeaponTendency");
	this->WeaponToAllies.Read(exINI, pSection, "Trajectory.Disperse.WeaponToAllies");
}

bool DisperseTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->UniqueCurve)
		.Process(this->PreAimCoord)
		.Process(this->LaunchSpeed)
		.Process(this->Acceleration)
		.Process(this->ROT)
		.Process(this->LockDirection)
		.Process(this->CruiseEnable)
		.Process(this->CruiseUnableRange)
		.Process(this->LeadTimeCalculate)
		.Process(this->TargetSnapDistance)
		.Process(this->RetargetAllies)
		.Process(this->RetargetRadius)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		.Process(this->Weapon)
		.Process(this->WeaponBurst)
		.Process(this->WeaponCount)
		.Process(this->WeaponDelay)
		.Process(this->WeaponTimer)
		.Process(this->WeaponScope)
		.Process(this->WeaponRetarget)
		.Process(this->WeaponLocation)
		.Process(this->WeaponTendency)
		.Process(this->WeaponToAllies)
		.Process(this->InStraight)
		.Process(this->TargetInAir)
		.Process(this->FinalHeight)
		.Process(this->LastTargetCoord)
		.Process(this->LastReviseMult)
		.Process(this->FirepowerMult)
		;

	return true;
}

bool DisperseTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->UniqueCurve)
		.Process(this->PreAimCoord)
		.Process(this->LaunchSpeed)
		.Process(this->Acceleration)
		.Process(this->ROT)
		.Process(this->LockDirection)
		.Process(this->CruiseEnable)
		.Process(this->CruiseUnableRange)
		.Process(this->LeadTimeCalculate)
		.Process(this->TargetSnapDistance)
		.Process(this->RetargetAllies)
		.Process(this->RetargetRadius)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		.Process(this->Weapon)
		.Process(this->WeaponBurst)
		.Process(this->WeaponCount)
		.Process(this->WeaponDelay)
		.Process(this->WeaponTimer)
		.Process(this->WeaponScope)
		.Process(this->WeaponRetarget)
		.Process(this->WeaponLocation)
		.Process(this->WeaponTendency)
		.Process(this->WeaponToAllies)
		.Process(this->InStraight)
		.Process(this->TargetInAir)
		.Process(this->FinalHeight)
		.Process(this->LastTargetCoord)
		.Process(this->LastReviseMult)
		.Process(this->FirepowerMult)
		;

	return true;
}

void DisperseTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto const pType = this->GetTrajectoryType<DisperseTrajectoryType>(pBullet);

	this->UniqueCurve = pType->UniqueCurve;
	this->PreAimCoord = pType->PreAimCoord;
	this->LaunchSpeed = pType->LaunchSpeed;
	this->Acceleration = pType->Acceleration > 0.1 ? pType->Acceleration : 0.1;
	this->ROT = pType->ROT > 0.1 ? pType->ROT : 0.1;
	this->LockDirection = pType->LockDirection;
	this->CruiseEnable = pType->CruiseEnable;
	this->CruiseUnableRange = pType->CruiseUnableRange;
	this->LeadTimeCalculate = pType->LeadTimeCalculate;
	this->TargetSnapDistance = pType->TargetSnapDistance;
	this->RetargetAllies = pType->RetargetAllies;
	this->RetargetRadius = pType->RetargetRadius;
	this->SuicideAboveRange = pType->SuicideAboveRange * 256;
	this->SuicideIfNoWeapon = pType->SuicideIfNoWeapon;
	this->Weapon = pType->Weapon;
	this->WeaponBurst = pType->WeaponBurst;
	this->WeaponCount = pType->WeaponCount;
	this->WeaponDelay = pType->WeaponDelay;
	this->WeaponTimer = pType->WeaponTimer;
	this->WeaponScope = pType->WeaponScope;
	this->WeaponRetarget = pType->WeaponRetarget;
	this->WeaponLocation = pType->WeaponLocation;
	this->WeaponTendency = pType->WeaponTendency;
	this->WeaponToAllies = pType->WeaponToAllies;
	this->InStraight = false;
	this->Accelerate = true;
	this->TargetInAir = false;
	this->LastReviseMult = 0;

	if (ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target))
		this->TargetInAir = (pTarget->GetHeight() > 0);

	this->FinalHeight = static_cast<int>(pBullet->TargetCoords.DistanceFrom(pBullet->SourceCoords));
	this->LastTargetCoord = pBullet->TargetCoords;
	this->FirepowerMult = 1.0;

	if (pBullet->Owner)
		this->FirepowerMult = pBullet->Owner->FirepowerMultiplier;

	if (this->UniqueCurve || this->LaunchSpeed > 256.0)
		this->LaunchSpeed = 256.0;
	else if (this->LaunchSpeed < 0.001)
		this->LaunchSpeed = 0.001;

	if (this->UniqueCurve)
	{
		pBullet->Velocity.X = 0;
		pBullet->Velocity.Y = 0;
		pBullet->Velocity.Z = 4.0;

		if (this->FinalHeight < 1280)
		{
			this->FinalHeight = static_cast<int>(this->FinalHeight * 1.2) + 512;
			this->SuicideAboveRange = 4 * this->FinalHeight;
		}
		else if (this->FinalHeight > 3840)
		{
			this->FinalHeight = static_cast<int>(this->FinalHeight * 0.4) + 512;
			this->SuicideAboveRange = 2 * this->FinalHeight;
		}
		else
		{
			this->FinalHeight = 2048;
			this->SuicideAboveRange = 3 * this->FinalHeight;
		}
	}
	else if (this->PreAimCoord.X == 0 && this->PreAimCoord.Y == 0 && this->PreAimCoord.Z == 0)
	{
		this->InStraight = true;
		pBullet->Velocity.X = pBullet->TargetCoords.X - pBullet->SourceCoords.X;
		pBullet->Velocity.Y = pBullet->TargetCoords.Y - pBullet->SourceCoords.Y;
		pBullet->Velocity.Z = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;

		if (CalculateBulletVelocity(pBullet, this->LaunchSpeed))
			this->SuicideAboveRange = 0.1;
	}
	else
	{
		double RotateAngle = 0.0;
		CoordStruct TheSource = pBullet->SourceCoords;

		if (pBullet->Owner)
			TheSource = pBullet->Owner->GetCoords();

		if (pBullet->TargetCoords.Y != TheSource.Y || pBullet->TargetCoords.X != TheSource.X)
		{
			RotateAngle = Math::atan2(pBullet->TargetCoords.Y - TheSource.Y , pBullet->TargetCoords.X - TheSource.X);
		}

		else
		{
			if (pBullet->Owner)
			{
				if (pBullet->Owner->HasTurret())
					RotateAngle = -(pBullet->Owner->TurretFacing().GetRadian<32>());
				else
					RotateAngle = -(pBullet->Owner->PrimaryFacing.Current().GetRadian<32>());
			}
			else
			{
				RotateAngle = Math::atan2(pBullet->TargetCoords.Y - TheSource.Y , pBullet->TargetCoords.X - TheSource.X);
			}
		}

		pBullet->Velocity.X = this->PreAimCoord.X * Math::cos(RotateAngle) + this->PreAimCoord.Y * Math::sin(RotateAngle);
		pBullet->Velocity.Y = this->PreAimCoord.X * Math::sin(RotateAngle) - this->PreAimCoord.Y * Math::cos(RotateAngle);
		pBullet->Velocity.Z = this->PreAimCoord.Z;

		if (CalculateBulletVelocity(pBullet, this->LaunchSpeed))
			this->SuicideAboveRange = 0.1;
	}
}

bool DisperseTrajectory::OnAI(BulletClass* pBullet)
{
	if (BulletDetonatePreCheck(pBullet))
		return true;

	HouseClass* pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
	bool VelocityUp = false;

	if (this->WeaponScope <=0 || pBullet->TargetCoords.DistanceFrom(pBullet->Location) <= this->WeaponScope)
	{
		if (this->WeaponCount > 0)
		{
			if (pOwner)
			{
				if (PrepareDisperseWeapon(pBullet, pOwner))
					return true;
			}
			else
			{
				return true;
			}
		}
	}

	if (this->UniqueCurve)
	{
		if (CurveVelocityChange(pBullet))
			return true;

		return false;
	}

	if (this->Accelerate)
	{
		double StraightSpeed = this->GetTrajectorySpeed(pBullet);
		this->LaunchSpeed += this->Acceleration;

		if (StraightSpeed > 256.0)
		{
			if (this->LaunchSpeed >= 256.0)
			{
				this->LaunchSpeed = 256.0;
				this->Accelerate = false;
			}
		}
		else
		{
			if (this->LaunchSpeed >= StraightSpeed)
			{
				this->LaunchSpeed = StraightSpeed;
				this->Accelerate = false;
			}
		}

		VelocityUp = true;
	}

	if (!this->LockDirection)
	{
		if (this->RetargetRadius != 0 && BulletRetargetTechno(pBullet, pOwner))
			return true;

		if (this->InStraight)
		{
			if (StandardVelocityChange(pBullet))
				return true;
		}
		else if (pBullet->SourceCoords.DistanceFromSquared(pBullet->Location) >= this->PreAimCoord.MagnitudeSquared())
		{
			if (StandardVelocityChange(pBullet))
				return true;

			this->InStraight = true;
		}

		VelocityUp = true;
	}

	if (VelocityUp && CalculateBulletVelocity(pBullet, this->LaunchSpeed))
		return true;

	return false;
}

void DisperseTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	CoordStruct pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= this->TargetSnapDistance)
	{
		auto const pExt = BulletExt::ExtMap.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

void DisperseTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
}

TrajectoryCheckReturnType DisperseTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

TrajectoryCheckReturnType DisperseTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

bool DisperseTrajectory::CalculateBulletVelocity(BulletClass* pBullet, double StraightSpeed)
{
	double VelocityLength = pBullet->Velocity.Magnitude();

	if (VelocityLength > 0)
		pBullet->Velocity *= StraightSpeed / VelocityLength;
	else
		return true;

	return false;
}

bool DisperseTrajectory::BulletDetonatePreCheck(BulletClass* pBullet)
{
	if (MapClass::Instance->GetCellFloorHeight(pBullet->Location) > pBullet->Location.Z)
		return true;

	double TargetDistance = pBullet->TargetCoords.DistanceFrom(pBullet->Location);

	if (this->UniqueCurve)
	{
		if (TargetDistance > 128)
			return false;
		else
			return true;
	}

	if (this->SuicideAboveRange > 0)
	{
		double BulletSpeed = this->LaunchSpeed;

		if (this->UniqueCurve)
			BulletSpeed = pBullet->Velocity.Magnitude();

		this->SuicideAboveRange -= BulletSpeed;

		if (this->SuicideAboveRange <= 0)
			return true;
	}

	if (TargetDistance < this->TargetSnapDistance)
		return true;

	return false;
}

bool DisperseTrajectory::BulletRetargetTechno(BulletClass* pBullet, HouseClass* pOwner)
{
	bool Check = false;

	if (!pBullet->Target)
	{
		Check = true;
	}
	else if (TechnoClass* pTarget = abstract_cast<TechnoClass*>(pBullet->Target))
	{
		if (pTarget->IsDead() || pTarget->InLimbo)
			Check = true;
	}

	if (!Check)
		return false;

	if (this->RetargetRadius < 0)
		return true;

	CoordStruct RetargetCoords = pBullet->TargetCoords;

	if (this->InStraight)
	{
		BulletVelocity FutureVelocity = pBullet->Velocity * (this->RetargetRadius * 256.0 / this->LaunchSpeed);
		RetargetCoords.X = pBullet->Location.X + static_cast<int>(FutureVelocity.X);
		RetargetCoords.Y = pBullet->Location.Y + static_cast<int>(FutureVelocity.Y);
		RetargetCoords.Z = pBullet->Location.Z;
	}

	std::vector<TechnoClass*> Technos = Helpers::Alex::getCellSpreadItems(RetargetCoords, this->RetargetRadius, this->TargetInAir);
	std::vector<TechnoClass*> ValidTechnos = GetValidTechnosInSame(Technos, pOwner, pBullet->WeaponType->Warhead, false);
	int ValidTechnoNums = ValidTechnos.size();

	if (ValidTechnoNums > 0)
	{
		int num = ScenarioClass::Instance->Random.RandomRanged(0, ValidTechnoNums - 1);
		TechnoClass* BulletTarget = ValidTechnos[num];

		pBullet->Target = BulletTarget;
		pBullet->TargetCoords = BulletTarget->GetCoords();

		this->LastTargetCoord = pBullet->TargetCoords;
	}

	return false;
}

bool DisperseTrajectory::CurveVelocityChange(BulletClass* pBullet)
{
	ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	CoordStruct TargetLocation = pTarget ? pTarget->GetCoords() : pBullet->TargetCoords;
	pBullet->TargetCoords = TargetLocation;

	if (!this->InStraight)
	{
		int OffHeight = this->FinalHeight - 1600;

		if (this->FinalHeight < 3200)
			OffHeight = this->FinalHeight / 2;

		CoordStruct HorizonVelocity { TargetLocation.X - pBullet->Location.X, TargetLocation.Y - pBullet->Location.Y, 0 };
		double HorizonDistance = HorizonVelocity.Magnitude();

		if (HorizonDistance > 0)
		{
			double HorizonMult = abs(pBullet->Velocity.Z / 64.0) / HorizonDistance;
			pBullet->Velocity.X += HorizonMult * HorizonVelocity.X;
			pBullet->Velocity.Y += HorizonMult * HorizonVelocity.Y;
			double HorizonLength = sqrt(pBullet->Velocity.X * pBullet->Velocity.X + pBullet->Velocity.Y * pBullet->Velocity.Y);

			if (HorizonLength > 64)
			{
				HorizonMult = 64 / HorizonLength;
				pBullet->Velocity.X *= HorizonMult;
				pBullet->Velocity.Y *= HorizonMult;
			}
		}

		if ((pBullet->Location.Z - pBullet->SourceCoords.Z) < OffHeight && this->Accelerate)
		{
			if (pBullet->Velocity.Z < 160.0)
				pBullet->Velocity.Z += 4.0;
		}
		else
		{
			this->Accelerate = false;
			double FutureLocation = pBullet->Location.Z + 8 * pBullet->Velocity.Z;

			if (pBullet->Velocity.Z > -160.0)
				pBullet->Velocity.Z -= 4.0;

			if (FutureLocation <= TargetLocation.Z)
				this->InStraight = true;
			else if (FutureLocation <= pBullet->SourceCoords.Z)
				this->InStraight = true;
		}
	}
	else
	{
		double TimeMult = TargetLocation.DistanceFrom(pBullet->Location) / 192.0;

		TargetLocation.Z += static_cast<int>(TimeMult * 32);

		TargetLocation.X += static_cast<int>(TimeMult * (TargetLocation.X - this->LastTargetCoord.X));
		TargetLocation.Y += static_cast<int>(TimeMult * (TargetLocation.Y - this->LastTargetCoord.Y));

		this->LastTargetCoord = pBullet->TargetCoords;

		if (ChangeBulletVelocity(pBullet, TargetLocation, 24.0, true))
			return true;
	}

	return false;
}

bool DisperseTrajectory::StandardVelocityChange(BulletClass* pBullet)
{
	ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	CoordStruct TargetLocation = pTarget ? pTarget->GetCoords() : pBullet->TargetCoords;
	pBullet->TargetCoords = TargetLocation;

	CoordStruct TargetHorizon { TargetLocation.X, TargetLocation.Y, 0 };
	CoordStruct BulletHorizon { pBullet->Location.X, pBullet->Location.Y, 0 };

	if (this->CruiseEnable && TargetHorizon.DistanceFrom(BulletHorizon) > this->CruiseUnableRange)
		TargetLocation.Z = pBullet->Location.Z;

	if (this->LeadTimeCalculate)
	{
		double LeadSpeed = (this->GetTrajectorySpeed(pBullet) + this->LaunchSpeed) / 2.0;
		LeadSpeed = LeadSpeed > 0 ? LeadSpeed : 1;
		double TimeMult = TargetLocation.DistanceFrom(pBullet->Location) / LeadSpeed;
		TargetLocation.X += static_cast<int>(TimeMult * (TargetLocation.X - this->LastTargetCoord.X));
		TargetLocation.Y += static_cast<int>(TimeMult * (TargetLocation.Y - this->LastTargetCoord.Y));
	}

	this->LastTargetCoord = pBullet->TargetCoords;
	double TurningRadius = this->ROT * this->LaunchSpeed * this->LaunchSpeed / 16384;

	if (ChangeBulletVelocity(pBullet, TargetLocation, TurningRadius, false))
		return true;

	return false;
}

bool DisperseTrajectory::ChangeBulletVelocity(BulletClass* pBullet, CoordStruct TargetLocation, double TurningRadius, bool Curve)
{
	CoordStruct TargetVelocity
	{
		TargetLocation.X - pBullet->Location.X,
		TargetLocation.Y - pBullet->Location.Y,
		TargetLocation.Z - pBullet->Location.Z
	};

	BulletVelocity MoveToVelocity = pBullet->Velocity;

	CoordStruct FutureVelocity
	{
		static_cast<int>(TargetVelocity.X - MoveToVelocity.X),
		static_cast<int>(TargetVelocity.Y - MoveToVelocity.Y),
		static_cast<int>(TargetVelocity.Z - MoveToVelocity.Z)
	};

	BulletVelocity ReviseVelocity = {0,0,0};
	BulletVelocity DirectVelocity = {0,0,0};

	double TargetSquared = TargetVelocity.MagnitudeSquared();
	double BulletSquared = MoveToVelocity.MagnitudeSquared();
	double FutureSquared = FutureVelocity.MagnitudeSquared();

	double TargetSide = sqrt(TargetSquared);
	double BulletSide = sqrt(BulletSquared);

	double ReviseMult = (TargetSquared + BulletSquared - FutureSquared);
	double ReviseBase = 2 * TargetSide * BulletSide;

	if (TargetSide > 0)
	{
		if (ReviseMult < 0.005 * ReviseBase && ReviseMult > -0.005 * ReviseBase)
		{
			double VelocityMult = TurningRadius / TargetSide;

			pBullet->Velocity.X += TargetVelocity.X * VelocityMult;
			pBullet->Velocity.Y += TargetVelocity.Y * VelocityMult;
			pBullet->Velocity.Z += TargetVelocity.Z * VelocityMult;
		}
		else
		{
			double DirectLength = ReviseBase * BulletSide / ReviseMult;
			double VelocityMult = DirectLength / TargetSide;

			DirectVelocity.X = TargetVelocity.X * VelocityMult;
			DirectVelocity.Y = TargetVelocity.Y * VelocityMult;
			DirectVelocity.Z = TargetVelocity.Z * VelocityMult;

			if (DirectVelocity.IsCollinearTo(MoveToVelocity))
			{
				if (ReviseMult < 0)
					ReviseVelocity.Z += TurningRadius;
			}
			else
			{
				if (ReviseMult > 0)
					ReviseVelocity = DirectVelocity - MoveToVelocity;
				else
					ReviseVelocity = MoveToVelocity - DirectVelocity;
			}

			double ReviseLength = ReviseVelocity.Magnitude();

			if (!Curve && ReviseMult < 0 && this->LastReviseMult > 0 && this->InStraight)
				return true;

			if (TurningRadius < ReviseLength)
			{
				ReviseVelocity *= TurningRadius / ReviseLength;

				pBullet->Velocity.X += ReviseVelocity.X;
				pBullet->Velocity.Y += ReviseVelocity.Y;
				pBullet->Velocity.Z += ReviseVelocity.Z;
			}
			else
			{
				pBullet->Velocity.X = TargetVelocity.X;
				pBullet->Velocity.Y = TargetVelocity.Y;
				pBullet->Velocity.Z = TargetVelocity.Z;
			}
		}
	}

	this->LastReviseMult = ReviseMult;

	if (Curve)
	{
		if (BulletSide < 192)
			BulletSide += 4;

		if (BulletSide > 192)
			BulletSide = 192;

		if (CalculateBulletVelocity(pBullet, BulletSide))
			return true;
	}

	return false;
}

bool DisperseTrajectory::PrepareDisperseWeapon(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->WeaponTimer == 0)
	{
		int ValidWeapons = static_cast<int>(Math::max(this->Weapon.size(), this->WeaponBurst.size()));

		AbstractClass* BulletTarget = pBullet->Target ? pBullet->Target
			: MapClass::Instance->TryGetCellAt(pBullet->TargetCoords);

		if (this->WeaponRetarget)
		{
			for (int i = 0; i < ValidWeapons; i++)
			{
				auto const pWeapon = this->Weapon[i];
				double Spread = pWeapon->Range / 256.0;
				CoordStruct CenterCoords = this->WeaponLocation ? pBullet->Location : pBullet->TargetCoords;
				bool IncludeInAir = (this->TargetInAir && pWeapon->Projectile->AA);

				std::vector<TechnoClass*> Technos = Helpers::Alex::getCellSpreadItems(CenterCoords, Spread, IncludeInAir);
				std::vector<TechnoClass*> ValidTechnos = GetValidTechnosInSame(Technos, pOwner, pWeapon->Warhead, true);
				int ValidTechnoNums = ValidTechnos.size();

				for (int j = 0; j < this->WeaponBurst[i]; j++)
				{
					if (ValidTechnoNums > 0)
					{
						int k = ScenarioClass::Instance->Random.RandomRanged(0, ValidTechnoNums - 1);
						BulletTarget = ValidTechnos[k];
					}
					else
					{
						CellClass* RandomCell = nullptr;
						int RandomRange = ScenarioClass::Instance->Random.RandomRanged(0, pWeapon->Range);
						CoordStruct RandomCoords = MapClass::GetRandomCoordsNear(CenterCoords, RandomRange, false);

						while (!RandomCell)
						{
							RandomCell = MapClass::Instance->TryGetCellAt(RandomCoords);
							RandomRange = (RandomRange > 256) ? RandomRange / 2 : 1;
							RandomCoords = MapClass::GetRandomCoordsNear(CenterCoords, RandomRange, false);
						}

						BulletTarget = RandomCell;
					}

					if (this->WeaponTendency && i == 0 && j == 0)
						BulletTarget = pBullet->Target ? pBullet->Target : MapClass::Instance->TryGetCellAt(pBullet->TargetCoords);

					CreateDisperseBullets(pBullet, pWeapon, BulletTarget, pOwner);
				}
			}
		}
		else
		{
			for (int i = 0; i < ValidWeapons; i++)
			{
				auto const pWeapon = this->Weapon[i];

				for (int j = 0; j < this->WeaponBurst[i]; j++)
					CreateDisperseBullets(pBullet, pWeapon, BulletTarget, pOwner);
			}
		}

		this->WeaponCount -= 1;
	}

	this->WeaponTimer += 1;
	if (this->WeaponTimer > 0)
		this->WeaponTimer %= this->WeaponDelay;

	if(this->SuicideIfNoWeapon && this->WeaponCount == 0)
		return true;

	return false;
}

std::vector<TechnoClass*> DisperseTrajectory::GetValidTechnosInSame(std::vector<TechnoClass*> Technos,
	HouseClass* pOwner, WarheadTypeClass* pWH, bool Mode)
{
	std::vector<TechnoClass*> ValidTechnos;
	ValidTechnos.reserve(Technos.size());
	bool CheckAllies = Mode ? !this->WeaponToAllies : !this->RetargetAllies;

	for (auto const& pTechno : Technos)
	{
		if (this->TargetInAir != pTechno->GetHeight() > 0)
			continue;

		if (pTechno->IsDead() || pTechno->InLimbo)
			continue;

		if (CheckAllies)
		{
			if (pOwner->IsAlliedWith(pTechno->Owner))
				continue;

			if (pTechno->WhatAmI() == AbstractType::Infantry && pTechno->IsDisguisedAs(pOwner))
				continue;
		}

		if (pTechno->WhatAmI() == AbstractType::Unit && pTechno->IsDisguised())
			continue;

		if (pTechno->CloakState == CloakState::Cloaked)
			continue;

		if (MapClass::GetTotalDamage(100, pWH, pTechno->GetTechnoType()->Armor, 0) == 0)
			continue;

		ValidTechnos.push_back(pTechno);
	}

	return ValidTechnos;
}

void DisperseTrajectory::CreateDisperseBullets(BulletClass* pBullet, WeaponTypeClass* pWeapon, AbstractClass* BulletTarget, HouseClass* pOwner)
{
	int FinalDamage = static_cast<int>(pWeapon->Damage * this->FirepowerMult);

	if (BulletClass* pCreateBullet = pWeapon->Projectile->CreateBullet(BulletTarget, pBullet->Owner,
		FinalDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
	{
		pCreateBullet->WeaponType = pWeapon;
		auto const pBulletExt = BulletExt::ExtMap.Find(pCreateBullet);
		pBulletExt->FirerHouse = BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		pCreateBullet->MoveTo(pBullet->Location, BulletVelocity::Empty);
	}

	if (pWeapon->IsLaser)
	{
		LaserDrawClass* pLaser;
		if (auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeapon))
		{
			if (pWeapon->IsHouseColor)
			{
				pLaser = GameCreate<LaserDrawClass>(pBullet->Location, BulletTarget->GetCoords(),
					pOwner->LaserColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, pWeapon->LaserDuration);
				pLaser->IsHouseColor = true;
			}
			else if (pWeaponTypeExt->Laser_IsSingleColor)
			{
				pLaser = GameCreate<LaserDrawClass>(pBullet->Location, BulletTarget->GetCoords(),
					pWeapon->LaserInnerColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, pWeapon->LaserDuration);
				pLaser->IsHouseColor = true;
			}
			else
			{
				pLaser = GameCreate<LaserDrawClass>(pBullet->Location, BulletTarget->GetCoords(),
					pWeapon->LaserInnerColor, pWeapon->LaserOuterColor, pWeapon->LaserOuterSpread, pWeapon->LaserDuration);
				pLaser->IsHouseColor = false;
			}

			pLaser->Thickness = 3;
			pLaser->IsSupported = false;
		}
	}

	if (pWeapon->IsElectricBolt)
	{
		if (EBolt* pEBolt = GameCreate<EBolt>())
		{
			if (pWeapon->IsAlternateColor)
				pEBolt->AlternateColor = true;
			else
				pEBolt->AlternateColor = false;

			pEBolt->Fire(pBullet->Location, BulletTarget->GetCoords(), 0);
		}
	}

	if (pWeapon->IsRadBeam)
	{
		RadBeamType pRadBeamType;
		if (pWeapon->Warhead->Temporal)
			pRadBeamType = RadBeamType::Temporal;
		else
			pRadBeamType = RadBeamType::RadBeam;

		if (RadBeam* pRadBeam = RadBeam::Allocate(pRadBeamType))
		{
			pRadBeam->SetCoordsSource(pBullet->Location);
			pRadBeam->SetCoordsTarget(BulletTarget->GetCoords());
			pRadBeam->Color = pWeapon->Warhead->Temporal ? ColorStruct { 255, 100, 0 } : ColorStruct { 128, 200, 255 };
		}
	}

	if (ParticleSystemTypeClass* pPSType = pWeapon->AttachedParticleSystem)
		GameCreate<ParticleSystemClass>(pPSType, pBullet->Location, BulletTarget, pBullet->Owner, BulletTarget->GetCoords(), pOwner);
}
