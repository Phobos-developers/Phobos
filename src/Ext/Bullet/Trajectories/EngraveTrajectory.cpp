#include "EngraveTrajectory.h"
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <TacticalClass.h>
#include <LaserDrawClass.h>

bool EngraveTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);

	Stm
		.Process(this->SourceCoord, false)
		.Process(this->TargetCoord, false)
		.Process(this->MirrorCoord, false)
		.Process(this->TheDuration, false)
		.Process(this->IsLaser, false)
		.Process(this->IsSupported, false)
		.Process(this->IsHouseColor, false)
		.Process(this->IsSingleColor, false)
		.Process(this->LaserInnerColor, false)
		.Process(this->LaserOuterColor, false)
		.Process(this->LaserOuterSpread, false)
		.Process(this->LaserThickness, false)
		.Process(this->LaserDuration, false)
		.Process(this->LaserDelay, false)
		.Process(this->DamageDelay, false)
		;

	return true;
}

bool EngraveTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);

	Stm
		.Process(this->SourceCoord)
		.Process(this->TargetCoord)
		.Process(this->MirrorCoord)
		.Process(this->TheDuration)
		.Process(this->IsLaser)
		.Process(this->IsSupported)
		.Process(this->IsHouseColor)
		.Process(this->IsSingleColor)
		.Process(this->LaserInnerColor)
		.Process(this->LaserOuterColor)
		.Process(this->LaserOuterSpread)
		.Process(this->LaserThickness)
		.Process(this->LaserDuration)
		.Process(this->LaserDelay)
		.Process(this->DamageDelay)
		;

	return true;
}

void EngraveTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	this->SourceCoord.Read(exINI, pSection, "Trajectory.Engrave.SourceCoord");
	this->TargetCoord.Read(exINI, pSection, "Trajectory.Engrave.TargetCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Engrave.MirrorCoord");
	this->TheDuration.Read(exINI, pSection, "Trajectory.Engrave.TheDuration");
	this->IsLaser.Read(exINI, pSection, "Trajectory.Engrave.IsLaser");
	this->IsSupported.Read(exINI, pSection, "Trajectory.Engrave.IsSupported");
	this->IsHouseColor.Read(exINI, pSection, "Trajectory.Engrave.IsHouseColor");
	this->IsSingleColor.Read(exINI, pSection, "Trajectory.Engrave.IsSingleColor");
	this->LaserInnerColor.Read(exINI, pSection, "Trajectory.Engrave.LaserInnerColor");
	this->LaserOuterColor.Read(exINI, pSection, "Trajectory.Engrave.LaserOuterColor");
	this->LaserOuterSpread.Read(exINI, pSection, "Trajectory.Engrave.LaserOuterSpread");
	this->LaserThickness.Read(exINI, pSection, "Trajectory.Engrave.LaserThickness");
	this->LaserDuration.Read(exINI, pSection, "Trajectory.Engrave.LaserDuration");
	this->LaserDelay.Read(exINI, pSection, "Trajectory.Engrave.LaserDelay");
	this->DamageDelay.Read(exINI, pSection, "Trajectory.Engrave.DamageDelay");
}

bool EngraveTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->SourceCoord)
		.Process(this->TargetCoord)
		.Process(this->MirrorCoord)
		.Process(this->TheDuration)
		.Process(this->IsLaser)
		.Process(this->IsSupported)
		.Process(this->IsHouseColor)
		.Process(this->IsSingleColor)
		.Process(this->LaserInnerColor)
		.Process(this->LaserOuterColor)
		.Process(this->LaserOuterSpread)
		.Process(this->LaserThickness)
		.Process(this->LaserDuration)
		.Process(this->LaserDelay)
		.Process(this->DamageDelay)
		.Process(this->LaserTimer)
		.Process(this->DamageTimer)
		.Process(this->TechnoInLimbo)
		.Process(this->NotMainWeapon)
		.Process(this->FirepowerMult)
		.Process(this->FLHCoord)
		.Process(this->BuildingCoord)
		.Process(this->TemporaryCoord)
		;

	return true;
}

bool EngraveTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->SourceCoord)
		.Process(this->TargetCoord)
		.Process(this->MirrorCoord)
		.Process(this->TheDuration)
		.Process(this->IsLaser)
		.Process(this->IsSupported)
		.Process(this->IsHouseColor)
		.Process(this->IsSingleColor)
		.Process(this->LaserInnerColor)
		.Process(this->LaserOuterColor)
		.Process(this->LaserOuterSpread)
		.Process(this->LaserThickness)
		.Process(this->LaserDuration)
		.Process(this->LaserDelay)
		.Process(this->DamageDelay)
		.Process(this->LaserTimer)
		.Process(this->DamageTimer)
		.Process(this->TechnoInLimbo)
		.Process(this->NotMainWeapon)
		.Process(this->FirepowerMult)
		.Process(this->FLHCoord)
		.Process(this->BuildingCoord)
		.Process(this->TemporaryCoord)
		;

	return true;
}

void EngraveTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto const pType = this->GetTrajectoryType<EngraveTrajectoryType>(pBullet);

	this->SourceCoord = pType->SourceCoord;
	this->TargetCoord = pType->TargetCoord;
	this->MirrorCoord = pType->MirrorCoord;
	this->TheDuration = pType->TheDuration;
	this->IsLaser = pType->IsLaser;
	this->IsSupported = pType->IsSupported;
	this->IsHouseColor = pType->IsHouseColor;
	this->IsSingleColor = pType->IsSingleColor;
	this->LaserInnerColor = pType->LaserInnerColor;
	this->LaserOuterColor = pType->LaserOuterColor;
	this->LaserOuterSpread = pType->LaserOuterSpread;
	this->LaserThickness = pType->LaserThickness > 0 ? pType->LaserThickness : 1;
	this->LaserDuration = pType->LaserDuration > 0 ? pType->LaserDuration : 1;
	this->LaserDelay = pType->LaserDelay > 0 ? pType->LaserDelay : 1;
	this->DamageDelay = pType->DamageDelay > 0 ? pType->DamageDelay : 1;
	this->LaserTimer.StartTime = 0;
	this->DamageTimer.StartTime = 0;
	this->FLHCoord = pBullet->SourceCoords;
	this->BuildingCoord = CoordStruct::Empty;
	this->TemporaryCoord = CoordStruct::Empty;

	if (pBullet->Owner)
	{
		this->TechnoInLimbo = static_cast<bool>(pBullet->Owner->Transporter);
		this->NotMainWeapon = false;
		this->FirepowerMult = pBullet->Owner->FirepowerMultiplier;

		this->GetTechnoFLHCoord(pBullet, pBullet->Owner);
		this->CheckMirrorCoord(pBullet->Owner);
		this->SetEngraveDirection(pBullet, pBullet->Owner->GetCoords(), pBullet->TargetCoords);
	}
	else
	{
		this->TechnoInLimbo = false;
		this->NotMainWeapon = true;
		this->FirepowerMult = 1.0;

		this->SetEngraveDirection(pBullet, pBullet->SourceCoords, pBullet->TargetCoords);
	}

	double StraightSpeed = this->GetTrajectorySpeed(pBullet);
	StraightSpeed = StraightSpeed > 128.0 ? 128.0 : StraightSpeed;
	const double CoordDistance = pBullet->Velocity.Magnitude();
	pBullet->Velocity *= (CoordDistance > 0) ? (StraightSpeed / CoordDistance) : 0;

	if (this->TheDuration <= 0)
		this->TheDuration = static_cast<int>(CoordDistance / StraightSpeed) + 1;
}

bool EngraveTrajectory::OnAI(BulletClass* pBullet)
{
	if ((!pBullet->Owner && !this->NotMainWeapon) || this->TechnoInLimbo != static_cast<bool>(pBullet->Owner->Transporter))
		return true;

	if (--this->TheDuration < 0)
		return true;
	else if (this->PlaceOnCorrectHeight(pBullet))
		return true;

	TechnoClass* const pTechno = pBullet->Owner;
	HouseClass* const pOwner = pBullet->Owner->Owner;

	if (this->IsLaser && this->LaserTimer.Completed())
		this->DrawEngraveLaser(pBullet, pTechno, pOwner);

	if (this->DamageTimer.Completed())
		this->DetonateLaserWarhead(pBullet, pTechno, pOwner);

	return false;
}

void EngraveTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	pBullet->UnInit(); //Prevent damage again.
}

void EngraveTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
}

TrajectoryCheckReturnType EngraveTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

TrajectoryCheckReturnType EngraveTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

void EngraveTrajectory::GetTechnoFLHCoord(BulletClass* pBullet, TechnoClass* pTechno)
{
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pTechno);

	if (!pExt || !pExt->LastWeaponStruct || !pExt->LastWeaponStruct->WeaponType || pExt->LastWeaponStruct->WeaponType->Projectile != pBullet->Type)
	{
		this->NotMainWeapon = true;
		return;
	}
	else if (pTechno->WhatAmI() == AbstractType::Building)
	{
		const BuildingClass* const pBuilding = static_cast<BuildingClass*>(pTechno);
		Matrix3D mtx;
		mtx.MakeIdentity();

		if (pTechno->HasTurret())
		{
			TechnoTypeExt::ApplyTurretOffset(pBuilding->Type, &mtx);
			mtx.RotateZ(static_cast<float>(pTechno->TurretFacing().GetRadian<32>()));
		}

		mtx.Translate(static_cast<float>(pExt->LastWeaponFLH.X), static_cast<float>(pExt->LastWeaponFLH.Y), static_cast<float>(pExt->LastWeaponFLH.Z));
		auto const result = mtx.GetTranslation();
		this->BuildingCoord = pBullet->SourceCoords - pBuilding->GetCoords() - CoordStruct { static_cast<int>(result.X), -static_cast<int>(result.Y), static_cast<int>(result.Z) };
	}

	this->FLHCoord = pExt->LastWeaponFLH;
}

void EngraveTrajectory::CheckMirrorCoord(TechnoClass* pTechno)
{
	if (pTechno->CurrentBurstIndex % 2 == 0)
		return;

	if (this->MirrorCoord)
	{
		this->SourceCoord.Y = -(this->SourceCoord.Y);
		this->TargetCoord.Y = -(this->TargetCoord.Y);
	}
}

void EngraveTrajectory::SetEngraveDirection(BulletClass* pBullet, CoordStruct Source, CoordStruct Target)
{
	const double RotateAngle = Math::atan2(Target.Y - Source.Y , Target.X - Source.X);

	if (this->SourceCoord.X != 0 || this->SourceCoord.Y != 0)
	{
		Source = Target;
		Source.X += static_cast<int>(this->SourceCoord.X * Math::cos(RotateAngle) + this->SourceCoord.Y * Math::sin(RotateAngle));
		Source.Y += static_cast<int>(this->SourceCoord.X * Math::sin(RotateAngle) - this->SourceCoord.Y * Math::cos(RotateAngle));
	}

	Source.Z = this->GetFloorCoordHeight(pBullet, Source);
	pBullet->SetLocation(Source);

	Target.X += static_cast<int>(this->TargetCoord.X * Math::cos(RotateAngle) + this->TargetCoord.Y * Math::sin(RotateAngle));
	Target.Y += static_cast<int>(this->TargetCoord.X * Math::sin(RotateAngle) - this->TargetCoord.Y * Math::cos(RotateAngle));

	pBullet->Velocity.X = Target.X - Source.X;
	pBullet->Velocity.Y = Target.Y - Source.Y;
	pBullet->Velocity.Z = 0;
}

int EngraveTrajectory::GetFloorCoordHeight(BulletClass* pBullet, CoordStruct Coord)
{
	if (const CellClass* const pCell = MapClass::Instance->GetCellAt(Coord))
	{
		const int OnFloor = MapClass::Instance->GetCellFloorHeight(Coord);
		const int OnBridge = pCell->GetCoordsWithBridge().Z;

		if (pBullet->SourceCoords.Z >= OnBridge || pBullet->TargetCoords.Z >= OnBridge)
			return OnBridge;

		return OnFloor;
	}

	return Coord.Z;
}

bool EngraveTrajectory::PlaceOnCorrectHeight(BulletClass* pBullet)
{
	CoordStruct BulletCoords = pBullet->Location;

	if (this->TemporaryCoord != CoordStruct::Empty)
	{
		pBullet->SetLocation(this->TemporaryCoord);
		this->TemporaryCoord = CoordStruct::Empty;
	}

	CoordStruct FutureCoords
	{
		BulletCoords.X + static_cast<int>(pBullet->Velocity.X),
		BulletCoords.Y + static_cast<int>(pBullet->Velocity.Y),
		BulletCoords.Z + static_cast<int>(pBullet->Velocity.Z)
	};

	const int CheckDifference = this->GetFloorCoordHeight(pBullet, FutureCoords) - FutureCoords.Z;

	if (abs(CheckDifference) >= 384)
	{
		if (pBullet->Type->SubjectToCliffs)
		{
			return true;
		}
		else if (CheckDifference > 0)
		{
			BulletCoords.Z += CheckDifference;
			pBullet->SetLocation(BulletCoords);
		}
		else
		{
			FutureCoords.Z += CheckDifference;
			this->TemporaryCoord = FutureCoords;
		}
	}
	else
	{
		pBullet->Velocity.Z += CheckDifference;
	}

	return false;
}

void EngraveTrajectory::DrawEngraveLaser(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner)
{
	this->LaserTimer.Start(this->LaserDelay);
	LaserDrawClass* pLaser;
	CoordStruct FireCoord = pTechno->GetCoords();

	if (this->NotMainWeapon)
	{
		FireCoord = this->FLHCoord;
	}
	else if (pTechno->WhatAmI() != AbstractType::Building)
	{
		if (this->TechnoInLimbo)
			FireCoord = TechnoExt::GetFLHAbsoluteCoords(pTechno->Transporter, this->FLHCoord, pTechno->Transporter->HasTurret());
		else
			FireCoord = TechnoExt::GetFLHAbsoluteCoords(pTechno, this->FLHCoord, pTechno->HasTurret());
	}
	else
	{
		const BuildingClass* const pBuilding = static_cast<BuildingClass*>(pTechno);
		Matrix3D mtx;
		mtx.MakeIdentity();

		if (pTechno->HasTurret())
		{
			TechnoTypeExt::ApplyTurretOffset(pBuilding->Type, &mtx);
			mtx.RotateZ(static_cast<float>(pTechno->TurretFacing().GetRadian<32>()));
		}

		mtx.Translate(static_cast<float>(this->FLHCoord.X), static_cast<float>(this->FLHCoord.Y), static_cast<float>(this->FLHCoord.Z));
		auto const result = mtx.GetTranslation();
		FireCoord = pBuilding->GetCoords() + this->BuildingCoord + CoordStruct { static_cast<int>(result.X), -static_cast<int>(result.Y), static_cast<int>(result.Z) };
	}

	if (this->IsHouseColor)
	{
		pLaser = GameCreate<LaserDrawClass>(FireCoord, pBullet->Location, pOwner->LaserColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, this->LaserDuration);
		pLaser->IsHouseColor = true;
	}
	else if (this->IsSingleColor)
	{
		pLaser = GameCreate<LaserDrawClass>(FireCoord, pBullet->Location, this->LaserInnerColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, this->LaserDuration);
		pLaser->IsHouseColor = true;
	}
	else
	{
		pLaser = GameCreate<LaserDrawClass>(FireCoord, pBullet->Location, this->LaserInnerColor, this->LaserOuterColor, this->LaserOuterSpread, this->LaserDuration);
		pLaser->IsHouseColor = false;
	}

	pLaser->Thickness = this->LaserThickness;
	pLaser->IsSupported = this->IsSupported;
}

void EngraveTrajectory::DetonateLaserWarhead(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner)
{
	this->DamageTimer.Start(this->DamageDelay);
	WarheadTypeExt::DetonateAt(pBullet->WH, pBullet->Location, pTechno, static_cast<int>(pBullet->Health * this->FirepowerMult), pOwner);
}
