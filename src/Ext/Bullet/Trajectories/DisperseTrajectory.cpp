#include "DisperseTrajectory.h"
#include "StraightTrajectory.h"
#include "EngraveTrajectory.h"
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
		.Process(this->RotateCoord, false)
		.Process(this->MirrorCoord, false)
		.Process(this->FacingCoord, false)
		.Process(this->ReduceCoord, false)
		.Process(this->UseDisperseBurst, false)
		.Process(this->AxisOfRotation, false)
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
		.Process(this->SuicideShortOfROT, false)
		.Process(this->SuicideAboveRange, false)
		.Process(this->SuicideIfNoWeapon, false)
		.Process(this->Weapon, false)
		.Process(this->WeaponBurst, false)
		.Process(this->WeaponCount, false)
		.Process(this->WeaponDelay, false)
		.Process(this->WeaponTimer, false)
		.Process(this->WeaponScope, false)
		.Process(this->WeaponSeparate, false)
		.Process(this->WeaponRetarget, false)
		.Process(this->WeaponLocation, false)
		.Process(this->WeaponTendency, false)
		.Process(this->WeaponToAllies, false)
		.Process(this->WeaponToGround, false)
		;

	return true;
}

bool DisperseTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);

	Stm
		.Process(this->UniqueCurve)
		.Process(this->PreAimCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->FacingCoord)
		.Process(this->ReduceCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
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
		.Process(this->SuicideShortOfROT)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		.Process(this->Weapon)
		.Process(this->WeaponBurst)
		.Process(this->WeaponCount)
		.Process(this->WeaponDelay)
		.Process(this->WeaponTimer)
		.Process(this->WeaponScope)
		.Process(this->WeaponSeparate)
		.Process(this->WeaponRetarget)
		.Process(this->WeaponLocation)
		.Process(this->WeaponTendency)
		.Process(this->WeaponToAllies)
		.Process(this->WeaponToGround)
		;

	return true;
}

void DisperseTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	this->UniqueCurve.Read(exINI, pSection, "Trajectory.Disperse.UniqueCurve");
	this->PreAimCoord.Read(exINI, pSection, "Trajectory.Disperse.PreAimCoord");
	this->RotateCoord.Read(exINI, pSection, "Trajectory.Disperse.RotateCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Disperse.MirrorCoord");
	this->FacingCoord.Read(exINI, pSection, "Trajectory.Disperse.FacingCoord");
	this->ReduceCoord.Read(exINI, pSection, "Trajectory.Disperse.ReduceCoord");
	this->UseDisperseBurst.Read(exINI, pSection, "Trajectory.Disperse.UseDisperseBurst");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.Disperse.AxisOfRotation");
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
	this->SuicideShortOfROT.Read(exINI, pSection, "Trajectory.Disperse.SuicideShortOfROT");
	this->SuicideAboveRange.Read(exINI, pSection, "Trajectory.Disperse.SuicideAboveRange");
	this->SuicideIfNoWeapon.Read(exINI, pSection, "Trajectory.Disperse.SuicideIfNoWeapon");
	this->Weapon.Read(exINI, pSection, "Trajectory.Disperse.Weapons");
	this->WeaponBurst.Read(exINI, pSection, "Trajectory.Disperse.WeaponBurst");
	this->WeaponCount.Read(exINI, pSection, "Trajectory.Disperse.WeaponCount");
	this->WeaponDelay.Read(exINI, pSection, "Trajectory.Disperse.WeaponDelay");
	this->WeaponTimer.Read(exINI, pSection, "Trajectory.Disperse.WeaponTimer");
	this->WeaponScope.Read(exINI, pSection, "Trajectory.Disperse.WeaponScope");
	this->WeaponSeparate.Read(exINI, pSection, "Trajectory.Disperse.WeaponSeparate");
	this->WeaponRetarget.Read(exINI, pSection, "Trajectory.Disperse.WeaponRetarget");
	this->WeaponLocation.Read(exINI, pSection, "Trajectory.Disperse.WeaponLocation");
	this->WeaponTendency.Read(exINI, pSection, "Trajectory.Disperse.WeaponTendency");
	this->WeaponToAllies.Read(exINI, pSection, "Trajectory.Disperse.WeaponToAllies");
	this->WeaponToGround.Read(exINI, pSection, "Trajectory.Disperse.WeaponToGround");
}

bool DisperseTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->UniqueCurve)
		.Process(this->PreAimCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->FacingCoord)
		.Process(this->ReduceCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
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
		.Process(this->SuicideShortOfROT)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		.Process(this->Weapon)
		.Process(this->WeaponBurst)
		.Process(this->WeaponCount)
		.Process(this->WeaponDelay)
		.Process(this->WeaponTimer)
		.Process(this->WeaponScope)
		.Process(this->WeaponSeparate)
		.Process(this->WeaponRetarget)
		.Process(this->WeaponLocation)
		.Process(this->WeaponTendency)
		.Process(this->WeaponToAllies)
		.Process(this->WeaponToGround)
		.Process(this->InStraight)
		.Process(this->Accelerate)
		.Process(this->TargetInAir)
		.Process(this->OriginalDistance)
		.Process(this->CurrentBurst)
		.Process(this->ThisWeaponIndex)
		.Process(this->LastTargetCoord)
		.Process(this->PreAimDistance)
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
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->FacingCoord)
		.Process(this->ReduceCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
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
		.Process(this->SuicideShortOfROT)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		.Process(this->Weapon)
		.Process(this->WeaponBurst)
		.Process(this->WeaponCount)
		.Process(this->WeaponDelay)
		.Process(this->WeaponTimer)
		.Process(this->WeaponScope)
		.Process(this->WeaponSeparate)
		.Process(this->WeaponRetarget)
		.Process(this->WeaponLocation)
		.Process(this->WeaponTendency)
		.Process(this->WeaponToAllies)
		.Process(this->WeaponToGround)
		.Process(this->InStraight)
		.Process(this->Accelerate)
		.Process(this->TargetInAir)
		.Process(this->OriginalDistance)
		.Process(this->CurrentBurst)
		.Process(this->ThisWeaponIndex)
		.Process(this->LastTargetCoord)
		.Process(this->PreAimDistance)
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
	this->RotateCoord = pType->RotateCoord;
	this->MirrorCoord = pType->MirrorCoord;
	this->FacingCoord = pType->FacingCoord;
	this->ReduceCoord = pType->ReduceCoord;
	this->UseDisperseBurst = pType->UseDisperseBurst;
	this->AxisOfRotation = pType->AxisOfRotation;
	this->LaunchSpeed = pType->LaunchSpeed;
	this->Acceleration = pType->Acceleration > 0.001 ? pType->Acceleration : 0.001;
	this->ROT = pType->ROT > 0.001 ? pType->ROT : 0.001;
	this->LockDirection = pType->LockDirection;
	this->CruiseEnable = pType->CruiseEnable;
	this->CruiseUnableRange = pType->CruiseUnableRange;
	this->LeadTimeCalculate = pType->LeadTimeCalculate;
	this->TargetSnapDistance = pType->TargetSnapDistance;
	this->RetargetAllies = pType->RetargetAllies;
	this->RetargetRadius = pType->RetargetRadius;
	this->SuicideShortOfROT = pType->SuicideShortOfROT;
	this->SuicideAboveRange = pType->SuicideAboveRange * 256;
	this->SuicideIfNoWeapon = pType->SuicideIfNoWeapon;
	this->Weapon = pType->Weapon;
	this->WeaponBurst = pType->WeaponBurst;
	this->WeaponCount = pType->WeaponCount;
	this->WeaponDelay = pType->WeaponDelay;
	this->WeaponTimer = pType->WeaponTimer;
	this->WeaponScope = pType->WeaponScope;
	this->WeaponSeparate = pType->WeaponSeparate;
	this->WeaponRetarget = pType->WeaponRetarget;
	this->WeaponLocation = pType->WeaponLocation;
	this->WeaponTendency = pType->WeaponTendency;
	this->WeaponToAllies = pType->WeaponToAllies;
	this->WeaponToGround = pType->WeaponToGround;
	this->InStraight = false;
	this->Accelerate = true;
	this->CurrentBurst = 0;
	this->ThisWeaponIndex = 0;
	this->LastReviseMult = 0;
	this->PreAimDistance = this->PreAimCoord.Magnitude();
	this->FirepowerMult = 1.0;

	if (ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target))
		this->TargetInAir = (pTarget->GetHeight() > 0);
	else
		this->TargetInAir = false;

	this->OriginalDistance = static_cast<int>(pBullet->TargetCoords.DistanceFrom(pBullet->SourceCoords));
	this->LastTargetCoord = pBullet->TargetCoords;

	if (pBullet->Owner)
	{
		this->CurrentBurst = pBullet->Owner->CurrentBurstIndex;
		this->FirepowerMult = pBullet->Owner->FirepowerMultiplier;

		if (this->MirrorCoord && pBullet->Owner->CurrentBurstIndex % 2 == 1)
			this->PreAimCoord.Y = -(this->PreAimCoord.Y);
	}

	if (this->UniqueCurve || this->LaunchSpeed > 256.0)
		this->LaunchSpeed = 256.0;
	else if (this->LaunchSpeed < 0.001)
		this->LaunchSpeed = 0.001;

	if (this->UniqueCurve)
	{
		pBullet->Velocity.X = 0;
		pBullet->Velocity.Y = 0;
		pBullet->Velocity.Z = 4.0;

		this->UseDisperseBurst = false;

		if (this->OriginalDistance < 1280)
		{
			this->OriginalDistance = static_cast<int>(this->OriginalDistance * 1.2) + 512;
			this->SuicideAboveRange = 4 * this->OriginalDistance;
		}
		else if (this->OriginalDistance > 3840)
		{
			this->OriginalDistance = static_cast<int>(this->OriginalDistance * 0.4) + 512;
			this->SuicideAboveRange = 2 * this->OriginalDistance;
		}
		else
		{
			this->OriginalDistance = 2048;
			this->SuicideAboveRange = 3 * this->OriginalDistance;
		}
	}
	else if (this->PreAimCoord == CoordStruct::Empty)
	{
		this->InStraight = true;
		pBullet->Velocity.X = pBullet->TargetCoords.X - pBullet->SourceCoords.X;
		pBullet->Velocity.Y = pBullet->TargetCoords.Y - pBullet->SourceCoords.Y;
		pBullet->Velocity.Z = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;

		if (CalculateBulletVelocity(pBullet, this->LaunchSpeed))
			this->SuicideAboveRange = 0.001;
	}
	else
	{
		double RotateAngle = 0.0;
		CoordStruct TheSource = pBullet->SourceCoords;

		if (pBullet->Owner)
			TheSource = pBullet->Owner->GetCoords();

		if (this->FacingCoord || (pBullet->TargetCoords.Y == TheSource.Y && pBullet->TargetCoords.X == TheSource.X))
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
		else
		{
			RotateAngle = Math::atan2(pBullet->TargetCoords.Y - TheSource.Y , pBullet->TargetCoords.X - TheSource.X);
		}

		double CoordMult = this->OriginalDistance / (32768 / this->ROT);

		if (this->ReduceCoord && CoordMult < 1.0)
		{
			CoordStruct TheAimCoord
			{
				static_cast<int>(this->PreAimCoord.X * Math::cos(RotateAngle) + this->PreAimCoord.Y * Math::sin(RotateAngle)),
				static_cast<int>(this->PreAimCoord.X * Math::sin(RotateAngle) - this->PreAimCoord.Y * Math::cos(RotateAngle)),
				this->PreAimCoord.Z
			};

			CoordStruct TheDistance = pBullet->TargetCoords - pBullet->SourceCoords;
			CoordStruct TheDifferece = TheDistance - TheAimCoord;

			pBullet->Velocity.X = TheAimCoord.X + (1 - CoordMult) * TheDifferece.X;
			pBullet->Velocity.Y = TheAimCoord.Y + (1 - CoordMult) * TheDifferece.Y;
			pBullet->Velocity.Z = TheAimCoord.Z + (1 - CoordMult) * TheDifferece.Z;
		}
		else
		{
			pBullet->Velocity.X = this->PreAimCoord.X * Math::cos(RotateAngle) + this->PreAimCoord.Y * Math::sin(RotateAngle);
			pBullet->Velocity.Y = this->PreAimCoord.X * Math::sin(RotateAngle) - this->PreAimCoord.Y * Math::cos(RotateAngle);
			pBullet->Velocity.Z = this->PreAimCoord.Z;
		}

		if (!this->UseDisperseBurst && this->RotateCoord != 0 && pBullet->WeaponType && pBullet->WeaponType->Burst > 1)
		{
			BulletVelocity RotationAxis
			{
				this->AxisOfRotation.X * Math::cos(RotateAngle) + this->AxisOfRotation.Y * Math::sin(RotateAngle),
				this->AxisOfRotation.X * Math::sin(RotateAngle) - this->AxisOfRotation.Y * Math::cos(RotateAngle),
				static_cast<double>(this->AxisOfRotation.Z)
			};

			double ExtraRotate = 0;

			if (this->MirrorCoord)
			{
				if (pBullet->Owner && pBullet->Owner->CurrentBurstIndex % 2 == 1)
					RotationAxis *= -1;

				ExtraRotate = Math::Pi * (this->RotateCoord * ((this->CurrentBurst / 2) / (pBullet->WeaponType->Burst - 1.0) - 0.5)) / 180;
			}
			else
			{
				ExtraRotate = Math::Pi * (this->RotateCoord * (this->CurrentBurst / (pBullet->WeaponType->Burst - 1.0) - 0.5)) / 180;
			}

			pBullet->Velocity = RotateAboutTheAxis(pBullet->Velocity, RotationAxis, ExtraRotate);
		}

		if (CalculateBulletVelocity(pBullet, this->LaunchSpeed))
			this->SuicideAboveRange = 0.001;
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
		if (this->WeaponCount != 0)
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
		else if (!this->ReduceCoord ? pBullet->SourceCoords.DistanceFrom(pBullet->Location) >= this->PreAimDistance
			: pBullet->SourceCoords.DistanceFrom(pBullet->Location) * 2560 >= this->PreAimDistance * this->OriginalDistance)
		{
			this->InStraight = true;

			if (StandardVelocityChange(pBullet))
				return true;
		}

		VelocityUp = true;
	}
	else if (!this->InStraight)
	{
		if (this->RetargetRadius != 0 && BulletRetargetTechno(pBullet, pOwner))
			return true;

		if (!this->ReduceCoord ? pBullet->SourceCoords.DistanceFrom(pBullet->Location) >= this->PreAimDistance
			: pBullet->SourceCoords.DistanceFrom(pBullet->Location) * 2560 >= this->PreAimDistance * this->OriginalDistance)
		{
			if (StandardVelocityChange(pBullet))
				return true;
		}
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
	std::vector<TechnoClass*> ValidTechnos = GetValidTechnosInSame(Technos, pOwner, pBullet->WH, nullptr);
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
		int OffHeight = this->OriginalDistance - 1600;

		if (this->OriginalDistance < 3200)
			OffHeight = this->OriginalDistance / 2;

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

	double TurningRadius = this->ROT * this->LaunchSpeed * this->LaunchSpeed / 16384;

	if (ChangeBulletVelocity(pBullet, TargetLocation, TurningRadius, false))
		return true;

	return false;
}

bool DisperseTrajectory::ChangeBulletVelocity(BulletClass* pBullet, CoordStruct TargetLocation, double TurningRadius, bool Curve)
{
	BulletVelocity TargetVelocity
	{
		static_cast<double>(TargetLocation.X - pBullet->Location.X),
		static_cast<double>(TargetLocation.Y - pBullet->Location.Y),
		static_cast<double>(TargetLocation.Z - pBullet->Location.Z)
	};

	BulletVelocity MoveToVelocity = pBullet->Velocity;
	BulletVelocity FutureVelocity = TargetVelocity - MoveToVelocity;

	BulletVelocity ReviseVelocity {0, 0, 0};
	BulletVelocity DirectVelocity {0, 0, 0};

	double TargetSquared = TargetVelocity.MagnitudeSquared();
	double BulletSquared = MoveToVelocity.MagnitudeSquared();
	double FutureSquared = FutureVelocity.MagnitudeSquared();

	double TargetSide = sqrt(TargetSquared);
	double BulletSide = sqrt(BulletSquared);

	double ReviseMult = (TargetSquared + BulletSquared - FutureSquared);
	double ReviseBase = 2 * TargetSide * BulletSide;

	if (TargetSide > 0)
	{
		if (ReviseMult < 0.001 * ReviseBase && ReviseMult > -0.001 * ReviseBase)
		{
			double VelocityMult = TurningRadius / TargetSide;
			pBullet->Velocity += TargetVelocity * VelocityMult;
		}
		else
		{
			double DirectLength = ReviseBase * BulletSide / ReviseMult;
			double VelocityMult = DirectLength / TargetSide;

			DirectVelocity = TargetVelocity * VelocityMult;

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

			if (!Curve && this->SuicideShortOfROT && ReviseMult < 0 && this->LastReviseMult > 0
				&& this->LastTargetCoord == pBullet->TargetCoords)
			{
				return true;
			}

			if (TurningRadius < ReviseLength)
			{
				ReviseVelocity *= TurningRadius / ReviseLength;
				pBullet->Velocity += ReviseVelocity;
			}
			else
			{
				pBullet->Velocity = TargetVelocity;

				if (!Curve && this->LockDirection)
					this->InStraight = true;
			}
		}
	}

	this->LastReviseMult = ReviseMult;
	this->LastTargetCoord = pBullet->TargetCoords;

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

BulletVelocity DisperseTrajectory::RotateAboutTheAxis(BulletVelocity TheSpeed, BulletVelocity TheAxis, double TheRadian)
{
	double TheAxisLengthSquared = TheAxis.MagnitudeSquared();

	if (TheAxisLengthSquared == 0)
		return TheSpeed;

	TheAxis *= 1 / sqrt(TheAxisLengthSquared);
	double CosRotate = Math::cos(TheRadian);

	return ((TheSpeed * CosRotate) + (TheAxis * ((1 - CosRotate) * (TheSpeed * TheAxis)))
		+ (TheAxis.CrossProduct(TheSpeed) * Math::sin(TheRadian)));
}

bool DisperseTrajectory::PrepareDisperseWeapon(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->WeaponTimer == 0)
	{
		size_t ValidWeapons = 0;
		size_t BurstSize = this->WeaponBurst.size();

		if (BurstSize > 0)
			ValidWeapons = this->Weapon.size();

		if (ValidWeapons == 0)
		{
			if(this->SuicideIfNoWeapon)
				return true;
			else
				return false;
		}

		if (this->WeaponCount > 0)
			this->WeaponCount -= 1;

		AbstractClass* BulletTarget = pBullet->Target ? pBullet->Target : MapClass::Instance->TryGetCellAt(pBullet->TargetCoords);

		for (size_t WeaponNum = 0; WeaponNum < ValidWeapons; WeaponNum++)
		{
			size_t CurIndex = WeaponNum;
			int BurstCount = 0;

			if (static_cast<int>(BurstSize) > this->ThisWeaponIndex)
				BurstCount = this->WeaponBurst[this->ThisWeaponIndex];
			else
				BurstCount = this->WeaponBurst[BurstSize - 1];

			if (BurstCount <= 0)
				continue;

			if (this->WeaponSeparate)
			{
				CurIndex = this->ThisWeaponIndex;
				WeaponNum = ValidWeapons;

				this->ThisWeaponIndex += 1;
				this->ThisWeaponIndex %= ValidWeapons;
			}

			WeaponTypeClass* pWeapon = this->Weapon[CurIndex];

			if (!this->WeaponRetarget)
			{
				for (int BurstNum = 0; BurstNum < BurstCount; BurstNum++)
					CreateDisperseBullets(pBullet, pWeapon, BulletTarget, pOwner, BurstNum, BurstCount);

				continue;
			}

			int BurstNow = 0;

			if (this->WeaponTendency && BurstCount > 0)
			{
				CreateDisperseBullets(pBullet, pWeapon, BulletTarget, pOwner, BurstNow, BurstCount);
				BurstNow += 1;
			}

			if (BurstCount <= 1)
				continue;

			double Spread = pWeapon->Range / 256.0;
			bool IncludeInAir = (this->TargetInAir && pWeapon->Projectile->AA);
			CoordStruct CenterCoords = this->WeaponLocation ? pBullet->Location : pBullet->TargetCoords;
			std::vector<TechnoClass*> Technos = Helpers::Alex::getCellSpreadItems(CenterCoords, Spread, IncludeInAir);
			std::vector<TechnoClass*> ValidTechnos = GetValidTechnosInSame(Technos, pOwner, pWeapon->Warhead, BulletTarget);
			size_t ValidTechnoNums = ValidTechnos.size();
			std::vector<AbstractClass*> ValidTargets;
			ValidTargets.reserve(BurstCount);

			if (this->WeaponToGround)
				ValidTechnoNums = 0;

			if (static_cast<int>(ValidTechnoNums) <= BurstCount - BurstNow)
			{
				for (int BurstNum = BurstNow; BurstNum < BurstCount; BurstNum++)
				{
					if (static_cast<int>(ValidTechnoNums) > BurstNum)
					{
						ValidTargets.push_back(ValidTechnos[BurstNum]);
					}
					else
					{
						CellClass* RandomCell = nullptr;
						int RandomRange = ScenarioClass::Instance->Random.RandomRanged(0, pWeapon->Range);
						CoordStruct RandomCoords = MapClass::GetRandomCoordsNear(CenterCoords, RandomRange, false);

						while (!RandomCell)
						{
							RandomCell = MapClass::Instance->TryGetCellAt(RandomCoords);

							if (RandomRange > 256)
								RandomRange = RandomRange / 2;
							else if (RandomRange > 1)
								RandomRange = 1;
							else
								break;

							RandomCoords = MapClass::GetRandomCoordsNear(CenterCoords, RandomRange, false);
						}

						ValidTargets.push_back(RandomCell);
					}
				}
			}
			else
			{
				int TechnoNum = ScenarioClass::Instance->Random.RandomRanged(0, ValidTechnoNums - 1);
				int OffsetNum = static_cast<int>(ValidTechnoNums) - BurstCount;
				double OffsetChance = static_cast<double>(OffsetNum) / static_cast<double>(ValidTechnoNums);
				double OffsetRandom = 0.0;

				for (int BurstNum = BurstNow; BurstNum < BurstCount; BurstNum++)
				{
					ValidTargets.push_back(ValidTechnos[TechnoNum]);

					TechnoNum++;
					TechnoNum %= ValidTechnoNums;

					OffsetRandom = ScenarioClass::Instance->Random.RandomDouble();

					while (OffsetNum > 0 && OffsetRandom < OffsetChance)
					{
						TechnoNum++;
						TechnoNum %= ValidTechnoNums;

						OffsetNum -= 1;
						OffsetRandom = ScenarioClass::Instance->Random.RandomDouble();
					}
				}
			}

			for (auto const& pTarget : ValidTargets)
			{
				CreateDisperseBullets(pBullet, pWeapon, pTarget, pOwner, BurstNow, BurstCount);
				BurstNow += 1;
			}
		}
	}

	this->WeaponTimer += 1;
	if (this->WeaponTimer > 0)
		this->WeaponTimer %= this->WeaponDelay;

	if(this->SuicideIfNoWeapon && this->WeaponCount == 0)
		return true;

	return false;
}

std::vector<TechnoClass*> DisperseTrajectory::GetValidTechnosInSame(std::vector<TechnoClass*> Technos,
	HouseClass* pOwner, WarheadTypeClass* pWH, AbstractClass* pTargetAbstract)
{
	std::vector<TechnoClass*> ValidTechnos;
	ValidTechnos.reserve(Technos.size());

	TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTargetAbstract);
	bool CheckAllies = pTargetAbstract ? !this->WeaponToAllies : !this->RetargetAllies;

	for (auto const& pTechno : Technos)
	{
		if (this->TargetInAir != pTechno->GetHeight() > 0)
			continue;

		if (pTechno->IsDead() || pTechno->InLimbo)
			continue;

		if (this->WeaponTendency && pTargetTechno && pTechno == pTargetTechno)
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

void DisperseTrajectory::CreateDisperseBullets(BulletClass* pBullet, WeaponTypeClass* pWeapon, AbstractClass* BulletTarget,
	HouseClass* pOwner, int CurBurst, int MaxBurst)
{
	int FinalDamage = static_cast<int>(pWeapon->Damage * this->FirepowerMult);

	if (BulletClass* pCreateBullet = pWeapon->Projectile->CreateBullet(BulletTarget, pBullet->Owner,
		FinalDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
	{
		pCreateBullet->WeaponType = pWeapon;
		auto const pBulletExt = BulletExt::ExtMap.Find(pCreateBullet);
		pBulletExt->FirerHouse = BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		pCreateBullet->MoveTo(pBullet->Location, BulletVelocity::Empty);

		if (CurBurst >= 0 && pBulletExt->Trajectory)
		{
			if (pBulletExt->Trajectory->Flag == TrajectoryFlag::Disperse)
			{
				DisperseTrajectory* pTrajectory = static_cast<DisperseTrajectory*>(pBulletExt->Trajectory);
				pTrajectory->FirepowerMult = this->FirepowerMult;

				//The created bullet's velocity calculation has been completed, so we should stack the calculations.
				if (!pTrajectory->UniqueCurve && pTrajectory->PreAimCoord != CoordStruct::Empty
					&& pTrajectory->UseDisperseBurst && pTrajectory->RotateCoord != 0 && MaxBurst > 1)
				{
					CoordStruct CreateBulletTargetToSource = pCreateBullet->TargetCoords - pCreateBullet->SourceCoords;
					double RotateAngle = Math::atan2(CreateBulletTargetToSource.Y , CreateBulletTargetToSource.X);

					BulletVelocity RotationAxis
					{
						pTrajectory->AxisOfRotation.X * Math::cos(RotateAngle) + pTrajectory->AxisOfRotation.Y * Math::sin(RotateAngle),
						pTrajectory->AxisOfRotation.X * Math::sin(RotateAngle) - pTrajectory->AxisOfRotation.Y * Math::cos(RotateAngle),
						static_cast<double>(pTrajectory->AxisOfRotation.Z)
					};

					double ExtraRotate = 0;

					if (pTrajectory->MirrorCoord)
					{
						if (CurBurst % 2 == 1)
							RotationAxis *= -1;

						ExtraRotate = Math::Pi * (pTrajectory->RotateCoord * ((CurBurst / 2) / (MaxBurst - 1.0) - 0.5)) / 180;
					}
					else
					{
						ExtraRotate = Math::Pi * (pTrajectory->RotateCoord * (CurBurst / (MaxBurst - 1.0) - 0.5)) / 180;
					}

					pCreateBullet->Velocity = RotateAboutTheAxis(pCreateBullet->Velocity, RotationAxis, ExtraRotate);
				}
			}
			else if (pBulletExt->Trajectory->Flag == TrajectoryFlag::Straight)
			{
				StraightTrajectory* pTrajectory = static_cast<StraightTrajectory*>(pBulletExt->Trajectory);
				pTrajectory->FirepowerMult = this->FirepowerMult;

				//The straight trajectory bullets has LeadTimeCalculate=true are not calculate its velocity yet.
				if (pTrajectory->LeadTimeCalculate)
				{
					pTrajectory->CurrentBurst = CurBurst;
					pTrajectory->CountOfBurst = MaxBurst;
					pTrajectory->UseDisperseBurst = false;
				}
				else if (pTrajectory->UseDisperseBurst && pTrajectory->RotateCoord != 0 && MaxBurst > 1)
				{
					CoordStruct CreateBulletTargetToSource = pCreateBullet->TargetCoords - pCreateBullet->SourceCoords;
					double RotateAngle = Math::atan2(CreateBulletTargetToSource.Y , CreateBulletTargetToSource.X);

					BulletVelocity RotationAxis
					{
						pTrajectory->AxisOfRotation.X * Math::cos(RotateAngle) + pTrajectory->AxisOfRotation.Y * Math::sin(RotateAngle),
						pTrajectory->AxisOfRotation.X * Math::sin(RotateAngle) - pTrajectory->AxisOfRotation.Y * Math::cos(RotateAngle),
						static_cast<double>(pTrajectory->AxisOfRotation.Z)
					};

					double ExtraRotate = 0;

					if (pTrajectory->MirrorCoord)
					{
						if (CurBurst % 2 == 1)
							RotationAxis *= -1;

						ExtraRotate = Math::Pi * (pTrajectory->RotateCoord * ((CurBurst / 2) / (MaxBurst - 1.0) - 0.5)) / 180;
					}
					else
					{
						ExtraRotate = Math::Pi * (pTrajectory->RotateCoord * (CurBurst / (MaxBurst - 1.0) - 0.5)) / 180;
					}

					pCreateBullet->Velocity = RotateAboutTheAxis(pCreateBullet->Velocity, RotationAxis, ExtraRotate);
				}
			}
			else if (pBulletExt->Trajectory->Flag == TrajectoryFlag::Engrave)
			{
				EngraveTrajectory* pTrajectory = static_cast<EngraveTrajectory*>(pBulletExt->Trajectory);
				pTrajectory->FirepowerMult = this->FirepowerMult;
			}
		}
	}

	if (pWeapon->IsLaser)
	{
		LaserDrawClass* pLaser;
		auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeapon);

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
