#include "PhobosVirtualTrajectory.h"

#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>

template<typename T>
void VirtualTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->VirtualSourceCoord)
		.Process(this->VirtualTargetCoord)
		.Process(this->AllowFirerTurning)
		;
}

bool VirtualTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool VirtualTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<VirtualTrajectoryType*>(this)->Serialize(Stm);
	return true;
}
/*
void VirtualTrajectoryType::Read(CCINIClass* const pINI, const char* pSection) // Read separately
{
	this->PhobosTrajectoryType::Read(pINI, pSection);
	INI_EX exINI(pINI);

	this->VirtualSourceCoord.Read(exINI, pSection, "Trajectory.VirtualSourceCoord");
	this->VirtualTargetCoord.Read(exINI, pSection, "Trajectory.VirtualTargetCoord");
	this->AllowFirerTurning.Read(exINI, pSection, "Trajectory.AllowFirerTurning");
}
*/
template<typename T>
void VirtualTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->SurfaceFirerID)
//		.Process(this->Laser) // Should not save
		.Process(this->LaserTimer)
		;
}

bool VirtualTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	this->Laser = nullptr;
	return true;
}

bool VirtualTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);
	const_cast<VirtualTrajectory*>(this)->Serialize(Stm);
	return true;
}

void VirtualTrajectory::OnUnlimbo()
{
	this->PhobosTrajectory::OnUnlimbo();

	// Virtual
	this->RemainingDistance = INT_MAX;
	const auto pBullet = this->Bullet;
	const auto pWeapon = pBullet->WeaponType;

	if (pWeapon && pWeapon->IsLaser)
		this->LaserTimer.Start(pWeapon->LaserDuration);

	// Find the outermost transporter
	if (const auto pFirer = this->GetSurfaceFirer(this->Bullet->Owner))
		this->SurfaceFirerID = pFirer->UniqueID;

	// Waiting for launch trigger
	if (!BulletExt::ExtMap.Find(this->Bullet)->DispersedTrajectory)
		this->OpenFire();
}

bool VirtualTrajectory::OnEarlyUpdate()
{
	if (!this->NotMainWeapon && this->InvalidFireCondition(this->Bullet->Owner))
		return true;

	// Check whether need to detonate first
	if (this->PhobosTrajectory::OnEarlyUpdate())
		return true;

	// In the phase of playing PreImpactAnim
	if (this->Bullet->SpawnNextAnim)
		return false;

	// Draw laser
	if (this->Laser)
		this->UpdateTrackingLaser();
	else if (this->LaserTimer.HasTimeLeft())
		this->DrawTrackingLaser();

	return false;
}

void VirtualTrajectory::OnPreDetonate()
{
	this->PhobosTrajectory::OnPreDetonate();

	if (const auto pLaser = this->Laser)
	{
		// Auto free
		pLaser->Duration = 0;
		this->Laser = nullptr;
	}
}

bool VirtualTrajectory::InvalidFireCondition(TechnoClass* pTechno)
{
	if (!pTechno)
		return true;

	// Find the outermost transporter
	pTechno = this->GetSurfaceFirer(pTechno);

	if (!TechnoExt::IsActive(pTechno) || this->SurfaceFirerID != pTechno->UniqueID)
		return true;

	if (static_cast<const VirtualTrajectoryType*>(this->GetType())->AllowFirerTurning)
		return false;

	const auto tgtDir = DirStruct(-PhobosTrajectory::Get2DOpRadian(pTechno->GetCoords(), this->Bullet->TargetCoords));
	const auto& face = pTechno->HasTurret() && pTechno->WhatAmI() == AbstractType::Unit ? pTechno->SecondaryFacing : pTechno->PrimaryFacing;
	const auto curDir = face.Current();

	// Similar to the vanilla 45 degree turret facing check design
	return (std::abs(static_cast<short>(static_cast<short>(tgtDir.Raw) - static_cast<short>(curDir.Raw))) >= 4096);
}

void VirtualTrajectory::DrawTrackingLaser()
{
	const auto pBullet = this->Bullet;
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon || !pWeapon->IsLaser)
		return;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto pFirer = pBullet->Owner;
	const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
	auto fireCoord = pBullet->SourceCoords;

	// Find the outermost transporter
	pFirer = this->GetSurfaceFirer(pFirer);

	// Considering that the CurrentBurstIndex may be different, it is not possible to call existing functions
	if (!this->NotMainWeapon && pFirer && !pFirer->InLimbo)
		fireCoord = TechnoExt::GetFLHAbsoluteCoords(pFirer, this->FLHCoord, pFirer->HasTurret());

	// Draw laser from head to tail
	if (pWeapon->IsHouseColor || pWeaponExt->Laser_IsSingleColor)
	{
		const auto pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, ((pWeapon->IsHouseColor && pOwner) ? pOwner->LaserColor : pWeapon->LaserInnerColor), ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, INT_MAX);
		this->Laser = pLaser;
		pLaser->IsHouseColor = true;
		pLaser->Thickness = pWeaponExt->LaserThickness;
		pLaser->IsSupported = pLaser->Thickness > 3;
		pLaser->Fades = false;
		pLaser->Progress.Value = 0;
	}
	else
	{
		const auto pLaser = GameCreate<LaserDrawClass>(fireCoord, pBullet->Location, pWeapon->LaserInnerColor, pWeapon->LaserOuterColor, pWeapon->LaserOuterSpread, INT_MAX);
		this->Laser = pLaser;
		pLaser->IsHouseColor = false;
		pLaser->Thickness = 3;
		pLaser->IsSupported = false;
		pLaser->Fades = false;
		pLaser->Progress.Value = 0;
	}
}

void VirtualTrajectory::UpdateTrackingLaser()
{
	const auto pLaser = this->Laser;

	// Check whether the timer expired
	if (!this->LaserTimer.HasTimeLeft())
	{
		// Auto free
		pLaser->Duration = 0;
		this->Laser = nullptr;
		return;
	}

	const auto pBullet = this->Bullet;

	// Find the outermost transporter
	const auto pFirer = this->GetSurfaceFirer(pBullet->Owner);

	// Considering that the CurrentBurstIndex may be different, it is not possible to call existing functions
	if (!this->NotMainWeapon && pFirer && !pFirer->InLimbo)
		pLaser->Source = TechnoExt::GetFLHAbsoluteCoords(pFirer, this->FLHCoord, pFirer->HasTurret());

	pLaser->Target = pBullet->Location;
}
