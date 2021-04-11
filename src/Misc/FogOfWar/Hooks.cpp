#include "FogOfWar.h"

// ;; loading
// 6B8E7A = ScenarioClass_LoadSpecialFlags, 6
// 686C03 = SetScenarioFlags_FogOfWar, 5

// ;; other bug fixes
// ;//457A10 = BuildingClass_IsFogged,5
// 5F4B3E = ObjectClass_DrawIfVisible, 6
// ;//6FA2B7 = TechnoClass_Update_DrawHidden_CheckFog, 6
// 6F5190 = TechnoClass_DrawExtras_CheckFog, 6
// ;//6924C0 = DisplayClass_ProcessClickCoords_SetFogged, 7
// ;//4D1C9B = FoggedObjectClass_DrawAll_SelectColorScheme, 6
// ;//4D2158 = FoggedObjectClass_DrawAll_SelectAnimPal, 6
// ;//4D129F = FoggedObjectClass_CTOR_Building_SetRecordAnimAdjust, 6
// ;//4D19A6 = FoggedObjectClass_DrawAll_DrawRecords, 6
// 48049E = CellClass_DrawTileAndSmudge_CheckFog, 6
// 6D6EDA = TacticalClass_Overlay_CheckFog1, A
// 6D70BC = TacticalClass_Overlay_CheckFog2, A
// 71CC8C = TerrainClass_DrawIfVisible, 6
// ;//4D1714 = FoggedObjectClass_DTOR, 6
// 5865E2 = IsLocationFogged, 5

/* Hook information from Xkein
;;optimize
;//4ACD5A = MapClass_TryFogCell_SetFlag, 7
;//6D871C = TacticalClass_GetOcclusion_Optimize, 8
;//47BD4A = CellClass_CTOR_InitMore, 6

;;network
;//4C800C = Networking_RespondToEvent_20, 5
*/

DEFINE_HOOK(6B8E7A, ScenarioClass_LoadSpecialFlags, 6)
{
	GET(ScenarioClass*, pScenario, ESI);

	pScenario->SpecialFlags.FogOfWar = 
		RulesClass::Instance->FogOfWar || R->EAX() || GameModeOptionsClass::Instance->FogOfWar;
	
	R->ECX(pScenario);
	return 0x6B8E8B;
}

DEFINE_HOOK(686C03, SetScenarioFlags_FogOfWar, 5)
{
	GET(ScenarioFlags, SFlags, EAX);

	SFlags.FogOfWar = RulesClass::Instance->FogOfWar || GameModeOptionsClass::Instance->FogOfWar;
	R->EDX(SFlags);

	return 0x686C0E;
}

DEFINE_HOOK(5F4B3E, ObjectClass_DrawIfVisible, 6)
{
	GET(ObjectClass*, pObject, ESI);

	/*if (pObject->InLimbo)
		return 0x5F4B7F;
	if(ScenarioClass::Instance->SpecialFlags.FogOfWar)
		return 0x5F4B48;*/

	UNREFERENCED_PARAMETER(pObject);

	return 0;
}

DEFINE_HOOK(6F5190, TechnoClass_DrawExtras_CheckFog, 6)
{
	GET(TechnoClass*, pTechno, ECX);

	auto coord = pTechno->GetCoords();

	return FogOfWar::IsLocationFogged(&coord) ? 0x6F5EEC : 0;
}

DEFINE_HOOK(48049E, CellClass_DrawTileAndSmudge_CheckFog, 6)
{
	GET(CellClass*, pCell, ESI);

	if (pCell->SmudgeTypeIndex == -1 || pCell->coord_4879B0())
		return 0x4804FB;
	return 0x4804A4;
}

DEFINE_HOOK(6D6EDA, TacticalClass_Overlay_CheckFog1, A)
{
	GET(CellClass*, pCell, EAX);

	if (pCell->OverlayTypeIndex == -1 || pCell->coord_4879B0())
		return 0x6D7006;
	return 0x6D6EE4;
}

DEFINE_HOOK(6D70BC, TacticalClass_Overlay_CheckFog2, A)
{
	GET(CellClass*, pCell, EAX);

	if (pCell->OverlayTypeIndex == -1 || pCell->coord_4879B0())
		return 0x6D71A4;
	return 0x6D70C6;
}

DEFINE_HOOK(71CC8C, TerrainClass_DrawIfVisible, 6)
{
	GET(TerrainClass*, pTerrain, EDI);

	auto coord = pTerrain->GetCoords();
	if (pTerrain->InLimbo || FogOfWar::IsLocationFogged(&coord))
		return 0x71CC9A;
	return 0x71CD8D;
}

DEFINE_HOOK(5865E2, IsLocationFogged, 5)
{
	GET_STACK(CoordStruct*, pCoord, 0x4);

	R->EAX(FogOfWar::IsLocationFogged(pCoord));
	
	return 0;
}