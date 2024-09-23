#include "EngraveTrajectory.h"
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <TacticalClass.h>
#include <LaserDrawClass.h>

PhobosTrajectory* EngraveTrajectoryType::CreateInstance() const
{
	return new EngraveTrajectory(this);
}

template<typename T>
void EngraveTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->SourceCoord)
		.Process(this->TargetCoord)
		.Process(this->MirrorCoord)
		.Process(this->ApplyRangeModifiers)
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
}

bool EngraveTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool EngraveTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<EngraveTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void EngraveTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	this->ApplyRangeModifiers.Read(exINI, pSection, "Trajectory.Engrave.ApplyRangeModifiers");
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

template<typename T>
void EngraveTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->SourceCoord)
		.Process(this->TargetCoord)
		.Process(this->MirrorCoord)
		.Process(this->ApplyRangeModifiers)
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
		.Process(this->FLHCoord)
		.Process(this->BuildingCoord)
		;
}

bool EngraveTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool EngraveTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);
	const_cast<EngraveTrajectory*>(this)->Serialize(Stm);
	return true;
}

void EngraveTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	this->FLHCoord = pBullet->SourceCoords;
	this->BuildingCoord = CoordStruct::Empty;

	TechnoClass* const pTechno = pBullet->Owner;

	if (pTechno)
	{
		this->TechnoInLimbo = static_cast<bool>(pTechno->Transporter);
		this->NotMainWeapon = false;

		this->GetTechnoFLHCoord(pBullet, pTechno);
		this->CheckMirrorCoord(pTechno);
		this->SetEngraveDirection(pBullet, pTechno->GetCoords(), pBullet->TargetCoords);
	}
	else
	{
		this->TechnoInLimbo = false;
		this->NotMainWeapon = true;

		this->SetEngraveDirection(pBullet, pBullet->SourceCoords, pBullet->TargetCoords);
	}

	double engraveSpeed = this->GetTrajectorySpeed(pBullet);
	engraveSpeed = engraveSpeed > 128.0 ? 128.0 : engraveSpeed;

	double coordDistance = pBullet->Velocity.Magnitude();
	pBullet->Velocity *= (coordDistance > 1e-10) ? (engraveSpeed / coordDistance) : 0;

	WeaponTypeClass* const pWeapon = pBullet->WeaponType;

	if (this->ApplyRangeModifiers && pWeapon && pTechno)
		coordDistance = static_cast<double>(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pTechno, static_cast<int>(coordDistance)));

	if (this->TheDuration <= 0)
		this->TheDuration = static_cast<int>(coordDistance / engraveSpeed) + 1;
}

bool EngraveTrajectory::OnAI(BulletClass* pBullet)
{
	TechnoClass* const pTechno = pBullet->Owner;

	if ((!pTechno && !this->NotMainWeapon) || this->TechnoInLimbo != static_cast<bool>(pTechno->Transporter))
		return true;

	if (--this->TheDuration < 0 || this->PlaceOnCorrectHeight(pBullet))
		return true;

	HouseClass* const pOwner = pTechno->Owner;

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

	if (!pExt || !pExt->LastWeaponType || pExt->LastWeaponType->Projectile != pBullet->Type)
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
	if (this->NotMainWeapon || !(pTechno->CurrentBurstIndex % 2))
		return;

	if (this->MirrorCoord)
	{
		this->SourceCoord.Y = -(this->SourceCoord.Y);
		this->TargetCoord.Y = -(this->TargetCoord.Y);
	}
}

void EngraveTrajectory::SetEngraveDirection(BulletClass* pBullet, CoordStruct theSource, CoordStruct theTarget)
{
	const double rotateAngle = Math::atan2(theTarget.Y - theSource.Y , theTarget.X - theSource.X);

	if (this->SourceCoord.X != 0 || this->SourceCoord.Y != 0)
	{
		theSource = theTarget;
		theSource.X += static_cast<int>(this->SourceCoord.X * Math::cos(rotateAngle) + this->SourceCoord.Y * Math::sin(rotateAngle));
		theSource.Y += static_cast<int>(this->SourceCoord.X * Math::sin(rotateAngle) - this->SourceCoord.Y * Math::cos(rotateAngle));
	}

	theSource.Z = this->GetFloorCoordHeight(pBullet, theSource);
	pBullet->SetLocation(theSource);

	theTarget.X += static_cast<int>(this->TargetCoord.X * Math::cos(rotateAngle) + this->TargetCoord.Y * Math::sin(rotateAngle));
	theTarget.Y += static_cast<int>(this->TargetCoord.X * Math::sin(rotateAngle) - this->TargetCoord.Y * Math::cos(rotateAngle));

	pBullet->Velocity.X = theTarget.X - theSource.X;
	pBullet->Velocity.Y = theTarget.Y - theSource.Y;
	pBullet->Velocity.Z = 0;
}

int EngraveTrajectory::GetFloorCoordHeight(BulletClass* pBullet, CoordStruct coord)
{
	if (const CellClass* const pCell = MapClass::Instance->GetCellAt(coord))
	{
		const int onFloor = MapClass::Instance->GetCellFloorHeight(coord);
		const int onBridge = pCell->GetCoordsWithBridge().Z;

		if (pBullet->SourceCoords.Z >= onBridge || pBullet->TargetCoords.Z >= onBridge)
			return onBridge;

		return onFloor;
	}

	return coord.Z;
}

bool EngraveTrajectory::PlaceOnCorrectHeight(BulletClass* pBullet)
{
	CoordStruct bulletCoords = pBullet->Location;
	CoordStruct futureCoords
	{
		bulletCoords.X + static_cast<int>(pBullet->Velocity.X),
		bulletCoords.Y + static_cast<int>(pBullet->Velocity.Y),
		bulletCoords.Z + static_cast<int>(pBullet->Velocity.Z)
	};

	const int checkDifference = this->GetFloorCoordHeight(pBullet, futureCoords) - futureCoords.Z;

	if (abs(checkDifference) >= 384)
	{
		if (pBullet->Type->SubjectToCliffs)
			return true;

		if (checkDifference > 0)
		{
			bulletCoords.Z += checkDifference;
			pBullet->SetLocation(bulletCoords);
		}
		else
		{
			const int nowDifference = bulletCoords.Z - this->GetFloorCoordHeight(pBullet, bulletCoords);

			if (nowDifference >= 256)
			{
				bulletCoords.Z -= nowDifference;
				pBullet->SetLocation(bulletCoords);
			}
		}
	}
	else
	{
		pBullet->Velocity.Z += checkDifference;
	}

	return false;
}

void EngraveTrajectory::DrawEngraveLaser(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner)
{
	this->LaserTimer.Start(this->LaserDelay);
	LaserDrawClass* pLaser;
	CoordStruct fireCoord = pTechno->GetCoords();

	if (this->NotMainWeapon)
	{
		fireCoord = this->FLHCoord;
	}
	else if (pTechno->WhatAmI() != AbstractType::Building)
	{
		if (this->TechnoInLimbo)
			fireCoord = TechnoExt::GetFLHAbsoluteCoords(pTechno->Transporter, this->FLHCoord, pTechno->Transporter->HasTurret());
		else
			fireCoord = TechnoExt::GetFLHAbsoluteCoords(pTechno, this->FLHCoord, pTechno->HasTurret());
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
		fireCoord = pBuilding->GetCoords() + this->BuildingCoord + CoordStruct { static_cast<int>(result.X), -static_cast<int>(result.Y), static_cast<int>(result.Z) };
	}

	if (this->IsHouseColor)
	{
		pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, pOwner->LaserColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, this->LaserDuration);
		pLaser->IsHouseColor = true;
	}
	else if (this->IsSingleColor)
	{
		pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, this->LaserInnerColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, this->LaserDuration);
		pLaser->IsHouseColor = true;
	}
	else
	{
		pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, this->LaserInnerColor, this->LaserOuterColor, this->LaserOuterSpread, this->LaserDuration);
		pLaser->IsHouseColor = false;
	}

	pLaser->Thickness = this->LaserThickness;
	pLaser->IsSupported = this->IsSupported;
}

void EngraveTrajectory::DetonateLaserWarhead(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner)
{
	this->DamageTimer.Start(this->DamageDelay);
	WarheadTypeExt::DetonateAt(pBullet->WH, pBullet->Location, pTechno, pBullet->Health, pOwner);
}
