#include "Body.h"

#include <Helpers\Macro.h>

#include <HouseClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <OverlayTypeClass.h>
#include <VocClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x71E940, TEventClass_Execute, 0x5)
{
	GET(TEventClass*, pThis, ECX);
	GET_STACK(const int, iEvent, 0x4); // now trigger what?
	GET_STACK(HouseClass*, pHouse, 0x8);
	GET_STACK(ObjectClass*, pObject, 0xC);
	GET_STACK(CDTimerClass*, pTimer, 0x10);
	GET_STACK(bool*, isPersitant, 0x14);
	GET_STACK(TechnoClass*, pSource, 0x18);

	const auto result = TEventExt::Execute(pThis, iEvent, pHouse, pObject, pTimer, isPersitant, pSource);

	if (!result.has_value())
		return 0;

	R->AL(result.value());
	return 0x71EA2D;
}

DEFINE_HOOK(0x7271F9, TEventClass_GetFlags, 0x5)
{
	GET(int, eAttach, EAX);
	GET(TEventClass*, pThis, ESI);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= PhobosTriggerEvent::LocalVariableGreaterThan && nEvent < PhobosTriggerEvent::_DummyMaximum)
	{
		eAttach |= TEventExt::GetFlags(nEvent);
	}

	R->EAX(eAttach);

	return 0;
}

DEFINE_HOOK(0x71F3FE, TEventClass_BuildINIEntry, 0x5)
{
	GET(int, eNeedType, EAX);
	GET(TEventClass*, pThis, ECX);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= PhobosTriggerEvent::LocalVariableGreaterThan && nEvent < PhobosTriggerEvent::_DummyMaximum)
		eNeedType = 43;

	R->EAX(eNeedType);

	return 0;
}

DEFINE_HOOK(0x726577, TEventClass_Persistable, 0x7)
{
	GET(TEventClass*, pThis, EDI);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= PhobosTriggerEvent::LocalVariableGreaterThan && nEvent < PhobosTriggerEvent::_DummyMaximum)
		R->AL(true);
	else
		R->AL(pThis->GetStateB());

	return 0x72657E;
}

DEFINE_HOOK_AGAIN(0x71ED5E, TriggerClass_SpyAsInfantryOrHouse, 0x8)		// SpyAsHouse
DEFINE_HOOK(0x71ECE1, TriggerClass_SpyAsInfantryOrHouse, 0x8)			// SpyAsInfantry
{
	GET(const int, iEvent, ESI);

	// This might form a unique condition that specifies the Country and InfantryType.
	if (iEvent == 53 || iEvent == 54)
		return R->Origin() + 0x8;

	return 0x71F163;
}

DEFINE_HOOK(0x71F58B, TEventClass_ReadINI_MaskedTEvents, 0x7)
{
	REF_STACK(TEventClass*, pThis, 0x4);

	switch (static_cast<int>(pThis->EventKind))
	{
	case PhobosTriggerEvent::EnteredByByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::EnteredBy;
		break;
	case PhobosTriggerEvent::SpiedByByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::SpiedBy;
		break;
	case PhobosTriggerEvent::HouseDiscoveredByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::HouseDiscovered;
		break;
	case PhobosTriggerEvent::DestroyedUnitsAllByID:
		pThis->Value = UnitTypeClass::FindIndex(pThis->String);
		pThis->EventKind = TriggerEvent::DestroyedUnitsAll;
		break;
	case PhobosTriggerEvent::DestroyedBuildingsAllByID:
		pThis->Value = BuildingTypeClass::FindIndex(pThis->String);
		pThis->EventKind = TriggerEvent::DestroyedBuildingsAll;
		break;
	case PhobosTriggerEvent::DestroyedAllByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::DestroyedAll;
		break;
	case PhobosTriggerEvent::BuildBuildingTypeByID:
		pThis->Value = BuildingTypeClass::FindIndex(pThis->String);
		pThis->EventKind = TriggerEvent::BuildBuildingType;
		break;
	case PhobosTriggerEvent::BuildUnitTypeByID:
		pThis->Value = UnitTypeClass::FindIndex(pThis->String);
		pThis->EventKind = TriggerEvent::BuildUnitType;
		break;
	case PhobosTriggerEvent::BuildInfantryTypeByID:
		pThis->Value = InfantryTypeClass::FindIndex(pThis->String);
		pThis->EventKind = TriggerEvent::BuildInfantryType;
		break;
	case PhobosTriggerEvent::BuildAircraftTypeByID:
		pThis->Value = AircraftTypeClass::FindIndex(pThis->String);
		pThis->EventKind = TriggerEvent::BuildAircraftType;
		break;
	case PhobosTriggerEvent::ZoneEntryByByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::ZoneEntryBy;
		break;
	case PhobosTriggerEvent::CrossesHorizontalLineByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::CrossesHorizontalLine;
		break;
	case PhobosTriggerEvent::CrossesVerticalLineByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::CrossesVerticalLine;
		break;
	case PhobosTriggerEvent::LowPowerByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::LowPower;
		break;
	case PhobosTriggerEvent::BuildingExistsByID:
		pThis->Value = BuildingTypeClass::FindIndex(pThis->String);
		pThis->EventKind = TriggerEvent::BuildingExists;
		break;
	case PhobosTriggerEvent::AttackedByHouseByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::AttackedByHouse;
		break;
	case PhobosTriggerEvent::SpyAsHouseByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::SpyAsHouse;
		break;
	case PhobosTriggerEvent::SpyAsInfantryByID:
		pThis->Value = InfantryTypeClass::FindIndex(pThis->String);
		pThis->EventKind = TriggerEvent::SpyAsInfantry;
		break;
	case PhobosTriggerEvent::DestroyedUnitsNavalByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::DestroyedUnitsNaval;
		break;
	case PhobosTriggerEvent::DestroyedUnitsLandByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::DestroyedUnitsLand;
		break;
	case PhobosTriggerEvent::BuildingDoesNotExistByID:
		pThis->Value = BuildingTypeClass::FindIndex(pThis->String);
		pThis->EventKind = TriggerEvent::BuildingDoesNotExist;
		break;
	case PhobosTriggerEvent::PowerFullByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::PowerFull;
		break;
	case PhobosTriggerEvent::EnteredOrOverflownByByID:
		pThis->Value = HouseTypeClass::FindIndexOfName(pThis->String);
		pThis->EventKind = TriggerEvent::EnteredOrOverflownBy;
		break;

	default:
		break;
	}

	return 0;
}
