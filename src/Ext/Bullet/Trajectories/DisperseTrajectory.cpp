#include "DisperseTrajectory.h"
// #include "StraightTrajectory.h" // TODO If merge #1294
// #include "ParabolaTrajectory.h" // TODO If merge #1374
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <AnimClass.h>
#include <LaserDrawClass.h>
#include <EBolt.h>
#include <RadBeam.h>
#include <ParticleSystemClass.h>
#include <ScenarioClass.h>
#include <Utilities/Helpers.Alex.h>
#include <AircraftTrackerClass.h>

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
		.Process(this->RetargetRadius, false)
		.Process(this->RetargetAllies, false)
		.Process(this->SuicideShortOfROT, false)
		.Process(this->SuicideAboveRange, false)
		.Process(this->SuicideIfNoWeapon, false)
		.Process(this->Weapons)
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
		.Process(this->RetargetRadius)
		.Process(this->RetargetAllies)
		.Process(this->SuicideShortOfROT)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		.Process(this->Weapons)
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

PhobosTrajectory* DisperseTrajectoryType::CreateInstance() const
{
	return new DisperseTrajectory(this);
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
	this->RetargetRadius.Read(exINI, pSection, "Trajectory.Disperse.RetargetRadius");
	this->RetargetAllies.Read(exINI, pSection, "Trajectory.Disperse.RetargetAllies");
	this->SuicideShortOfROT.Read(exINI, pSection, "Trajectory.Disperse.SuicideShortOfROT");
	this->SuicideAboveRange.Read(exINI, pSection, "Trajectory.Disperse.SuicideAboveRange");
	this->SuicideIfNoWeapon.Read(exINI, pSection, "Trajectory.Disperse.SuicideIfNoWeapon");
	this->Weapons.Read(exINI, pSection, "Trajectory.Disperse.Weapons");
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
		.Process(this->RetargetRadius)
		.Process(this->RetargetAllies)
		.Process(this->SuicideShortOfROT)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		.Process(this->Weapons)
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
		.Process(this->TargetInTheAir)
		.Process(this->TargetIsTechno)
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
		.Process(this->RetargetRadius)
		.Process(this->RetargetAllies)
		.Process(this->SuicideShortOfROT)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		.Process(this->Weapons)
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
		.Process(this->TargetInTheAir)
		.Process(this->TargetIsTechno)
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
	this->ReduceCoord = pType->ReduceCoord;
	this->UseDisperseBurst = pType->UseDisperseBurst;
	this->AxisOfRotation = pType->AxisOfRotation;
	this->LaunchSpeed = pType->LaunchSpeed;
	this->Acceleration = pType->Acceleration > 1e-10 ? pType->Acceleration : 0.001;
	this->ROT = pType->ROT > 1e-10 ? pType->ROT : 0.001;
	this->LockDirection = pType->LockDirection;
	this->CruiseEnable = pType->CruiseEnable;
	this->CruiseUnableRange = pType->CruiseUnableRange > 0.5 ? pType->CruiseUnableRange * Unsorted::LeptonsPerCell : Unsorted::LeptonsPerCell / 2;
	this->LeadTimeCalculate = pType->LeadTimeCalculate;
	this->TargetSnapDistance = pType->TargetSnapDistance;
	this->RetargetRadius = pType->RetargetRadius;
	this->RetargetAllies = pType->RetargetAllies;
	this->SuicideShortOfROT = pType->SuicideShortOfROT;
	this->SuicideAboveRange = pType->SuicideAboveRange * Unsorted::LeptonsPerCell;
	this->SuicideIfNoWeapon = pType->SuicideIfNoWeapon;
	this->Weapons = pType->Weapons;
	this->WeaponBurst = pType->WeaponBurst;
	this->WeaponCount = pType->WeaponCount;
	this->WeaponDelay = pType->WeaponDelay > 0 ? pType->WeaponDelay : 1;
	this->WeaponTimer.Start(pType->WeaponTimer > 0 ? pType->WeaponTimer : 0);
	this->WeaponScope = pType->WeaponScope;
	this->WeaponSeparate = pType->WeaponSeparate;
	this->WeaponRetarget = pType->WeaponRetarget;
	this->WeaponLocation = pType->WeaponLocation;
	this->WeaponTendency = pType->WeaponTendency;
	this->WeaponToAllies = pType->WeaponToAllies;
	this->WeaponToGround = pType->WeaponToGround;
	this->InStraight = false;
	this->Accelerate = true;
	this->TargetIsTechno = static_cast<bool>(abstract_cast<TechnoClass*>(pBullet->Target));
	this->OriginalDistance = static_cast<int>(pBullet->TargetCoords.DistanceFrom(pBullet->SourceCoords));
	this->CurrentBurst = 0;
	this->ThisWeaponIndex = 0;
	this->LastTargetCoord = pBullet->TargetCoords;
	this->LastReviseMult = 0;
	this->FirepowerMult = 1.0;

	if (const ObjectClass* const pTarget = abstract_cast<ObjectClass*>(pBullet->Target))
		this->TargetInTheAir = (pTarget->GetHeight() > Unsorted::CellHeight);
	else
		this->TargetInTheAir = false;

	if (this->LaunchSpeed > 256.0)
		this->LaunchSpeed = 256.0;
	else if (this->LaunchSpeed < 1e-10)
		this->LaunchSpeed = 0.001;

	this->PreAimDistance = !this->ReduceCoord ? this->PreAimCoord.Magnitude() + this->LaunchSpeed: this->PreAimCoord.Magnitude() * this->OriginalDistance / 2560 + this->LaunchSpeed;

	if (TechnoClass* const pFirer = pBullet->Owner)
	{
		this->CurrentBurst = pFirer->CurrentBurstIndex;
		this->FirepowerMult = pFirer->FirepowerMultiplier;

		if (this->MirrorCoord && pFirer->CurrentBurstIndex % 2 == 1)
			this->PreAimCoord.Y = -(this->PreAimCoord.Y);
	}

	if (this->UniqueCurve)
	{
		pBullet->Velocity.X = 0;
		pBullet->Velocity.Y = 0;
		pBullet->Velocity.Z = 4.0;

		this->UseDisperseBurst = false;

		if (this->OriginalDistance < 1280)
			this->OriginalDistance = static_cast<int>(this->OriginalDistance * 1.2) + 512;
		else if (this->OriginalDistance > 3840)
			this->OriginalDistance = static_cast<int>(this->OriginalDistance * 0.4) + 512;
		else
			this->OriginalDistance = 2048;
	}
	else
	{
		if (this->PreAimCoord == CoordStruct::Empty)
		{
			this->InStraight = true;
			pBullet->Velocity.X = pBullet->TargetCoords.X - pBullet->SourceCoords.X;
			pBullet->Velocity.Y = pBullet->TargetCoords.Y - pBullet->SourceCoords.Y;
			pBullet->Velocity.Z = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;
		}
		else
		{
			this->InitializeBulletNotCurve(pBullet, pType->FacingCoord);
		}

		if (this->CalculateBulletVelocity(pBullet, this->LaunchSpeed))
			this->SuicideAboveRange = 0.001;
	}
}

bool DisperseTrajectory::OnAI(BulletClass* pBullet)
{
	if (MapClass::Instance->GetCellFloorHeight(pBullet->Location) > pBullet->Location.Z)
		return true;

	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < (this->UniqueCurve ? 154 : this->TargetSnapDistance))
		return true;

	HouseClass* const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

	if (this->WeaponCount != 0 && (this->WeaponScope == 0 || pBullet->TargetCoords.DistanceFrom(pBullet->Location) <= this->WeaponScope) && (!pOwner || this->PrepareDisperseWeapon(pBullet, pOwner)))
		return true;

	if (this->UniqueCurve ? this->CurveVelocityChange(pBullet) : this->NotCurveVelocityChange(pBullet, pOwner))
		return true;

	return false;
}

void DisperseTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	const ObjectClass* const pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	const CoordStruct coords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (coords.DistanceFrom(pBullet->Location) <= this->TargetSnapDistance)
	{
		auto const pExt = BulletExt::ExtMap.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(coords);
	}

	if (this->WeaponScope < 0 && this->WeaponCount != 0)
	{
		HouseClass* const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		this->WeaponTimer.StartTime = 0;
		this->PrepareDisperseWeapon(pBullet, pOwner);
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

void DisperseTrajectory::InitializeBulletNotCurve(BulletClass* pBullet, bool facing)
{
	double rotateAngle = 0.0;
	TechnoClass* const pFirer = pBullet->Owner;
	const CoordStruct theSource = pFirer ? pFirer->GetCoords() : pBullet->SourceCoords;

	if ((facing || (pBullet->TargetCoords.Y == theSource.Y && pBullet->TargetCoords.X == theSource.X)) && pFirer)
	{
		if (pFirer->HasTurret())
			rotateAngle = -(pFirer->TurretFacing().GetRadian<32>());
		else
			rotateAngle = -(pFirer->PrimaryFacing.Current().GetRadian<32>());
	}
	else
	{
		rotateAngle = Math::atan2(pBullet->TargetCoords.Y - theSource.Y , pBullet->TargetCoords.X - theSource.X);
	}

	const double coordMult = this->OriginalDistance / (32768 / this->ROT);

	if (this->ReduceCoord && coordMult < 1.0)
	{
		CoordStruct theAimCoord
		{
			static_cast<int>(this->PreAimCoord.X * Math::cos(rotateAngle) + this->PreAimCoord.Y * Math::sin(rotateAngle)),
			static_cast<int>(this->PreAimCoord.X * Math::sin(rotateAngle) - this->PreAimCoord.Y * Math::cos(rotateAngle)),
			this->PreAimCoord.Z
		};

		CoordStruct theDistance = pBullet->TargetCoords - pBullet->SourceCoords;
		CoordStruct theDifferece = theDistance - theAimCoord;

		pBullet->Velocity.X = theAimCoord.X + (1 - coordMult) * theDifferece.X;
		pBullet->Velocity.Y = theAimCoord.Y + (1 - coordMult) * theDifferece.Y;
		pBullet->Velocity.Z = theAimCoord.Z + (1 - coordMult) * theDifferece.Z;
	}
	else
	{
		pBullet->Velocity.X = this->PreAimCoord.X * Math::cos(rotateAngle) + this->PreAimCoord.Y * Math::sin(rotateAngle);
		pBullet->Velocity.Y = this->PreAimCoord.X * Math::sin(rotateAngle) - this->PreAimCoord.Y * Math::cos(rotateAngle);
		pBullet->Velocity.Z = this->PreAimCoord.Z;
	}

	if (!this->UseDisperseBurst && abs(this->RotateCoord) > 1e-10 && pBullet->WeaponType && pBullet->WeaponType->Burst > 1)
	{
		BulletVelocity rotationAxis
		{
			this->AxisOfRotation.X * Math::cos(rotateAngle) + this->AxisOfRotation.Y * Math::sin(rotateAngle),
			this->AxisOfRotation.X * Math::sin(rotateAngle) - this->AxisOfRotation.Y * Math::cos(rotateAngle),
			static_cast<double>(this->AxisOfRotation.Z)
		};

		double extraRotate = 0.0;

		if (this->MirrorCoord)
		{
			if (pFirer && pFirer->CurrentBurstIndex % 2 == 1)
				rotationAxis *= -1;

			extraRotate = Math::Pi * (this->RotateCoord * ((this->CurrentBurst / 2) / (pBullet->WeaponType->Burst - 1.0) - 0.5)) / 180;
		}
		else
		{
			extraRotate = Math::Pi * (this->RotateCoord * (this->CurrentBurst / (pBullet->WeaponType->Burst - 1.0) - 0.5)) / 180;
		}

		pBullet->Velocity = this->RotateAboutTheAxis(pBullet->Velocity, rotationAxis, extraRotate);
	}
}

BulletVelocity DisperseTrajectory::RotateAboutTheAxis(BulletVelocity theSpeed, BulletVelocity theAxis, double theRadian)
{
	const double theAxisLengthSquared = theAxis.MagnitudeSquared();

	if (abs(theAxisLengthSquared) < 1e-10)
		return theSpeed;

	theAxis *= 1 / sqrt(theAxisLengthSquared);
	const double cosRotate = Math::cos(theRadian);

	return ((theSpeed * cosRotate) + (theAxis * ((1 - cosRotate) * (theSpeed * theAxis))) + (theAxis.CrossProduct(theSpeed) * Math::sin(theRadian)));
}

bool DisperseTrajectory::CalculateBulletVelocity(BulletClass* pBullet, double trajectorySpeed)
{
	const double velocityLength = pBullet->Velocity.Magnitude();

	if (velocityLength > 1e-10)
		pBullet->Velocity *= trajectorySpeed / velocityLength;
	else
		return true;

	return false;
}

bool DisperseTrajectory::BulletRetargetTechno(BulletClass* pBullet, HouseClass* pOwner)
{
	bool check = false;

	if (!pBullet->Target)
		check = true;
	else if (TechnoClass* const pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target))
		check = this->CheckTechnoIsInvalid(pTargetTechno);
	else if (this->TargetIsTechno)
		check = true;

	if (!check)
		return false;

	if (this->RetargetRadius < 1e-10)
		return true;

	const double retargetRange = this->RetargetRadius * Unsorted::LeptonsPerCell;
	CoordStruct retargetCoords = pBullet->TargetCoords;
	TechnoClass* pNewTechno = nullptr;

	if (this->InStraight)
	{
		const BulletVelocity futureVelocity = pBullet->Velocity * (retargetRange / this->LaunchSpeed);
		retargetCoords.X = pBullet->Location.X + static_cast<int>(futureVelocity.X);
		retargetCoords.Y = pBullet->Location.Y + static_cast<int>(futureVelocity.Y);
		retargetCoords.Z = pBullet->Location.Z;
	}

	if (!this->TargetInTheAir) // Only get same type (on ground / in air)
	{
		for (CellSpreadEnumerator thisCell(static_cast<size_t>(this->RetargetRadius + 0.99)); thisCell; ++thisCell)
		{
			if (const CellClass* const pCell = MapClass::Instance->GetCellAt(*thisCell + CellClass::Coord2Cell(retargetCoords)))
			{
				ObjectClass* pObject = pCell->FirstObject;

				while (pObject)
				{
					TechnoClass* const pTechno = abstract_cast<TechnoClass*>(pObject);
					pObject = pObject->NextObject;

					if (!pTechno || this->CheckTechnoIsInvalid(pTechno))
						continue;

					TechnoTypeClass* const pTechnoType = pTechno->GetTechnoType();

					if (!pTechnoType->LegalTarget)
						continue;

					const AbstractType absType = pTechno->WhatAmI();

					if (absType == AbstractType::Building && static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame)
						continue;

					HouseClass* const pHouse = pTechno->Owner;

					if (pOwner->IsAlliedWith(pHouse))
					{
						if (!this->RetargetAllies)
							continue;
					}
					else
					{
						if (!this->RetargetAllies && absType == AbstractType::Infantry && pTechno->IsDisguisedAs(pOwner) && !pCell->DisguiseSensors_InclHouse(pOwner->ArrayIndex))
							continue;

						if (absType == AbstractType::Unit && pTechno->IsDisguised() && !pCell->DisguiseSensors_InclHouse(pOwner->ArrayIndex))
							continue;

						if (pTechno->CloakState == CloakState::Cloaked && !pCell->Sensors_InclHouse(pOwner->ArrayIndex))
							continue;
					}

					if (MapClass::GetTotalDamage(100, pBullet->WH, pTechnoType->Armor, 0) == 0)
						continue;

					if (pTechno->GetCoords().DistanceFrom(retargetCoords) > retargetRange)
						continue;

					WeaponTypeClass* const pWeapon = pBullet->WeaponType;

					if (pWeapon)
					{
						TechnoClass* const pFirer = pBullet->Owner;

						if (pTechno->GetCoords().DistanceFrom(pFirer ? pFirer->GetCoords() : pBullet->SourceCoords) > pWeapon->Range)
							continue;

						if (!this->CheckWeaponCanTarget(WeaponTypeExt::ExtMap.Find(pWeapon), pFirer, pTechno, pOwner, pHouse))
							continue;
					}

					pNewTechno = pTechno;
					break;
				}
			}

			if (pNewTechno)
				break;
		}
	}
	else
	{
		AircraftTrackerClass* const airTracker = &AircraftTrackerClass::Instance.get();
		airTracker->FillCurrentVector(MapClass::Instance->GetCellAt(retargetCoords), Game::F2I(this->RetargetRadius));

		for (TechnoClass* pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
		{
			if (this->CheckTechnoIsInvalid(pTechno))
				continue;

			TechnoTypeClass* const pTechnoType = pTechno->GetTechnoType();

			if (!pTechnoType->LegalTarget)
				continue;

			HouseClass* const pHouse = pTechno->Owner;

			if (pOwner->IsAlliedWith(pHouse))
			{
				if (!this->RetargetAllies)
					continue;
			}
			else if (CellClass* const pCell = pTechno->GetCell())
			{
				if (pTechno->CloakState == CloakState::Cloaked && !pCell->Sensors_InclHouse(pOwner->ArrayIndex))
					continue;
			}

			if (MapClass::GetTotalDamage(100, pBullet->WH, pTechnoType->Armor, 0) == 0)
				continue;

			if (pTechno->GetCoords().DistanceFrom(retargetCoords) > retargetRange)
				continue;

			WeaponTypeClass* const pWeapon = pBullet->WeaponType;

			if (pWeapon)
			{
				TechnoClass* const pFirer = pBullet->Owner;

				if (pTechno->GetCoords().DistanceFrom(pFirer ? pFirer->GetCoords() : pBullet->SourceCoords) > pWeapon->Range)
					continue;

				if (!this->CheckWeaponCanTarget(WeaponTypeExt::ExtMap.Find(pWeapon), pFirer, pTechno, pOwner, pHouse))
					continue;
			}

			pNewTechno = pTechno;
			break;
		}
	}

	if (pNewTechno)
	{
		pBullet->SetTarget(pNewTechno);
		pBullet->TargetCoords = pNewTechno->GetCoords();
		this->LastTargetCoord = pBullet->TargetCoords;
	}

	return false;
}

bool DisperseTrajectory::CheckTechnoIsInvalid(TechnoClass* pTechno)
{
	return (!pTechno->IsAlive || !pTechno->IsOnMap || pTechno->InLimbo || pTechno->IsSinking || pTechno->Health <= 0);
}

bool DisperseTrajectory::CheckWeaponCanTarget(WeaponTypeExt::ExtData* pWeaponExt, TechnoClass* pFirer, TechnoClass* pTarget, HouseClass* pFirerHouse, HouseClass* pTargetHouse)
{
	return EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pFirerHouse, pTargetHouse) && EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget) && pWeaponExt->HasRequiredAttachedEffects(pTarget, pFirer);
}

bool DisperseTrajectory::CurveVelocityChange(BulletClass* pBullet)
{
	TechnoClass* const pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target);
	const bool checkValid = pTargetTechno && !CheckTechnoIsInvalid(pTargetTechno);
	CoordStruct targetLocation = pBullet->TargetCoords;

	if (checkValid)
		targetLocation = pTargetTechno->GetCoords();

	pBullet->TargetCoords = targetLocation;

	if (!this->InStraight)
	{
		int offHeight = this->OriginalDistance - 1600;

		if (this->OriginalDistance < 3200)
			offHeight = this->OriginalDistance / 2;

		const CoordStruct horizonVelocity { targetLocation.X - pBullet->Location.X, targetLocation.Y - pBullet->Location.Y, 0 };
		const double horizonDistance = horizonVelocity.Magnitude();

		if (horizonDistance > 1e-10)
		{
			double horizonMult = abs(pBullet->Velocity.Z / 64.0) / horizonDistance;
			pBullet->Velocity.X += horizonMult * horizonVelocity.X;
			pBullet->Velocity.Y += horizonMult * horizonVelocity.Y;
			const double horizonLength = sqrt(pBullet->Velocity.X * pBullet->Velocity.X + pBullet->Velocity.Y * pBullet->Velocity.Y);

			if (horizonLength > 64.0)
			{
				horizonMult = 64.0 / horizonLength;
				pBullet->Velocity.X *= horizonMult;
				pBullet->Velocity.Y *= horizonMult;
			}
		}

		if ((pBullet->Location.Z - pBullet->SourceCoords.Z) < offHeight && this->Accelerate)
		{
			if (pBullet->Velocity.Z < 160.0)
				pBullet->Velocity.Z += 4.0;
		}
		else
		{
			this->Accelerate = false;
			const double futureHeight = pBullet->Location.Z + 8 * pBullet->Velocity.Z;

			if (pBullet->Velocity.Z > -160.0)
				pBullet->Velocity.Z -= 4.0;

			if (futureHeight <= targetLocation.Z)
				this->InStraight = true;
			else if (futureHeight <= pBullet->SourceCoords.Z)
				this->InStraight = true;
		}
	}
	else
	{
		const double timeMult = targetLocation.DistanceFrom(pBullet->Location) / 192.0;
		targetLocation.Z += static_cast<int>(timeMult * 48);

		if (checkValid)
		{
			targetLocation.X += static_cast<int>(timeMult * (targetLocation.X - this->LastTargetCoord.X));
			targetLocation.Y += static_cast<int>(timeMult * (targetLocation.Y - this->LastTargetCoord.Y));
		}

		if (this->ChangeBulletVelocity(pBullet, targetLocation, 24.0, true))
			return true;
	}

	return false;
}

bool DisperseTrajectory::NotCurveVelocityChange(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->SuicideAboveRange > 1e-10)
	{
		this->SuicideAboveRange -= this->LaunchSpeed;

		if (this->SuicideAboveRange <= 1e-10)
			return true;
	}

	if (this->PreAimDistance > 1e-10)
		this->PreAimDistance -= this->LaunchSpeed;

	bool velocityUp = false;

	if (this->Accelerate)
	{
		this->LaunchSpeed += this->Acceleration;
		const double trajectorySpeed = this->GetTrajectorySpeed(pBullet);

		if (this->LaunchSpeed >= 256.0)
		{
			this->LaunchSpeed = 256.0;
			this->Accelerate = false;
		}
		else if (this->LaunchSpeed >= trajectorySpeed)
		{
			this->LaunchSpeed = trajectorySpeed;
			this->Accelerate = false;
		}

		velocityUp = true;
	}

	if (!this->LockDirection || !this->InStraight)
	{
		if (abs(this->RetargetRadius) < 1e-10 && this->BulletRetargetTechno(pBullet, pOwner))
			return true;

		if (this->PreAimDistance <= 1e-10 && this->StandardVelocityChange(pBullet))
			return true;

		velocityUp = true;
	}

	if (velocityUp && this->CalculateBulletVelocity(pBullet, this->LaunchSpeed))
		return true;

	return false;
}

bool DisperseTrajectory::StandardVelocityChange(BulletClass* pBullet)
{
	TechnoClass* const pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target);
	const bool checkValid = pTargetTechno && !this->CheckTechnoIsInvalid(pTargetTechno);
	CoordStruct targetLocation = pBullet->TargetCoords;

	if (checkValid)
		targetLocation = pTargetTechno->GetCoords();

	pBullet->TargetCoords = targetLocation;

	const CoordStruct targetHorizon { targetLocation.X, targetLocation.Y, 0 };
	const CoordStruct bulletHorizon { pBullet->Location.X, pBullet->Location.Y, 0 };

	if (this->CruiseEnable && targetHorizon.DistanceFrom(bulletHorizon) > this->CruiseUnableRange)
		targetLocation.Z = pBullet->Location.Z;

	const double trajectorySpeed = this->GetTrajectorySpeed(pBullet);

	if (this->LeadTimeCalculate && checkValid && trajectorySpeed > 64.0)
	{
		const double leadSpeed = (trajectorySpeed + this->LaunchSpeed) / 2;
		const double timeMult = targetLocation.DistanceFrom(pBullet->Location) / leadSpeed;
		targetLocation += (targetLocation - this->LastTargetCoord) * timeMult;
	}

	const double turningRadius = this->ROT * this->LaunchSpeed * this->LaunchSpeed / 16384;

	if (this->ChangeBulletVelocity(pBullet, targetLocation, turningRadius, false))
		return true;

	return false;
}

bool DisperseTrajectory::ChangeBulletVelocity(BulletClass* pBullet, CoordStruct targetLocation, double turningRadius, bool curve)
{
	const BulletVelocity targetVelocity
	{
		static_cast<double>(targetLocation.X - pBullet->Location.X),
		static_cast<double>(targetLocation.Y - pBullet->Location.Y),
		static_cast<double>(targetLocation.Z - pBullet->Location.Z)
	};

	const BulletVelocity moveToVelocity = pBullet->Velocity;
	const BulletVelocity futureVelocity = targetVelocity - moveToVelocity;

	BulletVelocity reviseVelocity {0, 0, 0};
	BulletVelocity directVelocity {0, 0, 0};

	const double targetSquared = targetVelocity.MagnitudeSquared();
	const double bulletSquared = moveToVelocity.MagnitudeSquared();
	const double futureSquared = futureVelocity.MagnitudeSquared();

	const double targetSide = sqrt(targetSquared);
	const double bulletSide = sqrt(bulletSquared);

	const double reviseMult = (targetSquared + bulletSquared - futureSquared);
	const double reviseBase = 2 * targetSide * bulletSide;

	if (targetSide > 1e-10)
	{
		if (reviseMult < 0.001 * reviseBase && reviseMult > -0.001 * reviseBase)
		{
			const double velocityMult = turningRadius / targetSide;
			pBullet->Velocity += targetVelocity * velocityMult;
		}
		else
		{
			const double directLength = reviseBase * bulletSide / reviseMult;
			const double velocityMult = directLength / targetSide;

			directVelocity = targetVelocity * velocityMult;

			if (directVelocity.IsCollinearTo(moveToVelocity))
			{
				if (reviseMult < 0)
					reviseVelocity.Z += turningRadius;
			}
			else
			{
				if (reviseMult > 0)
					reviseVelocity = directVelocity - moveToVelocity;
				else
					reviseVelocity = moveToVelocity - directVelocity;
			}

			const double reviseLength = reviseVelocity.Magnitude();

			if (!curve && this->SuicideShortOfROT && reviseMult < 0 && this->LastReviseMult > 0 && this->LastTargetCoord == pBullet->TargetCoords)
				return true;

			if (turningRadius < reviseLength)
			{
				reviseVelocity *= turningRadius / reviseLength;
				pBullet->Velocity += reviseVelocity;
			}
			else
			{
				pBullet->Velocity = targetVelocity;
				this->InStraight = true;
			}
		}
	}

	this->LastReviseMult = reviseMult;
	this->LastTargetCoord = pBullet->TargetCoords;

	if (curve)
	{
		double trajectorySpeed = bulletSide;

		if (trajectorySpeed < 192.0)
			trajectorySpeed += 4.0;

		if (trajectorySpeed > 192.0)
			trajectorySpeed = 192.0;

		if (this->CalculateBulletVelocity(pBullet, trajectorySpeed))
			return true;
	}

	return false;
}

bool DisperseTrajectory::PrepareDisperseWeapon(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->WeaponTimer.Completed())
	{
		this->WeaponTimer.Start(this->WeaponDelay);
		size_t validWeapons = 0;
		const size_t burstSize = this->WeaponBurst.size();\

		if (burstSize > 0)
			validWeapons = this->Weapons.size();

		if (validWeapons == 0)
			return this->SuicideIfNoWeapon;

		if (this->WeaponCount > 0)
			this->WeaponCount--;

		AbstractClass* const pTarget = pBullet->Target ? pBullet->Target : MapClass::Instance->TryGetCellAt(pBullet->TargetCoords);

		for (size_t weaponNum = 0; weaponNum < validWeapons; weaponNum++)
		{
			size_t curIndex = weaponNum;
			int burstCount = 0;

			if (static_cast<int>(burstSize) > this->ThisWeaponIndex)
				burstCount = this->WeaponBurst[this->ThisWeaponIndex];
			else
				burstCount = this->WeaponBurst[burstSize - 1];

			if (burstCount <= 0)
				continue;

			if (this->WeaponSeparate)
			{
				curIndex = this->ThisWeaponIndex;
				weaponNum = validWeapons;

				this->ThisWeaponIndex++;
				this->ThisWeaponIndex %= validWeapons;
			}

			WeaponTypeClass* const pWeapon = this->Weapons[curIndex];

			if (!this->WeaponRetarget)
			{
				if (pTarget)
				{
					for (int burstNum = 0; burstNum < burstCount; burstNum++)
					{
						this->CreateDisperseBullets(pBullet, pWeapon, pTarget, pOwner, burstNum, burstCount);
					}
				}

				continue;
			}

			int burstNow = 0;

			if (this->WeaponTendency && burstCount > 0 && pTarget)
			{
				this->CreateDisperseBullets(pBullet, pWeapon, pTarget, pOwner, burstNow, burstCount);
				burstNow++;

				if (burstCount <= 1)
					continue;
			}

			const double spread = static_cast<double>(pWeapon->Range) / Unsorted::LeptonsPerCell;
			const bool includeInAir = (this->TargetInTheAir && pWeapon->Projectile->AA);
			const CoordStruct centerCoords = this->WeaponLocation ? pBullet->Location : pBullet->TargetCoords;
			std::vector<TechnoClass*> technos = Helpers::Alex::getCellSpreadItems(centerCoords, spread, includeInAir);
			std::vector<TechnoClass*> validTechnos = this->GetValidTechnosInSame(technos, pBullet->Owner, pOwner, pWeapon, pTarget);
			size_t validTechnoNums = validTechnos.size();
			std::vector<AbstractClass*> validTargets;
			validTargets.reserve(burstCount);

			if (this->WeaponToGround)
				validTechnoNums = 0;

			if (static_cast<int>(validTechnoNums) <= burstCount - burstNow)
			{
				for (int burstNum = burstNow; burstNum < burstCount; burstNum++)
				{
					if (static_cast<int>(validTechnoNums) > burstNum)
					{
						validTargets.push_back(validTechnos[burstNum]);
					}
					else
					{
						CellClass* randomCell = nullptr;
						int randomRange = ScenarioClass::Instance->Random.RandomRanged(0, pWeapon->Range);
						CoordStruct randomCoords = MapClass::GetRandomCoordsNear(centerCoords, randomRange, false);

						do
						{
							randomCell = MapClass::Instance->TryGetCellAt(randomCoords);

							if (randomCell)
							{
								if (EnumFunctions::IsCellEligible(randomCell, WeaponTypeExt::ExtMap.Find(pWeapon)->CanTarget, true, true))
								{
									validTargets.push_back(randomCell);
									break;
								}
							}
							else
							{
								if (randomRange > Unsorted::LeptonsPerCell)
									randomRange = randomRange / 2;
								else if (randomRange > 1)
									randomRange = 1;
								else
									break;
							}

							randomCoords = MapClass::GetRandomCoordsNear(centerCoords, randomRange, false);
						}
						while (true);
					}
				}
			}
			else // TODO Looking forward to better solution
			{
				int technoNum = ScenarioClass::Instance->Random.RandomRanged(0, validTechnoNums - 1);
				int offsetNum = static_cast<int>(validTechnoNums) - burstCount;
				const double offsetChance = static_cast<double>(offsetNum) / static_cast<double>(validTechnoNums);
				double offsetRandom = 0.0;

				for (int burstNum = burstNow; burstNum < burstCount; burstNum++)
				{
					validTargets.push_back(validTechnos[technoNum]);

					technoNum++;
					technoNum %= validTechnoNums;

					offsetRandom = ScenarioClass::Instance->Random.RandomDouble();

					while (offsetNum > 0 && offsetRandom < offsetChance)
					{
						technoNum++;
						technoNum %= validTechnoNums;

						offsetNum--;
						offsetRandom = ScenarioClass::Instance->Random.RandomDouble();
					}
				}
			}

			for (auto const& pNewTarget : validTargets)
			{
				this->CreateDisperseBullets(pBullet, pWeapon, pNewTarget, pOwner, burstNow, burstCount);
				burstNow++;
			}
		}
	}

	if(this->SuicideIfNoWeapon && this->WeaponCount == 0)
		return true;

	return false;
}

std::vector<TechnoClass*> DisperseTrajectory::GetValidTechnosInSame(std::vector<TechnoClass*> technos, TechnoClass* pFirer, HouseClass* pOwner, WeaponTypeClass* pWeapon, AbstractClass* pTarget)
{
	std::vector<TechnoClass*> validTechnos;
	validTechnos.reserve(technos.size());
	const TechnoClass* const pTargetTechno = abstract_cast<TechnoClass*>(pTarget);

	for (auto const& pTechno : technos)
	{
		if (this->TargetInTheAir != pTechno->GetHeight() > Unsorted::CellHeight || this->CheckTechnoIsInvalid(pTechno))
			continue;

		TechnoTypeClass* const pTechnoType = pTechno->GetTechnoType();

		if (!pTechnoType->LegalTarget)
			continue;

		if (this->WeaponTendency && pTargetTechno && pTechno == pTargetTechno)
			continue;

		HouseClass* const pHouse = pTechno->Owner;

		if (pOwner->IsAlliedWith(pHouse))
		{
			if (!this->WeaponToAllies)
				continue;
		}
		else if (CellClass* const pCell = pTechno->GetCell())
		{
			const AbstractType absType = pTechno->WhatAmI();

			if (!this->WeaponToAllies && absType == AbstractType::Infantry && pTechno->IsDisguisedAs(pOwner) && !pCell->DisguiseSensors_InclHouse(pOwner->ArrayIndex))
				continue;

			if (absType == AbstractType::Unit && pTechno->IsDisguised() && !pCell->DisguiseSensors_InclHouse(pOwner->ArrayIndex))
				continue;

			if (pTechno->CloakState == CloakState::Cloaked && !pCell->Sensors_InclHouse(pOwner->ArrayIndex))
				continue;
		}

		if (MapClass::GetTotalDamage(100, pWeapon->Warhead, pTechnoType->Armor, 0) == 0)
			continue;

		if (!this->CheckWeaponCanTarget(WeaponTypeExt::ExtMap.Find(pWeapon), pFirer, pTechno, pOwner, pHouse))
			continue;

		validTechnos.push_back(pTechno);
	}

	return validTechnos;
}

void DisperseTrajectory::CreateDisperseBullets(BulletClass* pBullet, WeaponTypeClass* pWeapon, AbstractClass* pTarget, HouseClass* pOwner, int curBurst, int maxBurst)
{
	const int finalDamage = static_cast<int>(pWeapon->Damage * this->FirepowerMult);

	if (BulletClass* const pCreateBullet = pWeapon->Projectile->CreateBullet(pTarget, pBullet->Owner, finalDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
	{
		pCreateBullet->WeaponType = pWeapon;
		auto const pBulletExt = BulletExt::ExtMap.Find(pCreateBullet);
		pBulletExt->FirerHouse = BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		pCreateBullet->MoveTo(pBullet->Location, BulletVelocity::Empty);

		if (pBulletExt->Trajectory && curBurst >= 0)
		{
			const TrajectoryFlag flag = pBulletExt->Trajectory->Flag;

			if (flag == TrajectoryFlag::Disperse)
			{
				DisperseTrajectory* const pTrajectory = static_cast<DisperseTrajectory*>(pBulletExt->Trajectory);
				pTrajectory->FirepowerMult = this->FirepowerMult;

				//The created bullet's velocity calculation has been completed, so we should stack the calculations.
				if (!pTrajectory->UniqueCurve && pTrajectory->PreAimCoord != CoordStruct::Empty && pTrajectory->UseDisperseBurst && abs(pTrajectory->RotateCoord) > 1e-10 && maxBurst > 1)
				{
					const CoordStruct createBulletTargetToSource = pCreateBullet->TargetCoords - pCreateBullet->SourceCoords;
					const double rotateAngle = Math::atan2(createBulletTargetToSource.Y , createBulletTargetToSource.X);

					BulletVelocity rotationAxis
					{
						pTrajectory->AxisOfRotation.X * Math::cos(rotateAngle) + pTrajectory->AxisOfRotation.Y * Math::sin(rotateAngle),
						pTrajectory->AxisOfRotation.X * Math::sin(rotateAngle) - pTrajectory->AxisOfRotation.Y * Math::cos(rotateAngle),
						static_cast<double>(pTrajectory->AxisOfRotation.Z)
					};

					double extraRotate = 0.0;

					if (pTrajectory->MirrorCoord)
					{
						if (curBurst % 2 == 1)
							rotationAxis *= -1;

						extraRotate = Math::Pi * (pTrajectory->RotateCoord * ((curBurst / 2) / (maxBurst - 1.0) - 0.5)) / 180;
					}
					else
					{
						extraRotate = Math::Pi * (pTrajectory->RotateCoord * (curBurst / (maxBurst - 1.0) - 0.5)) / 180;
					}

					pCreateBullet->Velocity = this->RotateAboutTheAxis(pCreateBullet->Velocity, rotationAxis, extraRotate);
				}
			}
/*			else if (flag == TrajectoryFlag::Straight) // TODO If merge #1294
			{
				StraightTrajectory* const pTrajectory = static_cast<StraightTrajectory*>(pBulletExt->Trajectory);
				pTrajectory->FirepowerMult = this->FirepowerMult;

				//The straight trajectory bullets has LeadTimeCalculate=true are not calculate its velocity yet.
				if (pTrajectory->LeadTimeCalculate && abstract_cast<FootClass*>(pTarget))
				{
					pTrajectory->CurrentBurst = curBurst;
					pTrajectory->CountOfBurst = maxBurst;
					pTrajectory->UseDisperseBurst = false;
				}
				else if (pTrajectory->UseDisperseBurst && abs(pTrajectory->RotateCoord) > 1e-10 && maxBurst > 1)
				{
					const CoordStruct createBulletTargetToSource = pCreateBullet->TargetCoords - pCreateBullet->SourceCoords;
					const double rotateAngle = Math::atan2(createBulletTargetToSource.Y , createBulletTargetToSource.X);

					BulletVelocity rotationAxis
					{
						pTrajectory->AxisOfRotation.X * Math::cos(rotateAngle) + pTrajectory->AxisOfRotation.Y * Math::sin(rotateAngle),
						pTrajectory->AxisOfRotation.X * Math::sin(rotateAngle) - pTrajectory->AxisOfRotation.Y * Math::cos(rotateAngle),
						static_cast<double>(pTrajectory->AxisOfRotation.Z)
					};

					double extraRotate = 0.0;

					if (pTrajectory->MirrorCoord)
					{
						if (curBurst % 2 == 1)
							rotationAxis *= -1;

						extraRotate = Math::Pi * (pTrajectory->RotateCoord * ((curBurst / 2) / (maxBurst - 1.0) - 0.5)) / 180;
					}
					else
					{
						extraRotate = Math::Pi * (pTrajectory->RotateCoord * (curBurst / (maxBurst - 1.0) - 0.5)) / 180;
					}

					pCreateBullet->Velocity = this->RotateAboutTheAxis(pCreateBullet->Velocity, rotationAxis, extraRotate);
				}
			}*/
/*			else if (flag == TrajectoryFlag::Parabola) // TODO If merge #1374
			{
				ParabolaTrajectory* const pTrajectory = static_cast<ParabolaTrajectory*>(pBulletExt->Trajectory);

				//The parabola trajectory bullets has LeadTimeCalculate=true are not calculate its velocity yet.
				if (pTrajectory->LeadTimeCalculate && abstract_cast<FootClass*>(pTarget))
				{
					pTrajectory->CurrentBurst = curBurst;
					pTrajectory->CountOfBurst = maxBurst;
					pTrajectory->UseDisperseBurst = false;
				}
				else if (pTrajectory->UseDisperseBurst && abs(pTrajectory->RotateCoord) > 1e-10 && maxBurst > 1)
				{
					const CoordStruct createBulletTargetToSource = pCreateBullet->TargetCoords - pCreateBullet->SourceCoords;
					const double rotateAngle = Math::atan2(createBulletTargetToSource.Y , createBulletTargetToSource.X);

					BulletVelocity rotationAxis
					{
						pTrajectory->AxisOfRotation.X * Math::cos(rotateAngle) + pTrajectory->AxisOfRotation.Y * Math::sin(rotateAngle),
						pTrajectory->AxisOfRotation.X * Math::sin(rotateAngle) - pTrajectory->AxisOfRotation.Y * Math::cos(rotateAngle),
						static_cast<double>(pTrajectory->AxisOfRotation.Z)
					};

					double extraRotate = 0.0;

					if (pTrajectory->MirrorCoord)
					{
						if (curBurst % 2 == 1)
							rotationAxis *= -1;

						extraRotate = Math::Pi * (pTrajectory->RotateCoord * ((curBurst / 2) / (maxBurst - 1.0) - 0.5)) / 180;
					}
					else
					{
						extraRotate = Math::Pi * (pTrajectory->RotateCoord * (curBurst / (maxBurst - 1.0) - 0.5)) / 180;
					}

					pCreateBullet->Velocity = this->RotateAboutTheAxis(pCreateBullet->Velocity, rotationAxis, extraRotate);
				}
			}*/
		}

		const int animCounts = pWeapon->Anim.Count;

		if (animCounts > 0)
		{
			int animIndex = 0;

			if (animCounts % 8 == 0)
			{
				if (pBulletExt->Trajectory)
				{
					animIndex = static_cast<int>((Math::atan2(pCreateBullet->Velocity.Y , pCreateBullet->Velocity.X) + Math::TwoPi + Math::Pi) * animCounts / Math::TwoPi - (animCounts / 8) + 0.5) % animCounts;
				}
				else
				{
					const CoordStruct theSourceCoord = pBullet->Location;
					const CoordStruct theTargetCoord = pTarget->GetCoords();
					animIndex = static_cast<int>((Math::atan2(theTargetCoord.Y - theSourceCoord.Y , theTargetCoord.X - theSourceCoord.X) + Math::TwoPi + Math::Pi) * animCounts / Math::TwoPi - (animCounts / 8) + 0.5) % animCounts;
				}
			}
			else
			{
				animIndex = ScenarioClass::Instance->Random.RandomRanged(0 , animCounts - 1);
			}

			if (AnimTypeClass* const pAnimType = pWeapon->Anim[animIndex])
			{
				if (AnimClass* const pAnim = GameCreate<AnimClass>(pAnimType, pBullet->Location))
				{
					pAnim->SetOwnerObject(pBullet->Owner);
					pAnim->Owner = pOwner;
				}
			}
		}
	}
	else
	{
		return;
	}

	if (pWeapon->Report.Count > 0)
	{
		const int reportIndex = pWeapon->Report.GetItem((pBullet->Owner ? pBullet->Owner->unknown_short_3C8 : ScenarioClass::Instance->Random.Random()) % pWeapon->Report.Count);

		if (reportIndex != -1)
			VocClass::PlayAt(reportIndex, pBullet->Location, nullptr);
	}

	if (pWeapon->IsLaser)
	{
		LaserDrawClass* pLaser;
		auto const pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeapon);

		if (pWeapon->IsHouseColor)
		{
			pLaser = GameCreate<LaserDrawClass>(pBullet->Location, pTarget->GetCoords(), pOwner->LaserColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, pWeapon->LaserDuration);
			pLaser->IsHouseColor = true;
		}
		else if (pWeaponTypeExt->Laser_IsSingleColor)
		{
			pLaser = GameCreate<LaserDrawClass>(pBullet->Location, pTarget->GetCoords(), pWeapon->LaserInnerColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, pWeapon->LaserDuration);
			pLaser->IsHouseColor = true;
		}
		else
		{
			pLaser = GameCreate<LaserDrawClass>(pBullet->Location, pTarget->GetCoords(), pWeapon->LaserInnerColor, pWeapon->LaserOuterColor, pWeapon->LaserOuterSpread, pWeapon->LaserDuration);
			pLaser->IsHouseColor = false;
		}

		pLaser->Thickness = 3; //TODO Weapon's LaserThickness(Ares)
		pLaser->IsSupported = false;
	}

	if (pWeapon->IsElectricBolt)
	{
		if (EBolt* const pEBolt = GameCreate<EBolt>())
		{
			if (pWeapon->IsAlternateColor)
				pEBolt->AlternateColor = true;
			else
				pEBolt->AlternateColor = false;

			//TODO Weapon's Bolt.Color1, Bolt.Color2, Bolt.Color3(Ares)
			//Although I can reread the Ares tags but how to do with Bolt_Disable1, Bolt_Disable2, Bolt_Disable3, Bolt_Arcs(Phobos)
			pEBolt->Fire(pBullet->Location, pTarget->GetCoords(), 0);
		}
	}

	if (pWeapon->IsRadBeam)
	{
		RadBeamType pRadBeamType;

		if (pWeapon->Warhead->Temporal)
			pRadBeamType = RadBeamType::Temporal;
		else
			pRadBeamType = RadBeamType::RadBeam;

		if (RadBeam* const pRadBeam = RadBeam::Allocate(pRadBeamType))
		{
			pRadBeam->SetCoordsSource(pBullet->Location);
			pRadBeam->SetCoordsTarget(pTarget->GetCoords());

			//TODO Weapon's Beam.Color, Beam.Duration, Beam.Amplitude(Ares)
			pRadBeam->Color = pWeapon->Warhead->Temporal ? RulesClass::Instance->ChronoBeamColor : RulesClass::Instance->RadColor;
		}
	}

	if (ParticleSystemTypeClass* const pPSType = pWeapon->AttachedParticleSystem)
		GameCreate<ParticleSystemClass>(pPSType, pBullet->Location, pTarget, pBullet->Owner, pTarget->GetCoords(), pOwner);
}
