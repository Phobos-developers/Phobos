#include "EngraveTrajectory.h"
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
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
		.Process(this->TemporaryCoord)
		;

	return true;
}

void EngraveTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	if (!pBullet->WeaponType) //Bullets create from AirburstWeapon have no WeaponType.
		return;

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
	this->TemporaryCoord = CoordStruct::Empty;
	this->FLHCoord = pBullet->SourceCoords;

	if (pBullet->Owner)
	{
		this->TechnoInLimbo = pBullet->Owner->InLimbo;
		this->NotMainWeapon = false;
		this->FirepowerMult = pBullet->Owner->FirepowerMultiplier;

		this->CheckMirrorCoord(pBullet->Owner, (this->IsLaser ? this->GetTechnoFLHCoord(pBullet) : true));
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
	if (!pBullet->WeaponType || (!pBullet->Owner && !this->NotMainWeapon) || this->TechnoInLimbo != pBullet->Owner->InLimbo)
		return true;

	if (--this->TheDuration < 0)
		return true;
	else if (this->PlaceOnCorrectHeight(pBullet))
		return true;

	TechnoClass* const pTechno = pBullet->Owner;
	HouseClass* const pOwner = pBullet->Owner->Owner;

	if (this->IsLaser && this->LaserTimer.Completed() && this->DrawEngraveLaser(pBullet, pTechno, pOwner))
		return true;

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

bool EngraveTrajectory::GetTechnoFLHCoord(BulletClass* pBullet)
{
	int WeaponIndex = 0;
	bool AccurateFLHFound = false;

	if (pBullet->WeaponType != TechnoExt::GetCurrentWeapon(pBullet->Owner, WeaponIndex, false) && pBullet->WeaponType != TechnoExt::GetCurrentWeapon(pBullet->Owner, WeaponIndex, true))
	{
		this->NotMainWeapon = true;
		return true;
	}

	if (!this->TechnoInLimbo)
	{
		CoordStruct FLH = TechnoExt::GetBurstFLH(pBullet->Owner, WeaponIndex, AccurateFLHFound);

		if (!AccurateFLHFound)
		{
			if (InfantryClass* const pInfantry = abstract_cast<InfantryClass*>(pBullet->Owner))
				FLH = TechnoExt::GetSimpleFLH(pInfantry, WeaponIndex, AccurateFLHFound);
		}

		this->FLHCoord = AccurateFLHFound ? FLH : pBullet->Owner->GetWeapon(WeaponIndex)->FLH;
	}
	else if (const TechnoClass* const pTransporter = pBullet->Owner->Transporter)
	{
		const FootClass* pCurrentPassenger = pTransporter->Passengers.GetFirstPassenger();
		const FootClass* const pBulletOwnerFoot = abstract_cast<FootClass*>(pBullet->Owner);

		while (pCurrentPassenger)
		{
			if (pBulletOwnerFoot != pCurrentPassenger)
			{
				WeaponIndex += 1;
				pCurrentPassenger = abstract_cast<FootClass*>(pCurrentPassenger->NextObject);
			}
			else
			{
				break;
			}
		}

		AccurateFLHFound = true;
		auto const pTransporterTypeExt = TechnoTypeExt::ExtMap.Find(pTransporter->GetTechnoType());
		this->FLHCoord = (WeaponIndex < static_cast<int>(pTransporterTypeExt->AlternateFLHs.size())) ? pTransporterTypeExt->AlternateFLHs[WeaponIndex] : pTransporter->GetTechnoType()->Weapon[0].FLH;
	}
	else
	{
		this->NotMainWeapon = true;
	}

	return AccurateFLHFound;
}

void EngraveTrajectory::CheckMirrorCoord(TechnoClass* pTechno, bool Found)
{
	if (pTechno->CurrentBurstIndex % 2 == 0)
		return;

	if (!Found)
		this->FLHCoord.Y = -(this->FLHCoord.Y);

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

bool EngraveTrajectory::DrawEngraveLaser(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner)
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
		{
			if (TechnoClass* const pTransporter = pTechno->Transporter)
				FireCoord = TechnoExt::GetFLHAbsoluteCoords(pTransporter, this->FLHCoord, pTransporter->HasTurret());
			else
				return true;
		}
		else
		{
			FireCoord = TechnoExt::GetFLHAbsoluteCoords(pTechno, this->FLHCoord, pTechno->HasTurret());
		}
	}
	else // TODO Not accurate now, just get the similar FLH.
	{
		const double RotateAngle = pTechno->HasTurret() ? -(pTechno->TurretFacing().GetRadian<32>()) : -(pTechno->PrimaryFacing.Current().GetRadian<32>());
		FireCoord.X += static_cast<int>(this->FLHCoord.X * Math::cos(RotateAngle) + this->FLHCoord.Y * Math::sin(RotateAngle));
		FireCoord.Y += static_cast<int>(this->FLHCoord.X * Math::sin(RotateAngle) - this->FLHCoord.Y * Math::cos(RotateAngle));

		if (const BuildingTypeClass* const pBuildingType = static_cast<BuildingTypeClass*>(pTechno->GetTechnoType()))
			FireCoord.Z += this->FLHCoord.Z + 30 * (pBuildingType->GetFoundationWidth() + pBuildingType->GetFoundationHeight(false) + 2);
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

	return false;
}

void EngraveTrajectory::DetonateLaserWarhead(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner)
{
	this->DamageTimer.Start(this->DamageDelay);
	const int LaserDamage = static_cast<int>(pBullet->WeaponType->Damage * this->FirepowerMult);
	WarheadTypeExt::DetonateAt(pBullet->WH, pBullet->Location, pTechno, LaserDamage, pOwner);
}
