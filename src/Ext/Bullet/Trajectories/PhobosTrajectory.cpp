#include "PhobosTrajectory.h"
#include "ActualTrajectories/StraightTrajectory.h"
#include "ActualTrajectories/BombardTrajectory.h"
#include "ActualTrajectories/MissileTrajectory.h"
#include "VirtualTrajectories/EngraveTrajectory.h"
#include "ActualTrajectories/ParabolaTrajectory.h"
#include "VirtualTrajectories/TracingTrajectory.h"

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

	// Record some information of weapon
	if (const auto pWeapon = pBullet->WeaponType)
		this->CountOfBurst = pWeapon->Burst;

	// Due to various ways of firing weapons, the true firer may have already died
	if (const auto pFirer = pBullet->Owner)
	{
		const auto burst = pFirer->CurrentBurstIndex;
		this->CurrentBurst = (burst & 1) ? (-burst - 1) : burst;
	}
}

// Something that needs to be done before updating the velocity of the projectile
bool PhobosTrajectory::OnEarlyUpdate()
{
	const auto pBullet = this->Bullet;
	const auto pBulletExt = BulletExt::ExtMap.Find(pBullet);

	// Update group index for members by themselves
	if (pBulletExt->TrajectoryGroup)
		pBulletExt->UpdateGroupIndex();

	// In the phase of playing PreImpactAnim
	if (pBullet->SpawnNextAnim)
		return false;

	// The previous check requires detonation at this time
	if (pBulletExt->Status & (TrajectoryStatus::Detonate | TrajectoryStatus::Vanish))
		return true;

	// Check the remaining existence time
	if (pBulletExt->LifeDurationTimer.Completed())
		return true;

	// Check if the firer's target can be synchronized
	if (pBulletExt->CheckSynchronize())
		return true;

	// Check if the target needs to be changed
	if (pBulletExt->TypeExtData->RetargetRadius && pBulletExt->BulletRetargetTechno())
		return true;

	// After the new target is confirmed, check if the tolerance time has ended
	if (pBulletExt->CheckNoTargetLifeTime())
		return true;

	// Based on the new target location, check how to change bullet velocity
	if (this->OnVelocityCheck())
		return true;

	// Rotate orientation
	this->OnFacingUpdate();

	// Fire weapons or warheads
	if (pBulletExt->FireAdditionals())
		return true;

	// Detonate extra warhead on the obstacle
	pBulletExt->DetonateOnObstacle();
	return false;
}

// It is possible to detonate here in advance or decide how to change the speed
bool PhobosTrajectory::OnVelocityCheck()
{
	const auto pBullet = this->Bullet;
	const auto pBulletExt = BulletExt::ExtMap.Find(pBullet);
	double ratio = 1.0;

	// If there is an obstacle on the route, the bullet should need to reduce its speed so it will not penetrate the obstacle.
	const auto pBulletTypeExt = pBulletExt->TypeExtData;
	const bool checkThrough = (!pBulletTypeExt->ThroughBuilding || !pBulletTypeExt->ThroughVehicles);
	const auto velocity = BulletExt::Get2DVelocity(this->MovingVelocity);

	// Low speed with checkSubject was already done well
	if (velocity < Unsorted::LeptonsPerCell)
	{
		// Blocked by obstacles?
		if (checkThrough)
		{
			const auto pFirer = pBullet->Owner;
			const auto pOwner = pFirer ? pFirer->Owner : pBulletExt->FirerHouse;

			// Check for additional obstacles on the ground
			if (pBulletExt->CheckThroughAndSubjectInCell(MapClass::Instance.GetCellAt(pBullet->Location), pOwner))
			{
				if (velocity > PhobosTrajectory::LowSpeedOffset)
					ratio = (PhobosTrajectory::LowSpeedOffset / velocity);
			}
		}

		// Check whether about to fall into the ground
		if (std::abs(this->MovingVelocity.Z) > Unsorted::CellHeight && this->GetCanHitGround())
		{
			const auto theTargetCoords = pBullet->Location + BulletExt::Vector2Coord(this->MovingVelocity);
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
		const bool subjectToWCS = pBulletType->SubjectToWalls || pBulletType->SubjectToCliffs || pBulletTypeExt->SubjectToSolid;
		const bool subjectToFirestorm = !pBulletType->IgnoresFirestorm;
		const bool checkCoords = subjectToGround || checkThrough || subjectToWCS;

		// If no inspection is needed, just skip it
		if (checkCoords || subjectToFirestorm)
		{
			double locationDistance = 0.0;
			bool velocityCheck = false;
			const auto& theSourceCoords = pBullet->Location;
			const auto theTargetCoords = theSourceCoords + BulletExt::Vector2Coord(this->MovingVelocity);
			const auto pFirer = pBullet->Owner;
			const auto pOwner = pFirer ? pFirer->Owner : pBulletExt->FirerHouse;

			// Skip when no inspection is needed
			if (checkCoords)
			{
				const auto pSourceCell = MapClass::Instance.GetCellAt(theSourceCoords);
				const auto pTargetCell = MapClass::Instance.GetCellAt(theTargetCoords);
				const auto sourceCell = pSourceCell->MapCoords;
				const auto targetCell = pTargetCell->MapCoords;
				const bool checkLevel = !pBulletTypeExt->SubjectToLand.isset() && !pBulletTypeExt->SubjectToWater.isset();
				const auto cellDist = sourceCell - targetCell;
				const auto cellPace = CellStruct { static_cast<short>(std::abs(cellDist.X)), static_cast<short>(std::abs(cellDist.Y)) };

				// Take big steps as much as possible to reduce check times, just ensure that each cell is inspected
				const auto largePace = static_cast<size_t>(Math::max(cellPace.X, cellPace.Y));
				const auto stepCoord = !largePace ? CoordStruct::Empty : (theTargetCoords - theSourceCoords) * (1.0 / largePace);
				auto curCoord = theSourceCoords;
				auto pCurCell = pSourceCell;
				auto pLastCell = MapClass::Instance.GetCellAt(pBullet->LastMapCoords);

				// Check one by one towards the direction of the next frame's position
				for (size_t i = 0; i < largePace; ++i)
				{
					if ((subjectToGround && (curCoord.Z + 16) < MapClass::Instance.GetCellFloorHeight(curCoord)) // Below ground level? (16 -> error range)
						|| (checkThrough && pBulletExt->CheckThroughAndSubjectInCell(pCurCell, pOwner)) // Blocked by obstacles?
						|| (subjectToWCS && TrajectoryHelper::GetObstacle(pSourceCell, pTargetCell, pLastCell, curCoord, pBulletType, pOwner)) // Impact on the wall/cliff/solid?
						|| (checkLevel ? (pBulletType->Level && pCurCell->IsOnFloor()) // Level or above land/water?
							: ((pCurCell->LandType == LandType::Water || pCurCell->LandType == LandType::Beach)
								? (pBulletTypeExt->SubjectToWater.Get(false) && pBulletTypeExt->SubjectToWater_Detonate)
								: (pBulletTypeExt->SubjectToLand.Get(false) && pBulletTypeExt->SubjectToLand_Detonate))))
					{
						locationDistance = BulletExt::Get2DDistance(curCoord, theSourceCoords);
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
					const auto distance = BulletExt::Get2DDistance(fireStormCoords, theSourceCoords);

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
				locationDistance += PhobosTrajectory::LowSpeedOffset;

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
	if (this->MovingSpeed >= 0.5)
		*pSpeed = this->MovingVelocity;
	else
		*pSpeed = BulletVelocity::Empty;
}

// Something that needs to be done after updating the velocity of the projectile
TrajectoryCheckReturnType PhobosTrajectory::OnDetonateUpdate(const CoordStruct& position)
{
	// Need to detonate at the next location
	if (BulletExt::ExtMap.Find(this->Bullet)->Status & (TrajectoryStatus::Detonate | TrajectoryStatus::Vanish))
		return TrajectoryCheckReturnType::Detonate;

	// Below ground level? (16 -> error range)
	if (this->GetCanHitGround() && MapClass::Instance.GetCellFloorHeight(position) >= (position.Z + 16))
		return TrajectoryCheckReturnType::Detonate;

	// Skip all vanilla checks
	return TrajectoryCheckReturnType::SkipGameCheck;
}

// Something that needs to be done before detonation (and PreImpactAnim)
void PhobosTrajectory::OnPreDetonate()
{
	const auto pBullet = this->Bullet;
	const auto pBulletExt = BulletExt::ExtMap.Find(pBullet);
	const auto pBulletTypeExt = pBulletExt->TypeExtData;

	// Special circumstances, similar to airburst behavior
	if (pBulletTypeExt->DisperseEffectiveRange.Get() < 0)
		pBulletExt->PrepareDisperseWeapon();

	if (!(pBulletExt->Status & TrajectoryStatus::Vanish))
	{
		if (!pBulletTypeExt->PeacefulVanish.Get(this->Flag() == TrajectoryFlag::Engrave || pBulletTypeExt->ProximityImpact || pBulletTypeExt->DisperseCycle))
		{
			// Calculate the current damage
			pBullet->Health = pBulletExt->GetTrueDamage(pBullet->Health, true);
			return;
		}

		pBulletExt->Status |= TrajectoryStatus::Vanish;
	}

	// To skip all extra effects, no damage, no anims...
	pBullet->Health = 0;
	pBullet->Limbo();
	pBullet->UnInit();
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
	if (pType->BulletFacing == TrajectoryFacing::Spin || pType->BulletFacingOnPlane)
		pBullet->Velocity.Z = 0;

	// When the speed is delicate, there is a problem with the vanilla processing at the starting position
	if (BulletExt::Get2DVelocity(this->MovingVelocity) < Unsorted::LeptonsPerCell)
	{
		const auto pBulletType = pBullet->Type;

		if (pBulletType->SubjectToWalls || pBulletType->SubjectToCliffs || BulletTypeExt::ExtMap.Find(pBulletType)->SubjectToSolid)
		{
			const auto pSourceCell = MapClass::Instance.GetCellAt(source);
			const auto pTargetCell = MapClass::Instance.GetCellAt(target);
			const auto pFirer = pBullet->Owner;
			const auto pOwner = pFirer ? pFirer->Owner : BulletExt::ExtMap.Find(pBullet)->FirerHouse;

			if (TrajectoryHelper::GetObstacle(pSourceCell, pTargetCell, pSourceCell, pBullet->Location, pBulletType, pOwner))
				BulletExt::ExtMap.Find(pBullet)->Status |= TrajectoryStatus::Detonate;
		}
	}
}

// Something that needs to be done when changing the target of the projectile
void PhobosTrajectory::SetBulletNewTarget(AbstractClass* const pTarget)
{
	const auto pBullet = this->Bullet;
	pBullet->Target = pTarget;
	pBullet->TargetCoords = pTarget->GetCoords();
}

// Something that needs to be done when setting the new speed of the projectile
bool PhobosTrajectory::CalculateBulletVelocity(const double speed)
{
	const double velocityLength = this->MovingVelocity.Magnitude();

	// Check if it is a zero vector
	if (velocityLength < BulletExt::Epsilon)
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
		BulletExt::ExtMap.Find(this->Bullet)->Status |= TrajectoryStatus::Detonate;
}

/*!
	Rotate one vector by a certain angle towards the direction of another vector.

	\param vector Vector that needs to be rotated. This function directly modifies this value.
	\param aim The final direction vector that needs to be oriented. It doesn't need to be a standardized vector.
	\param turningRadian The maximum radius that can rotate. Note that it must be a positive number.

	\returns No return value, result is vector.

	\author CrimRecya
*/
void PhobosTrajectory::RotateVector(BulletVelocity& vector, const BulletVelocity& aim, const double turningRadian)
{
	const double baseFactor = sqrt(aim.MagnitudeSquared() * vector.MagnitudeSquared());

	// Not valid vector
	if (baseFactor <= BulletExt::Epsilon)
	{
		vector = aim;
		return;
	}

	// Try using the vector to calculate the included angle
	const double dotProduct = (aim * vector);

	// Calculate the cosine of the angle when the conditions are suitable
	const double cosTheta = dotProduct / baseFactor;

	// Ensure that the result range of cos is correct
	const double radian = Math::acos(Math::clamp(cosTheta, -1.0, 1.0));

	// When the angle is small, aim directly at the target
	if (std::abs(radian) <= turningRadian)
	{
		vector = aim;
		return;
	}

	// Calculate the rotation axis
	auto rotationAxis = aim.CrossProduct(vector);

	// The radian can rotate, input the correct direction
	const double rotateRadian = (radian < 0 ? turningRadian : -turningRadian);

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
void PhobosTrajectory::RotateAboutTheAxis(BulletVelocity& vector, BulletVelocity& axis, const double radian)
{
	const auto axisLengthSquared = axis.MagnitudeSquared();

	// Zero axis vector is not acceptable
	if (axisLengthSquared < BulletExt::Epsilon)
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
	const auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
	const auto pType = this->GetType();

	if (!pBulletTypeExt->DisperseFaceCheck)
		return true;

	const auto facing = pType->BulletFacing;

	if (facing == TrajectoryFacing::Velocity || facing == TrajectoryFacing::Spin)
		return true;

	const auto flag = this->Flag();

	if (pBulletTypeExt->DisperseFromFirer.Get(flag == TrajectoryFlag::Engrave || flag == TrajectoryFlag::Tracing))
		return true;

	const auto targetDir = DirStruct { BulletExt::Get2DOpRadian(pBullet->Location, pBullet->TargetCoords) };
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
			desiredFacing = BulletExt::Coord2Vector(pTarget->GetCoords() - pBullet->Location);
		else
			desiredFacing = BulletExt::Coord2Vector(pBullet->TargetCoords - pBullet->Location);
	}
	else if (facing == TrajectoryFacing::Destination)
	{
		desiredFacing = BulletExt::Coord2Vector(pBullet->TargetCoords - pBullet->Location);
	}
	else if (const auto pFirer = pBullet->Owner)
	{
		const double radian = -(facing == TrajectoryFacing::FirerTurret ? pFirer->TurretFacing() : pFirer->PrimaryFacing.Current()).GetRadian<65536>();
		desiredFacing.X = Math::cos(radian);
		desiredFacing.Y = Math::sin(radian);
		desiredFacing.Z = 0;
	}
	else
	{
		return;
	}

	if (pType->BulletROT <= 0)
	{
		pBullet->Velocity = desiredFacing;
		pBullet->Velocity *= (1 / pBullet->Velocity.Magnitude());
	}
	else
	{
		// Restricted to rotation only on a horizontal plane
		if (pType->BulletFacingOnPlane)
		{
			pBullet->Velocity.Z = 0;
			desiredFacing.Z = 0;
		}

		// Calculate specifically only when the ROT is reasonable
		PhobosTrajectory::RotateVector(pBullet->Velocity, desiredFacing, (std::abs(pType->BulletROT) * ratio));

		// Standardizing
		pBullet->Velocity *= (1 / pBullet->Velocity.Magnitude());
	}
}

// =============================
// load / save

void PhobosTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->Speed.Read(exINI, pSection, "Trajectory.Speed");
	this->Speed = Math::max(0.001, this->Speed);
	this->BulletROT.Read(exINI, pSection, "Trajectory.BulletROT");
	this->BulletFacing.Read(exINI, pSection, "Trajectory.BulletFacing");
	this->BulletFacingOnPlane.Read(exINI, pSection, "Trajectory.BulletFacingOnPlane");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.MirrorCoord");
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
		.Process(this->BulletROT)
		.Process(this->BulletFacing)
		.Process(this->BulletFacingOnPlane)
		.Process(this->MirrorCoord)
		.Process(this->Ranged)
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
		.Process(this->RemainingDistance)
		.Process(this->CurrentBurst)
		.Process(this->CountOfBurst)
		;
}

// =============================
// hooks

DEFINE_HOOK(0x468B72, BulletClass_Unlimbo_Trajectories, 0x5)
{
	GET(BulletClass* const, pThis, EBX);

	const auto pExt = BulletExt::ExtMap.Find(pThis);

	// Initialize before trajectory unlimbo
	pExt->InitializeOnUnlimbo();

	if (const auto pTrajType = pExt->TypeExtData->TrajectoryType.get())
	{
		pExt->Trajectory = pTrajType->CreateInstance(pThis);
		pExt->Trajectory->OnUnlimbo();
	}
	else if (pExt->TypeExtData->LifeDuration > 0)
	{
		pExt->LifeDurationTimer.Start(pExt->TypeExtData->LifeDuration);
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
			const auto futureCoords = BulletExt::Vector2Coord(*pSpeed + *pPosition);

			for (const auto& pTrail : pExt->LaserTrails)
			{
				if (!pTrail->LastLocation.isset())
					pTrail->LastLocation = pThis->Location;

				pTrail->Update(futureCoords);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x467609, BulletClass_Update_TrajectoriesSkipResetHeight, 0x6)
{
	enum { SkipGameCode = 0x46777A };

	GET(BulletClass* const, pThis, EBP);

	if (!BulletExt::ExtMap.Find(pThis)->Trajectory)
		return 0;

	R->ECX(0);
	return SkipGameCode;
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
	else if (pExt->Status & (TrajectoryStatus::Detonate | TrajectoryStatus::Vanish))
	{
		pThis->SetLocation(position);
		return Detonate;
	}

	return 0;
}

DEFINE_HOOK(0x467BAC, BulletClass_Update_TrajectoriesCheckObstacle, 0x6)
{
	enum { SkipVanillaCheck = 0x467C0C };

	GET(BulletClass* const, pThis, EBP);

	if (const auto pTraj = BulletExt::ExtMap.Find(pThis)->Trajectory.get())
	{
		// Already checked when the speed is high
		if (BulletExt::Get2DVelocity(pTraj->MovingVelocity) >= Unsorted::LeptonsPerCell)
			return SkipVanillaCheck;
	}

	return 0;
}

DEFINE_HOOK(0x467E53, BulletClass_Update_TrajectoriesPreDetonation, 0x6)
{
	GET(BulletClass* const, pThis, EBP);

	const auto pExt = BulletExt::ExtMap.Find(pThis);

	if (const auto pTraj = pExt->Trajectory.get())
		pTraj->OnPreDetonate();
	else // Due to virtual call in trajectory and the different processing order of base class, this should be separated
		pExt->CheckOnPreDetonate();

	return 0;
}

DEFINE_HOOK(0x468585, BulletClass_PointerExpired_Trajectories, 0x9)
{
	enum { SkipSetCellAsTarget = 0x46859C };

	GET(BulletClass* const, pThis, ESI);

	return BulletExt::ExtMap.Find(pThis)->Trajectory ? SkipSetCellAsTarget : 0;
}

// Vanilla inertia effect only for bullets with ROT=0
DEFINE_HOOK(0x415F25, AircraftClass_Fire_TrajectorySkipInertiaEffect, 0x6)
{
	enum { SkipCheck = 0x4160BC };

	GET(BulletClass*, pThis, ESI);

	if (BulletExt::ExtMap.Find(pThis)->Trajectory)
		return SkipCheck;

	return 0;
}

// Engrave laser using the unique logic
DEFINE_HOOK(0x6FD217, TechnoClass_CreateLaser_EngraveDrawNoLaser, 0x5)
{
	enum { SkipCreate = 0x6FD456 };

	GET(WeaponTypeClass*, pWeapon, EAX);

	if (const auto pTrajType = BulletTypeExt::ExtMap.Find(pWeapon->Projectile)->TrajectoryType.get())
	{
		const auto flag = pTrajType->Flag();

		if (flag == TrajectoryFlag::Engrave || flag == TrajectoryFlag::Tracing)
			return SkipCreate;
	}

	return 0;
}

// Update trajectories target
DEFINE_HOOK(0x46B5A4, BulletClass_SetTarget_SetTrajectoryTarget, 0x6)
{
	enum { SkipGameCode = 0x46B5AA };

	GET(BulletClass*, pThis, ECX);
	GET(AbstractClass*, pTarget, EAX);

	if (const auto pTraj = BulletExt::ExtMap.Find(pThis)->Trajectory.get())
	{
		if (pTarget)
			pTraj->SetBulletNewTarget(pTarget);

		return SkipGameCode;
	}

	return 0;
}
