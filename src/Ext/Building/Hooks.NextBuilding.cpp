#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

#include <SidebarClass.h>
#include <InfantryClass.h> // The easiest way to get BuildingClass pass compile
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <MouseClass.h>

// #include <windowsx.h> // for mouse related macros

// vvv FactoryBuildingMe vvv
DEFINE_HOOK(0x4C9DA2, FactoryClass_Set_BuildingSetFactoryBuildingMe, 0x5)
{
	GET(FactoryClass*, pThis, ESI);
	GET(ObjectClass*, pObject, EAX);

	if (pObject->WhatAmI() == AbstractType::Building)
	{
		const auto pBld = reinterpret_cast<BuildingClass*>(pObject);
		const auto pExt = BuildingExt::ExtMap.Find(pBld);
		pExt->FactoryBuildingMe = pThis;
	}

	return 0;
}

DEFINE_HOOK(0x4CA004, FactoryClass_Abandon_BuildingUnsetFactoryBuildingMe, 0x9)
{
	GET(ObjectClass*, pObject, ECX);

	if (pObject->WhatAmI() == AbstractType::Building)
	{
		const auto pBld = reinterpret_cast<BuildingClass*>(pObject);
		const auto pExt = BuildingExt::ExtMap.Find(pBld);
		pExt->FactoryBuildingMe = nullptr;
	}

	return 0;
}
// ^^^ FactoryBuildingMe ^^^

static bool HandleBuildingLink(bool bUp)
{
	const auto pBldType = SidebarClass::Instance->CurrentBuildingType;
	if (!pBldType)
		return false;

	auto pBld = reinterpret_cast<BuildingClass*>(SidebarClass::Instance->CurrentBuilding);
	if (!pBld)
		return false;

	auto pBldExt = BuildingExt::ExtMap.Find(pBld);
	const auto pBldTypeExt = pBldExt->TypeExtData;

	const auto pNextBldType = bUp ? pBldTypeExt->NextBuilding_Prev : pBldTypeExt->NextBuilding_Next;
	if (!pNextBldType)
		return false;

	// destruct the old building
	const auto pFactory = BuildingExt::ExtMap.Find(pBld)->FactoryBuildingMe;
	reference<BuildingClass*, 0xB0FE5C> SidebarBuildingTabObject;
	const bool bNeedSetSidebar = SidebarBuildingTabObject == pBld;
	GameDelete(pBld);

	// create the new building, remember to set the FactoryBuildingMe in Ext!
	pBld = reinterpret_cast<BuildingClass*>(pNextBldType->CreateObject(pFactory->Owner));
	pFactory->Object = pBld;
	pBldExt = BuildingExt::ExtMap.Find(pBld);
	pBldExt->FactoryBuildingMe = pFactory;
	SidebarClass::Instance->CurrentBuilding = pBld;
	SidebarClass::Instance->CurrentBuildingType = pNextBldType;
	SidebarClass::Instance->SetActiveFoundation(pBld->GetFoundationData(true));
	if (bNeedSetSidebar)
		SidebarBuildingTabObject = pBld;
	pBldTypeExt->NextBuilding_CurrentHeapId = pNextBldType->ArrayIndex;

	return true;
}

DEFINE_HOOK(0x777985, Main_Window_Proc_OnMouseWheel, 0x6)
{
	// Singlethread, no need to worry about it, westwood!

	// GET_STACK(HWND, hWnd, STACK_OFFSET(0x10, 0x4));
	// GET_STACK(UINT, uMsg, STACK_OFFSET(0x10, 0x8)); WM_MOUSEWHEEL
	GET_STACK(WPARAM, wParam, STACK_OFFSET(0x10, 0xC));
	// GET_STACK(LPARAM, lParam, STACK_OFFSET(0x10, 0x10));

	const auto fwKeys = GET_KEYSTATE_WPARAM(wParam);
	const auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	// const auto xPos = GET_X_LPARAM(lParam);
	// const auto yPos = GET_Y_LPARAM(lParam);

	// Check if there's pending building placement
	// If there's pending building placement and it have a linked building, then handle it
	const bool bUp = zDelta > 0;

	if (!HandleBuildingLink(bUp))
		SidebarClass::Instance->Scroll(bUp, -1);

	return 0x7779B5;
}


DEFINE_HOOK(0x4FB8E4, Manual_Place_ResetBuildingTypeCurrentHeapId, 0x6)
{
	if (const auto pType = reinterpret_cast<BuildingTypeClass*>(SidebarClass::Instance->CurrentBuildingType))
	{
		const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		pTypeExt->NextBuilding_CurrentHeapId = pType->ArrayIndex;
	}
	return 0;
}

DEFINE_HOOK(0x679B9B, RulesClass_Objects_BuildingTypesAfterReadFromINI, 0x0)
{
	// use a dynamic bitset(vector<bool>) to record the processed building types
	std::vector<bool> aBuildingConnected;
	aBuildingConnected.resize(BuildingTypeClass::Array->Count, false);

	// We require the INIer write the NextBuilding.Next correctly at least, so we can fix the NextBuilding.Prev for them
	for (auto pBldType : *BuildingTypeClass::Array)
	{
		const auto pExt = BuildingTypeExt::ExtMap.Find(pBldType);
		if (pExt->NextBuilding_Next)
		{
			const auto pNextExt = BuildingTypeExt::ExtMap.Find(pExt->NextBuilding_Next);
			if (pNextExt->NextBuilding_Prev == nullptr)
				pNextExt->NextBuilding_Prev = pBldType;
		}
		else
			aBuildingConnected[pBldType->ArrayIndex] = true;
	}

	// Pre connect those already connected ones
	for (auto pBldType : *BuildingTypeClass::Array)
	{
		if (aBuildingConnected[pBldType->ArrayIndex])
			continue;

		const auto pExt = BuildingTypeExt::ExtMap.Find(pBldType);
		auto pNextBldType = pExt->NextBuilding_Next;
		while (true)
		{
			if (pNextBldType == nullptr)
				break;
			if (pNextBldType == pBldType)
			{
				aBuildingConnected[pBldType->ArrayIndex] = true;
				break;
			}

			aBuildingConnected[pNextBldType->ArrayIndex] = true;
			const auto pNextExt = BuildingTypeExt::ExtMap.Find(pNextBldType);
			pNextBldType = pNextExt->NextBuilding_Next;
		}
	}

	// Check whether there's unmatched NextBuilding_Prev and NextBuilding_Next
	for (auto pBldType : *BuildingTypeClass::Array)
	{
		const auto pExt = BuildingTypeExt::ExtMap.Find(pBldType);
		if (const auto pNextBldType = pExt->NextBuilding_Next)
		{
			const auto pNextExt = BuildingTypeExt::ExtMap.Find(pNextBldType);
			if (pNextExt->NextBuilding_Prev != pBldType)
				Debug::FatalErrorAndExit(Debug::ExitCode::BadINIUsage, "Invalid NextBuilding.Prev for %s and %s", pBldType->ID, pNextBldType->ID);
		}
	}

	// Now fix up the loop automatically
	for (auto pBldType : *BuildingTypeClass::Array)
	{
		if (aBuildingConnected[pBldType->ArrayIndex])
			continue;

		aBuildingConnected[pBldType->ArrayIndex] = true;

		auto pHeadBldType = pBldType;
		while (true)
		{
			const auto pExt = BuildingTypeExt::ExtMap.Find(pHeadBldType);
			if (!pExt->NextBuilding_Prev)
				break;
			pHeadBldType = pExt->NextBuilding_Prev;
			aBuildingConnected[pHeadBldType->ArrayIndex] = true;
		}

		auto pTailBldType = pBldType;
		while (true)
		{
			const auto pExt = BuildingTypeExt::ExtMap.Find(pTailBldType);
			if (!pExt->NextBuilding_Next)
				break;
			pTailBldType = pExt->NextBuilding_Next;
			aBuildingConnected[pTailBldType->ArrayIndex] = true;
		}

		const auto pHeadExt = BuildingTypeExt::ExtMap.Find(pHeadBldType);
		const auto pTailExt = BuildingTypeExt::ExtMap.Find(pTailBldType);
		pHeadExt->NextBuilding_Prev = pTailBldType;
		pTailExt->NextBuilding_Next = pHeadBldType;
	}

	return 0x679BBE;
}

DEFINE_HOOK(0x6AAFD0, SidebarClass_StripClass_SelectClass_Action_Cancelled_NextBuilding_E_ABANDON, 0x5)
{
	GET(unsigned int, heapid, ECX);
	GET(AbstractType, rttiid, EBP);

	if (rttiid == AbstractType::BuildingType)
	{
		const auto pBldType = BuildingTypeClass::Array->GetItem(heapid);
		if (const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBldType))
			R->ECX(pTypeExt->NextBuilding_CurrentHeapId);
	}

	return 0;
}

DEFINE_HOOK(0x6AAE5C, SidebarClass_StripClass_SelectClass_Action_Cancelled_NextBuilding_E_ABANDONALL, 0x5)
{
	GET(unsigned int, heapid, EDX);
	GET(AbstractType, rttiid, EBP);

	if (rttiid == AbstractType::BuildingType)
	{
		const auto pBldType = BuildingTypeClass::Array->GetItem(heapid);
		if (const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBldType))
			R->EDX(pTypeExt->NextBuilding_CurrentHeapId);
	}

	return 0;
}
