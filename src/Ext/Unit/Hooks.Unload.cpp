#include <Helpers/Macro.h>
#include <TunnelLocomotionClass.h>

#include <Ext/TechnoType/Body.h>

// Prevent subterranean units from deploying while underground.
DEFINE_HOOK(0x73D6E6, UnitClass_Unload_Subterranean, 0x6)
{
	enum { ReturnFromFunction = 0x73DFB0, SkipPassengers = 0x73DCD3 };

	GET(UnitClass* const, pThis, ESI);

	if (auto const pLoco = locomotion_cast<TunnelLocomotionClass*>(pThis->Locomotor))
	{
		if (pLoco->State != TunnelLocomotionClass::State::Idle)
			return ReturnFromFunction;
	}

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->Unload_SkipPassengers)
	{
		R->EAX(pThis->Type);
		return SkipPassengers;
	}
	else if (pTypeExt->Unload_NoPassengers
		&& pThis->Passengers.NumPassengers <= 0 && pThis->MissionStatus == 0)
	{
		R->EAX(pThis->Type);
		return SkipPassengers;
	}

	return 0;
}

DEFINE_HOOK(0x740015, UnitClass_MouseOverObject_NoPassengers, 0x6)
{
	enum { SkipPassengers = 0x7400F0 };

	GET(UnitClass* const, pThis, ESI);
	GET(UnitTypeClass* const, pType, EAX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	return pTypeExt->Unload_SkipPassengers
		|| (pTypeExt->Unload_NoPassengers && pThis->Passengers.NumPassengers <= 0)
		? SkipPassengers : 0;
}
