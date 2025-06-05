#include <AircraftClass.h>

#include <ScenarioClass.h>
#include <TunnelLocomotionClass.h>
#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x7364DC, UnitClass_Update_SinkSpeed, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	GET(int, CoordZ, EDX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	R->EDX(CoordZ - (pTypeExt->SinkSpeed - 5));
	return 0;
}

DEFINE_HOOK(0x737DE2, UnitClass_ReceiveDamage_Sinkable, 0x6)
{
	enum { GoOtherChecks = 0x737E18, NoSink = 0x737E63 };

	GET(UnitTypeClass*, pType, EAX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	bool ShouldSink = pType->Weight > RulesClass::Instance->ShipSinkingWeight && pType->Naval && !pType->Underwater && !pType->Organic;

	return pTypeExt->Sinkable.Get(ShouldSink) ? GoOtherChecks : NoSink;
}

DEFINE_HOOK(0x629C67, ParasiteClass_UpdateSquid_SinkableBySquid, 0x9)
{
	enum { ret = 0x629C86 };

	GET(ParasiteClass*, pThis, ESI);
	GET(FootClass*, pVictim, EDI);

	auto pVictimType = pVictim->GetTechnoType();
	auto pVictimTypeExt = TechnoTypeExt::ExtMap.Find(pVictimType);

	if (pVictimTypeExt->Sinkable_SquidGrab || pVictim->WhatAmI() != AbstractType::Unit)
	{
		pVictim->IsSinking = true;
		pVictim->Destroyed(pThis->Owner);
		pVictim->Stun();
	}
	else
	{
		auto damage = pVictimType->Strength;
		pVictim->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, pThis->Owner, true, false, pThis->Owner->Owner);
	}

	return ret;
}
