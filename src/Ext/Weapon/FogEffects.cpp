#include <Phobos.h>
#include <Misc/FogOfWar.h>

#include <LaserDrawClass.h>
#include <ParticleSystemClass.h>
#include <AnimClass.h>

// Producer-side fog gating for weapon effects
// No binary patches, no crashes - just don't create effects in fog

namespace WeaponFog {

	// Lasers - hide if either endpoint is fogged
	bool SpawnLaserIfVisible(const CoordStruct& source, const CoordStruct& target,
		const ColorStruct& innerColor, const ColorStruct& outerColor,
		const ColorStruct& outerSpread, int duration) {

		if (!Fog::ShouldShowActiveAt(source) || !Fog::ShouldShowActiveAt(target))
			return false; // Don't spawn laser in fog

		// Create the laser using existing game mechanics
		auto pLaser = GameCreate<LaserDrawClass>(source, target, innerColor, outerColor, outerSpread, duration);
		return pLaser != nullptr;
	}

	// Particles/sparks - hide if spawn location is fogged
	bool SpawnParticleSystemIfVisible(const CoordStruct& at, ParticleSystemTypeClass* pType) {
		if (!pType || !Fog::ShouldShowActiveAt(at))
			return false;

		// Create particle system at location
		auto pSystem = GameCreate<ParticleSystemClass>(pType, at, nullptr, nullptr, CoordStruct::Empty, nullptr);
		return pSystem != nullptr;
	}

	// Explosion animations - hide if explosion location is fogged
	bool SpawnExplosionAnimIfVisible(const CoordStruct& at, AnimTypeClass* pType) {
		if (!pType || !Fog::ShouldShowActiveAt(at))
			return false;

		// Create explosion animation
		auto pAnim = GameCreate<AnimClass>(pType, at);
		return pAnim != nullptr;
	}

	// Electric bolts/arcs - hide if either endpoint is fogged
	bool SpawnElectricBoltIfVisible(const CoordStruct& source, const CoordStruct& target) {
		if (!Fog::ShouldShowActiveAt(source) || !Fog::ShouldShowActiveAt(target))
			return false;

		// Your electric bolt creation logic here
		// This is a placeholder - replace with actual implementation
		return true;
	}

	// Weapon trail effects - hide if weapon location is fogged
	bool SpawnWeaponTrailIfVisible(const CoordStruct& at, const CoordStruct& target) {
		if (!Fog::ShouldShowActiveAt(at) || !Fog::ShouldShowActiveAt(target))
			return false;

		// Your weapon trail creation logic here
		// This is a placeholder - replace with actual implementation
		return true;
	}
}

// Usage examples:
// Replace direct laser creation:
//   auto laser = GameCreate<LaserDrawClass>(source, target, colors...);
// With:
//   WeaponFog::SpawnLaserIfVisible(source, target, colors...);
//
// Replace direct particle creation:
//   auto particles = GameCreate<ParticleSystemClass>(type, at, ...);
// With:
//   WeaponFog::SpawnParticleSystemIfVisible(at, type);