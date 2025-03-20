#include "SampleTrajectory.h"

#include <Ext/Bullet/Body.h>

// Create
std::unique_ptr<PhobosTrajectory> SampleTrajectoryType::CreateInstance(BulletClass* pBullet) const
{
	return std::make_unique<SampleTrajectory>(this, pBullet);
}

// Save and Load for type
template<typename T>
void SampleTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->TargetSnapDistance)
		;
}

bool SampleTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool SampleTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<SampleTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

// INI reading stuff
void SampleTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->PhobosTrajectoryType::Read(pINI, pSection);
	INI_EX exINI(pINI);
	// Sample
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.TargetSnapDistance");
}

// Save and Load for entity
template<typename T>
void SampleTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		;
}

bool SampleTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool SampleTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);
	const_cast<SampleTrajectory*>(this)->Serialize(Stm);
	return true;
}

// Record some information for your bullet.
void SampleTrajectory::OnUnlimbo()
{
	this->PhobosTrajectory::OnUnlimbo();
	// Sample
	const auto pBullet = this->Bullet;
	this->RemainingDistance += static_cast<int>(pBullet->SourceCoords.DistanceFrom(pBullet->TargetCoords));
	// Waiting for launch trigger
	if (!BulletExt::ExtMap.Find(pBullet)->DispersedTrajectory)
		this->OpenFire();
}

// Some checks here, returns whether or not to detonate the bullet.
// You can change the bullet's true velocity or set its location here. If you modify them here, it will affect the incoming parameters in OnVelocityUpdate().
bool SampleTrajectory::OnEarlyUpdate()
{
	return this->PhobosTrajectory::OnEarlyUpdate();
}

// What needs to be done before launching the weapon after calculating the new speed.
bool SampleTrajectory::OnVelocityCheck()
{
	return this->PhobosTrajectory::OnVelocityCheck();
}

// Where you can update the bullet's speed and position. But I would recommend that you complete the calculation at OnEarlyUpdate().
// pSpeed: From the basic `Velocity` of the bullet plus gravity. It is only used in the calculation of this frame and will not be retained to the next frame.
// pPosition: From the current `Location` of the bullet, then the bullet will be set location to (*pSpeed + *pPosition). So don't use SetLocation here.
// You can also do additional processing here so that the position of the bullet will not change with its true velocity.
void SampleTrajectory::OnVelocityUpdate(BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	this->PhobosTrajectory::OnVelocityUpdate(pSpeed, pPosition);
}

// Where additional checks based on bullet reaching its target coordinate can be done.
// Vanilla code will do additional checks regarding buildings on target coordinate and Vertical projectiles and will detonate the projectile if they pass.
// Return value determines what is done regards to the game checks: they can be skipped, executed as normal or treated as if the condition is already satisfied.
TrajectoryCheckReturnType SampleTrajectory::OnDetonateUpdate(const CoordStruct& position)
{
	if (this->PhobosTrajectory::OnDetonateUpdate(position) == TrajectoryCheckReturnType::Detonate)
		return TrajectoryCheckReturnType::Detonate;

	this->RemainingDistance -= static_cast<int>(this->MovingSpeed);

	if (this->RemainingDistance < 0)
		return TrajectoryCheckReturnType::Detonate;

	return TrajectoryCheckReturnType::SkipGameCheck;
}

// At this time, the bullet has hit the target and is ready to detonate.
// You can make it change before detonating.
void SampleTrajectory::OnPreDetonate()
{
	const auto pBullet = this->Bullet;
	auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->TargetCoords;
	// Can snap to target?
	if (pCoords.DistanceFrom(pBullet->Location) <= this->Type->TargetSnapDistance.Get())
	{
		BulletExt::ExtMap.Find(pBullet)->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}

	this->PhobosTrajectory::OnPreDetonate();
}

// Do some math here to set the initial speed or location of your bullet.
// Be careful not to let the bullet speed too fast without other processing.
void SampleTrajectory::OpenFire()
{
	const auto pBullet = this->Bullet;
	this->MovingVelocity = PhobosTrajectory::Coord2Vector(pBullet->TargetCoords - pBullet->SourceCoords);
	this->CalculateBulletVelocity(this->Type->Speed);
	this->PhobosTrajectory::OpenFire();
}

// Does the projectile detonate when it lands below the ground
bool SampleTrajectory::GetCanHitGround() const
{
	return true;
}

// If need to research a target, where is the search center
CoordStruct SampleTrajectory::GetRetargetCenter() const
{
	return this->Bullet->TargetCoords;
}

// How to calculate when inputting velocity values after updating the velocity vector each time
bool SampleTrajectory::CalculateBulletVelocity(const double speed)
{
	return this->PhobosTrajectory::CalculateBulletVelocity(speed);
};

// How to do when should change to a new target
void SampleTrajectory::SetBulletNewTarget(AbstractClass* const pTarget)
{
	this->PhobosTrajectory::SetBulletNewTarget(pTarget);
}
