#include "PhobosActualTrajectory.h"

#include <Ext/Bullet/Body.h>

template<typename T>
void ActualTrajectoryType::Serialize(T& Stm)
{
	Stm
		.Process(this->RotateCoord)
		.Process(this->OffsetCoord)
		.Process(this->AxisOfRotation)
		.Process(this->LeadTimeCalculate)
		.Process(this->SubjectToGround)
		.Process(this->EarlyDetonation)
		.Process(this->DetonationHeight)
		.Process(this->DetonationDistance)
		.Process(this->TargetSnapDistance)
		;
}

bool ActualTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool ActualTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<ActualTrajectoryType*>(this)->Serialize(Stm);
	return true;
}
/*
void ActualTrajectoryType::Read(CCINIClass* const pINI, const char* pSection) // Read separately
{
	this->PhobosTrajectoryType::Read(pINI, pSection);
	INI_EX exINI(pINI);

	this->RotateCoord.Read(exINI, pSection, "Trajectory.RotateCoord");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.OffsetCoord");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.AxisOfRotation");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.LeadTimeCalculate");
	this->EarlyDetonation.Read(exINI, pSection, "Trajectory.EarlyDetonation");
	this->DetonationHeight.Read(exINI, pSection, "Trajectory.DetonationHeight");
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.TargetSnapDistance");
}
*/
template<typename T>
void ActualTrajectory::Serialize(T& Stm)
{
	Stm
		.Process(this->LastTargetCoord)
		.Process(this->WaitOneFrame)
		;
}

bool ActualTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool ActualTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);
	const_cast<ActualTrajectory*>(this)->Serialize(Stm);
	return true;
}

void ActualTrajectory::OnUnlimbo()
{
	this->PhobosTrajectory::OnUnlimbo();
	// Actual
	const auto pType = this->GetType();
	this->LastTargetCoord = this->Bullet->TargetCoords;
	// Survival time
	if (pType->Duration > 0)
		this->DurationTimer.Start(pType->Duration);
}

bool ActualTrajectory::OnEarlyUpdate()
{
	if (this->WaitOneFrame && this->BulletPrepareCheck())
		return false;

	return this->PhobosTrajectory::OnEarlyUpdate();
}

void ActualTrajectory::OnPreDetonate()
{
	const auto targetSnapDistance = static_cast<const ActualTrajectoryType*>(this->GetType())->TargetSnapDistance.Get();
	// Can snap to target?
	if (targetSnapDistance > 0)
	{
		const auto pBullet = this->Bullet;
		const auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
		const auto coords = pTarget ? pTarget->GetCoords() : pBullet->TargetCoords;
		// Whether to snap to target?
		if (coords.DistanceFrom(pBullet->Location) <= targetSnapDistance)
		{
			const auto pExt = BulletExt::ExtMap.Find(pBullet);
			pExt->SnappedToTarget = true;
			pBullet->SetLocation(coords);
		}
	}

	this->PhobosTrajectory::OnPreDetonate();
}

bool ActualTrajectory::BulletPrepareCheck()
{
	// The time between bullets' Unlimbo() and Update() is completely uncertain.
	// Target will update location after techno firing, which may result in inaccurate
	// target position recorded by the LastTargetCoord in Unlimbo(). Therefore, it's
	// necessary to record the position during the first Update(). - CrimRecya
	if (this->WaitOneFrame == 2)
	{
		if (const auto pTarget = this->Bullet->Target)
		{
			this->LastTargetCoord = pTarget->GetCoords();
			this->WaitOneFrame = 1;
			return true;
		}
	}
	// Confirm the launch of the trajectory
	this->WaitOneFrame = 0;
	this->FireTrajectory();
	return false;
}

CoordStruct ActualTrajectory::GetOnlyStableOffsetCoords(double rotateRadian)
{
	const auto pType = static_cast<const ActualTrajectoryType*>(this->GetType());
	auto offsetCoord = pType->OffsetCoord.Get();
	// Check if mirroring is required
	if (pType->MirrorCoord && this->CurrentBurst < 0)
		offsetCoord.Y = -offsetCoord.Y;
	// Rotate the angle and return
	return PhobosTrajectory::Vector2Coord(PhobosTrajectory::HorizontalRotate(offsetCoord, rotateRadian));
}

CoordStruct ActualTrajectory::GetInaccurateTargetCoords(const CoordStruct& baseCoord, double distance)
{
	const auto pBullet = this->Bullet;
	const auto pWeapon = pBullet->WeaponType;
	const auto pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
	// Don't know whether the weapon is correctly set, if not, a fixed value of 10 will be used
	const auto offsetMult = distance / (pWeapon ? pWeapon->Range : (10.0 * Unsorted::LeptonsPerCell));
	const auto offsetMin = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Min.Get(Leptons(0)));
	const auto offsetMax = static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter)));
	const auto offsetDistance = ScenarioClass::Instance->Random.RandomRanged(offsetMin, offsetMax);
	// Substitute to calculate random coordinates
	return MapClass::GetRandomCoordsNear(baseCoord, offsetDistance, false);
}

void ActualTrajectory::DisperseBurstSubstitution(double baseRadian)
{
	const auto pType = static_cast<const ActualTrajectoryType*>(this->GetType());
	const auto axis = pType->AxisOfRotation.Get();
	// Calculate the actual rotation axis
	auto rotationAxis = PhobosTrajectory::HorizontalRotate(axis, baseRadian);
	double extraRotate = 0.0;
	const auto burst = (this->CurrentBurst < 0) ? (-this->CurrentBurst - 1) : this->CurrentBurst;
	// Symmetry and initial direction can be calculated separately
	if (pType->MirrorCoord)
	{
		if (this->CurrentBurst < 0)
			rotationAxis *= -1;
		// Rotate half the angle in the opposite direction
		extraRotate = Math::Pi * (pType->RotateCoord * ((burst / 2) / (this->CountOfBurst - 1.0) - 0.5)) / 180;
	}
	else
	{
		extraRotate = Math::Pi * (pType->RotateCoord * (burst / (this->CountOfBurst - 1.0) - 0.5)) / 180;
	}
	// Rotate the selected angle
	this->MovingVelocity = PhobosTrajectory::RotateAboutTheAxis(this->MovingVelocity, rotationAxis, extraRotate);
}
