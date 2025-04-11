#include "PhobosTrajectory.h"
#include "StraightTrajectory.h"
#include "BombardTrajectory.h"
#include "MissileTrajectory.h"
#include "EngraveTrajectory.h"
#include "ParabolaTrajectory.h"
#include "TracingTrajectory.h"

#include <OverlayTypeClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>

TrajectoryTypePointer::TrajectoryTypePointer(TrajectoryFlag flag)
{
	switch (flag)
	{
	case TrajectoryFlag::Straight:
		_ptr = std::make_unique<StraightTrajectoryType>();
		return;
	case TrajectoryFlag::Bombard:
		_ptr = std::make_unique<BombardTrajectoryType>();
		return;
	case TrajectoryFlag::Missile:
		_ptr = std::make_unique<MissileTrajectoryType>();
		return;
	case TrajectoryFlag::Engrave:
		_ptr = std::make_unique<EngraveTrajectoryType>();
		return;
	case TrajectoryFlag::Parabola:
		_ptr = std::make_unique<ParabolaTrajectoryType>();
		return;
	case TrajectoryFlag::Tracing:
		_ptr = std::make_unique<TracingTrajectoryType>();
		return;
	}
	_ptr.reset();
}

namespace detail
{
	template <>
	inline bool read<TrajectoryFlag>(TrajectoryFlag& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			static std::pair<const char*, TrajectoryFlag> FlagNames[] =
			{
				{"Straight", TrajectoryFlag::Straight},
				{"Bombard" ,TrajectoryFlag::Bombard},
				{"Missile", TrajectoryFlag::Missile},
				{"Engrave" ,TrajectoryFlag::Engrave},
				{"Parabola", TrajectoryFlag::Parabola},
				{"Tracing" ,TrajectoryFlag::Tracing},
			};
			for (auto [name, flag] : FlagNames)
			{
				if (_strcmpi(parser.value(), name) == 0)
				{
					value = flag;
					return true;
				}
			}
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a new trajectory type");
		}

		return false;
	}

	template <>
	inline bool read<TrajectoryFacing>(TrajectoryFacing& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			static std::pair<const char*, TrajectoryFacing> FlagNames[] =
			{
				{"Velocity", TrajectoryFacing::Velocity},
				{"Spin" ,TrajectoryFacing::Spin},
				{"Stable", TrajectoryFacing::Stable},
				{"Target" ,TrajectoryFacing::Target},
				{"Destination", TrajectoryFacing::Destination},
				{"FirerBody" ,TrajectoryFacing::FirerBody},
				{"FirerTurret", TrajectoryFacing::FirerTurret},
			};
			for (auto [name, flag] : FlagNames)
			{
				if (_strcmpi(parser.value(), name) == 0)
				{
					value = flag;
					return true;
				}
			}
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a new trajectory facing type");
		}

		return false;
	}
}

void TrajectoryTypePointer::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	Nullable<TrajectoryFlag> flag;
	flag.Read(exINI, pSection, "Trajectory");// I assume this shit is parsed once and only once, so I keep the impl here
	if (flag.isset())
	{
		if (!_ptr || _ptr->Flag() != flag.Get())
			std::construct_at(this, flag.Get());
	}
	if (_ptr)
		_ptr->Read(pINI, pSection);
}

bool TrajectoryTypePointer::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	PhobosTrajectoryType* PTR = nullptr;
	if (!Stm.Load(PTR))
		return false;
	TrajectoryFlag flag;
	if (PTR && Stm.Load(flag))
	{
		std::construct_at(this, flag);
		PhobosSwizzle::RegisterChange(PTR, _ptr.get());
		return _ptr->Load(Stm, RegisterForChange);
	}
	return true;
}

bool TrajectoryTypePointer::Save(PhobosStreamWriter& Stm) const
{
	auto* raw = get();
	Stm.Save(raw);
	if (raw)
	{
		auto rtti = raw->Flag();
		Stm.Save(rtti);
		return raw->Save(Stm);
	}
	return true;
}

bool TrajectoryPointer::Load(PhobosStreamReader& Stm, bool registerForChange)
{
	PhobosTrajectory* PTR = nullptr;
	if (!Stm.Load(PTR))
		return false;
	TrajectoryFlag flag = TrajectoryFlag::Invalid;
	if (PTR && Stm.Load(flag))
	{
		switch (flag)
		{
		case TrajectoryFlag::Straight:
			_ptr = std::make_unique<StraightTrajectory>(noinit_t {});
			break;
		case TrajectoryFlag::Bombard:
			_ptr = std::make_unique<BombardTrajectory>(noinit_t {});
			break;
		case TrajectoryFlag::Missile:
			_ptr = std::make_unique<MissileTrajectory>(noinit_t {});
			break;
		case TrajectoryFlag::Engrave:
			_ptr = std::make_unique<EngraveTrajectory>(noinit_t {});
			break;
		case TrajectoryFlag::Parabola:
			_ptr = std::make_unique<ParabolaTrajectory>(noinit_t {});
			break;
		case TrajectoryFlag::Tracing:
			_ptr = std::make_unique<TracingTrajectory>(noinit_t {});
			break;
		default:
			_ptr.reset();
			break;
		}
		if (_ptr.get())
		{
			// PhobosSwizzle::RegisterChange(PTR, _ptr.get()); // not used elsewhere yet, if anyone does then reenable this shit
			return _ptr->Load(Stm, registerForChange);
		}
	}
	return true;
}

bool TrajectoryPointer::Save(PhobosStreamWriter& Stm) const
{
	auto* raw = get();
	Stm.Save(raw);
	if (raw)
	{
		auto rtti = raw->Flag();
		Stm.Save(rtti);
		return raw->Save(Stm);
	}
	return true;
}

// ------------------------------------------------------------------------------ //

// Have generated projectile on the map and prepare for launch
void PhobosTrajectory::OnUnlimbo()
{
	const auto pBullet = this->Bullet;
	// Without a target, the game will inevitably crash before, so no need to check here
	const auto pTarget = pBullet->Target;
	// Due to various ways of firing weapons, the true firer may have already died
	const auto pFirer = pBullet->Owner;
	// Just like GetTechnoType()
	const auto pType = this->GetType();
	// Record the status of the target
	this->TargetIsTechno = (pTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None;
	this->TargetInTheAir = (pTarget->AbstractFlags & AbstractFlags::Object) ? (static_cast<ObjectClass*>(pTarget)->GetHeight() > Unsorted::CellHeight) : false;
	int damage = pBullet->Health;
	// Record some information of weapon
	if (const auto pWeapon = pBullet->WeaponType)
	{
		this->AttenuationRange = pWeapon->Range;
		this->CountOfBurst = pWeapon->Burst;

		if (pType->ApplyRangeModifiers && pFirer)
			this->AttenuationRange = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pFirer);

		damage = pWeapon->Damage;
	}
	// Set basic damage
	this->ProximityDamage = pType->ProximityDamage.Get(damage);
	this->PassDetonateDamage = pType->PassDetonateDamage.Get(damage);
	// Record some information of firer
	if (pFirer)
	{
		const auto burst = pFirer->CurrentBurstIndex;
		this->CurrentBurst = (burst & 1) ? (-burst - 1) : burst;
		this->FirepowerMult = pFirer->FirepowerMultiplier * TechnoExt::ExtMap.Find(pFirer)->AE.FirepowerMultiplier;
		const auto flag = this->Flag();
		// Obtain the launch location
		if (pType->DisperseWeapons.size() && pType->RecordSourceCoord || flag == TrajectoryFlag::Engrave || flag == TrajectoryFlag::Tracing)
			this->GetTechnoFLHCoord();
		else
			this->NotMainWeapon = true;

		const auto pFirerExt = TechnoExt::ExtMap.Find(pFirer);
		// Check trajectory capacity
		if (pType->CreateCapacity >= 0 && pFirerExt->CurrentTracingCount >= pType->CreateCapacity)
		{
			pBullet->Health = 0;
			this->ShouldDetonate = true;
		}
		// Increase trajectory count
		++pFirerExt->CurrentTracingCount;
	}
	else
	{
		this->NotMainWeapon = true;
	}
	// Initialize additional warheads
	if (pType->PassDetonate)
		this->PassDetonateTimer.Start(pType->PassDetonateInitialDelay);
	// Initialize additional weapons
	if (!pType->DisperseWeapons.empty() && !pType->DisperseCounts.empty() && this->DisperseCycle)
	{
		this->DisperseCount = pType->DisperseCounts[0];
		this->DisperseTimer.Start(pType->DisperseInitialDelay);
	}
}

// Something that needs to be done before updating the velocity of the projectile
bool PhobosTrajectory::OnEarlyUpdate()
{
	// The previous check requires detonation at this time
	if (this->ShouldDetonate)
		return true;
	// Check the remaining existence time
	if (this->DurationTimer.Completed())
		return true;
	// Check if the firer's target can be synchronized
	if (this->CheckSynchronize())
		return true;
	// Check if the target needs to be changed
	if (std::abs(this->GetType()->RetargetRadius) > 1e-10 && this->BulletRetargetTechno())
		return true;
	// Check if the tolerance time has ended
	if (this->CheckTolerantTime())
		return true;
	// Check whether need to slow down then detonate
	if (this->OnVelocityCheck())
		return true;
	// Rotate bullet
	this->OnFacingUpdate();
	// Fire weapons or warheads
	if (this->FireAdditionals())
		return true;
	// Detonate extra warhead on the obstacle
	this->DetonateOnObstacle();
	return false;
}

// It is possible to detonate here in advance or decide how to change the speed
bool PhobosTrajectory::OnVelocityCheck()
{
	const auto pBullet = this->Bullet;
	double ratio = 1.0;
	// If there is an obstacle on the route, the bullet should need to reduce its speed so it will not penetrate the obstacle.
	const auto pType = this->GetType();
	const bool checkThrough = (!pType->ThroughBuilding || !pType->ThroughVehicles);
	const auto velocity = PhobosTrajectory::Get2DVelocity(this->MovingVelocity);
	// Low speed with checkSubject was already done well
	if (velocity < Unsorted::LeptonsPerCell)
	{
		// Blocked by obstacles?
		if (checkThrough)
		{
			const auto pFirer = pBullet->Owner;
			const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
			// Check for additional obstacles on the ground
			if (this->CheckThroughAndSubjectInCell(MapClass::Instance.GetCellAt(pBullet->Location), pOwner))
			{
				if (32.0 < velocity)
					ratio = (32.0 / velocity);
			}
		}
		// Check whether about to fall into the ground
		if (std::abs(this->MovingVelocity.Z) > Unsorted::CellHeight && this->GetCanHitGround())
		{
			const auto theTargetCoords = pBullet->Location + PhobosTrajectory::Vector2Coord(this->MovingVelocity);
			const auto cellHeight = MapClass::Instance.GetCellFloorHeight(theTargetCoords);
			// Check whether the height of the ground is about to exceed the height of the projectile
			if (cellHeight >= theTargetCoords.Z)
			{
				// How much reduction is needed to calculate the velocity vector
				const auto newRatio = std::abs((pBullet->Location.Z - cellHeight) / this->MovingVelocity.Z);
				// Only when the proportion is smaller, it needs to be recorded
				if (ratio > newRatio)
					ratio = newRatio;
			}
		}
	}
	else
	{
		// When in high speed, it's necessary to check each cell on the path that the next frame will pass through
		const bool subjectToGround = this->GetCanHitGround();
		const auto pBulletType = pBullet->Type;
		const auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBulletType);
		const bool subjectToWCS = pBulletType->SubjectToWalls || pBulletType->SubjectToCliffs || pBulletTypeExt->SubjectToSolid;
		const bool subjectToFirestorm = !pBulletType->IgnoresFirestorm;
		const bool checkCoords = subjectToGround || checkThrough || subjectToWCS;
		// If no inspection is needed, just skip it
		if (checkCoords || subjectToFirestorm)
		{
			double locationDistance = 0.0;
			bool velocityCheck = false;
			const auto& theSourceCoords = pBullet->Location;
			const auto theTargetCoords = theSourceCoords + PhobosTrajectory::Vector2Coord(this->MovingVelocity);
			const auto pFirer = pBullet->Owner;
			const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
			// Skip when no inspection is needed
			if (checkCoords)
			{
				const auto pSourceCell = MapClass::Instance.GetCellAt(theSourceCoords);
				const auto sourceCell = pSourceCell->MapCoords;
				const auto pTargetCell = MapClass::Instance.GetCellAt(theTargetCoords);
				const auto targetCell = pTargetCell->MapCoords;
				auto pLastCell = MapClass::Instance.GetCellAt(pBullet->LastMapCoords);
				const bool checkLevel = !pBulletTypeExt->SubjectToLand.isset() && !pBulletTypeExt->SubjectToWater.isset();
				const auto cellDist = sourceCell - targetCell;
				const auto cellPace = CellStruct { static_cast<short>(std::abs(cellDist.X)), static_cast<short>(std::abs(cellDist.Y)) };
				// Take big steps as much as possible to reduce check times, just ensure that each cell is inspected
				const auto largePace = static_cast<size_t>(std::max(cellPace.X, cellPace.Y));
				const auto stepCoord = !largePace ? CoordStruct::Empty : (theTargetCoords - theSourceCoords) * (1.0 / largePace);
				auto curCoord = theSourceCoords;
				auto pCurCell = MapClass::Instance.GetCellAt(sourceCell);
				// Check one by one towards the direction of the next frame's position
				for (size_t i = 0; i < largePace; ++i)
				{
					if ((subjectToGround && (curCoord.Z + 16) < MapClass::Instance.GetCellFloorHeight(curCoord)) // Below ground level? (16 ->error range)
						|| (checkThrough && this->CheckThroughAndSubjectInCell(pCurCell, pOwner)) // Blocked by obstacles?
						|| (subjectToWCS && TrajectoryHelper::GetObstacle(pSourceCell, pTargetCell, pLastCell, curCoord, pBulletType, pOwner)) // Impact on the wall/cliff/solid?
						|| (checkLevel ? (pBulletType->Level && pCurCell->IsOnFloor()) // Level or above land/water?
							: ((pCurCell->LandType == LandType::Water || pCurCell->LandType == LandType::Beach)
								? (pBulletTypeExt->SubjectToWater.Get(false) && pBulletTypeExt->SubjectToWater_Detonate)
								: (pBulletTypeExt->SubjectToLand.Get(false) && pBulletTypeExt->SubjectToLand_Detonate))))
					{
						locationDistance = PhobosTrajectory::Get2DDistance(curCoord, theSourceCoords);
						velocityCheck = true;
						break;
					}
					// There are no obstacles, continue to check the next cell
					curCoord += stepCoord;
					pLastCell = pCurCell;
					pCurCell = MapClass::Instance.GetCellAt(curCoord);
				}
			}
			// Check whether ignore firestorm wall before searching
			if (subjectToFirestorm)
			{
				const auto fireStormCoords = MapClass::Instance.FindFirstFirestorm(theSourceCoords, theTargetCoords, pOwner);
				// Not empty when firestorm wall exists
				if (fireStormCoords != CoordStruct::Empty)
				{
					const auto distance = PhobosTrajectory::Get2DDistance(fireStormCoords, theSourceCoords);
					// Only record when the ratio is smaller
					if (!velocityCheck || distance < locationDistance)
						locationDistance = distance;

					velocityCheck = true;
				}
			}
			// Check if the bullet needs to slow down the speed
			if (velocityCheck)
			{
				// Let the distance slightly exceed
				locationDistance += 32.0;
				// It may not be necessary to compare them again, but still do so
				if (locationDistance < velocity)
					ratio = (locationDistance / velocity);
			}
		}
	}
	// Check if the distance to the destination exceeds the speed limit
	if (this->RemainingDistance < this->MovingSpeed)
	{
		const auto newRatio = this->RemainingDistance / this->MovingSpeed;
		// Only record when the ratio is smaller
		if (ratio > newRatio)
			ratio = newRatio;
	}
	// Only when the speed is very low will there be situations where the conditions are not met
	if (ratio < 1.0)
		this->MultiplyBulletVelocity(ratio, true);
	// Anyway, wait until later before detonating
	return false;
}

// How does the velocity of the projectile change
void PhobosTrajectory::OnVelocityUpdate(BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	// Set true moving velocity
	*pSpeed = this->MovingVelocity;
}

// Something that needs to be done after updating the velocity of the projectile
TrajectoryCheckReturnType PhobosTrajectory::OnDetonateUpdate(const CoordStruct& position)
{
	// Need to detonate at the next location
	if (this->ShouldDetonate)
		return TrajectoryCheckReturnType::Detonate;
	// Below ground level? (16 ->error range)
	if (this->GetCanHitGround() && MapClass::Instance.GetCellFloorHeight(position) >= (position.Z + 16))
		return TrajectoryCheckReturnType::Detonate;
	// Skip all vanilla checks
	return TrajectoryCheckReturnType::SkipGameCheck;
}

// Something that needs to be done before detonation
void PhobosTrajectory::OnPreDetonate()
{
	const auto pType = this->GetType();
	// Special circumstances, similar to airburst behavior
	if (pType->DisperseEffectiveRange.Get() < 0)
		this->PrepareDisperseWeapon();

	const auto pBullet = this->Bullet;
	// Decrease trajectory count, considering that there is no function to transfer the firer, only simple processing will be carried out
	if (const auto pFirerExt = TechnoExt::ExtMap.Find(pBullet->Owner))
		--pFirerExt->CurrentTracingCount;
	// No damage, no anims...
	if (pType->PeacefulVanish.Get(this->Flag() == TrajectoryFlag::Engrave || pType->ProximityImpact || pType->DisperseCycle))
	{
		pBullet->Health = 0;
		pBullet->Limbo();
		pBullet->UnInit();
	}
	else
	{
		// Calculate the current damage
		pBullet->Health = this->GetTheTrueDamage(pBullet->Health, true);
	}
}

// Something that needs to be done when the projectile is actually launched
void PhobosTrajectory::OpenFire()
{
	const auto pBullet = this->Bullet;
	const auto& source = pBullet->SourceCoords;
	const auto& target = pBullet->TargetCoords;
	// There may be a frame that hasn't started updating yet but will be drawn on the screen
	if (this->MovingVelocity != BulletVelocity::Empty)
		pBullet->Velocity = this->MovingVelocity;
	else // Lead time bug
		pBullet->Velocity = BulletVelocity { static_cast<double>(target.X - source.X), static_cast<double>(target.Y - source.Y), 0 };

	const auto pType = this->GetType();
	// Restricted to rotation only on a horizontal plane
	if (pType->BulletFacing == TrajectoryFacing::Spin || pType->BulletROT < 0)
		pBullet->Velocity.Z = 0;
}

// Something that needs to be done when changing the target of the projectile
void PhobosTrajectory::SetBulletNewTarget(AbstractClass* const pTarget)
{
	const auto pBullet = this->Bullet;
	pBullet->SetTarget(pTarget);
	pBullet->TargetCoords = pTarget->GetCoords();
}

// Something that needs to be done when setting the new speed of the projectile
bool PhobosTrajectory::CalculateBulletVelocity(const double speed)
{
	const auto velocityLength = this->MovingVelocity.Magnitude();
	// Check if it is a zero vector
	if (velocityLength < 1e-10)
		return true;
	// Reset speed vector
	this->MovingVelocity *= speed / velocityLength;
	this->MovingSpeed = speed;
	return false;
}

// Something that needs to be done when Multipling the speed of the projectile
void PhobosTrajectory::MultiplyBulletVelocity(const double ratio, const bool shouldDetonate)
{
	// Reset speed vector
	this->MovingVelocity *= ratio;
	this->MovingSpeed = this->MovingSpeed * ratio;
	// The next frame needs to detonate itself
	if (shouldDetonate)
		this->ShouldDetonate = true;
}

/*!
	Rotate one vector by a certain angle towards the direction of another vector.

	\param vector Vector that needs to be rotated. This function directly modifies this value.
	\param aim The final direction vector that needs to be oriented. It doesn't need to be a standardized vector.
	\param turningRadian The maximum radius that can rotate. Note that it must be a positive number.

	\returns No return value, result is vector.

	\author CrimRecya
*/
void PhobosTrajectory::RotateVector(BulletVelocity& vector, const BulletVelocity& aim, double turningRadian)
{
	const auto baseFactor = sqrt(aim.MagnitudeSquared() * vector.MagnitudeSquared());
	// Not valid vector
	if (baseFactor <= 1e-10)
	{
		vector = aim;
		return;
	}
	// Try using the vector to calculate the included angle
	const auto dotProduct = (aim * vector);
	// Calculate the cosine of the angle when the conditions are suitable
	const auto cosTheta = dotProduct / baseFactor;
	// Ensure that the result range of cos is correct
	const auto radian = Math::acos(Math::clamp(cosTheta, -1.0, 1.0));
	// When the angle is small, aim directly at the target
	if (std::abs(radian) <= turningRadian)
	{
		vector = aim;
		return;
	}
	// Calculate the rotation axis
	auto rotationAxis = aim.CrossProduct(vector);
	// The radian can rotate, input the correct direction
	const auto rotateRadian = (radian < 0 ? turningRadian : -turningRadian);
	// Substitute to calculate new velocity
	PhobosTrajectory::RotateAboutTheAxis(vector, rotationAxis, rotateRadian);
}

/*!
	Rotate the vector around the axis of rotation by a fixed angle.

	\param vector Vector that needs to be rotated. This function directly modifies this value.
	\param axis The vector of rotation axis. This operation will standardize it.
	\param radian The angle of rotation, positive or negative determines its direction of rotation.

	\returns No return value, result is vector.

	\author CrimRecya
*/
void PhobosTrajectory::RotateAboutTheAxis(BulletVelocity& vector, BulletVelocity& axis, double radian)
{
	const auto axisLengthSquared = axis.MagnitudeSquared();
	// Zero axis vector is not acceptable
	if (axisLengthSquared < 1e-10)
		return;
	// Standardize rotation axis
	axis *= 1 / sqrt(axisLengthSquared);
	// Rotate around the axis of rotation
	const auto cosRotate = Math::cos(radian);
	// Substitute the formula to calculate the new vector
	vector = (vector * cosRotate) + (axis * ((1 - cosRotate) * (vector * axis))) + (axis.CrossProduct(vector) * Math::sin(radian));
}

// Inspection of projectile orientation
bool PhobosTrajectory::OnFacingCheck()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->GetType();

	if (!pType->DisperseFaceCheck)
		return true;

	const auto facing = pType->BulletFacing;

	if (facing == TrajectoryFacing::Velocity || facing == TrajectoryFacing::Spin)
		return true;

	const auto flag = this->Flag();

	if (pType->DisperseFromFirer.Get(flag == TrajectoryFlag::Engrave || flag == TrajectoryFlag::Tracing))
		return true;

	const auto targetDir = DirStruct { PhobosTrajectory::Get2DOpRadian(pBullet->Location, pBullet->TargetCoords) };
	const auto bulletDir = DirStruct { Math::atan2(pBullet->Velocity.Y, pBullet->Velocity.X) };
	// Their directions Y are all opposite, so they can still be used
	return std::abs(static_cast<short>(static_cast<short>(targetDir.Raw) - static_cast<short>(bulletDir.Raw))) <= (2048 + (pType->BulletROT << 8));
}

// Update of projectile facing direction
void PhobosTrajectory::OnFacingUpdate()
{
	const auto pType = this->GetType();
	const auto facing = pType->BulletFacing;
	// Cannot rotate
	if (facing == TrajectoryFacing::Stable)
		return;

	const auto pBullet = this->Bullet;
	constexpr double ratio = Math::TwoPi / 256;

	if (facing == TrajectoryFacing::Spin)
	{
		const auto radian = Math::atan2(pBullet->Velocity.Y, pBullet->Velocity.X) + (pType->BulletROT * ratio);
		pBullet->Velocity.X = Math::cos(radian);
		pBullet->Velocity.Y = Math::sin(radian);
		pBullet->Velocity.Z = 0;
		return;
	}

	auto desiredFacing = BulletVelocity::Empty;

	if (facing == TrajectoryFacing::Velocity)
	{
		desiredFacing = this->MovingVelocity;
	}
	else if (facing == TrajectoryFacing::Target)
	{
		if (const auto pTarget = pBullet->Target)
			desiredFacing = PhobosTrajectory::Coord2Vector(pTarget->GetCoords() - pBullet->Location);
		else
			desiredFacing = PhobosTrajectory::Coord2Vector(pBullet->TargetCoords - pBullet->Location);
	}
	else if (facing == TrajectoryFacing::Destination)
	{
		desiredFacing = PhobosTrajectory::Coord2Vector(pBullet->TargetCoords - pBullet->Location);
	}
	else if (const auto pFirer = pBullet->Owner)
	{
		const auto radian = -(facing == TrajectoryFacing::FirerTurret ? pFirer->TurretFacing() : pFirer->PrimaryFacing.Current()).GetRadian<65536>();
		desiredFacing.X = Math::cos(radian);
		desiredFacing.Y = Math::sin(radian);
		desiredFacing.Z = 0;
	}
	else
	{
		return;
	}

	if (!pType->BulletROT)
	{
		pBullet->Velocity = desiredFacing;
		pBullet->Velocity *= (1 / pBullet->Velocity.Magnitude());
	}
	else
	{
		// Restricted to rotation only on a horizontal plane
		if (pType->BulletROT < 0)
		{
			pBullet->Velocity.Z = 0;
			desiredFacing.Z = 0;
		}
		// Calculate specifically only when the ROT is reasonable
		PhobosTrajectory::RotateVector(pBullet->Velocity, desiredFacing, (pType->BulletROT * ratio));
		// Standardizing
		pBullet->Velocity *= (1 / pBullet->Velocity.Magnitude());
	}
}

// Launch additional weapons and warheads
bool PhobosTrajectory::FireAdditionals()
{
	const auto pType = this->GetType();
	// Detonate the warhead at the current location
	if (pType->PassDetonate)
		this->PassWithDetonateAt();
	// Detonate the warhead on the technos passing through
	if (this->ProximityImpact != 0 && pType->ProximityRadius.Get() > 0)
		this->PrepareForDetonateAt();
	// Launch additional weapons towards the target
	if (!this->DisperseTimer.Completed())
		return false;

	const auto pBullet = this->Bullet;
	const auto range = pType->DisperseEffectiveRange.Get();
	// Weapons can only be fired when the distance is close enough
	if (range && pBullet->TargetCoords.DistanceFrom(pBullet->Location) > range)
		return false;
	// Fire after checking the orientation
	return this->OnFacingCheck() && this->PrepareDisperseWeapon();
}

// Detonate a extra warhead on the obstacle then detonate bullet itself
void PhobosTrajectory::DetonateOnObstacle()
{
	const auto pDetonateAt = this->ExtraCheck;
	// Obstacles were detected in the current frame here
	if (!pDetonateAt)
		return;
	// Slow down and reset the target
	this->ExtraCheck = nullptr;
	const auto pBullet = this->Bullet;
	const auto distance = pDetonateAt->GetCoords().DistanceFrom(pBullet->Location);
	// Set the new target so that the snap function can take effect
	this->SetBulletNewTarget(pDetonateAt);
	const auto speed = this->MovingSpeed;
	// Check whether need to slow down
	if (speed && distance < speed)
		this->MultiplyBulletVelocity(distance / speed, true);
	else
		this->ShouldDetonate = true;
	// Need to cause additional damage?
	if (!this->ProximityImpact)
		return;
	// Detonate extra warhead
	const auto pFirer = pBullet->Owner;
	const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;
	this->ProximityDetonateAt(pOwner, pDetonateAt);
}

// Synchronization target inspection
bool PhobosTrajectory::CheckSynchronize()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->GetType();
	// Find the outermost transporter
	const auto pFirer = GetSurfaceFirer(pBullet->Owner);
	// Synchronize to the target of the firer
	if (pType->Synchronize && pFirer)
	{
		auto pTarget = pFirer->Target;
		// Check should detonate when changing target
		if (pBullet->Target != pTarget && !this->GetType()->TolerantTime)
			return true;
		// Check if the target can be synchronized
		if (pTarget && (pTarget->IsInAir() != this->TargetInTheAir))
			pTarget = nullptr;
		// Replace with a new target
		pBullet->SetTarget(pTarget);
	}

	return false;
}

// Tolerance timer inspection
bool PhobosTrajectory::CheckTolerantTime()
{
	const auto pBullet = this->Bullet;
	const auto pType = this->GetType();
	// Check should detonate when no target
	if (!pBullet->Target && !pType->TolerantTime)
		return true;
	// Update timer
	if (pBullet->Target)
	{
		this->TolerantTimer.Stop();
	}
	else if (pType->TolerantTime > 0)
	{
		if (this->TolerantTimer.Completed())
			return true;
		else if (!this->TolerantTimer.IsTicking())
			this->TolerantTimer.Start(pType->TolerantTime);
	}

	return false;
}

// =============================
// load / save

void PhobosTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->Speed.Read(exINI, pSection, "Trajectory.Speed");
	this->Speed = Math::max(0.001, this->Speed);
	this->Duration.Read(exINI, pSection, "Trajectory.Duration");
	this->TolerantTime.Read(exINI, pSection, "Trajectory.TolerantTime");
	this->CreateCapacity.Read(exINI, pSection, "Trajectory.CreateCapacity");
	this->BulletROT.Read(exINI, pSection, "Trajectory.BulletROT");
	this->BulletFacing.Read(exINI, pSection, "Trajectory.BulletFacing");
	this->RetargetInterval.Read(exINI, pSection, "Trajectory.RetargetInterval");
	this->RetargetInterval = Math::max(1, this->RetargetInterval);
	this->RetargetRadius.Read(exINI, pSection, "Trajectory.RetargetRadius");
	this->Synchronize.Read(exINI, pSection, "Trajectory.Synchronize");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.MirrorCoord");
	this->PeacefulVanish.Read(exINI, pSection, "Trajectory.PeacefulVanish");
	this->ApplyRangeModifiers.Read(exINI, pSection, "Trajectory.ApplyRangeModifiers");
	this->UseDisperseCoord.Read(exINI, pSection, "Trajectory.UseDisperseCoord");
	this->RecordSourceCoord.Read(exINI, pSection, "Trajectory.RecordSourceCoord");

	this->PassDetonate.Read(exINI, pSection, "Trajectory.PassDetonate");
	this->PassDetonateLocal.Read(exINI, pSection, "Trajectory.PassDetonateLocal");
	this->PassDetonateWarhead.Read<true>(exINI, pSection, "Trajectory.PassDetonateWarhead");
	this->PassDetonateDamage.Read(exINI, pSection, "Trajectory.PassDetonateDamage");
	this->PassDetonateDelay.Read(exINI, pSection, "Trajectory.PassDetonateDelay");
	this->PassDetonateDelay = Math::max(1, this->PassDetonateDelay);
	this->PassDetonateInitialDelay.Read(exINI, pSection, "Trajectory.PassDetonateInitialDelay");
	this->PassDetonateInitialDelay = Math::max(0, this->PassDetonateInitialDelay);
	this->ProximityImpact.Read(exINI, pSection, "Trajectory.ProximityImpact");
	this->ProximityWarhead.Read<true>(exINI, pSection, "Trajectory.ProximityWarhead");
	this->ProximityDamage.Read(exINI, pSection, "Trajectory.ProximityDamage");
	this->ProximityRadius.Read(exINI, pSection, "Trajectory.ProximityRadius");
	this->ProximityDirect.Read(exINI, pSection, "Trajectory.ProximityDirect");
	this->ProximityMedial.Read(exINI, pSection, "Trajectory.ProximityMedial");
	this->ProximityAllies.Read(exINI, pSection, "Trajectory.ProximityAllies");
	this->ProximityFlight.Read(exINI, pSection, "Trajectory.ProximityFlight");
	this->ThroughVehicles.Read(exINI, pSection, "Trajectory.ThroughVehicles");
	this->ThroughBuilding.Read(exINI, pSection, "Trajectory.ThroughBuilding");
	this->DamageEdgeAttenuation.Read(exINI, pSection, "Trajectory.DamageEdgeAttenuation");
	this->DamageEdgeAttenuation = Math::max(0.0, this->DamageEdgeAttenuation);
	this->DamageCountAttenuation.Read(exINI, pSection, "Trajectory.DamageCountAttenuation");
	this->DamageCountAttenuation = Math::max(0.0, this->DamageCountAttenuation);

	this->DisperseWeapons.Read(exINI, pSection, "Trajectory.DisperseWeapons");
	this->DisperseBursts.Read(exINI, pSection, "Trajectory.DisperseBursts");
	this->DisperseCounts.Read(exINI, pSection, "Trajectory.DisperseCounts");
	this->DisperseDelays.Read(exINI, pSection, "Trajectory.DisperseDelays");
	this->DisperseCycle.Read(exINI, pSection, "Trajectory.DisperseCycle");
	this->DisperseInitialDelay.Read(exINI, pSection, "Trajectory.DisperseInitialDelay");
	this->DisperseEffectiveRange.Read(exINI, pSection, "Trajectory.DisperseEffectiveRange");
	this->DisperseSeparate.Read(exINI, pSection, "Trajectory.DisperseSeparate");
	this->DisperseRetarget.Read(exINI, pSection, "Trajectory.DisperseRetarget");
	this->DisperseLocation.Read(exINI, pSection, "Trajectory.DisperseLocation");
	this->DisperseTendency.Read(exINI, pSection, "Trajectory.DisperseTendency");
	this->DisperseHolistic.Read(exINI, pSection, "Trajectory.DisperseHolistic");
	this->DisperseMarginal.Read(exINI, pSection, "Trajectory.DisperseMarginal");
	this->DisperseDoRepeat.Read(exINI, pSection, "Trajectory.DisperseDoRepeat");
	this->DisperseSuicide.Read(exINI, pSection, "Trajectory.DisperseSuicide");
	this->DisperseFromFirer.Read(exINI, pSection, "Trajectory.DisperseFromFirer");
	this->DisperseFaceCheck.Read(exINI, pSection, "Trajectory.DisperseFaceCheck");
	this->DisperseForceFire.Read(exINI, pSection, "Trajectory.DisperseForceFire");
	this->DisperseCoord.Read(exINI, pSection, "Trajectory.DisperseCoord");
}

bool PhobosTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Serialize(Stm);
	return true;
}

bool PhobosTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	const_cast<PhobosTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

template<typename T>
void PhobosTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->Speed)
		.Process(this->Duration)
		.Process(this->TolerantTime)
		.Process(this->CreateCapacity)
		.Process(this->BulletROT)
		.Process(this->BulletFacing)
		.Process(this->RetargetInterval)
		.Process(this->RetargetRadius)
		.Process(this->Synchronize)
		.Process(this->MirrorCoord)
		.Process(this->PeacefulVanish)
		.Process(this->ApplyRangeModifiers)
		.Process(this->UseDisperseCoord)
		.Process(this->RecordSourceCoord)
		.Process(this->Ranged)

		.Process(this->PassDetonate)
		.Process(this->PassDetonateLocal)
		.Process(this->PassDetonateWarhead)
		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateDelay)
		.Process(this->PassDetonateInitialDelay)
		.Process(this->ProximityImpact)
		.Process(this->ProximityWarhead)
		.Process(this->ProximityDamage)
		.Process(this->ProximityRadius)
		.Process(this->ProximityDirect)
		.Process(this->ProximityMedial)
		.Process(this->ProximityAllies)
		.Process(this->ProximityFlight)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->DamageEdgeAttenuation)
		.Process(this->DamageCountAttenuation)

		.Process(this->DisperseWeapons)
		.Process(this->DisperseBursts)
		.Process(this->DisperseCounts)
		.Process(this->DisperseDelays)
		.Process(this->DisperseCycle)
		.Process(this->DisperseInitialDelay)
		.Process(this->DisperseEffectiveRange)
		.Process(this->DisperseSeparate)
		.Process(this->DisperseRetarget)
		.Process(this->DisperseLocation)
		.Process(this->DisperseTendency)
		.Process(this->DisperseHolistic)
		.Process(this->DisperseMarginal)
		.Process(this->DisperseDoRepeat)
		.Process(this->DisperseSuicide)
		.Process(this->DisperseFromFirer)
		.Process(this->DisperseFaceCheck)
		.Process(this->DisperseForceFire)
		.Process(this->DisperseCoord)
		;
}

bool PhobosTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Serialize(Stm);
	return true;
}

bool PhobosTrajectory::Save(PhobosStreamWriter& Stm) const
{
	const_cast<PhobosTrajectory*>(this)->Serialize(Stm);
	return true;
}

template<typename T>
void PhobosTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Bullet)
		.Process(this->MovingVelocity)
		.Process(this->MovingSpeed)
		.Process(this->DurationTimer)
		.Process(this->TolerantTimer)
		.Process(this->RetargetTimer)
		.Process(this->FirepowerMult)
		.Process(this->AttenuationRange)
		.Process(this->RemainingDistance)
		.Process(this->TargetInTheAir)
		.Process(this->TargetIsTechno)
		.Process(this->NotMainWeapon)
		.Process(this->ShouldDetonate)
		.Process(this->FLHCoord)
		.Process(this->CurrentBurst)
		.Process(this->CountOfBurst)

		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateTimer)
		.Process(this->ProximityImpact)
		.Process(this->ProximityDamage)
		.Process(this->ExtraCheck)
		.Process(this->TheCasualty)

		.Process(this->DisperseIndex)
		.Process(this->DisperseCount)
		.Process(this->DisperseCycle)
		.Process(this->DisperseTimer)
		;
}

// =============================
// hooks

DEFINE_HOOK(0x468B72, BulletClass_Unlimbo_Trajectories, 0x5)
{
	GET(BulletClass* const, pThis, EBX);

	const auto pExt = BulletExt::ExtMap.Find(pThis);
	const auto pTypeExt = pExt->TypeExtData;

	if (pTypeExt && pTypeExt->TrajectoryType)
	{
		pExt->Trajectory = pTypeExt->TrajectoryType->CreateInstance(pThis);
		pExt->Trajectory->OnUnlimbo();
	}

	return 0;
}

DEFINE_HOOK(0x46745C, BulletClass_Update_TrajectoriesVelocityUpdate, 0x7)
{
	GET(BulletClass* const, pThis, EBP);
	LEA_STACK(BulletVelocity*, pSpeed, STACK_OFFSET(0x1AC, -0x11C));
	LEA_STACK(BulletVelocity*, pPosition, STACK_OFFSET(0x1AC, -0x144));

	const auto pExt = BulletExt::ExtMap.Find(pThis);

	if (const auto pTraj = pExt->Trajectory.get())
	{
		pTraj->OnVelocityUpdate(pSpeed, pPosition);

		// Trajectory can use Velocity only for turning Image's direction
		// The true position in the next frame will be calculate after here
		if (pExt->LaserTrails.size())
		{
			const auto futureCoords = PhobosTrajectory::Vector2Coord(*pSpeed + *pPosition);

			for (auto& trail : pExt->LaserTrails)
			{
				if (!trail.LastLocation.isset())
					trail.LastLocation = pThis->Location;

				trail.Update(futureCoords);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4677D3, BulletClass_Update_TrajectoriesDetonateUpdate, 0x5)
{
	enum { SkipCheck = 0x467B7A, ContinueAfterCheck = 0x467879, Detonate = 0x467E53 };

	GET(BulletClass* const, pThis, EBP);
	REF_STACK(const CoordStruct, position, STACK_OFFSET(0x1AC, -0x188));

	const auto pExt = BulletExt::ExtMap.Find(pThis);

	if (const auto pTraj = pExt->Trajectory.get())
	{
		switch (pTraj->OnDetonateUpdate(position))
		{
		case TrajectoryCheckReturnType::SkipGameCheck:
			return SkipCheck; // Skip all vanilla check
		case TrajectoryCheckReturnType::SatisfyGameCheck:
			return ContinueAfterCheck; // Continue next vanilla check
		case TrajectoryCheckReturnType::Detonate:
			pThis->SetLocation(position);
			return Detonate; // Directly detonate
		default: // TrajectoryCheckReturnType::ExecuteGameCheck
			break; // Do vanilla check
		}
	}

	return 0;
}

DEFINE_HOOK(0x467BAC, BulletClass_Update_CheckObstacle_Trajectories, 0x6)
{
	enum { SkipVanillaCheck = 0x467C0C };

	GET(BulletClass* const, pThis, EBP);

	const auto pExt = BulletExt::ExtMap.Find(pThis);

	if (const auto pTraj = pExt->Trajectory.get())
	{
		// Already checked when the speed is high
		if (PhobosTrajectory::Get2DVelocity(pTraj->MovingVelocity) >= Unsorted::LeptonsPerCell)
			return SkipVanillaCheck;
	}

	return 0;
}

DEFINE_HOOK(0x467E53, BulletClass_Update_PreDetonation_Trajectories, 0x6)
{
	GET(BulletClass* const, pThis, EBP);

	const auto pExt = BulletExt::ExtMap.Find(pThis);

	if (const auto pTraj = pExt->Trajectory.get())
		pTraj->OnPreDetonate();

	return 0;
}
