#include "RebuildTab3Subtypes.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/House/Body.h>
#include <HouseClass.h>
#include <EventClass.h>
#include <SidebarClass.h>

// Helper: issue a Produce event for a tracked subtype and focus sidebar to tab 3.
static void ExecuteRebuildSubtype(int typeIndex, AbstractType rtti, bool isNaval)
{
	auto const pPlayer = HouseClass::CurrentPlayer;
	if (!pPlayer)
		return;

	if (typeIndex < 0)
		return;

	if (SidebarClass::Instance.IsSidebarActive)
	{
		SidebarClass::Instance.ActiveTabIndex = 3;
		SidebarClass::Instance.SidebarNeedsRepaint();
	}

	EventClass::OutList.Add(EventClass(
		pPlayer->ArrayIndex,
		EventType::Produce,
		static_cast<int>(rtti),
		typeIndex,
		isNaval ? TRUE : FALSE
	));
}

// --- RebuildVehicle (non-naval UnitType) ---

const char* RebuildVehicleCommandClass::GetName() const
{
	return "RebuildVehicleOnly";
}

const wchar_t* RebuildVehicleCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("RebuildVehicleOnly", L"Rebuild Vehicle");
}

const wchar_t* RebuildVehicleCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* RebuildVehicleCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("RebuildVehicleOnly_Desc", L"Re-queue the last produced ground vehicle (non-naval).");
}

void RebuildVehicleCommandClass::Execute(WWKey eInput) const
{
	auto const pPlayer = HouseClass::CurrentPlayer;
	if (!pPlayer)
		return;

	auto const pExt = HouseExt::ExtMap.Find(pPlayer);
	if (!pExt)
		return;

	ExecuteRebuildSubtype(pExt->LastBuiltVehicleTypeIndex, pExt->LastBuiltVehicleRTTI, false);
}

// --- RebuildAircraft (AircraftType) ---

const char* RebuildAircraftCommandClass::GetName() const
{
	return "RebuildAircraft";
}

const wchar_t* RebuildAircraftCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("RebuildAircraft", L"Rebuild Aircraft");
}

const wchar_t* RebuildAircraftCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* RebuildAircraftCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("RebuildAircraft_Desc", L"Re-queue the last produced aircraft.");
}

void RebuildAircraftCommandClass::Execute(WWKey eInput) const
{
	auto const pPlayer = HouseClass::CurrentPlayer;
	if (!pPlayer)
		return;

	auto const pExt = HouseExt::ExtMap.Find(pPlayer);
	if (!pExt)
		return;

	ExecuteRebuildSubtype(pExt->LastBuiltAircraftTypeIndex, pExt->LastBuiltAircraftRTTI, false);
}

// --- RebuildNaval (naval UnitType) ---

const char* RebuildNavalCommandClass::GetName() const
{
	return "RebuildNaval";
}

const wchar_t* RebuildNavalCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("RebuildNaval", L"Rebuild Naval");
}

const wchar_t* RebuildNavalCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* RebuildNavalCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("RebuildNaval_Desc", L"Re-queue the last produced naval unit.");
}

void RebuildNavalCommandClass::Execute(WWKey eInput) const
{
	auto const pPlayer = HouseClass::CurrentPlayer;
	if (!pPlayer)
		return;

	auto const pExt = HouseExt::ExtMap.Find(pPlayer);
	if (!pExt)
		return;

	ExecuteRebuildSubtype(pExt->LastBuiltNavalTypeIndex, pExt->LastBuiltNavalRTTI, true);
}
