#include "SampleTrajectory.h"

#include <Ext/Bullet/Body.h>

// Create
std::unique_ptr<PhobosTrajectory> SampleTrajectoryType::CreateInstance() const
{
	return std::make_unique<SampleTrajectory>(this);
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
	INI_EX exINI(pINI);
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Sample.TargetSnapDistance");
}

// Save and Load for entity
template<typename T>
void SampleTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->Type)
		.Process(this->TargetSnapDistance)
		;
}

bool SampleTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Serialize(Stm);
	return true;
}

bool SampleTrajectory::Save(PhobosStreamWriter& Stm) const
{
	const_cast<SampleTrajectory*>(this)->Serialize(Stm);
	return true;
}

// Do some math here to set the initial speed or location of your bullet.
// Be careful not to let the bullet speed too fast without other processing.
void SampleTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->Type->Trajectory_Speed / pBullet->Velocity.Magnitude();
}

// Some early checks here, returns whether or not to detonate the bullet.
// You can change the bullet's true velocity or set its location here. If you modify them here, it will affect the incoming parameters in OnAIVelocity.
bool SampleTrajectory::OnAI(BulletClass* pBullet)
{
	const auto distance = pBullet->TargetCoords.DistanceFrom(pBullet->Location);
	const auto velocity = pBullet->Velocity.Magnitude();

	if (distance < velocity)
		pBullet->Velocity *= (distance / velocity);

	return false;
}

// At this time, the bullet has hit the target and is ready to detonate.
// You can make it change before detonating.
void SampleTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= this->TargetSnapDistance)
	{
		BulletExt::ExtMap.Find(pBullet)->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

// Where you can update the bullet's speed and position.
// pSpeed: From the basic `Velocity` of the bullet plus gravity. It is only used in the calculation of this frame and will not be retained to the next frame.
// pPosition: From the current `Location` of the bullet, then the bullet will be set location to (*pSpeed + *pPosition). So don't use SetLocation here.
// You can also do additional processing here so that the position of the bullet will not change with its true velocity.
void SampleTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
}

// Where additional checks based on bullet reaching its target coordinate can be done.
// Vanilla code will do additional checks regarding buildings on target coordinate and Vertical projectiles and will detonate the projectile if they pass.
// Return value determines what is done regards to the game checks: they can be skipped, executed as normal or treated as if the condition is already satisfied.
TrajectoryCheckReturnType SampleTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

// Where additional checks based on a TechnoClass instance in same cell as the bullet can be done.
// Vanilla code will do additional trajectory alterations here if there is an enemy techno in the cell.
// Return value determines what is done regards to the game checks: they can be skipped, executed as normal or treated as if the condition is already satisfied.
// pTechno: TechnoClass instance in same cell as the bullet. Note that you should first check whether it is a nullptr.
TrajectoryCheckReturnType SampleTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}
