#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>

DEFINE_HOOK(0x446366, BuildingClass_Place_Academy, 0x6)
{
	GET(BuildingClass*, pThis, EBP);

	if (BuildingTypeExt::Fetch(pThis->Type)->Academy)
		BuildingTypeExt::UpdateAcademy(HouseExt::Fetch(pThis->Owner), pThis, true);

	return 0;
}

DEFINE_HOOK(0x445905, BuildingClass_Remove_Academy, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->IsOnMap && BuildingTypeExt::Fetch(pThis->Type)->Academy)
		BuildingTypeExt::UpdateAcademy(HouseExt::Fetch(pThis->Owner), pThis, false);

	return 0;
}

DEFINE_HOOK(0x448AB2, BuildingClass_SetOwningHouse_Remove_Academy, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->IsOnMap && BuildingTypeExt::Fetch(pThis->Type)->Academy)
		BuildingTypeExt::UpdateAcademy(HouseExt::Fetch(pThis->Owner), pThis, false);

	return 0;
}

DEFINE_HOOK(0x4491D5, BuildingClass_SetOwningHouse_Add_Academy, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->IsOnMap && BuildingTypeExt::Fetch(pThis->Type)->Academy)
		BuildingTypeExt::UpdateAcademy(HouseExt::Fetch(pThis->Owner), pThis, true);

	return 0;
}

DEFINE_HOOK(0x517D57, InfantryClass_Init_Academy, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	BuildingTypeExt::SetOriginVeterancy(pThis);
	BuildingTypeExt::ApplyAcademy(pThis, AbstractType::Infantry);

	return 0;
}

DEFINE_HOOK_AGAIN(0x735678, UnitClass_Init_Academy, 0x6) // CTOR
DEFINE_HOOK(0x74689B, UnitClass_Init_Academy, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto const pType = pThis->Type;
	BuildingTypeExt::SetOriginVeterancy(pThis);

	if (pType->ConsideredAircraft)
		BuildingTypeExt::ApplyAcademy(pThis, AbstractType::Aircraft);
	else if (pType->Organic)
		BuildingTypeExt::ApplyAcademy(pThis, AbstractType::Infantry);
	else
		BuildingTypeExt::ApplyAcademy(pThis, AbstractType::Unit);

	return 0;
}

DEFINE_HOOK(0x413FD2, AircraftClass_Init_Academy, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	BuildingTypeExt::SetOriginVeterancy(pThis);
	BuildingTypeExt::ApplyAcademy(pThis, AbstractType::Aircraft);

	return 0;
}

DEFINE_HOOK(0x442D34, BuildingClass_Init_Academy, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	BuildingTypeExt::SetOriginVeterancy(pThis);
	BuildingTypeExt::ApplyAcademy(pThis, AbstractType::Building);

	return 0;
}
