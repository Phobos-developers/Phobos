#include "EngraveTrajectory.h"

#include <TacticalClass.h>
#include <LaserDrawClass.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>

std::unique_ptr<PhobosTrajectory> EngraveTrajectoryType::CreateInstance() const
{
	return std::make_unique<EngraveTrajectory>(this);
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

	if (this->Trajectory_Speed > 128.0)
		this->Trajectory_Speed = 128.0;

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

	if (this->LaserThickness <= 0)
		this->LaserThickness = 1;

	this->LaserDuration.Read(exINI, pSection, "Trajectory.Engrave.LaserDuration");

	if (this->LaserDuration <= 0)
		this->LaserDuration = 1;

	this->LaserDelay.Read(exINI, pSection, "Trajectory.Engrave.LaserDelay");

	if (this->LaserDelay <= 0)
		this->LaserDelay = 1;

	this->DamageDelay.Read(exINI, pSection, "Trajectory.Engrave.DamageDelay");

	if (this->DamageDelay <= 0)
		this->DamageDelay = 1;
}

template<typename T>
void EngraveTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->SourceCoord)
		.Process(this->TargetCoord)
		.Process(this->TheDuration)
		.Process(this->LaserTimer)
		.Process(this->DamageTimer)
		.Process(this->TechnoInTransport)
		.Process(this->NotMainWeapon)
		.Process(this->FLHCoord)
		.Process(this->BuildingCoord)
		;
}

bool EngraveTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Serialize(Stm);
	return true;
}

bool EngraveTrajectory::Save(PhobosStreamWriter& Stm) const
{
	const_cast<EngraveTrajectory*>(this)->Serialize(Stm);
	return true;
}

void EngraveTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	const auto pType = this->Type;
	this->LaserTimer.Start(0);
	this->DamageTimer.Start(0);
	this->FLHCoord = pBullet->SourceCoords;
	const auto pTechno = pBullet->Owner;

	if (pTechno)
	{
		this->TechnoInTransport = static_cast<bool>(pTechno->Transporter);

		this->GetTechnoFLHCoord(pBullet, pTechno);
		this->CheckMirrorCoord(pTechno);
		this->SetEngraveDirection(pBullet, pTechno->GetCoords(), pBullet->TargetCoords);
	}
	else
	{
		this->TechnoInTransport = false;
		this->NotMainWeapon = true;

		this->SetEngraveDirection(pBullet, pBullet->SourceCoords, pBullet->TargetCoords);
	}

	auto coordDistance = pBullet->Velocity.Magnitude();
	pBullet->Velocity *= (coordDistance > 1e-10) ? (pType->Trajectory_Speed / coordDistance) : 0;

	const auto pWeapon = pBullet->WeaponType;

	if (pType->ApplyRangeModifiers && pWeapon && pTechno)
		coordDistance = static_cast<double>(WeaponTypeExt::GetRangeWithModifiers(pWeapon, pTechno, static_cast<int>(coordDistance)));

	if (this->TheDuration <= 0)
		this->TheDuration = static_cast<int>(coordDistance / pType->Trajectory_Speed) + 1;
}

bool EngraveTrajectory::OnAI(BulletClass* pBullet)
{
	const auto pTechno = pBullet->Owner;

	if (!this->NotMainWeapon && this->InvalidFireCondition(pTechno))
		return true;

	if (--this->TheDuration < 0 || this->PlaceOnCorrectHeight(pBullet))
		return true;

	const auto pOwner = pTechno ? pTechno->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

	if (this->Type->IsLaser && this->LaserTimer.Completed())
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
	const auto pExt = TechnoExt::ExtMap.Find(pTechno);

	if (!pExt || !pExt->LastWeaponType || pExt->LastWeaponType->Projectile != pBullet->Type)
	{
		this->NotMainWeapon = true;
		return;
	}
	else if (pTechno->WhatAmI() == AbstractType::Building)
	{
		const auto pBuilding = static_cast<BuildingClass*>(pTechno);
		Matrix3D mtx;
		mtx.MakeIdentity();

		if (pTechno->HasTurret())
		{
			TechnoTypeExt::ApplyTurretOffset(pBuilding->Type, &mtx);
			mtx.RotateZ(static_cast<float>(pTechno->TurretFacing().GetRadian<32>()));
		}

		mtx.Translate(static_cast<float>(pExt->LastWeaponFLH.X), static_cast<float>(pExt->LastWeaponFLH.Y), static_cast<float>(pExt->LastWeaponFLH.Z));
		const auto result = mtx.GetTranslation();
		this->BuildingCoord = pBullet->SourceCoords - pBuilding->GetCoords() - CoordStruct { static_cast<int>(result.X), -static_cast<int>(result.Y), static_cast<int>(result.Z) };
	}

	this->FLHCoord = pExt->LastWeaponFLH;
}

void EngraveTrajectory::CheckMirrorCoord(TechnoClass* pTechno)
{
	if (this->NotMainWeapon || !(pTechno->CurrentBurstIndex % 2))
		return;

	if (this->Type->MirrorCoord)
	{
		this->SourceCoord.Y = -(this->SourceCoord.Y);
		this->TargetCoord.Y = -(this->TargetCoord.Y);
	}
}

void EngraveTrajectory::SetEngraveDirection(BulletClass* pBullet, CoordStruct theSource, CoordStruct theTarget)
{
	const auto rotateAngle = Math::atan2(theTarget.Y - theSource.Y , theTarget.X - theSource.X);

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

bool EngraveTrajectory::InvalidFireCondition(TechnoClass* pTechno)
{
	return (!pTechno
		|| !pTechno->IsAlive
		|| (pTechno->InLimbo && !pTechno->Transporter)
		|| pTechno->IsSinking
		|| pTechno->Health <= 0
		|| this->TechnoInTransport != static_cast<bool>(pTechno->Transporter)
		|| pTechno->Deactivated
		|| pTechno->TemporalTargetingMe
		|| pTechno->BeingWarpedOut
		|| pTechno->IsUnderEMP());
}

int EngraveTrajectory::GetFloorCoordHeight(BulletClass* pBullet, CoordStruct coord)
{
	if (const auto pCell = MapClass::Instance->GetCellAt(coord))
	{
		const auto onFloor = MapClass::Instance->GetCellFloorHeight(coord);
		const auto onBridge = pCell->GetCoordsWithBridge().Z;

		if (pBullet->SourceCoords.Z >= onBridge || pBullet->TargetCoords.Z >= onBridge)
			return onBridge;

		return onFloor;
	}

	return coord.Z;
}

bool EngraveTrajectory::PlaceOnCorrectHeight(BulletClass* pBullet)
{
	auto bulletCoords = pBullet->Location;
	CoordStruct futureCoords
	{
		bulletCoords.X + static_cast<int>(pBullet->Velocity.X),
		bulletCoords.Y + static_cast<int>(pBullet->Velocity.Y),
		bulletCoords.Z + static_cast<int>(pBullet->Velocity.Z)
	};

	const auto checkDifference = this->GetFloorCoordHeight(pBullet, futureCoords) - futureCoords.Z;

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
			const auto nowDifference = bulletCoords.Z - this->GetFloorCoordHeight(pBullet, bulletCoords);

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
	const auto pType = this->Type;
	this->LaserTimer.Start(pType->LaserDelay);

	const auto pTransporter = pTechno->Transporter;
	auto fireCoord = pBullet->SourceCoords;

	if (!this->NotMainWeapon && pTechno && (pTransporter || !pTechno->InLimbo))
	{
		if (pTechno->WhatAmI() != AbstractType::Building)
		{
			if (pTransporter)
				fireCoord = TechnoExt::GetFLHAbsoluteCoords(pTransporter, this->FLHCoord, pTransporter->HasTurret());
			else
				fireCoord = TechnoExt::GetFLHAbsoluteCoords(pTechno, this->FLHCoord, pTechno->HasTurret());
		}
		else
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			Matrix3D mtx;
			mtx.MakeIdentity();

			if (pTechno->HasTurret())
			{
				TechnoTypeExt::ApplyTurretOffset(pBuilding->Type, &mtx);
				mtx.RotateZ(static_cast<float>(pTechno->TurretFacing().GetRadian<32>()));
			}

			mtx.Translate(static_cast<float>(this->FLHCoord.X), static_cast<float>(this->FLHCoord.Y), static_cast<float>(this->FLHCoord.Z));
			const auto result = mtx.GetTranslation();
			fireCoord = pBuilding->GetCoords() + this->BuildingCoord + CoordStruct { static_cast<int>(result.X), -static_cast<int>(result.Y), static_cast<int>(result.Z) };
		}
	}

	if (pType->IsHouseColor || pType->IsSingleColor)
	{
		const auto pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, ((pType->IsHouseColor && pOwner) ? pOwner->LaserColor : pType->LaserInnerColor), ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, pType->LaserDuration);
		pLaser->IsHouseColor = true;
		pLaser->Thickness = pType->LaserThickness;
		pLaser->IsSupported = pType->IsSupported;
	}
	else
	{
		const auto pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, pType->LaserInnerColor, pType->LaserOuterColor, pType->LaserOuterSpread, pType->LaserDuration);
		pLaser->IsHouseColor = false;
		pLaser->Thickness = 3;
		pLaser->IsSupported = false;
	}
}

void EngraveTrajectory::DetonateLaserWarhead(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner)
{
	const auto pType = this->Type;
	this->DamageTimer.Start(pType->DamageDelay);
	WarheadTypeExt::DetonateAt(pBullet->WH, pBullet->Location, pTechno, pBullet->Health, pOwner);
}
