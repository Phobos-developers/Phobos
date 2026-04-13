#include <Helpers/Macro.h>
#include <TunnelLocomotionClass.h>

#include <Ext/TechnoType/Body.h>

namespace UnitUnloadTemp
{
	TechnoTypeExt::ExtData* TypeExtData = nullptr;
}

// Prevent subterranean units from deploying while underground.
DEFINE_HOOK(0x73D63B, UnitClass_Mi_Unload_Subterranean, 0x6)
{
	enum { ReturnFromFunction = 0x73DFB0, SkipHarvester = 0x73D694, SkipPassengers = 0x73DCD3, Harvester = 0x73DEE7, Continue = 0x73D6EC };

	GET(UnitClass* const, pThis, ESI);

	if (auto const pLoco = locomotion_cast<TunnelLocomotionClass*>(pThis->Locomotor))
	{
		if (pLoco->State != TunnelLocomotionClass::State::Idle)
			return ReturnFromFunction;
	}

	auto const pType = pThis->Type;
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	UnitUnloadTemp::TypeExtData = pTypeExt;

	// It should be the highest priority.
	if (pThis->BunkerLinkedItem)
	{
		if (auto const pBuilding = pThis->GetCell()->GetBuilding())
			pBuilding->EmptyBunker();

		// It can fix the issue where mining carts cannot move.
		R->EAX(pType);
		return SkipHarvester;
	}

	// Miners should not be hindered by other deployment actions while unloading minerals.
	if (pType->Harvester || pType->Weeder)
	{
		const bool hasAnyLink = pThis->HasAnyLink();

		if (hasAnyLink || pThis->Unloading)
		{
			R->AL(hasAnyLink);
			return Harvester;
		}
	}

	R->EAX(pType);

	if (pTypeExt->Deploy_SkipPassengerUnload)
		return SkipPassengers;
	else if (pTypeExt->Deploy_NoPassenger && pThis->Passengers.NumPassengers <= 0 && pThis->MissionStatus == 0)
		return SkipPassengers;

	return Continue;
}

DEFINE_HOOK(0x73DEEB, UnitClass_Mi_Unload_SkipHarvester, 0x5)
{
	GET(UnitClass* const, pThis, ESI);
	enum { SkipHarvester = 0x73D694 };

	auto const pTypeExt = UnitUnloadTemp::TypeExtData;

	if (!pThis->Unloading && (!pTypeExt->Deploy_NoTiberium || pThis->Tiberium.GetTotalValue() == 0))
	{
		R->EAX(pThis->Type);
		return SkipHarvester;
	}

	return 0;
}

DEFINE_HOOK(0x740015, UnitClass_MouseOverObject_SkipPassengers, 0x6)
{
	enum { SkipPassengers = 0x7400F0 };

	GET(UnitClass* const, pThis, ESI);
	GET(UnitTypeClass* const, pType, EAX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	return pTypeExt->Deploy_SkipPassengerUnload
		|| (pTypeExt->Deploy_NoPassenger && pThis->Passengers.NumPassengers <= 0)
		? SkipPassengers : 0;
}
