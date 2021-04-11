#include "FogOfWar.h"

// issue #28 : Fix vanilla YR Fog of War bugs & issues
// Reimplement it would be nicer.

// Make codes easier to read
#define OR(expr) ||(expr)
#define AND(expr) &&(expr)

void FogOfWar::Reveal_DisplayClass_All_To_Look_Ground(TechnoClass* pTechno, DWORD dwUnk, DWORD dwUnk2)
{
#define _LOOK_ \
	{ \
		auto coords = pTechno->GetCoords(); \
		pTechno->See(0, dwUnk2); \
		if (pTechno->IsInAir()) \
			MapClass::Global()->RevealArea3(&coords, \
				pTechno->LastSightRange - 3, pTechno->LastSightRange + 3, false); \
		return; \
	}

	if (!dwUnk || pTechno->WhatAmI() != AbstractType::Building)
	{
		if (pTechno->GetTechnoType()->RevealToAll)
			_LOOK_;
		if (pTechno->DiscoveredByPlayer)
		{
			if (SessionClass::Instance && pTechno->Owner == HouseClass::Player)
				_LOOK_;
			if (pTechno->Owner->CurrentPlayer || pTechno->Owner->PlayerControl)
				_LOOK_;
		}
		auto const pHouse = pTechno->Owner;
		auto const pPlayer = HouseClass::Player;
		if (pPlayer)
		{
			if (pPlayer == pHouse)
				_LOOK_;
			if (pPlayer->ArrayIndex == pHouse->ArrayIndex || pPlayer->ArrayIndex != -1 && pHouse->IsAlliedWith(pPlayer))
				_LOOK_;
		}
	}

#undef _LOOK_
}

/* Hook information from Xkein
;;loading
6B8E7A = ScenarioClass_LoadSpecialFlags, 6
686C03 = SetScenarioFlags_FogOfWar, 5

;process cell
;//4ACBC2 = MapClass_UpdateFogOnMap, 7
;//4A9D74 = MapClass_RevealFogShroud_RegisterCell, A
4ACE3C = MapClass_TryReshroudCell_SetCopyFlag, 6
4A9CA0 = MapClass_RevealFogShroud, 7
486BF0 = CellClass_CleanFog, 9
486A70 = CellClass_FogCell, 5
;//457AA0 = BuildingClass_FreezeInFog, 5
440B8D = BuildingClass_Put_CheckFog, 6
486C50 = CellClass_ClearFoggedObjects, 6

70076E = TechnoClass_GetCursorOverCell_OverFog, 5
6D3470 = TacticalClass_DrawFoggedObject, 8
51F97C = InfantryClass_MouseOverCell_OverFog, 5

;;optimize
;//4ACD5A = MapClass_TryFogCell_SetFlag, 7
;//6D871C = TacticalClass_GetOcclusion_Optimize, 8
;//47BD4A = CellClass_CTOR_InitMore, 6

;;network
;//4C800C = Networking_RespondToEvent_20, 5

;;other bug fixes
;//457A10 = BuildingClass_IsFogged,5
5F4B3E = ObjectClass_DrawIfVisible, 6
;//6FA2B7 = TechnoClass_Update_DrawHidden_CheckFog, 6
6F5190 = TechnoClass_DrawExtras_CheckFog, 6
;//6924C0 = DisplayClass_ProcessClickCoords_SetFogged, 7
;//4D1C9B = FoggedObjectClass_DrawAll_SelectColorScheme, 6
;//4D2158 = FoggedObjectClass_DrawAll_SelectAnimPal, 6
;//4D129F = FoggedObjectClass_CTOR_Building_SetRecordAnimAdjust, 6
;//4D19A6 = FoggedObjectClass_DrawAll_DrawRecords, 6
48049E = CellClass_DrawTileAndSmudge_CheckFog, 6
6D6EDA = TacticalClass_Overlay_CheckFog1, A
6D70BC = TacticalClass_Overlay_CheckFog2, A
71CC8C = TerrainClass_DrawIfVisible, 6
;//4D1714 = FoggedObjectClass_DTOR, 6
5865E2 = IsLocationFogged, 5
*/


