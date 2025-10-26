// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#pragma once
#include <BulletClass.h>

#include <Ext/BulletType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Entity/LaserTrailClass.h>
#include "Trajectories/PhobosTrajectory.h"

class BulletExt
{
public:
	using base_type = BulletClass;

	static constexpr DWORD Canary = 0x2A2A2A2A;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<BulletClass>
	{
	public:
		BulletTypeExt::ExtData* TypeExtData;
		HouseClass* FirerHouse;
		int CurrentStrength;
		TechnoTypeExt::ExtData* InterceptorTechnoType;
		InterceptedStatus InterceptedStatus;
		bool DetonateOnInterception;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		bool SnappedToTarget; // Used for custom trajectory projectile target snap checks
		int DamageNumberOffset;
		int ParabombFallRate;
		bool IsInstantDetonation;

		TrajectoryPointer Trajectory;

		ExtData(BulletClass* OwnerObject) : Extension<BulletClass>(OwnerObject)
			, TypeExtData { nullptr }
			, FirerHouse { nullptr }
			, CurrentStrength { 0 }
			, InterceptorTechnoType { nullptr }
			, InterceptedStatus { InterceptedStatus::None }
			, DetonateOnInterception { true }
			, LaserTrails {}
			, Trajectory { nullptr }
			, SnappedToTarget { false }
			, DamageNumberOffset { INT32_MIN }
			, ParabombFallRate { 0 }
			, IsInstantDetonation { false }
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void InterceptBullet(TechnoClass* pSource, BulletClass* pInterceptor);
		void ApplyRadiationToCell(CellStruct cell, int spread, int radLevel);
		void InitializeLaserTrails();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BulletExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void Detonate(const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse, AbstractClass* pTarget, bool isBright, WeaponTypeClass* pWeapon, WarheadTypeClass* pWarhead);
	static void ApplyArcingFix(BulletClass* pThis, const CoordStruct& sourceCoords, const CoordStruct& targetCoords, BulletVelocity& velocity);

	static void SimulatedFiringUnlimbo(BulletClass* pBullet, HouseClass* pHouse, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords, bool randomVelocity);
	static void SimulatedFiringEffects(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach, bool firingEffect, bool visualEffect);
	static inline void SimulatedFiringAnim(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach);
	static inline void SimulatedFiringReport(BulletClass* pBullet);
	static inline void SimulatedFiringLaser(BulletClass* pBullet, HouseClass* pHouse);
	static inline void SimulatedFiringElectricBolt(BulletClass* pBullet);
	static inline void SimulatedFiringRadBeam(BulletClass* pBullet, HouseClass* pHouse);
	static inline void SimulatedFiringParticleSystem(BulletClass* pBullet, HouseClass* pHouse);
};
