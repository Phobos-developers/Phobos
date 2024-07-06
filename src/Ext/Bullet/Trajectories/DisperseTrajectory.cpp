#include "DisperseTrajectory.h"
#include "StraightTrajectory.h"
//#include "EngraveTrajectory.h"
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <AnimClass.h>
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
	this->FacingCoord = pType->FacingCoord;
	this->ReduceCoord = pType->ReduceCoord;
	this->UseDisperseBurst = pType->UseDisperseBurst;
	this->AxisOfRotation = pType->AxisOfRotation;
	this->LaunchSpeed = pType->LaunchSpeed;
	this->Acceleration = pType->Acceleration > 0.001 ? pType->Acceleration : 0.001;
	this->ROT = pType->ROT > 0.001 ? pType->ROT : 0.001;
	this->LockDirection = pType->LockDirection;
	this->CruiseEnable = pType->CruiseEnable;
	this->CruiseUnableRange = pType->CruiseUnableRange > 0.25 ? pType->CruiseUnableRange * 256 : 64;
	this->LeadTimeCalculate = pType->LeadTimeCalculate;
	this->TargetSnapDistance = pType->TargetSnapDistance;
	this->RetargetRadius = pType->RetargetRadius;
	this->RetargetAllies = pType->RetargetAllies;
	this->SuicideShortOfROT = pType->SuicideShortOfROT;
	this->SuicideAboveRange = pType->SuicideAboveRange * 256;
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
		this->TargetInTheAir = (pTarget->GetHeight() > 200);
	else
		this->TargetInTheAir = false;

	if (this->LaunchSpeed > 256.0)
		this->LaunchSpeed = 256.0;
	else if (this->LaunchSpeed < 0.001)
		this->LaunchSpeed = 0.001;

	this->PreAimDistance = !this->ReduceCoord ? this->PreAimCoord.Magnitude() + this->LaunchSpeed: this->PreAimCoord.Magnitude() * this->OriginalDistance / 2560 + this->LaunchSpeed;

	if (pBullet->Owner)
	{
		this->CurrentBurst = pBullet->Owner->CurrentBurstIndex;
		this->FirepowerMult = pBullet->Owner->FirepowerMultiplier;

		if (this->MirrorCoord && pBullet->Owner->CurrentBurstIndex % 2 == 1)
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
			this->InitializeBulletNotCurve(pBullet);
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
	const CoordStruct pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= this->TargetSnapDistance)
	{
		auto const pExt = BulletExt::ExtMap.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
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

void DisperseTrajectory::InitializeBulletNotCurve(BulletClass* pBullet)
{
	double RotateAngle = 0.0;
	CoordStruct TheSource = pBullet->SourceCoords;

	if (pBullet->Owner)
		TheSource = pBullet->Owner->GetCoords();

	if (this->FacingCoord || (pBullet->TargetCoords.Y == TheSource.Y && pBullet->TargetCoords.X == TheSource.X) && pBullet->Owner)
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

	const double CoordMult = this->OriginalDistance / (32768 / this->ROT);

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

		pBullet->Velocity = this->RotateAboutTheAxis(pBullet->Velocity, RotationAxis, ExtraRotate);
	}
}

BulletVelocity DisperseTrajectory::RotateAboutTheAxis(BulletVelocity TheSpeed, BulletVelocity TheAxis, double TheRadian)
{
	const double TheAxisLengthSquared = TheAxis.MagnitudeSquared();

	if (TheAxisLengthSquared == 0)
		return TheSpeed;

	TheAxis *= 1 / sqrt(TheAxisLengthSquared);
	const double CosRotate = Math::cos(TheRadian);

	return ((TheSpeed * CosRotate) + (TheAxis * ((1 - CosRotate) * (TheSpeed * TheAxis))) + (TheAxis.CrossProduct(TheSpeed) * Math::sin(TheRadian)));
}

bool DisperseTrajectory::CalculateBulletVelocity(BulletClass* pBullet, double StraightSpeed)
{
	const double VelocityLength = pBullet->Velocity.Magnitude();

	if (VelocityLength > 0)
		pBullet->Velocity *= StraightSpeed / VelocityLength;
	else
		return true;

	return false;
}

bool DisperseTrajectory::BulletRetargetTechno(BulletClass* pBullet, HouseClass* pOwner)
{
	bool Check = false;

	if (!pBullet->Target)
		Check = true;
	else if (TechnoClass* const pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target))
		Check = this->CheckTechnoIsInvalid(pTargetTechno);
	else if (this->TargetIsTechno)
		Check = true;

	if (!Check)
		return false;

	if (this->RetargetRadius < 0)
		return true;

	const double RetargetRange = this->RetargetRadius * 256.0;
	CoordStruct RetargetCoords = pBullet->TargetCoords;
	TechnoClass* pNewTechno = nullptr;

	if (this->InStraight)
	{
		const BulletVelocity FutureVelocity = pBullet->Velocity * (RetargetRange / this->LaunchSpeed);
		RetargetCoords.X = pBullet->Location.X + static_cast<int>(FutureVelocity.X);
		RetargetCoords.Y = pBullet->Location.Y + static_cast<int>(FutureVelocity.Y);
		RetargetCoords.Z = pBullet->Location.Z;
	}

	if (!this->TargetInTheAir)
	{
		for (CellSpreadEnumerator ThisCell(static_cast<size_t>(this->RetargetRadius + 0.99)); ThisCell; ++ThisCell)
		{
			if (const CellClass* const pCell = MapClass::Instance->GetCellAt(*ThisCell + CellClass::Coord2Cell(RetargetCoords)))
			{
				ObjectClass* pObject = pCell->FirstObject;

				while (pObject)
				{
					auto const pTechno = abstract_cast<TechnoClass*>(pObject);
					pObject = pObject->NextObject;

					if (!pTechno || pTechno->GetHeight() > 200 || this->CheckTechnoIsInvalid(pTechno))
						continue;

					const AbstractType TechnoType = pTechno->WhatAmI();

					if (TechnoType == AbstractType::Building && static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame)
						continue;

					if (TechnoType == AbstractType::Unit && pTechno->IsDisguised())
						continue;

					if (!this->RetargetAllies && (pOwner->IsAlliedWith(pTechno->Owner) || TechnoType == AbstractType::Infantry && pTechno->IsDisguisedAs(pOwner)))
						continue;

					if (pTechno->CloakState == CloakState::Cloaked)
						continue;

					if (MapClass::GetTotalDamage(100, pBullet->WH, pTechno->GetTechnoType()->Armor, 0) == 0)
						continue;

					if (pTechno->GetCoords().DistanceFrom(RetargetCoords) > RetargetRange)
						continue;

					if (pBullet->WeaponType && pTechno->GetCoords().DistanceFrom(pBullet->Owner ? pBullet->Owner->GetCoords() : pBullet->SourceCoords) > pBullet->WeaponType->Range)
						continue;

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
		for (auto const& pTechno : *TechnoClass::Array)
		{
			if (pTechno->GetHeight() <= 200 || this->CheckTechnoIsInvalid(pTechno))
				continue;

			if (!this->RetargetAllies && pOwner->IsAlliedWith(pTechno->Owner))
				continue;

			if (pTechno->CloakState == CloakState::Cloaked)
				continue;

			if (MapClass::GetTotalDamage(100, pBullet->WH, pTechno->GetTechnoType()->Armor, 0) == 0)
				continue;

			if (pTechno->GetCoords().DistanceFrom(RetargetCoords) > RetargetRange)
				continue;

			if (pBullet->WeaponType && pTechno->GetCoords().DistanceFrom(pBullet->Owner ? pBullet->Owner->GetCoords() : pBullet->SourceCoords) > pBullet->WeaponType->Range)
				continue;

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

bool DisperseTrajectory::CurveVelocityChange(BulletClass* pBullet)
{
	TechnoClass* const pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target);
	const bool CheckValid = pTargetTechno && !CheckTechnoIsInvalid(pTargetTechno);
	CoordStruct TargetLocation = pBullet->TargetCoords;

	if (CheckValid)
		TargetLocation = pTargetTechno->GetCoords();

	pBullet->TargetCoords = TargetLocation;

	if (!this->InStraight)
	{
		int OffHeight = this->OriginalDistance - 1600;

		if (this->OriginalDistance < 3200)
			OffHeight = this->OriginalDistance / 2;

		const CoordStruct HorizonVelocity { TargetLocation.X - pBullet->Location.X, TargetLocation.Y - pBullet->Location.Y, 0 };
		const double HorizonDistance = HorizonVelocity.Magnitude();

		if (HorizonDistance > 0)
		{
			double HorizonMult = abs(pBullet->Velocity.Z / 64.0) / HorizonDistance;
			pBullet->Velocity.X += HorizonMult * HorizonVelocity.X;
			pBullet->Velocity.Y += HorizonMult * HorizonVelocity.Y;
			const double HorizonLength = sqrt(pBullet->Velocity.X * pBullet->Velocity.X + pBullet->Velocity.Y * pBullet->Velocity.Y);

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
			const double FutureHeight = pBullet->Location.Z + 8 * pBullet->Velocity.Z;

			if (pBullet->Velocity.Z > -160.0)
				pBullet->Velocity.Z -= 4.0;

			if (FutureHeight <= TargetLocation.Z)
				this->InStraight = true;
			else if (FutureHeight <= pBullet->SourceCoords.Z)
				this->InStraight = true;
		}
	}
	else
	{
		const double TimeMult = TargetLocation.DistanceFrom(pBullet->Location) / 192.0;
		TargetLocation.Z += static_cast<int>(TimeMult * 48);

		if (CheckValid)
		{
			TargetLocation.X += static_cast<int>(TimeMult * (TargetLocation.X - this->LastTargetCoord.X));
			TargetLocation.Y += static_cast<int>(TimeMult * (TargetLocation.Y - this->LastTargetCoord.Y));
		}

		if (this->ChangeBulletVelocity(pBullet, TargetLocation, 24.0, true))
			return true;
	}

	return false;
}

bool DisperseTrajectory::NotCurveVelocityChange(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->SuicideAboveRange > 0)
	{
		this->SuicideAboveRange -= this->LaunchSpeed;

		if (this->SuicideAboveRange <= 0)
			return true;
	}

	if (this->PreAimDistance > 0)
		this->PreAimDistance -= this->LaunchSpeed;

	bool VelocityUp = false;

	if (this->Accelerate)
	{
		this->LaunchSpeed += this->Acceleration;
		const double TrajectorySpeed = this->GetTrajectorySpeed(pBullet);

		if (this->LaunchSpeed >= 256.0)
		{
			this->LaunchSpeed = 256.0;
			this->Accelerate = false;
		}
		else if (this->LaunchSpeed >= TrajectorySpeed)
		{
			this->LaunchSpeed = TrajectorySpeed;
			this->Accelerate = false;
		}

		VelocityUp = true;
	}

	if (!this->LockDirection || !this->InStraight)
	{
		if (this->RetargetRadius != 0 && this->BulletRetargetTechno(pBullet, pOwner))
			return true;

		if (this->PreAimDistance <= 0 && this->StandardVelocityChange(pBullet))
			return true;

		VelocityUp = true;
	}

	if (VelocityUp && this->CalculateBulletVelocity(pBullet, this->LaunchSpeed))
		return true;

	return false;
}

bool DisperseTrajectory::StandardVelocityChange(BulletClass* pBullet)
{
	TechnoClass* const pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target);
	const bool CheckValid = pTargetTechno && !this->CheckTechnoIsInvalid(pTargetTechno);
	CoordStruct TargetLocation = pBullet->TargetCoords;

	if (CheckValid)
		TargetLocation = pTargetTechno->GetCoords();

	pBullet->TargetCoords = TargetLocation;

	const CoordStruct TargetHorizon { TargetLocation.X, TargetLocation.Y, 0 };
	const CoordStruct BulletHorizon { pBullet->Location.X, pBullet->Location.Y, 0 };

	if (this->CruiseEnable && TargetHorizon.DistanceFrom(BulletHorizon) > this->CruiseUnableRange)
		TargetLocation.Z = pBullet->Location.Z;

	const double TrajectorySpeed = this->GetTrajectorySpeed(pBullet);

	if (this->LeadTimeCalculate && CheckValid && TrajectorySpeed > 64.0)
	{
		double LeadSpeed = (this->GetTrajectorySpeed(pBullet) + this->LaunchSpeed) / 2;
		double TimeMult = TargetLocation.DistanceFrom(pBullet->Location) / LeadSpeed;
		TargetLocation += (TargetLocation - this->LastTargetCoord) * TimeMult;
	}

	const double TurningRadius = this->ROT * this->LaunchSpeed * this->LaunchSpeed / 16384;

	if (this->ChangeBulletVelocity(pBullet, TargetLocation, TurningRadius, false))
		return true;

	return false;
}

bool DisperseTrajectory::ChangeBulletVelocity(BulletClass* pBullet, CoordStruct TargetLocation, double TurningRadius, bool Curve)
{
	const BulletVelocity TargetVelocity
	{
		static_cast<double>(TargetLocation.X - pBullet->Location.X),
		static_cast<double>(TargetLocation.Y - pBullet->Location.Y),
		static_cast<double>(TargetLocation.Z - pBullet->Location.Z)
	};

	const BulletVelocity MoveToVelocity = pBullet->Velocity;
	const BulletVelocity FutureVelocity = TargetVelocity - MoveToVelocity;

	BulletVelocity ReviseVelocity {0, 0, 0};
	BulletVelocity DirectVelocity {0, 0, 0};

	const double TargetSquared = TargetVelocity.MagnitudeSquared();
	const double BulletSquared = MoveToVelocity.MagnitudeSquared();
	const double FutureSquared = FutureVelocity.MagnitudeSquared();

	const double TargetSide = sqrt(TargetSquared);
	const double BulletSide = sqrt(BulletSquared);

	const double ReviseMult = (TargetSquared + BulletSquared - FutureSquared);
	const double ReviseBase = 2 * TargetSide * BulletSide;

	if (TargetSide > 0)
	{
		if (ReviseMult < 0.001 * ReviseBase && ReviseMult > -0.001 * ReviseBase)
		{
			const double VelocityMult = TurningRadius / TargetSide;
			pBullet->Velocity += TargetVelocity * VelocityMult;
		}
		else
		{
			const double DirectLength = ReviseBase * BulletSide / ReviseMult;
			const double VelocityMult = DirectLength / TargetSide;

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

			const double ReviseLength = ReviseVelocity.Magnitude();

			if (!Curve && this->SuicideShortOfROT && ReviseMult < 0 && this->LastReviseMult > 0 && this->LastTargetCoord == pBullet->TargetCoords)
				return true;

			if (TurningRadius < ReviseLength)
			{
				ReviseVelocity *= TurningRadius / ReviseLength;
				pBullet->Velocity += ReviseVelocity;
			}
			else
			{
				pBullet->Velocity = TargetVelocity;
				this->InStraight = true;
			}
		}
	}

	this->LastReviseMult = ReviseMult;
	this->LastTargetCoord = pBullet->TargetCoords;

	if (Curve)
	{
		double TheVelocity = BulletSide;

		if (TheVelocity < 192)
			TheVelocity += 4;

		if (TheVelocity > 192)
			TheVelocity = 192;

		if (this->CalculateBulletVelocity(pBullet, TheVelocity))
			return true;
	}

	return false;
}

bool DisperseTrajectory::PrepareDisperseWeapon(BulletClass* pBullet, HouseClass* pOwner)
{
	if (this->WeaponTimer.Completed())
	{
		this->WeaponTimer.Start(this->WeaponDelay);
		size_t ValidWeapons = 0;
		const size_t BurstSize = this->WeaponBurst.size();\

		if (BurstSize > 0)
			ValidWeapons = this->Weapons.size();

		if (ValidWeapons == 0)
			return this->SuicideIfNoWeapon;

		if (this->WeaponCount > 0)
			this->WeaponCount--;

		AbstractClass* const pTarget = pBullet->Target ? pBullet->Target : MapClass::Instance->TryGetCellAt(pBullet->TargetCoords);

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

				this->ThisWeaponIndex++;
				this->ThisWeaponIndex %= ValidWeapons;
			}

			WeaponTypeClass* const pWeapon = this->Weapons[CurIndex];

			if (!this->WeaponRetarget)
			{
				for (int BurstNum = 0; BurstNum < BurstCount; BurstNum++)
					this->CreateDisperseBullets(pBullet, pWeapon, pTarget, pOwner, BurstNum, BurstCount);

				continue;
			}

			int BurstNow = 0;

			if (this->WeaponTendency && BurstCount > 0)
			{
				this->CreateDisperseBullets(pBullet, pWeapon, pTarget, pOwner, BurstNow, BurstCount);
				BurstNow++;

				if (BurstCount <= 1)
					continue;
			}

			const double Spread = pWeapon->Range / 256.0;
			const bool IncludeInAir = (this->TargetInTheAir && pWeapon->Projectile->AA);
			const CoordStruct CenterCoords = this->WeaponLocation ? pBullet->Location : pBullet->TargetCoords;
			std::vector<TechnoClass*> Technos = Helpers::Alex::getCellSpreadItems(CenterCoords, Spread, IncludeInAir);
			std::vector<TechnoClass*> ValidTechnos = this->GetValidTechnosInSame(Technos, pOwner, pWeapon->Warhead, pTarget);
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
				const double OffsetChance = static_cast<double>(OffsetNum) / static_cast<double>(ValidTechnoNums);
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

						OffsetNum--;
						OffsetRandom = ScenarioClass::Instance->Random.RandomDouble();
					}
				}
			}

			for (auto const& pNewTarget : ValidTargets)
			{
				this->CreateDisperseBullets(pBullet, pWeapon, pNewTarget, pOwner, BurstNow, BurstCount);
				BurstNow++;
			}
		}
	}

	if(this->SuicideIfNoWeapon && this->WeaponCount == 0)
		return true;

	return false;
}

std::vector<TechnoClass*> DisperseTrajectory::GetValidTechnosInSame(std::vector<TechnoClass*> Technos,
	HouseClass* pOwner, WarheadTypeClass* pWH, AbstractClass* pTarget)
{
	std::vector<TechnoClass*> ValidTechnos;
	ValidTechnos.reserve(Technos.size());
	const TechnoClass* const pTargetTechno = abstract_cast<TechnoClass*>(pTarget);

	for (auto const& pTechno : Technos)
	{
		if (this->TargetInTheAir != pTechno->GetHeight() > 200 || this->CheckTechnoIsInvalid(pTechno))
			continue;

		if (this->WeaponTendency && pTargetTechno && pTechno == pTargetTechno)
			continue;

		if (!this->WeaponToAllies && (pOwner->IsAlliedWith(pTechno->Owner) || pTechno->WhatAmI() == AbstractType::Infantry && pTechno->IsDisguisedAs(pOwner)))
			continue;

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

void DisperseTrajectory::CreateDisperseBullets(BulletClass* pBullet, WeaponTypeClass* pWeapon, AbstractClass* pTarget,
	HouseClass* pOwner, int CurBurst, int MaxBurst)
{
	const int FinalDamage = static_cast<int>(pWeapon->Damage * this->FirepowerMult);

	if (BulletClass* const pCreateBullet = pWeapon->Projectile->CreateBullet(pTarget, pBullet->Owner, FinalDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
	{
		pCreateBullet->WeaponType = pWeapon;
		auto const pBulletExt = BulletExt::ExtMap.Find(pCreateBullet);
		pBulletExt->FirerHouse = BulletExt::ExtMap.Find(pBullet)->FirerHouse;
		pCreateBullet->MoveTo(pBullet->Location, BulletVelocity::Empty);

		if (pBulletExt->Trajectory && CurBurst >= 0)
		{
			if (pBulletExt->Trajectory->Flag == TrajectoryFlag::Disperse)
			{
				DisperseTrajectory* const pTrajectory = static_cast<DisperseTrajectory*>(pBulletExt->Trajectory);
				pTrajectory->FirepowerMult = this->FirepowerMult;

				//The created bullet's velocity calculation has been completed, so we should stack the calculations.
				if (!pTrajectory->UniqueCurve && pTrajectory->PreAimCoord != CoordStruct::Empty && pTrajectory->UseDisperseBurst && pTrajectory->RotateCoord != 0 && MaxBurst > 1)
				{
					const CoordStruct CreateBulletTargetToSource = pCreateBullet->TargetCoords - pCreateBullet->SourceCoords;
					const double RotateAngle = Math::atan2(CreateBulletTargetToSource.Y , CreateBulletTargetToSource.X);

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

					pCreateBullet->Velocity = this->RotateAboutTheAxis(pCreateBullet->Velocity, RotationAxis, ExtraRotate);
				}
			}
/*			else if (pBulletExt->Trajectory->Flag == TrajectoryFlag::Straight) //TODO If merge
			{
				StraightTrajectory* const pTrajectory = static_cast<StraightTrajectory*>(pBulletExt->Trajectory);
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
					const CoordStruct CreateBulletTargetToSource = pCreateBullet->TargetCoords - pCreateBullet->SourceCoords;
					const double RotateAngle = Math::atan2(CreateBulletTargetToSource.Y , CreateBulletTargetToSource.X);

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

					pCreateBullet->Velocity = this->RotateAboutTheAxis(pCreateBullet->Velocity, RotationAxis, ExtraRotate);
				}
			}*/
/*			else if (pBulletExt->Trajectory->Flag == TrajectoryFlag::Engrave) //TODO If merge
			{
				EngraveTrajectory* const pTrajectory = static_cast<EngraveTrajectory*>(pBulletExt->Trajectory);
				pTrajectory->FirepowerMult = this->FirepowerMult;
			}*/
		}

		const int AnimCounts = pWeapon->Anim.Count;

		if (AnimCounts > 0)
		{
			int AnimIndex = 0;

			if (AnimCounts % 8 == 0)
			{
				if (pBulletExt->Trajectory)
				{
					AnimIndex = static_cast<int>((Math::atan2(pCreateBullet->Velocity.Y , pCreateBullet->Velocity.X) + Math::TwoPi + Math::Pi) * AnimCounts / Math::TwoPi - (AnimCounts / 8) + 0.5) % AnimCounts;
				}
				else
				{
					const CoordStruct SourceCoord = pBullet->Location;
					const CoordStruct TargetCoord = pTarget->GetCoords();
					AnimIndex = static_cast<int>((Math::atan2(TargetCoord.Y - SourceCoord.Y , TargetCoord.X - SourceCoord.X) + Math::TwoPi + Math::Pi) * AnimCounts / Math::TwoPi - (AnimCounts / 8) + 0.5) % AnimCounts;
				}
			}
			else
			{
				AnimIndex = ScenarioClass::Instance->Random.RandomRanged(0 , AnimCounts - 1);
			}

			if (AnimClass* const pAnim = GameCreate<AnimClass>(pWeapon->Anim[AnimIndex], pBullet->Location))
			{
				pAnim->SetOwnerObject(pBullet->Owner);
				pAnim->Owner = pOwner;
			}
		}
	}
	else
	{
		return;
	}

	if (pWeapon->Report.Count > 0)
	{
		const int ReportIndex = pWeapon->Report.GetItem(ScenarioClass::Instance->Random.RandomRanged(0 , pWeapon->Report.Count - 1));

		if (ReportIndex != -1)
			VocClass::PlayAt(ReportIndex, pBullet->Location, nullptr);
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
