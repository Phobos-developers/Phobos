#include "Body.h"

#include <queue>
#include <sstream>

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

// Bugfix: TAction 125 Build At do not display the buildups
// Author: secsome
DEFINE_HOOK(0x6E427D, TActionClass_CreateBuildingAt, 0x9)
{
	GET(TActionClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBldType, ECX);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(CoordStruct, coord, STACK_OFFS(0x24, 0x18));

	bool bPlayBuildUp = pThis->Param3;

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


namespace ActionsString
{
	std::string ActionsString;
	std::deque<std::string> SubStrings;
}

DEFINE_HOOK(0x727544, TriggerClass_LoadFromINI_Actions, 0x5)
{
	GET(const char*, pString, EDX);
	ActionsString::ActionsString = pString;
	std::stringstream sin(ActionsString::ActionsString);
	std::deque<std::string>& substrs = ActionsString::SubStrings;
	substrs.clear();
	std::string tmp;
	while (std::getline(sin, tmp, ','))
	{
		substrs.emplace_back(tmp);
	}
	if (!ActionsString::SubStrings.empty())
		ActionsString::SubStrings.pop_front();
	return 0;
}

DEFINE_HOOK(0x6DD5B0, TActionClass_LoadFromINI_Parm, 0x5)
{
	GET(TActionClass*, pThis, ECX);
	auto pExt = TActionExt::ExtMap.Find(pThis);
	std::deque<std::string>& substrs = ActionsString::SubStrings;

	if (substrs.empty())
		return 0;

	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Value1 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Value2 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Parm3 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Parm4 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Parm5 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Parm6 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	substrs.pop_front();

	return 0;
}
