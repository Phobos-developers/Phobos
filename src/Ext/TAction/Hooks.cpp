#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/Rules/Body.h>

#include <Helpers/Macro.h>

#include <HouseClass.h>
#include <BuildingClass.h>
#include <OverlayTypeClass.h>
#include <LightSourceClass.h>
#include <RadSiteClass.h>
#include <VocClass.h>
#include <ScenarioClass.h>
#include <ThemeClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x6DD8B0, TActionClass_Execute, 0x6)
{
	GET(TActionClass*, pThis, ECX);
	GET_STACK(HouseClass*, pHouse, 0x4);
	GET_STACK(ObjectClass*, pObject, 0x8);
	GET_STACK(TriggerClass*, pTrigger, 0xC);
	GET_STACK(CellStruct const*, pLocation, 0x10);

	bool handled;

	R->AL(TActionExt::Execute(pThis, pHouse, pObject, pTrigger, *pLocation, handled));

	return handled ? 0x6DD910 : 0;
}

// Bugfix: TAction 125 Build At could neither display the buildups nor be AI-repairable in singleplayer mode
// Sep 9, 2025 - Starkku: Fixed issues with buildups potentially ending up in infinite loops etc.
// A separate issue remains where buildup sequence will interrupt if building's house changes mid-buildup,
// but this applies to all buildings and not just ones created through the trigger.
// Also restored Param3 to control the buildup display, only this time it is inverted (set to >0 to disable buildups).
DEFINE_HOOK(0x6E427D, TActionClass_CreateBuildingAt, 0x9)
{
	GET(TActionClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBuildingType, ECX);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(CoordStruct, coord, STACK_OFFSET(0x24, -0x18));

	const bool playBuildup = pBuildingType->LoadBuildup();
	bool created = false;

	if (auto pBuilding = static_cast<BuildingClass*>(pBuildingType->CreateObject(pHouse)))
	{
		// Set before unlimbo cause otherwise it will call BuildingClass::Place.
		pBuilding->QueueMission(Mission::Construction, false);
		pBuilding->NextMission();

		if (!pBuilding->ForceCreate(coord))
		{
			pBuilding->UnInit();
		}
		else
		{
			// Reset mission and build state if we're not going to play buildup afterwards.
			if (!playBuildup)
			{
				pBuilding->BeginMode(BStateType::Idle);
				pBuilding->QueueMission(Mission::Guard, false);
				pBuilding->NextMission();
				pBuilding->Place(false); // Manually call this now.
			}

			if (SessionClass::IsCampaign() && !pHouse->IsControlledByHuman())
				pBuilding->ShouldRebuild = pThis->Param4 > 0;

			created = true;
		}
	}

	R->AL(created);
	return 0x6E42C1;
}

#pragma region RetintFix

namespace RetintTemp
{
	bool UpdateLightSources = false;
}

// Bugfix, #issue 429: Retint map script disables RGB settings on light source
// Author: secsome, Starkku
DEFINE_HOOK_AGAIN(0x6E2F47, TActionClass_Retint_LightSourceFix, 0x3) // Blue
DEFINE_HOOK_AGAIN(0x6E2EF7, TActionClass_Retint_LightSourceFix, 0x3) // Green
DEFINE_HOOK(0x6E2EA7, TActionClass_Retint_LightSourceFix, 0x3) // Red
{
	// Flag the light sources to update, actually do it later and only once to prevent redundancy.
	RetintTemp::UpdateLightSources = RulesExt::Global()->UseRetintFix;

	return 0;
}

// Update light sources if they have been flagged to be updated.
DEFINE_HOOK(0x6D4455, Tactical_Render_UpdateLightSources, 0x8)
{
	if (RetintTemp::UpdateLightSources)
	{
		for (auto const lSource : LightSourceClass::Array)
		{
			if (lSource->Activated)
			{
				lSource->Activated = false;
				lSource->Activate();
			}
		}

		RetintTemp::UpdateLightSources = false;
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x6E2368, TActionClass_PlayAnimAt, 0x7)
{
	enum { SkipGameCode = 0x6E236F };

	GET(TActionClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	GET_STACK(HouseClass*, pHouse, STACK_OFFSET(0x18, 0x4));

	AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, nullptr, false, true);
	pAnim->IsInert = !pThis->Param3;

	return SkipGameCode;
}

DEFINE_HOOK(0x6DD791, TActionClass_ReadINI_MaskedTActions, 0xB)
{
	GET(TActionClass*, pThis, EBP);

	switch (static_cast<PhobosTriggerAction>(pThis->ActionKind))
	{
	case PhobosTriggerAction::WinByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::Win;
		break;
	case PhobosTriggerAction::LoseByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::Lose;
		break;
	case PhobosTriggerAction::ProductionBeginsByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::ProductionBegins;
		break;
	case PhobosTriggerAction::AllToHuntByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::AllToHunt;
		break;
	case PhobosTriggerAction::FireSaleByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::FireSale;
		break;
		// Action 10 "Play movie", not possible yet because YRpp doesn't have code so no 19010 :-(
	case PhobosTriggerAction::AutocreateBeginsByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::AutocreateBegins;
		break;
	case PhobosTriggerAction::ChangeHouseByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::ChangeHouse;
		break;
	case PhobosTriggerAction::PlayMusicThemeByID:
		pThis->Value = ThemeClass::Instance.FindIndex(pThis->Text);
		pThis->ActionKind = TriggerAction::PlayMusicTheme;
		break;
	case PhobosTriggerAction::AddOneTimeSuperWeaponByID:
		pThis->Value = SuperWeaponTypeClass::FindIndex(pThis->Text);
		pThis->ActionKind = TriggerAction::AddOneTimeSuperWeapon;
		break;
	case PhobosTriggerAction::AddRepeatingSuperWeaponByID:
		pThis->Value = SuperWeaponTypeClass::FindIndex(pThis->Text);
		pThis->ActionKind = TriggerAction::AddRepeatingSuperWeapon;
		break;
	case PhobosTriggerAction::AllChangeHouseByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::AllChangeHouse;
		break;
	case PhobosTriggerAction::MakeAllyByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::MakeAlly;
		break;
	case PhobosTriggerAction::MakeEnemyByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::MakeEnemy;
		break;
	case PhobosTriggerAction::PlayAnimAtByID:
		pThis->Value = AnimTypeClass::FindIndex(pThis->Text);
		pThis->ActionKind = TriggerAction::PlayAnimAt;
		break;
	case PhobosTriggerAction::DoExplosionAtByID:
		pThis->Value = WeaponTypeClass::FindIndex(pThis->Text);
		pThis->ActionKind = TriggerAction::DoExplosionAt;
		break;
	case PhobosTriggerAction::CreateVoxelAnimByID:
		pThis->Value = VoxelAnimTypeClass::FindIndex(pThis->Text);
		pThis->ActionKind = TriggerAction::CreateVoxelAnim;
		break;
	case PhobosTriggerAction::AITriggersBeginByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::AITriggersBegin;
		break;
	case PhobosTriggerAction::AITriggersStopByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::AITriggersStop;
		break;
	case PhobosTriggerAction::ParticleAnimByID:
		pThis->Value = ParticleSystemTypeClass::FindIndex(pThis->Text);
		pThis->ActionKind = TriggerAction::ParticleAnim;
		break;
	case PhobosTriggerAction::MakeHouseCheerByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::MakeHouseCheer;
		break;
	case PhobosTriggerAction::DestroyAllByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::DestroyAll;
		break;
	case PhobosTriggerAction::DestroyAllBuildingsByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::DestroyAllBuildings;
		break;
	case PhobosTriggerAction::DestroyAllLandUnitsByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::DestroyAllLandUnits;
		break;
	case PhobosTriggerAction::DestroyAllNavalUnitsByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::DestroyAllNavalUnits;
		break;
	case PhobosTriggerAction::MindControlBaseByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::MindControlBase;
		break;
	case PhobosTriggerAction::RestoreMindControlledBaseByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::RestoreMindControlledBase;
		break;
	case PhobosTriggerAction::RestoreStartingUnitsByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::RestoreStartingUnits;
		break;
	case PhobosTriggerAction::RestoreStartingBuildingsByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->Text);
		pThis->ActionKind = TriggerAction::RestoreStartingBuildings;
		break;
		
	default:
		break;
	}

	return 0;
}
