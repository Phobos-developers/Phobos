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
