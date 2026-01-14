#include <Helpers/Macro.h>
#include <TunnelLocomotionClass.h>

#include <Ext/TechnoType/Body.h>

namespace UnitUnloadTemp
{
	TechnoTypeExt::ExtData* TypeExtData = nullptr;
}

// Prevent subterranean units from deploying while underground.
DEFINE_HOOK(0x73D6E6, UnitClass_Unload_Subterranean, 0x6)
{
	enum { ReturnFromFunction = 0x73DFB0, SkipPassengers = 0x73DCD3, DeployFireAfter = 0x73D672 };

	GET(UnitClass* const, pThis, ESI);

	if (auto const pLoco = locomotion_cast<TunnelLocomotionClass*>(pThis->Locomotor))
	{
		if (pLoco->State != TunnelLocomotionClass::State::Idle)
			return ReturnFromFunction;
	}

	auto const pType = pThis->Type;
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	UnitUnloadTemp::TypeExtData = pTypeExt;

	// Miners should not be hindered by other deployment actions while unloading minerals.
	if ((pType->Harvester || pType->Weeder)
		&& (pThis->HasAnyLink() || pThis->Unloading))
	{
		return DeployFireAfter;
	}

	if (pTypeExt->Unload_SkipPassengers)
	{
		R->EAX(pType);
		return SkipPassengers;
	}
	else if (pTypeExt->Unload_NoPassengers
		&& pThis->Passengers.NumPassengers <= 0 && pThis->MissionStatus == 0)
	{
		R->EAX(pType);
		return SkipPassengers;
	}

	return 0;
}

DEFINE_HOOK(0x73DEEB, UnitClass_Mi_Unload_SkipHarvester, 0x5)
{
	GET(UnitClass* const, pThis, ESI);
	enum { SkipHarvester = 0x73D694 };

	auto const pTypeExt = UnitUnloadTemp::TypeExtData;

	if (!pThis->Unloading
		&& (pTypeExt->Unload_SkipHarvester || (pTypeExt->Unload_NoTiberiums && pThis->Tiberium.GetTotalValue() == 0)))
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

	return pTypeExt->Unload_SkipPassengers
		|| (pTypeExt->Unload_NoPassengers && pThis->Passengers.NumPassengers <= 0)
		? SkipPassengers : 0;
}
