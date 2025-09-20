#pragma once

#include <GeneralDefinitions.h>
#include <BuildingTypeClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <TacticalClass.h>

// Central fog of war utility functions - safe, producer-side approach

// Active = must be visible now; Passive = visible once revealed, hidden only by shroud.
namespace Fog {
	inline bool IsFogged(const CoordStruct& c) {
		auto cs = CellClass::Coord2Cell(c);
		auto* p = (&MapClass::Instance) ? MapClass::Instance.TryGetCellAt(cs) : nullptr;
		CellStruct se{ short(cs.X + 1), short(cs.Y + 1) };
		auto* pse = (&MapClass::Instance) ? MapClass::Instance.TryGetCellAt(se) : nullptr;
		const bool e1 = p && (p->Flags & CellFlags::EdgeRevealed);
		const bool e2 = pse && (pse->Flags & CellFlags::EdgeRevealed);
		return !(e1 || e2);
	}

	inline bool IsShrouded(const CoordStruct& c) {
		// Same test as above — engine treats 'edge revealed' as "ever seen".
		return IsFogged(c);
	}

	// Policy helpers
	inline bool ShouldShowActiveAt(const CoordStruct& c) { return !IsFogged(c); }
	inline bool ShouldShowPassiveAt(const CoordStruct& c) { return !IsShrouded(c); }
}

// Helper for creation-time particle gates
namespace FoW {
	inline bool EnemyTechnoUnderFog(AbstractClass* owner) {
		if (!(ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar)) return false;
		if (!HouseClass::CurrentPlayer || !owner) return false;

		TechnoClass* t = abstract_cast<TechnoClass*>(owner);
		if (!t) { if (auto* b = abstract_cast<BuildingClass*>(owner)) t = b; }
		if (!t || !t->Owner) return false;
		if (HouseClass::CurrentPlayer->IsAlliedWith(t->Owner)) return false;

		const auto cs = CellClass::Coord2Cell(t->GetCoords());
		const auto* cell = MapClass::Instance.TryGetCellAt(cs);
		return !cell || !(cell->Flags & CellFlags::EdgeRevealed);
	}
}

// Check if building placement should be allowed at location (respects fog like shroud)
bool CanPlaceBuildingInFog(const CoordStruct& coords, BuildingTypeClass* pBuildingType);