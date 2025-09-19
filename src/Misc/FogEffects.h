#pragma once

#include <CoordStruct.h>
#include <ColorScheme.h>

// Forward declarations
class LaserDrawClass;
class ParticleSystemTypeClass;
class AnimTypeClass;
class BuildingTypeClass;
class HouseClass;

// Producer-side fog effect wrappers
// Safe, crash-free alternatives to low-level draw hooks

// Weapon effects - hide active visuals in fog
namespace WeaponFog {
	bool SpawnLaserIfVisible(const CoordStruct& source, const CoordStruct& target,
		const ColorStruct& innerColor, const ColorStruct& outerColor,
		const ColorStruct& outerSpread, int duration);

	bool SpawnParticleSystemIfVisible(const CoordStruct& at, ParticleSystemTypeClass* pType);
	bool SpawnExplosionAnimIfVisible(const CoordStruct& at, AnimTypeClass* pType);
	bool SpawnElectricBoltIfVisible(const CoordStruct& source, const CoordStruct& target);
	bool SpawnWeaponTrailIfVisible(const CoordStruct& at, const CoordStruct& target);
}

// World text - hide floating text in fog
namespace WorldTextFog {
	void SpawnDamageTextIfVisible(int damage, const CoordStruct& at, ColorSchemeIndex color);
	void SpawnBountyTextIfVisible(int credits, const CoordStruct& at, HouseClass* pHouse);
	void SpawnCashDisplayTextIfVisible(int credits, const CoordStruct& at, HouseClass* pHouse);
	void SpawnWorldTextIfVisible(const wchar_t* text, const CoordStruct& at, ColorSchemeIndex color, HouseClass* pHouse = nullptr);
	void SpawnPromotionTextIfVisible(const wchar_t* rankText, const CoordStruct& at, HouseClass* pHouse);
}

// Building placement - block placement in fog
namespace BuildingPlacementFog {
	bool PlacementAllowedHere(const BuildingTypeClass* pType, HouseClass* pHouse);
	bool PlacementAllowedAt(const CoordStruct& coords, const BuildingTypeClass* pType, HouseClass* pHouse);
	bool AIPlacementAllowedAt(const CoordStruct& coords, const BuildingTypeClass* pType);
	bool DeploymentAllowedHere(HouseClass* pHouse);
}

// Usage Guide:
//
// 1. Include this header: #include <Misc/FogEffects.h>
//
// 2. Replace direct effect creation with fog-aware wrappers:
//    OLD: auto laser = GameCreate<LaserDrawClass>(source, target, colors...);
//    NEW: WeaponFog::SpawnLaserIfVisible(source, target, colors...);
//
// 3. Replace direct text creation with fog-aware wrappers:
//    OLD: FlyingStrings::AddMoneyString(text, at, color, house, true);
//    NEW: WorldTextFog::SpawnDamageTextIfVisible(damage, at, color);
//
// 4. Add placement validation in your UI code:
//    if (!BuildingPlacementFog::PlacementAllowedHere(type, house)) {
//        // Show red preview, block placement
//    }
//
// Benefits:
// - Zero crashes (no binary patches)
// - Engine-friendly (works with existing fog mechanics)
// - Easy to use (drop-in replacements)
// - Maintainable (clear separation of concerns)