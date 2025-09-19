#include <Phobos.h>
#include <Misc/FogOfWar.h>

#include <WWMouseClass.h>
#include <TacticalClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>

// Producer-side fog gating for building placement
// No binary patches, just client-side preview validation

namespace BuildingPlacementFog {

	// Check if building placement is allowed at current mouse position
	bool PlacementAllowedHere(const BuildingTypeClass* pType, HouseClass* pHouse) {
		if (!pType || !pHouse)
			return true; // Let other validation handle null cases

		// Only apply fog restrictions to human players
		if (!pHouse->IsControlledByCurrentPlayer())
			return true; // AI can place anywhere

		// Only apply if fog of war is enabled
		if (!ScenarioClass::Instance->SpecialFlags.FogOfWar)
			return true; // No restrictions if fog disabled

		// Get mouse world coordinates
		if (!WWMouseClass::Instance || !TacticalClass::Instance)
			return true; // Safety check - allow if instances not ready

		const Point2D mouse = WWMouseClass::Instance->XY1;
		const CoordStruct world = TacticalClass::Instance->ClientToCoords(mouse);

		// Use the safe helper function
		return CanPlaceBuildingInFog(world, const_cast<BuildingTypeClass*>(pType));
	}

	// Check if specific coordinates allow building placement
	bool PlacementAllowedAt(const CoordStruct& coords, const BuildingTypeClass* pType, HouseClass* pHouse) {
		if (!pType || !pHouse)
			return true;

		// Only apply fog restrictions to human players
		if (!pHouse->IsControlledByCurrentPlayer())
			return true;

		// Only apply if fog of war is enabled
		if (!ScenarioClass::Instance->SpecialFlags.FogOfWar)
			return true;

		// Use the safe helper function
		return CanPlaceBuildingInFog(coords, const_cast<BuildingTypeClass*>(pType));
	}

	// Wrapper for AI placement that respects fog (if desired)
	bool AIPlacementAllowedAt(const CoordStruct& coords, const BuildingTypeClass* pType) {
		if (!pType)
			return true;

		// For AI, you might want different rules
		// Currently allows AI to place anywhere, but you can customize this
		return true;
	}

	// Check if deployment (MCV -> Construction Yard) is allowed
	bool DeploymentAllowedHere(HouseClass* pHouse) {
		if (!pHouse || !pHouse->IsControlledByCurrentPlayer())
			return true; // AI can deploy anywhere

		if (!ScenarioClass::Instance->SpecialFlags.FogOfWar)
			return true; // No restrictions if fog disabled

		// Get mouse world coordinates
		if (!WWMouseClass::Instance || !TacticalClass::Instance)
			return true;

		const Point2D mouse = WWMouseClass::Instance->XY1;
		const CoordStruct world = TacticalClass::Instance->ClientToCoords(mouse);

		// For deployment, just check the single cell
		return Fog::ShouldShowActiveAt(world);
	}
}

// Usage examples in your UI/sidebar code:
//
// In building placement preview logic:
//   if (!BuildingPlacementFog::PlacementAllowedHere(buildingType, currentPlayer)) {
//       // Show red preview, block placement
//       return false;
//   }
//
// In MCV deployment logic:
//   if (!BuildingPlacementFog::DeploymentAllowedHere(currentPlayer)) {
//       // Block deployment, show message
//       return false;
//   }
//
// For AI placement validation:
//   if (!BuildingPlacementFog::AIPlacementAllowedAt(coords, buildingType)) {
//       // AI finds different location
//       return false;
//   }