#include "TracingTrajectory.h"

#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>

namespace detail
{
	template <>
	inline bool read<TraceTargetMode>(TraceTargetMode& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			static std::pair<const char*, TraceTargetMode> FlagNames[] =
			{
				{"Connection", TraceTargetMode::Connection},
				{"Global", TraceTargetMode::Global},
				{"Body", TraceTargetMode::Body},
				{"Turret", TraceTargetMode::Turret},
				{"RotateCW", TraceTargetMode::RotateCW},
				{"RotateCCW", TraceTargetMode::RotateCCW},
			};
			for (auto [name, flag] : FlagNames)
			{
				if (_strcmpi(parser.value(), name) == 0)
				{
					value = flag;
					return true;
				}
			}
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a new trace target mode");
		}

		return false;
	}
}

std::unique_ptr<PhobosTrajectory> TracingTrajectoryType::CreateInstance(BulletClass* pBullet) const
{
	return std::make_unique<TracingTrajectory>(this, pBullet);
}

template<typename T>
void TracingTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->TraceMode)
		.Process(this->TraceTheTarget)
		.Process(this->CreateAtTarget)
		.Process(this->StableRotation)
		.Process(this->ChasableDistance)
		;
}

bool TracingTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->VirtualTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool TracingTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->VirtualTrajectoryType::Save(Stm);
	const_cast<TracingTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

void TracingTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->PhobosTrajectoryType::Read(pINI, pSection);
	INI_EX exINI(pINI);
	// Virtual
	this->VirtualSourceCoord.Read(exINI, pSection, "Trajectory.Tracing.CreateCoord");
	this->VirtualTargetCoord.Read(exINI, pSection, "Trajectory.Tracing.AttachCoord");
	this->AllowFirerTurning.Read(exINI, pSection, "Trajectory.AllowFirerTurning");
	// Tracing
	this->TraceMode.Read(exINI, pSection, "Trajectory.Tracing.TraceMode");
	this->TraceTheTarget.Read(exINI, pSection, "Trajectory.Tracing.TraceTheTarget");
	this->CreateAtTarget.Read(exINI, pSection, "Trajectory.Tracing.CreateAtTarget");
	this->StableRotation.Read(exINI, pSection, "Trajectory.Tracing.StableRotation");
	this->ChasableDistance.Read(exINI, pSection, "Trajectory.Tracing.ChasableDistance");
}

template<typename T>
void TracingTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->RotateRadian)
		;
}

bool TracingTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->VirtualTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool TracingTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->VirtualTrajectory::Save(Stm);
	const_cast<TracingTrajectory*>(this)->Serialize(Stm);
	return true;
}

void TracingTrajectory::OnUnlimbo()
{
	this->VirtualTrajectory::OnUnlimbo();
	// Waiting for launch trigger
	if (!BulletExt::ExtMap.Find(this->Bullet)->DispersedTrajectory)
		this->OpenFire();
}

bool TracingTrajectory::OnEarlyUpdate()
{
	if (this->VirtualTrajectory::OnEarlyUpdate())
		return true;

	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	const auto pFirer = pBullet->Owner;
	// Followed the launcher, but the launcher was destroyed
	return !pType->TraceTheTarget && !pFirer;
}

bool TracingTrajectory::OnVelocityCheck()
{
	// Calculate speed changes
	if (this->ChangeVelocity())
		return true;

	return this->PhobosTrajectory::OnVelocityCheck();
}

void TracingTrajectory::OpenFire()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	const auto& coords = pType->VirtualSourceCoord.Get();
	CoordStruct offset = coords;
	// Offset during creation
	if (coords.X != 0 || coords.Y != 0)
	{
		const auto rotateRadian = this->Get2DOpRadian(pBullet->SourceCoords, pBullet->TargetCoords);
		// Check if mirroring is required
		if (pType->MirrorCoord && this->CurrentBurst < 0)
			offset.Y = -offset.Y;
		// Rotate the angle
		offset = PhobosTrajectory::Vector2Coord(PhobosTrajectory::HorizontalRotate(offset, rotateRadian));
	}
	// Add the basic coordinate position and then set it
	if (!pType->CreateAtTarget)
		pBullet->SetLocation(pBullet->SourceCoords + offset);
	else if (const auto pTarget = pBullet->Target)
		pBullet->SetLocation(pTarget->GetCoords() + offset);
	else
		pBullet->SetLocation(pBullet->TargetCoords + offset);

	this->PhobosTrajectory::OpenFire();

	const auto duration = pType->Duration.Get();
	// Calculate survival time
	if (duration < 0)
		return;
	else if (duration > 0)
		this->DurationTimer.Start(duration);
	else if (const auto pWeapon = pBullet->WeaponType)
		this->DurationTimer.Start((pWeapon->ROF > 10) ? pWeapon->ROF - 10 : 1);
	else
		this->DurationTimer.Start(120);
}

bool TracingTrajectory::ChangeVelocity()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->Type;
	// Find the outermost transporter
	const auto pFirer = this->GetSurfaceFirer(pBullet->Owner);
	// Tracing the target
	if (const auto pTarget = pBullet->Target)
		pBullet->TargetCoords = pTarget->GetCoords();
	// Confirm the center position of the tracing target
	auto destination = (pType->TraceTheTarget || !pFirer) ? pBullet->TargetCoords : pFirer->GetCoords();
	// Calculate the maximum separation distance
	const auto pWeapon = pBullet->WeaponType;
	const auto cRange = pType->ChasableDistance.Get();
	const auto bRange = cRange ? std::abs(cRange) : (pWeapon ? pWeapon->Range : (10 * Unsorted::LeptonsPerCell));
	const auto aRange = (pType->ApplyRangeModifiers && pFirer && pWeapon ? WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer, bRange) : bRange) + 32;
	// Calculate the distance between the projectile and the firer
	const auto source = (pFirer && !this->NotMainWeapon) ? pFirer->GetCoords() : pBullet->SourceCoords;
	const auto delta = destination - source;
	const auto distance = (this->NotMainWeapon || this->TargetInTheAir || (pFirer && pFirer->IsInAir())) ? PhobosTrajectory::Get2DDistance(delta) : delta.Magnitude();
	// Check if the limit has been exceeded
	if (static_cast<int>(distance) >= aRange)
	{
		if (cRange < 0)
			return true;
		else
			destination = source + delta * (aRange / distance);
	}

	CoordStruct offset = pType->VirtualTargetCoord.Get();
	// Calculate only when there is an offset value
	if (offset.X != 0 || offset.Y != 0)
	{
		bool cw = false;

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
				const auto rotateRadian = -(pTechno->PrimaryFacing.Current().GetRadian<32>());
				// Rotate the body angle
				offset = PhobosTrajectory::Vector2Coord(PhobosTrajectory::HorizontalRotate(offset, rotateRadian));
			}
			else
			{
				offset.X = 0;
				offset.Y = 0;
			}

			break;
		}
		case TraceTargetMode::Turret:
		{
			if (const auto pTechno = abstract_cast<TechnoClass*>(pType->TraceTheTarget ? pBullet->Target : pBullet->Owner))
			{
				const auto rotateRadian = (pTechno->HasTurret() ? -(pTechno->TurretFacing().GetRadian<32>()) : -(pTechno->PrimaryFacing.Current().GetRadian<32>()));
				// Rotate the turret angle
				offset = PhobosTrajectory::Vector2Coord(PhobosTrajectory::HorizontalRotate(offset, rotateRadian));
			}
			else
			{
				offset.X = 0;
				offset.Y = 0;
			}

			break;
		}
		case TraceTargetMode::RotateCW:
		{
			cw = true;
		}
		case TraceTargetMode::RotateCCW:
		{
			const auto radius = PhobosTrajectory::Get2DDistance(offset);
			// Individual or entirety
			if (!pType->StableRotation || !this->TrajectoryGroup)
			{
				const auto distanceCoords = pBullet->Location - destination;
				// Rotate around the center only when the distance is less than 1.2 times the radius
				if ((radius * 1.2) > PhobosTrajectory::Get2DDistance(distanceCoords))
				{
					// Recalculate
					this->RotateRadian = Math::atan2(distanceCoords.Y, distanceCoords.X);
					// The arc of rotation per frame can be determined by the radius and speed
					if (std::abs(radius) > 1e-10)
						this->RotateRadian = cw ? (this->RotateRadian + pType->Speed / radius) : (this->RotateRadian - pType->Speed / radius);
				}
			}
			else
			{
				auto& groupData = (*this->TrajectoryGroup)[pBullet->Type->UniqueID];
				// Valid group
				if (const auto size = static_cast<int>(groupData.Bullets.size()))
				{
					// Record radian by main bullet and add stable interval to others
					if (!this->GroupIndex)
						this->RotateRadian = groupData.Angle = cw ? (this->RotateRadian + pType->Speed / 2 / radius) : (this->RotateRadian - pType->Speed / 2 / radius);
					else
						this->RotateRadian = groupData.Angle + (Math::TwoPi * this->GroupIndex / size);
				}
			}
			// Calculate the actual offset value
			offset.X = static_cast<int>(radius * Math::cos(this->RotateRadian));
			offset.Y = static_cast<int>(radius * Math::sin(this->RotateRadian));

			break;
		}
		default:
		{
			const auto rotateRadian = this->Get2DOpRadian(pBullet->SourceCoords, pBullet->TargetCoords);
			// Check if mirroring is required
			if (pType->MirrorCoord && this->CurrentBurst < 0)
				offset.Y = -offset.Y;
			// Rotate the angle
			offset = PhobosTrajectory::Vector2Coord(PhobosTrajectory::HorizontalRotate(offset, rotateRadian));
			break;
		}
		}
	}
	// Calculate distance
	const auto difference = ((destination + offset) - pBullet->Location);
	const auto differenceDistance = difference.Magnitude();
	// Set as speed
	this->MovingVelocity = BulletVelocity { static_cast<double>(difference.X), static_cast<double>(difference.Y), static_cast<double>(difference.Z) };
	this->MovingSpeed = differenceDistance;
	// Prevent exceeding the actual speed
	if (pType->Speed <= differenceDistance)
		this->MultiplyBulletVelocity(pType->Speed / differenceDistance, false);

	return false;
}
