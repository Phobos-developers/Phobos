#include "TracingTrajectory.h"
#include "StraightTrajectory.h"
// #include "DisperseTrajectory.h" // TODO If merge #1295
// #include "EngraveTrajectory.h" // TODO If merge #1293

#include <AnimClass.h>
#include <LaserDrawClass.h>
#include <EBolt.h>
#include <RadBeam.h>
#include <ParticleSystemClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Anim/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.Alex.h>

std::unique_ptr<PhobosTrajectory> TracingTrajectoryType::CreateInstance() const
{
	return std::make_unique<TracingTrajectory>(this);
}

template<typename T>
void TracingTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->TraceMode)
		.Process(this->TheDuration)
		.Process(this->TolerantTime)
		.Process(this->ROT)
		.Process(this->BulletSpin)
		.Process(this->PeacefullyVanish)
		.Process(this->TraceTheTarget)
		.Process(this->CreateAtTarget)
		.Process(this->CreateCoord)
		.Process(this->OffsetCoord)
		.Process(this->WeaponCoord)
		.Process(this->UseDisperseCoord)
		.Process(this->AllowFirerTurning)
		.Process(this->Weapons)
		.Process(this->WeaponCount)
		.Process(this->WeaponDelay)
		.Process(this->WeaponInitialDelay)
		.Process(this->WeaponCycle)
		.Process(this->WeaponCheck)
		.Process(this->Synchronize)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		;
}

bool TracingTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool TracingTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<TracingTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void TracingTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	pINI->ReadString(pSection, "Trajectory.Tracing.TraceMode", "", Phobos::readBuffer);
	if (INIClass::IsBlank(Phobos::readBuffer))
		this->TraceMode = TraceTargetMode::Connection;
	else if (_stricmp(Phobos::readBuffer, "global") == 0)
		this->TraceMode = TraceTargetMode::Global;
	else if (_stricmp(Phobos::readBuffer, "body") == 0)
		this->TraceMode = TraceTargetMode::Body;
	else if (_stricmp(Phobos::readBuffer, "turret") == 0)
		this->TraceMode = TraceTargetMode::Turret;
	else if (_stricmp(Phobos::readBuffer, "rotatecw") == 0)
		this->TraceMode = TraceTargetMode::RotateCW;
	else if (_stricmp(Phobos::readBuffer, "rotateccw") == 0)
		this->TraceMode = TraceTargetMode::RotateCCW;
	else
		this->TraceMode = TraceTargetMode::Connection;

	this->TheDuration.Read(exINI, pSection, "Trajectory.Tracing.TheDuration");
	this->TolerantTime.Read(exINI, pSection, "Trajectory.Tracing.TolerantTime");
	this->ROT.Read(exINI, pSection, "Trajectory.Tracing.ROT");
	this->BulletSpin.Read(exINI, pSection, "Trajectory.Tracing.BulletSpin");
	this->PeacefullyVanish.Read(exINI, pSection, "Trajectory.Tracing.PeacefullyVanish");
	this->TraceTheTarget.Read(exINI, pSection, "Trajectory.Tracing.TraceTheTarget");
	this->CreateAtTarget.Read(exINI, pSection, "Trajectory.Tracing.CreateAtTarget");
	this->CreateCoord.Read(exINI, pSection, "Trajectory.Tracing.CreateCoord");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.Tracing.OffsetCoord");
	this->WeaponCoord.Read(exINI, pSection, "Trajectory.Tracing.WeaponCoord");
	this->UseDisperseCoord.Read(exINI, pSection, "Trajectory.Tracing.UseDisperseCoord");
	this->AllowFirerTurning.Read(exINI, pSection, "Trajectory.Engrave.AllowFirerTurning");
	this->Weapons.Read(exINI, pSection, "Trajectory.Tracing.Weapons");
	this->WeaponCount.Read(exINI, pSection, "Trajectory.Tracing.WeaponCount");
	this->WeaponDelay.Read(exINI, pSection, "Trajectory.Tracing.WeaponDelay");
	this->WeaponInitialDelay.Read(exINI, pSection, "Trajectory.Tracing.WeaponInitialDelay");
	this->WeaponInitialDelay = Math::max(0, this->WeaponInitialDelay);
	this->WeaponCycle.Read(exINI, pSection, "Trajectory.Tracing.WeaponCycle");
	this->WeaponCheck.Read(exINI, pSection, "Trajectory.Tracing.WeaponCheck");
	this->Synchronize.Read(exINI, pSection, "Trajectory.Tracing.Synchronize");
	this->SuicideAboveRange.Read(exINI, pSection, "Trajectory.Tracing.SuicideAboveRange");
	this->SuicideIfNoWeapon.Read(exINI, pSection, "Trajectory.Tracing.SuicideIfNoWeapon");
}

template<typename T>
void TracingTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->WeaponIndex)
		.Process(this->WeaponCount)
		.Process(this->WeaponCycle)
		.Process(this->ExistTimer)
		.Process(this->WeaponTimer)
		.Process(this->TolerantTimer)
		.Process(this->TechnoInTransport)
		.Process(this->NotMainWeapon)
		.Process(this->FLHCoord)
		.Process(this->BuildingCoord)
		.Process(this->FirepowerMult)
		;
}

bool TracingTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Serialize(Stm);
	return true;
}

bool TracingTrajectory::Save(PhobosStreamWriter& Stm) const
{
	const_cast<TracingTrajectory*>(this)->Serialize(Stm);
	return true;
}

void TracingTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	const auto pType = this->Type;

	if (!pType->Weapons.empty() && !pType->WeaponCount.empty() && this->WeaponCycle)
	{
		this->WeaponCount = pType->WeaponCount[0];
		this->WeaponTimer.Start(pType->WeaponInitialDelay);
	}

	this->FLHCoord = pBullet->SourceCoords;

	if (const auto pFirer = pBullet->Owner)
	{
		this->TechnoInTransport = static_cast<bool>(pFirer->Transporter);
		this->FirepowerMult = pFirer->FirepowerMultiplier;

		if (const auto pExt = TechnoExt::ExtMap.Find(pFirer))
			this->FirepowerMult *= pExt->AE.FirepowerMultiplier;

		this->GetTechnoFLHCoord(pBullet, pFirer);
	}
	else
	{
		this->TechnoInTransport = false;
		this->NotMainWeapon = true;
	}

	this->SetSourceLocation(pBullet);
	this->InitializeDuration(pBullet, pType->TheDuration);
}

bool TracingTrajectory::OnAI(BulletClass* pBullet)
{
	const auto pTechno = pBullet->Owner;

	if (!this->NotMainWeapon && this->InvalidFireCondition(pBullet, pTechno))
		return true;

	if (this->BulletDetonatePreCheck(pBullet))
		return true;

	if (this->WeaponTimer.Completed() && this->CheckFireFacing(pBullet) && this->PrepareTracingWeapon(pBullet))
		return true;

	if (this->ExistTimer.Completed())
		return true;

	this->ChangeFacing(pBullet);

	return false;
}

void TracingTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	if (this->Type->PeacefullyVanish)
	{
		pBullet->Health = 0;
		pBullet->Limbo();
		pBullet->UnInit();
	}
}

void TracingTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	*pSpeed = this->ChangeVelocity(pBullet); // This is for location calculation
	const auto pType = this->Type;

	if (pType->ROT < 0 && !pType->BulletSpin) // This is for image drawing
		pBullet->Velocity = *pSpeed;
}

TrajectoryCheckReturnType TracingTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

TrajectoryCheckReturnType TracingTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

void TracingTrajectory::GetTechnoFLHCoord(BulletClass* pBullet, TechnoClass* pTechno)
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

void TracingTrajectory::SetSourceLocation(BulletClass* pBullet)
{
	const auto pType = this->Type;
	const auto theCoords = pType->CreateCoord.Get();
	auto theOffset = theCoords;

	if (theCoords.X != 0 || theCoords.Y != 0)
	{
		const auto& theSource = pBullet->SourceCoords;
		const auto& theTarget = pBullet->TargetCoords;
		const auto rotateAngle = Math::atan2(theTarget.Y - theSource.Y , theTarget.X - theSource.X);

		theOffset.X = static_cast<int>(theCoords.X * Math::cos(rotateAngle) + theCoords.Y * Math::sin(rotateAngle));
		theOffset.Y = static_cast<int>(theCoords.X * Math::sin(rotateAngle) - theCoords.Y * Math::cos(rotateAngle));
	}

	if (pType->CreateAtTarget)
	{
		if (const auto pTarget = pBullet->Target)
			pBullet->SetLocation(pTarget->GetCoords() + theOffset);
		else
			pBullet->SetLocation(pBullet->TargetCoords + theOffset);
	}
	else
	{
		pBullet->SetLocation(pBullet->SourceCoords + theOffset);
	}

	BulletVelocity facing
	{
		static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X),
		static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y),
		0
	};

	pBullet->Velocity = facing * (1 / facing.Magnitude());
}

void TracingTrajectory::InitializeDuration(BulletClass* pBullet, int duration)
{
	if (duration <= 0)
	{
		if (const auto pWeapon = pBullet->WeaponType)
			duration = (pWeapon->ROF > 10) ? pWeapon->ROF - 10 : 1;
		else
			duration = 120;
	}

	this->ExistTimer.Start(duration);
}

bool TracingTrajectory::InvalidFireCondition(BulletClass* pBullet, TechnoClass* pTechno)
{
	if (!pTechno
		|| !pTechno->IsAlive
		|| (pTechno->InLimbo && !pTechno->Transporter)
		|| pTechno->IsSinking
		|| pTechno->Health <= 0
		|| this->TechnoInTransport != static_cast<bool>(pTechno->Transporter)
		|| pTechno->Deactivated
		|| pTechno->TemporalTargetingMe
		|| pTechno->BeingWarpedOut
		|| pTechno->IsUnderEMP())
	{
		return true;
	}

	if (this->Type->AllowFirerTurning)
		return false;

	const auto SourceCrd = pTechno->GetCoords();
	const auto TargetCrd = pBullet->TargetCoords;

	const auto rotateAngle = Math::atan2(TargetCrd.Y - SourceCrd.Y , TargetCrd.X - SourceCrd.X);
	const auto tgtDir = DirStruct(-rotateAngle);

	const auto& face = pTechno->HasTurret() ? pTechno->SecondaryFacing : pTechno->PrimaryFacing;
	const auto curDir = face.Current();

	return (std::abs(static_cast<short>(static_cast<short>(tgtDir.Raw) - static_cast<short>(curDir.Raw))) >= 4096);
}

bool TracingTrajectory::BulletDetonatePreCheck(BulletClass* pBullet)
{
	const auto pType = this->Type;

	if (!pBullet->Target && !pType->TolerantTime)
		return true;

	const auto pTechno = pBullet->Owner;

	if (!pType->TraceTheTarget && !pTechno)
		return true;

	if (pType->Synchronize)
	{
		if (pTechno)
		{
			if (pBullet->Target != pTechno->Target && !pType->TolerantTime)
				return true;

			pBullet->Target = pTechno->Target;
		}
	}

	if (const auto pTarget = pBullet->Target)
	{
		pBullet->TargetCoords = pTarget->GetCoords();
		this->TolerantTimer.Stop();
	}
	else if (pType->TolerantTime > 0)
	{
		if (this->TolerantTimer.Completed())
			return true;
		else if (!this->TolerantTimer.IsTicking())
			this->TolerantTimer.Start(pType->TolerantTime);
	}

	if (pType->SuicideAboveRange)
	{
		if (const auto pWeapon = pBullet->WeaponType)
		{
			auto source = (pTechno && !this->NotMainWeapon) ? pTechno->GetCoords() : pBullet->SourceCoords;
			auto target = pBullet->TargetCoords;

			if (this->NotMainWeapon || (pTechno && pTechno->IsInAir()))
			{
				source.Z = 0;
				target.Z = 0;
			}

			if (static_cast<int>(source.DistanceFrom(target)) >= (pWeapon->Range + 256))
				return true;
		}
	}

	return false;
}

void TracingTrajectory::ChangeFacing(BulletClass* pBullet)
{
	const auto pType = this->Type;
	constexpr double ratio = Math::TwoPi / 256;

	if (!pType->BulletSpin)
	{
		if (pType->ROT < 0)
			return;

		BulletVelocity desiredFacing
		{
			static_cast<double>(pBullet->TargetCoords.X - pBullet->Location.X),
			static_cast<double>(pBullet->TargetCoords.Y - pBullet->Location.Y),
			0
		};

		if (!pType->ROT)
		{
			pBullet->Velocity = desiredFacing * (1 / desiredFacing.Magnitude());
			return;
		}

		const auto current = Math::atan2(pBullet->Velocity.Y, pBullet->Velocity.X);
		const auto desired = Math::atan2(desiredFacing.Y, desiredFacing.X);
		const auto rotate = pType->ROT * ratio;

		const auto differenceP = desired - current;
		const bool dir1 = differenceP > 0;
		const auto delta = dir1 ? differenceP : -differenceP;

		const bool dir2 = delta > Math::Pi;
		const auto differenceR = dir2 ? (Math::TwoPi - delta) : delta;
		const bool dirR = dir1 ^ dir2;

		if (differenceR <= rotate)
		{
			pBullet->Velocity = desiredFacing * (1 / desiredFacing.Magnitude());
			return;
		}

		const auto facing = current + (dirR ? rotate : -rotate);
		pBullet->Velocity.X = Math::cos(facing);
		pBullet->Velocity.Y = Math::sin(facing);
		pBullet->Velocity.Z = 0;
	}
	else
	{
		const auto radian = Math::atan2(pBullet->Velocity.Y, pBullet->Velocity.X) + (pType->ROT * ratio);
		pBullet->Velocity.X = Math::cos(radian);
		pBullet->Velocity.Y = Math::sin(radian);
		pBullet->Velocity.Z = 0;
	}
}

bool TracingTrajectory::CheckFireFacing(BulletClass* pBullet)
{
	const auto pType = this->Type;

	if (!pType->WeaponCheck || !pType->Synchronize || pType->TraceTheTarget || pType->ROT < 0 || pType->BulletSpin)
		return true;

	const auto& theBullet = pBullet->Location;
	const auto& theTarget = pBullet->TargetCoords;
	const auto targetDir = DirStruct { Math::atan2(theTarget.Y - theBullet.Y, theTarget.X - theBullet.X) };
	const auto bulletDir = DirStruct { Math::atan2(pBullet->Velocity.Y, pBullet->Velocity.X) };

	return std::abs(static_cast<short>(static_cast<short>(targetDir.Raw) - static_cast<short>(bulletDir.Raw))) <= (2048 + (pType->ROT << 8));
}

BulletVelocity TracingTrajectory::ChangeVelocity(BulletClass* pBullet)
{
	const auto pType = this->Type;
	const auto pFirer = pBullet->Owner;

	const auto destination = (pType->TraceTheTarget || !pFirer) ? pBullet->TargetCoords : (pFirer->Transporter ? pFirer->Transporter->GetCoords() : pFirer->GetCoords());
	const auto theCoords = pType->OffsetCoord.Get();
	auto theOffset = theCoords;

	if (theCoords.X != 0 || theCoords.Y != 0)
	{
		switch (pType->TraceMode)
		{
		case TraceTargetMode::Global:
		{
			break;
		}
		case TraceTargetMode::Body:
		{
			if (const auto pTechno = abstract_cast<TechnoClass*>(pType->TraceTheTarget ? pBullet->Target : pBullet->Owner))
			{
				const auto rotateAngle = -(pTechno->PrimaryFacing.Current().GetRadian<32>());

				theOffset.X = static_cast<int>(theCoords.X * Math::cos(rotateAngle) + theCoords.Y * Math::sin(rotateAngle));
				theOffset.Y = static_cast<int>(theCoords.X * Math::sin(rotateAngle) - theCoords.Y * Math::cos(rotateAngle));
			}
			else
			{
				theOffset.X = 0;
				theOffset.Y = 0;
			}

			break;
		}
		case TraceTargetMode::Turret:
		{
			if (const auto pTechno = abstract_cast<TechnoClass*>(pType->TraceTheTarget ? pBullet->Target : pBullet->Owner))
			{
				const auto rotateAngle = (pTechno->HasTurret() ? -(pTechno->TurretFacing().GetRadian<32>()) : -(pTechno->PrimaryFacing.Current().GetRadian<32>()));

				theOffset.X = static_cast<int>(theCoords.X * Math::cos(rotateAngle) + theCoords.Y * Math::sin(rotateAngle));
				theOffset.Y = static_cast<int>(theCoords.X * Math::sin(rotateAngle) - theCoords.Y * Math::cos(rotateAngle));
			}
			else
			{
				theOffset.X = 0;
				theOffset.Y = 0;
			}

			break;
		}
		case TraceTargetMode::RotateCW:
		{
			const auto distanceCoords = pBullet->Location - destination;
			const auto radius = Point2D{theCoords.X,theCoords.Y}.Magnitude();

			if ((radius * 1.2) > Point2D{distanceCoords.X,distanceCoords.Y}.Magnitude())
			{
				auto rotateAngle = Math::atan2(distanceCoords.Y, distanceCoords.X);

				if (std::abs(radius) > 1e-10)
					rotateAngle += (pType->Trajectory_Speed / radius);

				theOffset.X = static_cast<int>(radius * Math::cos(rotateAngle));
				theOffset.Y = static_cast<int>(radius * Math::sin(rotateAngle));
			}
			else
			{
				theOffset.X = 0;
				theOffset.Y = 0;
			}

			break;
		}
		case TraceTargetMode::RotateCCW:
		{
			const auto distanceCoords = pBullet->Location - destination;
			const auto radius = Point2D{theCoords.X,theCoords.Y}.Magnitude();

			if ((radius * 1.2) > Point2D{distanceCoords.X,distanceCoords.Y}.Magnitude())
			{
				auto rotateAngle = Math::atan2(distanceCoords.Y, distanceCoords.X);

				if (std::abs(radius) > 1e-10)
					rotateAngle -= (pType->Trajectory_Speed / radius);

				theOffset.X = static_cast<int>(radius * Math::cos(rotateAngle));
				theOffset.Y = static_cast<int>(radius * Math::sin(rotateAngle));
			}
			else
			{
				theOffset.X = 0;
				theOffset.Y = 0;
			}

			break;
		}
		default:
		{
			const auto& theSource = pBullet->SourceCoords;
			const auto& theTarget = pBullet->TargetCoords;

			const auto rotateAngle = Math::atan2(theTarget.Y - theSource.Y , theTarget.X - theSource.X);

			theOffset.X = static_cast<int>(theCoords.X * Math::cos(rotateAngle) + theCoords.Y * Math::sin(rotateAngle));
			theOffset.Y = static_cast<int>(theCoords.X * Math::sin(rotateAngle) - theCoords.Y * Math::cos(rotateAngle));
			break;
		}
		}
	}

	const auto distanceCoords = ((destination + theOffset) - pBullet->Location);
	const auto distance = distanceCoords.Magnitude();

	BulletVelocity velocity
	{
		static_cast<double>(distanceCoords.X),
		static_cast<double>(distanceCoords.Y),
		static_cast<double>(distanceCoords.Z)
	};

	if (distance > pType->Trajectory_Speed)
		velocity *= (pType->Trajectory_Speed / distance);

	return velocity;
}

AbstractClass* TracingTrajectory::GetBulletTarget(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner, WeaponTypeClass* pWeapon)
{
	const auto pType = this->Type;

	if (pType->TraceTheTarget)
		return pBullet;

	if (pType->Synchronize)
		return pBullet->Target;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	const auto vec = Helpers::Alex::getCellSpreadItems(pBullet->Location, (pWeapon->Range / 256.0), pWeapon->Projectile->AA);

	for (const auto& pOpt : vec)
	{
		if (!pOpt->IsAlive || !pOpt->IsOnMap || pOpt->InLimbo || pOpt->IsSinking || pOpt->Health <= 0)
			continue;

		const auto pOptType = pOpt->GetTechnoType();

		if (!pOptType->LegalTarget || pOpt == pTechno)
			continue;

		if (pOwner->IsAlliedWith(pOpt->Owner))
			continue;

		if (pOpt->IsDisguisedAs(pOwner))
			continue;

		if (pOpt->CloakState == CloakState::Cloaked && !pOpt->GetCell()->Sensors_InclHouse(pOwner->ArrayIndex))
			continue;

		if (MapClass::GetTotalDamage(100, pWeapon->Warhead, pOptType->Armor, 0) == 0)
			continue;

		if (pWeaponExt && (!EnumFunctions::IsTechnoEligible(pOpt, pWeaponExt->CanTarget) || !pWeaponExt->HasRequiredAttachedEffects(pOpt, pTechno)))
			continue;

		return pOpt;
	}

	return pBullet->Target;
}

CoordStruct TracingTrajectory::GetWeaponFireCoord(BulletClass* pBullet, TechnoClass* pTechno)
{
	const auto pType = this->Type;

	if (pType->TraceTheTarget)
	{
		const auto pTransporter = pTechno->Transporter;

		if (!this->NotMainWeapon && pTechno && (pTransporter || !pTechno->InLimbo))
		{
			if (pTechno->WhatAmI() != AbstractType::Building)
			{
				if (!pTransporter)
					return TechnoExt::GetFLHAbsoluteCoords(pTechno, this->FLHCoord, pTechno->HasTurret());

				return TechnoExt::GetFLHAbsoluteCoords(pTransporter, this->FLHCoord, pTransporter->HasTurret());
			}

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

			return (pBuilding->GetCoords() + this->BuildingCoord + CoordStruct { static_cast<int>(result.X), -static_cast<int>(result.Y), static_cast<int>(result.Z) });
		}

		return pBullet->SourceCoords;
	}

	const auto& weaponCoord = pType->WeaponCoord.Get();

	if (weaponCoord == CoordStruct::Empty)
		return pBullet->Location;

	const auto rotateRadian = Math::atan2(pBullet->TargetCoords.Y - pBullet->Location.Y , pBullet->TargetCoords.X - pBullet->Location.X);
	CoordStruct fireOffsetCoord
	{
		static_cast<int>(weaponCoord.X * Math::cos(rotateRadian) + weaponCoord.Y * Math::sin(rotateRadian)),
		static_cast<int>(weaponCoord.X * Math::sin(rotateRadian) - weaponCoord.Y * Math::cos(rotateRadian)),
		weaponCoord.Z
	};

	return pBullet->Location + fireOffsetCoord;
}

bool TracingTrajectory::PrepareTracingWeapon(BulletClass* pBullet)
{
	const auto pType = this->Type;

	if (const auto pWeapon = pType->Weapons[this->WeaponIndex])
		this->CreateTracingBullets(pBullet, pWeapon);

	if (this->WeaponCount < 0 || --this->WeaponCount > 0)
		return false;

	if (++this->WeaponIndex < static_cast<int>(pType->Weapons.size()))
	{
		const int validCounts = pType->WeaponCount.size();
		this->WeaponCount = pType->WeaponCount[(this->WeaponIndex < validCounts) ? this->WeaponIndex : (validCounts - 1)];

		return false;
	}

	this->WeaponIndex = 0;
	this->WeaponCount = pType->WeaponCount[0];

	if (this->WeaponCycle < 0 || --this->WeaponCycle > 0)
		return false;

	this->WeaponTimer.Stop();

	return pType->SuicideIfNoWeapon;
}

void TracingTrajectory::CreateTracingBullets(BulletClass* pBullet, WeaponTypeClass* pWeapon)
{
	const auto pType = this->Type;
	const auto pTechno = pBullet->Owner;
	auto pOwner = pTechno ? pTechno->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

	if (!pOwner || pOwner->Defeated)
	{
		if (const auto pNeutral = HouseClass::FindNeutral())
			pOwner = pNeutral;
		else
			return;
	}

	const auto pTarget = this->GetBulletTarget(pBullet, pTechno, pOwner, pWeapon);

	if (!pTarget)
		return;

	if (const int validDelays = pType->WeaponDelay.size())
	{
		const auto delay = pType->WeaponDelay[(this->WeaponIndex < validDelays) ? this->WeaponIndex : (validDelays - 1)];
		this->WeaponTimer.Start((delay > 0) ? delay : 1);
	}

	if (!this->WeaponCount)
		return;

	const auto fireCoord = this->GetWeaponFireCoord(pBullet, pTechno);
	const auto targetCoords = pTarget->GetCoords();
	const auto finalDamage = static_cast<int>(pWeapon->Damage * this->FirepowerMult);

	if (const auto pCreateBullet = pWeapon->Projectile->CreateBullet(pTarget, pTechno, finalDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
	{
		BulletExt::SimulatedFiringUnlimbo(pCreateBullet, pOwner, pWeapon, fireCoord, false);
		const auto pBulletExt = BulletExt::ExtMap.Find(pCreateBullet);

		if (const auto pTraj = pBulletExt->Trajectory.get())
		{
			const auto flag = pTraj->Flag();

			if (flag == TrajectoryFlag::Tracing)
			{
				const auto pTrajectory = static_cast<TracingTrajectory*>(pTraj);
				pTrajectory->FirepowerMult = this->FirepowerMult;
				pTrajectory->NotMainWeapon = true;
			}
			else if (flag == TrajectoryFlag::Straight)
			{
				const auto pTrajectory = static_cast<StraightTrajectory*>(pTraj);
				pTrajectory->FirepowerMult = this->FirepowerMult;
			}
/*			else if (flag == TrajectoryFlag::Disperse) // TODO If merge #1295
			{
				const auto pTrajectory = static_cast<DisperseTrajectory*>(pTraj);
				pTrajectory->FirepowerMult = this->FirepowerMult;
			}*/
/*			else if (flag == TrajectoryFlag::Engrave) // TODO If merge #1293
			{
				const auto pTrajectory = static_cast<EngraveTrajectory*>(pTraj);
				pTrajectory->NotMainWeapon = true;
			}*/
		}

		const auto pAttach = pType->TraceTheTarget ? (!this->NotMainWeapon ? static_cast<ObjectClass*>(pTechno) : nullptr) : pBullet;
		BulletExt::SimulatedFiringEffects(pCreateBullet, pOwner, pAttach, true, true);
	}
}
