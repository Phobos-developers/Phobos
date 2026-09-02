#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/EBolt/Body.h>
#include <New/Entity/LaserTrailClass.h>

namespace LaserRT
{
	void SetLaserTrackingData(LaserDrawClass* pLaser, TechnoClass* pShooter, AbstractClass* pTarget, int weaponIdx, PositionFollow mode, bool ignoreShooter);
}

BulletExt::ExtContainer BulletExt::ExtMap;

void BulletExt::InterceptBullet(TechnoClass* pSource, BulletClass* pInterceptor)
{
	const auto pThis = this->OwnerObject();
	auto pTypeExt = this->TypeExtData;
	const auto pInterceptorType = BulletExt::Fetch(pInterceptor)->InterceptorTechnoType->InterceptorType.get();

	if (!pTypeExt->Armor.isset())
	{
		if (!pInterceptorType->KeepIntact)
			this->InterceptedStatus |= InterceptedStatus::Intercepted;
	}
	else
	{
		const double versus = GeneralUtils::GetWarheadVersusArmor(pInterceptor->WH, pTypeExt->Armor.Get());

		if (versus == 0.0)
			return;

		const int damage = static_cast<int>(pInterceptor->Health * versus);
		this->CurrentStrength -= damage;

		if (Phobos::DisplayDamageNumbers && damage != 0)
			GeneralUtils::DisplayDamageNumberString(damage, DamageDisplayType::Intercept, pThis->GetRenderCoords(), this->DamageNumberOffset);

		if (this->CurrentStrength <= 0)
		{
			this->CurrentStrength = 0;

			if (!pInterceptorType->KeepIntact)
				this->InterceptedStatus |= InterceptedStatus::Intercepted;
		}
	}

	this->DetonateOnInterception = !pInterceptorType->DeleteOnIntercept.Get(pTypeExt->Interceptable_DeleteOnIntercept);

	if (const auto pWeaponOverride = pInterceptorType->WeaponOverride.Get(pTypeExt->Interceptable_WeaponOverride))
	{
		pThis->WeaponType = pWeaponOverride;
		pThis->Health = pInterceptorType->WeaponCumulativeDamage.Get() ? pThis->Health + pWeaponOverride->Damage : pWeaponOverride->Damage;
		pThis->WH = pWeaponOverride->Warhead;
		pThis->Bright = pWeaponOverride->Bright;

		if (pInterceptorType->WeaponReplaceProjectile
			&& pWeaponOverride->Projectile
			&& pWeaponOverride->Projectile != pThis->Type)
		{
			pThis->Speed = pWeaponOverride->Speed;
			pThis->Type = pWeaponOverride->Projectile;
			pTypeExt = BulletTypeExt::Fetch(pThis->Type);
			this->TypeExtData = pTypeExt;

			if (this->LaserTrails.size())
				this->LaserTrails.clear();

			if (!pThis->Type->Inviso)
				this->InitializeLaserTrails();

			// Lose target if the current bullet is no longer interceptable.
			if (pSource && (!pTypeExt->Interceptable.Get(RulesExt::Global()->ProjectileInterceptable) || (pTypeExt->Armor.isset() && GeneralUtils::GetWarheadVersusArmor(pInterceptor->WH, pTypeExt->Armor.Get()) == 0.0)))
				pSource->SetTarget(nullptr);
		}
	}
}

void BulletExt::ApplyRadiationToCell(CellStruct cell, int spread, int radLevel)
{
	const auto pCell = MapClass::Instance.TryGetCellAt(cell);

	if (!pCell)
		return;

	const auto pThis = this->OwnerObject();
	const auto pWeapon = pThis->GetWeaponType();
	const auto pWeaponExt = WeaponTypeExt::Fetch(pWeapon);
	const auto pRadType = pWeaponExt->RadType;
	const auto pCellExt = CellExt::Fetch(pCell);

	const auto it = std::find_if(pCellExt->RadSites.cbegin(), pCellExt->RadSites.cend(),
		[=](const auto pSite)
		{
			const auto pRadExt = RadSiteExt::Fetch(pSite);

			if (pRadExt->Type != pRadType || spread != pSite->Spread)
				return false;

			if (pRadExt->RadInvoker && pThis->Owner)
				return pRadExt->RadInvoker == pThis->Owner;

			return true;
		}
	);

	if (it != pCellExt->RadSites.cend())
	{
		const auto pRadExt = RadSiteExt::Fetch(*it);
		// Handle It
		pRadExt->Add(std::min(radLevel, pRadType->GetLevelMax() - (*it)->GetRadLevel()));
		return;
	}

	const auto pThisHouse = pThis->Owner ? pThis->Owner->Owner : this->FirerHouse;
	RadSiteExt::CreateInstance(cell, spread, radLevel, pWeaponExt, pThisHouse, pThis->Owner);
}

void BulletExt::InitializeLaserTrails()
{
	if (this->LaserTrails.size())
		return;

	auto const pThis = this->OwnerObject();
	auto const pTypeExt = BulletTypeExt::Fetch(pThis->Type);
	auto const pOwner = pThis->Owner ? pThis->Owner->Owner : nullptr;
	this->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

	for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
		this->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(LaserTrailTypeClass::Array[idxTrail].get(), pOwner));
}

static inline int SetBuildingFireAnimZAdjust(BuildingClass* pBuilding, int animY)
{
	if (pBuilding->GetOccupantCount() > 0)
		return -200;

	const auto renderCoords = pBuilding->GetRenderCoords();
	const auto zAdj = (animY - renderCoords.Y) / -4;
	return (zAdj >= 0) ? 0 : zAdj;
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringAnim(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach)
{
	const auto pWeapon = pBullet->WeaponType;
	const auto animCounts = pWeapon->Anim.Count;

	if (animCounts <= 0)
		return;

	const auto pFirer = pBullet->Owner;
	const auto pAnimType = pWeapon->Anim[(animCounts % 8 == 0) // Have direction
		? (static_cast<int>((Math::atan2(pBullet->Velocity.Y , pBullet->Velocity.X) / Math::TwoPi + 1.5) * animCounts - (animCounts / 8) + 0.5) % animCounts) // Calculate direction
		: ScenarioClass::Instance->Random.RandomRanged(0 , animCounts - 1)]; // Simple random;
/*
	const auto velocityRadian = Math::atan2(pBullet->Velocity.Y , pBullet->Velocity.X);
	const auto ratioOfRotateAngle = velocityRadian / Math::TwoPi;
	const auto correctRatioOfRotateAngle = ratioOfRotateAngle + 1.5; // Correct the Y-axis in reverse and ensure that the ratio is a positive number
	const auto animIndex = correctRatioOfRotateAngle * animCounts;
	const auto correctAnimIndex = animIndex - (animCounts / 8); // A multiple of 8 greater than 8 will have an additional offset
	const auto trueAnimIndex = static_cast<int>(correctAnimIndex + 0.5) % animCounts; // Round down and prevent exceeding the scope
*/

	if (!pAnimType)
		return;

	const auto pAnim = GameCreate<AnimClass>(pAnimType, pBullet->SourceCoords);

	AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, nullptr, false, true);
	AnimExt::Fetch(pAnim)->SetInvoker(pFirer, pHouse);

	if (pAttach)
	{
		if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pAttach))
			pAnim->ZAdjust = SetBuildingFireAnimZAdjust(pBuilding, pBullet->SourceCoords.Y);
		else
			pAnim->SetOwnerObject(pAttach);
	}
	else if (const auto pBuilding = abstract_cast<BuildingClass*>(pFirer))
	{
		pAnim->ZAdjust = SetBuildingFireAnimZAdjust(pBuilding, pBullet->SourceCoords.Y);
	}
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringReport(BulletClass* pBullet)
{
	const auto pWeapon = pBullet->WeaponType;

	if (pWeapon->Report.Count <= 0)
		return;

	const auto pFirer = pBullet->Owner;
	const auto reportIndex = pWeapon->Report[(pFirer ? pFirer->unknown_short_3C8 : ScenarioClass::Instance->Random.Random()) % pWeapon->Report.Count];
	VocClass::PlayAt(reportIndex, pBullet->Location, nullptr);
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringLaser(BulletClass* pBullet, HouseClass* pHouse)
{
	// Can not use 0x6FD210 because the firer may die
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon->IsLaser)
		return;

	const auto pWeaponExt = WeaponTypeExt::Fetch(pWeapon);

	LaserDrawClass* pLaser = nullptr;

	if (pWeapon->IsHouseColor || pWeaponExt->Laser_IsSingleColor)
	{
		const auto black = ColorStruct { 0, 0, 0 };
		pLaser = GameCreate<LaserDrawClass>(pBullet->SourceCoords, BulletExt::GetTargetCoordsForFiring(pBullet),
			((pWeapon->IsHouseColor && pHouse) ? pHouse->LaserColor : pWeapon->LaserInnerColor), black, black, pWeapon->LaserDuration);

		pLaser->IsHouseColor = true;
		pLaser->Thickness = pWeaponExt->LaserThickness;
		pLaser->IsSupported = (pLaser->Thickness > 3);
	}
	else
	{
		pLaser = GameCreate<LaserDrawClass>(pBullet->SourceCoords, BulletExt::GetTargetCoordsForFiring(pBullet),
			pWeapon->LaserInnerColor, pWeapon->LaserOuterColor, pWeapon->LaserOuterSpread, pWeapon->LaserDuration);

		pLaser->IsHouseColor = false;
		pLaser->Thickness = 3;
		pLaser->IsSupported = false;
	}

	// LaserPositionUpdate
	if (pLaser)
	{
		auto mode = pWeaponExt->LaserPositionUpdate;
		const bool isSplit = BulletExt::Fetch(pBullet)->IsSplitFromAirburst;

		if (isSplit)
		{
			if (mode == PositionFollow::Firer)
				mode = PositionFollow::None;
			else if (mode == PositionFollow::All)
				mode = PositionFollow::Target;
		}

		if (mode != PositionFollow::None)
		{
			auto const pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
			LaserRT::SetLaserTrackingData(pLaser, pBullet->Owner, pTarget, 0, mode, isSplit);
		}
	}
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringElectricBolt(BulletClass* pBullet)
{
	// Can not use 0x6FD460 because the firer may die
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon->IsElectricBolt)
		return;

	const auto pBolt = EBoltExt::CreateEBolt(pWeapon);
	pBolt->AlternateColor = pWeapon->IsAlternateColor;

	const auto targetCoords = BulletExt::GetTargetCoordsForFiring(pBullet);
	const auto pWeaponExt = WeaponTypeExt::Fetch(pWeapon);
	int zAdjust = pWeaponExt->EBoltZAdjust.Get(RulesExt::Global()->EBoltZAdjust);

	const auto pOwner = pBullet->Owner;
	if (pOwner && pOwner->WhatAmI() == AbstractType::Building)
	{
		const bool clamp = pWeaponExt->EBoltZAdjust_ClampInitialDepthForBuilding.Get(RulesExt::Global()->EBoltZAdjust_ClampInitialDepthForBuilding);
		if (clamp && zAdjust > 0)
			zAdjust = 0;
	}

	pBolt->Fire(pBullet->SourceCoords, targetCoords, zAdjust);

	if (const auto particle = WeaponTypeExt::Fetch(pWeapon)->Bolt_ParticleSystem.Get(RulesClass::Instance->DefaultSparkSystem))
		GameCreate<ParticleSystemClass>(particle, targetCoords, nullptr, nullptr, CoordStruct::Empty, nullptr);
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringRadBeam(BulletClass* pBullet, HouseClass* pHouse)
{
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon->IsRadBeam)
		return;

	const auto pWH = pWeapon->Warhead;
	const bool isTemporal = pWH && pWH->Temporal;
	const auto pRadBeam = RadBeam::Allocate(isTemporal ? RadBeamType::Temporal : RadBeamType::RadBeam);

	pRadBeam->SetCoordsSource(pBullet->SourceCoords);
	pRadBeam->SetCoordsTarget(BulletExt::GetTargetCoordsForFiring(pBullet));

	const auto pWeaponExt = WeaponTypeExt::Fetch(pWeapon);

	pRadBeam->Color = (pWeaponExt->Beam_IsHouseColor && pHouse) ? pHouse->LaserColor
		: pWeaponExt->Beam_Color.Get(isTemporal ? RulesClass::Instance->ChronoBeamColor : RulesClass::Instance->RadColor);

	pRadBeam->Period = pWeaponExt->Beam_Duration;
	pRadBeam->Amplitude = pWeaponExt->Beam_Amplitude;
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
inline void BulletExt::SimulatedFiringParticleSystem(BulletClass* pBullet, HouseClass* pHouse)
{
	if (const auto pPSType = pBullet->WeaponType->AttachedParticleSystem)
	{
		GameCreate<ParticleSystemClass>(pPSType, pBullet->SourceCoords, pBullet->Target, pBullet->Owner, BulletExt::GetTargetCoordsForFiring(pBullet), pHouse);
	}
}

// Make sure pBullet is not empty before call
void BulletExt::SimulatedFiringUnlimbo(BulletClass* pBullet, HouseClass* pHouse, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords, bool headToTarget, const RadialFireStruct& radialFire)
{
	// Initialize bullet characteristics such as weapon type, range, house etc.
	const auto pType = pBullet->Type;
	const int projectileRange = WeaponTypeExt::Fetch(pWeapon)->ProjectileRange.Get();
	auto velocity = BulletVelocity::Empty;
	pBullet->WeaponType = pWeapon;
	pBullet->Range = projectileRange;
	BulletExt::Fetch(pBullet)->FirerHouse = pHouse;

	if (pType->FirersPalette)
		pBullet->InheritedColor = pHouse->ColorSchemeIndex;

	// If someone asks me, I would say Arcing is just a piece of shit
	// But there are still people who like to use it, so anyway, it has been fixed
	if (pType->Arcing)
	{
		// The target must exist during launch
		const auto targetCoords = pBullet->Target->GetCenterCoords();
		const auto gravity = BulletTypeExt::GetAdjustedGravity(pType);
		const auto distanceCoords = targetCoords - sourceCoords;
		const auto horizontalDistance = Point2D { distanceCoords.X, distanceCoords.Y }.Magnitude();
		const bool lobber = pWeapon->Lobber || static_cast<int>(horizontalDistance) < distanceCoords.Z; // 0x70D590
		// The lower the horizontal velocity, the higher the trajectory
		// WW calculates the launch angle (and limits it) before calculating the velocity
		// Here, some magic numbers are used to directly simulate its calculation
		const auto speedMult = (lobber ? 0.45 : (distanceCoords.Z > 0 ? 0.68 : 1.0)); // Simulated 0x48A9D0
		const auto speed = speedMult * sqrt(horizontalDistance * gravity * 1.2); // 0x48AB90

		// Simulate firing Arcing bullet
		if (horizontalDistance < 1e-10 || speed < 1e-10)
		{
			// No solution
			velocity.Z = speed;
		}
		else
		{
			const auto mult = speed / horizontalDistance;
			velocity.X = static_cast<double>(distanceCoords.X) * mult;
			velocity.Y = static_cast<double>(distanceCoords.Y) * mult;
			velocity.Z = static_cast<double>(distanceCoords.Z) * mult + (gravity * horizontalDistance) / (2 * speed);
		}
	}
	else
	{
		const double speed = pBullet->Speed;

		if (headToTarget) // Home in on target.
		{
			const auto targetCoords = pBullet->Target->GetCenterCoords();
			const auto distanceCoords = targetCoords - sourceCoords;

			Vector3D<double> distanceVector {
				static_cast<double>(distanceCoords.X),
				static_cast<double>(distanceCoords.Y),
				static_cast<double>(distanceCoords.Z) };

			double len = distanceVector.Magnitude();

			if (len > 0.0)
			{
				distanceVector /= len;
				velocity = { distanceVector.X * speed, distanceVector.Y * speed, distanceVector.Z * speed };
			}
		}
		else // Drop down.
		{
			DirStruct dir;
			dir.SetValue<5>(ScenarioClass::Instance->Random.RandomRanged(0, 31));
			const auto cos_factor = -2.44921270764e-16; // cos(1.5 * Math::Pi * 1.00001)
			const auto flatSpeed = cos_factor * speed;
			const auto radians = dir.GetRadian<32>();
			velocity = { Math::cos(radians) * flatSpeed, Math::sin(radians) * flatSpeed, -speed };
		}
	}

	if (radialFire.Segments > 0)
		velocity = ApplyRadialFireVelocityWarp(velocity, radialFire);

	// Unlimbo
	pBullet->MoveTo(sourceCoords, velocity);
}

BulletVelocity BulletExt::ApplyRadialFireVelocityWarp(BulletVelocity velocity, const RadialFireStruct& radialFire)
{
	if (radialFire.Segments <= 0)
		return velocity;

	const double speedXY = std::hypot(velocity.X, velocity.Y);

	if (speedXY <= 0.0)
		return velocity;

	const double offset =
		(Math::Pi / radialFire.Segments)
		* radialFire.Index
		- Math::HalfPi;

	const double baseAngle = radialFire.Direction.GetRadian<32>();
	const double angle = baseAngle + offset;

	velocity.X = std::cos(angle) * speedXY;
	velocity.Y = -std::sin(angle) * speedXY;

	return velocity;
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
void BulletExt::SimulatedFiringEffects(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach, bool firingEffect, bool visualEffect)
{
	if (firingEffect)
	{
		BulletExt::SimulatedFiringAnim(pBullet, pHouse, pAttach);
		BulletExt::SimulatedFiringReport(pBullet);
	}

	if (visualEffect)
	{
		BulletExt::SimulatedFiringLaser(pBullet, pHouse);
		BulletExt::SimulatedFiringElectricBolt(pBullet);
		BulletExt::SimulatedFiringRadBeam(pBullet, pHouse);
		BulletExt::SimulatedFiringParticleSystem(pBullet, pHouse);
	}
}

CoordStruct BulletExt::GetTargetCoordsForFiring(BulletClass* pBullet)
{
	if (pBullet->Type->Inviso && pBullet->Type->FlakScatter)
		return pBullet->Location;
	else if (const auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target))
		return pTarget->GetTargetCoords();

	return pBullet->TargetCoords;
}

void BulletExt::ApplyArcingFix(BulletClass* pThis, const CoordStruct& sourceCoords, const CoordStruct& targetCoords, BulletVelocity& velocity)
{
	const auto distanceCoords = targetCoords - sourceCoords;
	const auto horizontalDistance = Point2D { distanceCoords.X, distanceCoords.Y }.Magnitude();
	const bool lobber = pThis->WeaponType->Lobber || static_cast<int>(horizontalDistance) < distanceCoords.Z; // 0x70D590
	// The lower the horizontal velocity, the higher the trajectory
	// WW calculates the launch angle (and limits it) before calculating the velocity
	// Here, some magic numbers are used to directly simulate its calculation
	const auto speedMult = (lobber ? 0.45 : (distanceCoords.Z > 0 ? 0.68 : 1.0)); // Simulated 0x48A9D0
	const auto gravity = BulletTypeExt::GetAdjustedGravity(pThis->Type);
	const auto speed = speedMult * sqrt(horizontalDistance * gravity * 1.2); // 0x48AB90

	if (horizontalDistance < 1e-10 || speed < 1e-10)
	{
		// No solution
		velocity.Z = speed;
	}
	else
	{
		const auto mult = speed / horizontalDistance;
		velocity.X = static_cast<double>(distanceCoords.X) * mult;
		velocity.Y = static_cast<double>(distanceCoords.Y) * mult;
		velocity.Z = (static_cast<double>(distanceCoords.Z) + velocity.Z) * mult + (gravity * horizontalDistance) / (2 * speed);
	}
}

// Detonate weapon/warhead using a bullet.
void BulletExt::Detonate(const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse, AbstractClass* pTarget, bool isBright, WeaponTypeClass* pWeapon, WarheadTypeClass* pWarhead)
{
	auto const pType = pWeapon ? pWeapon->Projectile : BulletTypeExt::GetDefaultBulletType();
	auto const pBullet = pType->CreateBullet(pTarget, pOwner, damage, pWarhead, 100, isBright);
	pBullet->WeaponType = pWeapon;

	auto const pBulletExt = BulletExt::Fetch(pBullet);
	pBulletExt->IsInstantDetonation = true;

	if (pFiringHouse)
		pBulletExt->FirerHouse = pFiringHouse;

	pBullet->SetLocation(coords);
	pBullet->Explode(true);
	pBullet->UnInit();
}

// =============================
// VectorRevibed：多实例并存调度（挂载 + 每帧 Step + 结果合并 + Next 列表分叉/移除）
// =============================
void BulletExt::VectorAI()
{
	BulletClass* pBullet = static_cast<BulletClass*>(this->OwnerObject());
	ObjectClass* pLauncher = pBullet ? pBullet->Owner : nullptr;

	LastResult = VectorRevibedResult();
	VectorStartPos = pBullet->GetCoords();

	// 挂载：一次性（VectorStarted 防止 Duration 结束后重挂）。
	// 仅弹体首次进入 AI 时从 BulletType 的 Vector_Types 初始化（Vector=Vector1,Vector2 并存）
	if (!VectorStarted && VectorList.empty() && TypeExtData && TypeExtData->Vector_Types.size())
	{
		VectorStarted = true;
		// 官方 ValueableIdxVector 遍历给 int 索引，用 Array[idx].get() 转指针
		for (auto idx : TypeExtData->Vector_Types)
		{
			auto pType = VectorTypeClass::Array[idx].get();
			if (!pType)
				continue;
			VectorBulletRuntime rt;
			rt.Type = pType;
			rt.NeedInit = true;
			VectorList.push_back(rt);
		}
	}
	else if (!VectorStarted && TypeExtData && !TypeExtData->Vector_Types.size())
	{
		// 该抛射体没有 Vector=：置标记避免每帧空查（防御）
		VectorStarted = true;
	}

	// 每帧：各自 Init/Step，结果并存合并
	// Next 分叉新段先收集到 pendingNext，循环结束后统一追加——
	// vector::insert/reallocation 会使 rt 引用与 it 失效，不能在循环内插
	std::vector<VectorBulletRuntime> pendingNext;
	auto it = VectorList.begin();
	while (it != VectorList.end())
	{
		auto& rt = *it;
		if (rt.NeedInit)
		{
			rt.State = VectorRevibedState();
			VectorRevibedAI_Init(rt.State, rt.Type->Data, pBullet, pLauncher, nullptr, rt.Type->Duration.Get());
			rt.NeedInit = false;
		}

		VectorRevibedResult r;
		VectorRevibedAI_Step(rt.State, rt.Type->Data, pBullet, pLauncher, nullptr, rt.Type->Duration.Get(), r);

		// 段真实帧计时：无条件推进（Step 内 Freeze/Disabled/TimeStep 跳帧分支不
		// AdvanceFrame，_elapsedFrames 不能当到期时钟）。Duration 到期据此强制结束。
		rt.State._lifeFrames++;

		// 并存位移求和（官方 YRpp 的 Vector3D 无 IsEmpty，用 == CoordStruct::Empty）
		if (LastResult.MoveDisp == CoordStruct::Empty)
			LastResult.MoveDisp = r.MoveDisp;
		else if (!(r.MoveDisp == CoordStruct::Empty))
			LastResult.MoveDisp += r.MoveDisp;
		LastResult.Force |= r.Force;
		LastResult.Freeze |= r.Freeze;
		LastResult.AllowRotateUnit |= r.AllowRotateUnit;
		if (LastResult.Freeze && LastResult.FrozenPos == CoordStruct::Empty)
			LastResult.FrozenPos = r.FrozenPos;

		// 瞬移：仅唯一活跃 Vector 时生效（并存时忽略，该 Vector 结束引擎接管）
		if (!(r.TeleportTo == CoordStruct::Empty) && VectorList.size() == 1)
			LastResult.TeleportTo = r.TeleportTo;

		// 结束：Deactivate 或 Duration 到期（段真实帧 _lifeFrames）→ Next 列表 / 移除
		// Freeze 期间 _elapsedFrames 不推进，Duration 判定必须用 _lifeFrames，
		// 否则 Freeze 段永不结束（Kratos 的到期时钟在 AE 层，移植后归此）
		bool ended = r.Deactivate
			|| (rt.Type->Duration.Get() >= 0 && rt.State._lifeFrames >= rt.Type->Duration.Get());

		if (ended)
		{
			// Next 全列表生效（多值分叉并存）：首个复用本槽，其余收集待追加；
			// 空/全无效 = 链尾移除
			std::vector<VectorTypeClass*> nexts;
			nexts.reserve(rt.Type->Next.size());
			for (auto idx : rt.Type->Next)
			{
				auto pCandidate = VectorTypeClass::Array[idx].get();
				if (pCandidate)
					nexts.push_back(pCandidate);
			}

			if (nexts.empty())
			{
				it = VectorList.erase(it);
			}
			else
			{
				rt.Type = nexts[0];
				rt.State = VectorRevibedState();
				rt.NeedInit = true;
				for (size_t i = 1; i < nexts.size(); ++i)
				{
					VectorBulletRuntime nrt;
					nrt.Type = nexts[i];
					nrt.NeedInit = true;
					pendingNext.push_back(nrt);
				}
				++it;
			}
		}
		else
		{
			++it;
		}
	}

	// 分叉新段统一挂入（下帧 Init + Step）
	if (!pendingNext.empty())
		VectorList.insert(VectorList.end(), pendingNext.begin(), pendingNext.end());
}

// =============================
// load / save

template <typename T>
void BulletExt::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeExtData)
		.Process(this->FirerHouse)
		.Process(this->CurrentStrength)
		.Process(this->InterceptorTechnoType)
		.Process(this->InterceptedStatus)
		.Process(this->DetonateOnInterception)
		.Process(this->LaserTrails)
		.Process(this->SnappedToTarget)
		.Process(this->DamageNumberOffset)
		.Process(this->ParabombFallRate)
		.Process(this->IsInstantDetonation)
		.Process(this->FirepowerMult)
		.Process(this->IsSplitFromAirburst)
		.Process(this->DistanceTraveled)

		.Process(this->VectorList)
		.Process(this->LastResult)
		.Process(this->VectorStartPos)
		.Process(this->VectorStarted)

		.Process(this->Trajectory) // Keep this shit at last
		;
}

void BulletExt::LoadFromStream(PhobosStreamReader& Stm)
{
	ObjectExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BulletExt::SaveToStream(PhobosStreamWriter& Stm)
{
	ObjectExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

BulletExt::ExtContainer::ExtContainer() : Container("BulletClass") { }

BulletExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);
	BulletExt::ExtMap.Remove(pItem);
	return 0;
}

