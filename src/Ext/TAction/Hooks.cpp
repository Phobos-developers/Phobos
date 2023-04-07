#include "Body.h"

#include <Helpers\Macro.h>

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

// TODO: Sometimes Buildup anims plays while the building image is already there in faster gamespeed.
// Bugfix: TAction 125 Build At could neither display the buildups nor be AI-repairable in singleplayer mode
DEFINE_HOOK(0x6E427D, TActionClass_CreateBuildingAt, 0x9)
{
	GET(TActionClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBldType, ECX);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(CoordStruct, coord, STACK_OFFSET(0x24, -0x18));

	bool bPlayBuildUp = pThis->Param3 || pBldType->LoadBuildup();
	//Param3 can be used for other purposes in the future
	bool bCreated = false;
	if (auto pBld = static_cast<BuildingClass*>(pBldType->CreateObject(pHouse)))
	{
		if (bPlayBuildUp)
		{
			pBld->BeginMode(BStateType::Construction);
			pBld->QueueMission(Mission::Construction, false);
		}
		else
		{
			pBld->BeginMode(BStateType::Idle);
			pBld->QueueMission(Mission::Guard, false);
		}

		if (!pBld->ForceCreate(coord))
		{
			pBld->UnInit();
		}
		else
		{
			if(!bPlayBuildUp)
				pBld->Place(false);

			pBld->IsReadyToCommence = true;

			if (SessionClass::IsCampaign() && !pHouse->IsControlledByHuman())
				pBld->ShouldRebuild = pThis->Param4 > 0;

			bCreated = true;
		}
	}

	R->AL(bCreated);
	return 0x6E42C1;
}

// Bugfix, #issue 429: Retint map script disables RGB settings on light source
// Author: secsome
DEFINE_HOOK_AGAIN(0x6E2F47, TActionClass_Retint_LightSourceFix, 0x3) // Blue
DEFINE_HOOK_AGAIN(0x6E2EF7, TActionClass_Retint_LightSourceFix, 0x3) // Green
DEFINE_HOOK(0x6E2EA7, TActionClass_Retint_LightSourceFix, 0x3) // Red
{
	// Yeah, we just simply recreating these lightsource...
	// Stupid but works fine.

	for (auto pBld : *BuildingClass::Array)
	{
		if (pBld->LightSource)
		{
			GameDelete(pBld->LightSource);
			if (pBld->Type->LightIntensity)
			{
				TintStruct color { pBld->Type->LightRedTint, pBld->Type->LightGreenTint, pBld->Type->LightBlueTint };

				pBld->LightSource = GameCreate<LightSourceClass>(pBld->GetCoords(),
					pBld->Type->LightVisibility, pBld->Type->LightIntensity, color);

				pBld->LightSource->Activate();
			}
		}
	}

	for (auto pRadSite : *RadSiteClass::Array)
	{
		if (pRadSite->LightSource)
		{
			auto coord = pRadSite->LightSource->Location;
			auto color = pRadSite->LightSource->LightTint;
			auto intensity = pRadSite->LightSource->LightIntensity;
			auto visibility = pRadSite->LightSource->LightVisibility;

			GameDelete(pRadSite->LightSource);

			pRadSite->LightSource = GameCreate<LightSourceClass>(coord,
				visibility, intensity, color);

			pRadSite->LightSource->Activate();
		}
	}

	return 0;
}
